/**********************************************************************************************************************
 * Copyright (c) Prophesee S.A.                                                                                       *
 *                                                                                                                    *
 * Licensed under the Apache License, Version 2.0 (the "License");                                                    *
 * you may not use this file except in compliance with the License.                                                   *
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0                                 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed   *
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.                      *
 * See the License for the specific language governing permissions and limitations under the License.                 *
 **********************************************************************************************************************/
#ifndef V4L2_DEVICE_H
#define V4L2_DEVICE_H

#include <fcntl.h> /* low-level i/o */
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <linux/videodev2.h>
#include <linux/media-bus-format.h>

#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>

#include <metavision/sdk/embedded/dma_buf_heap.h>

namespace Metavision {

class V4l2Device {
protected:
    int fd_;
    void raise_error(const std::string &str) const {
        throw std::runtime_error(str + " (" + std::to_string(errno) + " - " + strerror(errno) + ")");
    }

private:
    std::string dev_name_;

    void open_device() {
        struct stat st;
        if (-1 == stat(dev_name_.c_str(), &st))
            raise_error(dev_name_ + "Cannot identify device.");

        if (!S_ISCHR(st.st_mode))
            throw std::runtime_error(dev_name_ + " is not a device");

        // std::cout << "Opening device " << dev_name << std::endl;
        fd_ = open(dev_name_.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0);

        if (-1 == fd_)
            raise_error(dev_name_ + "Cannot open device");
    }

    void init_device() {
        struct v4l2_capability cap;
        if (ioctl(fd_, VIDIOC_QUERYCAP, &cap)) {
            if (EINVAL == errno)
                throw std::runtime_error(dev_name_ + " is not a V4L2 device");
            else
                raise_error("VIDIOC_QUERYCAP failed");
        }
        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
            throw std::runtime_error(dev_name_ + " is not video capture device");
        if (!(cap.capabilities & V4L2_CAP_STREAMING))
            throw std::runtime_error(dev_name_ + " does not support streaming i/o");

        struct v4l2_format fmt;
        std::memset(&fmt, 0, sizeof(fmt));
        fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
        fmt.fmt.pix.field       = V4L2_FIELD_ANY;
        fmt.fmt.pix.width       = 65536;
        fmt.fmt.pix.height      = 64;

        if (ioctl(fd_, VIDIOC_S_FMT, &fmt))
            raise_error("VIDIOC_S_FMT failed");
    }

protected:
    template<class Data>
    static typename std::vector<uint64_t>::const_iterator iterator_cast(Data *ptr) {
        return typename std::vector<uint64_t>::const_iterator(reinterpret_cast<const uint64_t *>(ptr));
    }

    /* Count the number of bytes received in the buffer. The complexity is log(n) */
    template<typename Data>
    static std::size_t nb_not_null_data(const Data *const buf_beg_addr, std::size_t length_in_bytes) {
        auto is_not_null = [](const auto &d) { return d != 0; };
        auto beg         = iterator_cast(buf_beg_addr);
        auto end         = beg + length_in_bytes / sizeof(*beg);

        auto it_pp = std::partition_point(beg, end, is_not_null);
        return std::distance(beg, it_pp) * sizeof(*beg);
    }

protected:
    unsigned int request_buffers(v4l2_memory memory, unsigned int nb_buffers) {
        struct v4l2_requestbuffers req;
        std::memset(&req, 0, sizeof(req));
        req.count  = nb_buffers;
        req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = memory;

        if (-1 == ioctl(fd_, VIDIOC_REQBUFS, &req)) {
            raise_error("VIDIOC_QUERYBUF failed");
        }
        return req.count;
    }

    virtual unsigned int get_nb_buffers() const = 0;

public:
    V4l2Device(const char *dev_name) : dev_name_(dev_name) {
        open_device();
        init_device();
    }
    virtual ~V4l2Device() = default;

    void start() {
        std::cout << " Nb buffers pre allocated: " << get_nb_buffers() << std::endl;
        for (unsigned int i = 0; i < get_nb_buffers(); ++i) {
            release_buffer(i);
        }
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (ioctl(fd_, VIDIOC_STREAMON, &type))
            raise_error("VIDIOC_STREAMON failed");
    }

    void stop() {
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (ioctl(fd_, VIDIOC_STREAMOFF, &type))
            raise_error("VIDIOC_STREAMOFF failed");
    }

    virtual void release_buffer(int idx) const                            = 0;
    virtual int get_buffer() const                                        = 0;
    virtual std::pair<void *, std::size_t> get_buffer_desc(int idx) const = 0;
};

/* ==================================================================== */
/** Manage buffer manipulation through the V4L2 interface.
 * In this implementation, buffers are allocated in the driver after a request during the .
 */
class V4l2DeviceMmap : public V4l2Device {
private:
    struct BufferDesc {
        void *start;
        std::size_t length; /* In bytes. */
    };
    std::vector<BufferDesc> buffers_desc_;

    void query_buffers(unsigned int nb_buffers) {
        for (unsigned int i = 0; i < nb_buffers; ++i) {
            /* Get a buffer allocated in Kernel space. */
            struct v4l2_buffer buf;
            std::memset(&buf, 0, sizeof(buf));
            buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index  = i;
            if (ioctl(fd_, VIDIOC_QUERYBUF, &buf))
                raise_error("VIDIOC_QUERYBUF failed");

            void *start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, buf.m.offset);
            if (MAP_FAILED == start)
                raise_error("mmap failed");
            memset(start, 0, buf.length);

            /* Record the handle to manage the life cycle. */
            buffers_desc_.push_back(BufferDesc{start, buf.length});
        }
    }
    void free_buffers() {
        /* Close dmabuf fd */
        for (const auto &buf : buffers_desc_) {
            if (-1 == munmap(buf.start, buf.length))
                raise_error("munmap failed");
        }
        buffers_desc_.clear();
    }

    unsigned int get_nb_buffers() const final {
        return buffers_desc_.size();
    }

public:
    V4l2DeviceMmap(const char *dev_name, unsigned int nb_buffers = 32) : V4l2Device(dev_name) {
        auto granted_buffers = request_buffers(V4L2_MEMORY_MMAP, nb_buffers);
        query_buffers(granted_buffers);
    }
    ~V4l2DeviceMmap() {
        free_buffers();
    }
    /** Release the buffer designed by the index to the driver. */
    void release_buffer(int idx) const final {
        struct v4l2_buffer buf;
        std::memset(&buf, 0, sizeof(buf));
        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index  = idx;
        if (ioctl(fd_, VIDIOC_QBUF, &buf))
            raise_error("VIDIOC_QBUF failed");
    }

    /** Poll a MIPI frame buffer through the V4L2 interface.
     * Return the buffer index.
     * */
    int get_buffer() const final {
        struct v4l2_buffer buf;
        std::memset(&buf, 0, sizeof(buf));
        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        while (ioctl(fd_, VIDIOC_DQBUF, &buf)) {}

        return buf.index;
    }

    /** Return the buffer address and size (in bytes) designed by the index. */
    std::pair<void *, std::size_t> get_buffer_desc(int idx) const final {
        auto desc = buffers_desc_.at(idx);
        return std::make_pair(desc.start, nb_not_null_data(desc.start, desc.length));
    }
};

/* ==================================================================== */

/** Manage buffer manipulation through the V4L2 interface.
 * In this implementation, buffers are allocated in user space using a dma_buf allocator. This allocator allocates
 * continuous buffers in physical memory which is necessary as buffers are used by DMA without gather/scatter
 * facility.
 */
class V4l2DeviceUserPtr : public V4l2Device {
    struct BufferDesc {
        void *start;
        unsigned int dmabuf_fd;
    };

private:
    std::unique_ptr<Metavision::DmaBufHeap> dma_buf_heap_;
    std::size_t length_;
    std::vector<BufferDesc> buffers_desc_;

    void allocate_buffers(unsigned int nb_buffers) {
        for (unsigned int i = 0; i < nb_buffers; ++i) {
            /* Get a buffer using CMA allocator in user space. */
            auto dmabuf_fd = dma_buf_heap_->alloc(length_);

            void *start = mmap(NULL, length_, PROT_READ | PROT_WRITE, MAP_SHARED, dmabuf_fd, 0);
            if (MAP_FAILED == start)
                raise_error("mmap failed");

            dma_buf_heap_->cpu_sync_start(dmabuf_fd);
            memset(start, 0, length_);

            std::cout << "Allocate buffer: " << i << " at: " << std::hex << start << " of " << std::dec << length_
                      << " bytes." << std::endl;

            /* Record the handle to manage the life cycle. */
            buffers_desc_.push_back(BufferDesc{start, dmabuf_fd});
        }
    }
    void free_buffers() {
        int i = get_nb_buffers();

        while (0 < i) {
            auto idx = get_buffer();
            std::cout << "Release " << i << " buffer: " << idx << std::endl;
            auto buf = buffers_desc_.at(idx);
            if (-1 == munmap(buf.start, length_))
                raise_error("munmap failed");
            dma_buf_heap_->free(buf.dmabuf_fd);
            --i;
        }

        buffers_desc_.clear();
    }

    unsigned int get_nb_buffers() const final {
        return buffers_desc_.size();
    }

public:
    V4l2DeviceUserPtr(const char *dev_name, std::unique_ptr<Metavision::DmaBufHeap> dma_buf_heap, std::size_t length,
                      unsigned int nb_buffers = 32) :
        V4l2Device(dev_name), dma_buf_heap_(std::move(dma_buf_heap)), length_(length) {
        auto granted_buffers = request_buffers(V4L2_MEMORY_USERPTR, nb_buffers);
        std::cout << "Requested buffers: " << nb_buffers << " granted buffers: " << granted_buffers << std::endl;
        allocate_buffers(granted_buffers);
    }
    ~V4l2DeviceUserPtr() {
        free_buffers();
    }
    /** Release the buffer designed by the index to the driver. */
    void release_buffer(int idx) const final {
        auto desc = buffers_desc_.at(idx);

        dma_buf_heap_->cpu_sync_stop(desc.dmabuf_fd);
        std::cout << "Release buffer: " << idx << " at " << std::hex << desc.start << " of " << std::dec << length_
                  << " bytes." << std::endl;
        struct v4l2_buffer buf;
        std::memset(&buf, 0, sizeof(buf));
        buf.type      = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory    = V4L2_MEMORY_USERPTR;
        buf.index     = idx;
        buf.m.userptr = (unsigned long)desc.start;
        buf.length    = length_;
        if (ioctl(fd_, VIDIOC_QBUF, &buf))
            raise_error("VIDIOC_QBUF failed");
    }

    /** Poll a MIPI frame buffer through the V4L2 interface.
     * Return the buffer index.
     * */
    int get_buffer() const final {
        struct v4l2_buffer buf;
        std::memset(&buf, 0, sizeof(buf));
        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_USERPTR;
        while (ioctl(fd_, VIDIOC_DQBUF, &buf)) {}

        int idx   = buf.index;
        auto desc = buffers_desc_.at(idx);
        dma_buf_heap_->cpu_sync_start(desc.dmabuf_fd);
        return idx;
    }

    /** Return the buffer address and size (in bytes) designed by the index. */
    std::pair<void *, std::size_t> get_buffer_desc(int idx) const final {
        auto desc = buffers_desc_.at(idx);
        return std::make_pair(desc.start, nb_not_null_data(desc.start, length_));
    }
};

} // namespace Metavision
#endif // V4L2_DEVICE_H