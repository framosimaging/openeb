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

#include <system_error>

#include <unistd.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#include "boards/v4l2/v4l2_data_transfer.h"
#include "boards/v4l2/dma_buf_heap.h"

#include "metavision/hal/utils/hal_log.h"

using namespace Metavision;
using Allocator = DataTransfer::Allocator;

constexpr bool allow_buffer_drop = true;

constexpr size_t device_buffer_preload_number = 4;

V4l2DataTransfer::V4l2DataTransfer(int fd, uint32_t raw_event_size_bytes) :
    DataTransfer(raw_event_size_bytes,
                 // To create device_buffer_number std::vectors with a V4L2 MMAP allocator
                 DataTransfer::BufferPool::make_bounded(device_buffer_number,
                                                        Allocator(Allocator::ImplPtr(new V4l2MmapAllocator(fd)))),
                 allow_buffer_drop),
    memtype_(V4L2_MEMORY_MMAP),
    fd_(dup(fd)) {
    auto res = request_buffers(device_buffer_number);
    if (res.count != device_buffer_number)
        throw std::system_error(ENOMEM, std::generic_category(), "Unexepected amount of V4L2 buffers allocated");
}

V4l2DataTransfer::V4l2DataTransfer(int fd, uint32_t raw_event_size_bytes, const std::string &heap_path,
                                   const std::string &heap_name) :
    DataTransfer(raw_event_size_bytes,
                 // To create device_buffer_number std::vectors from a heap, using DMABUF
                 DataTransfer::BufferPool::make_bounded(device_buffer_number,
                                                        Allocator(Allocator::ImplPtr(new DmabufAllocator(
                                                            fd, std::make_unique<DmaBufHeap>(heap_path, heap_name))))),
                 allow_buffer_drop),
    memtype_(V4L2_MEMORY_DMABUF),
    fd_(dup(fd)) {
    auto res = request_buffers(device_buffer_number);
    if (res.count != device_buffer_number)
        throw std::system_error(ENOMEM, std::generic_category(), "Unexepected amount of V4L2 buffers allocated");
}

V4l2DataTransfer::~V4l2DataTransfer() {
    // Release the previously acquired buffers
    request_buffers(0);
    // and release this file handler
    close(fd_);
}

V4l2RequestBuffers V4l2DataTransfer::request_buffers(uint32_t nb_buffers) {
    V4l2RequestBuffers req{0};
    req.count  = nb_buffers;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = memtype_;

    if (-1 == ioctl(fd_, VIDIOC_REQBUFS, &req)) {
        throw std::system_error(errno, std::generic_category(), "VIDIOC_REQBUFS failed");
    }

    return req;
}

void V4l2DataTransfer::start_impl(BufferPtr input_buff) {
    MV_HAL_LOG_INFO() << "V4l2DataTransfer - start_impl() ";

    V4l2Buffer buffer = {.index = 0, .type = V4L2_BUF_TYPE_VIDEO_CAPTURE, .memory = memtype_};

    fill_v4l2_buffer(input_buff, buffer);

    if (ioctl(fd_, VIDIOC_QBUF, &buffer) < 0) {
        throw std::system_error(errno, std::generic_category(), "VIDIOC_QBUF failed");
    }
    queued_buffers_[buffer.index] = std::move(input_buff);

    // DMA usually need 2 buffers to run, to be able to quickly switch from one transfer to the next
    // The way datatransfer is built, run_impl sequentially dequeue a buffer, pass it to EventStream, query another
    // from the buffer pool, queue it, then dequeue the next buffer.
    // If all buffer are queued it becomes impossible to dequeue 2 of them to process them in parallel.
    // Since 2 queued buffers are usually enough, and we have 32 of them, queuing 4 should avoid issues with hardware
    // expecting more, while allowing 28 buffers in parallel (or in a 28-stage pipeline) in the app.
    for (unsigned int i = 1; i < device_buffer_preload_number; ++i) {
        input_buff = get_buffer();
        // Using DMABUF, the allocator handles the pool of buffers through file descriptors, we need to choose a free
        // index to queue a buffer.
        // On the other hand, with MMAP, the pool is handled through indices, and fill_v4l2_buffer will fix the index
        // in the V4l2Buffer descriptor.
        buffer.index = i;
        fill_v4l2_buffer(input_buff, buffer);

        if (ioctl(fd_, VIDIOC_QBUF, &buffer) < 0) {
            throw std::system_error(errno, std::generic_category(), "VIDIOC_QBUF failed");
        }
        queued_buffers_[buffer.index] = std::move(input_buff);
    }
}

void V4l2DataTransfer::run_impl() {
    MV_HAL_LOG_INFO() << "V4l2DataTransfer - run_impl() ";
    struct pollfd fds[1];

    fds[0].fd      = fd_;
    fds[0].events  = POLLIN;
    fds[0].revents = 0;

    while (!should_stop()) {
        V4l2Buffer buf{0};

        if (poll(fds, 1, -1) < 0) {
            MV_HAL_LOG_ERROR() << "V4l2DataTransfer: poll failed" << strerror(errno);
            break;
        }

        if (fds[0].revents & POLLERR) {
            // When stopping, STREAMOFF ioctl will return all buffers and epoll will signal an error, since there is no
            // queued buffer anymore. This will usually trig faster than calling DataTransfer::stop, and should_stop()
            // will still return false, even though I_EventStream is stopping.
            // Stop polling and wait for DataTransfer to call stop_impl before cleaning
            MV_HAL_LOG_TRACE() << "V4l2DataTransfer: poll returned" << std::hex << fds[0].revents << std::dec;
            break;
        }

        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = memtype_;
        if (ioctl(fd_, VIDIOC_DQBUF, &buf) < 0) {
            MV_HAL_LOG_ERROR() << "V4l2DataTransfer: DQBUF failed" << strerror(errno);
            break;
        }

        MV_HAL_LOG_DEBUG() << "Grabbed buffer" << buf.index << "of:" << buf.bytesused << "Bytes.";

        // Get the vector corresponding to this buffer and transfer the data
        queued_buffers_[buf.index]->resize(buf.bytesused);

        auto [next, dropped] = transfer_data(queued_buffers_[buf.index]);
        // Release the buffer to DataTransfer BufferPool
        queued_buffers_[buf.index].reset();

        // buf is filled with the info of the dequeued buffer
        // update it with next information
        fill_v4l2_buffer(next, buf);

        // Queue the next buffer to keep the device running
        if (ioctl(fd_, VIDIOC_QBUF, &buf) < 0) {
            throw std::system_error(errno, std::generic_category(), "VIDIOC_QBUF failed");
        }
        queued_buffers_[buf.index] = std::move(next);
    }
}

void V4l2DataTransfer::stop_impl() {
    MV_HAL_LOG_TRACE() << "V4l2DataTransfer - stop_impl() ";
    // Here we trust that I_EventStream has also stopped V4L2 Device Control, which does STREAMOFF
    // and return every buffer, allowing to release buffers here
    for (size_t i = 0; i < device_buffer_number; i++)
        queued_buffers_[i].reset();
}

// Fills the V4L2 descriptor with the buffer info. This information depends on the Allocator implementation
// For instance in MMAP the buffer is mapped to an index of the device
// But on DMABUF, the buffer is mapped to a standalone file descriptor
// fill_v4l2_buffer requires v4l2_buf to be filled with a non-queued index of the V4L2 device, but may
// rewrite it, in case there is a fixed mapping between buffers and indices, as in MMAP use case
// The caller shall not assume that v4l2_buf.index is unchanged after a call to fill_v4l2_buffer
void V4l2DataTransfer::fill_v4l2_buffer(BufferPtr &buf, V4l2Buffer &v4l2_buf) const {
    // Since we inherit from DataTransfer, which requires a BufferPool, we can't query the actual buffer
    // size before building the BufferPool, and it is built with empty buffers. This ensure the vectors
    // are allocated before trying to map their allocations with the V4L2 buffers
    if (!buf->data())
        buf->reserve(buf->get_allocator().max_size());

    // The std::vectors in BufferPool are built with a V4l2Allocator, which can do this work
    V4l2Allocator *alloc = dynamic_cast<V4l2Allocator *>(buf->get_allocator().get_impl().get());
    if (!alloc)
        throw std::system_error(EPERM, std::generic_category(), "Impl is expected to be V4l2Allocator");
    alloc->fill_v4l2_buffer(buf->data(), v4l2_buf);
}

V4l2DataTransfer::V4l2Allocator::V4l2Allocator(int videodev_fd) {
    struct v4l2_format format {
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE
    };

    // Technically, the format is not locked yet, it will be locked when V4l2DataTransfer constructor does
    // request_buffers, but we need to build the BufferPool with an Allocator first
    if (ioctl(videodev_fd, VIDIOC_G_FMT, &format))
        throw std::system_error(errno, std::generic_category(), "VIDIOC_G_FMT failed");

    buffer_size_ = format.fmt.pix.sizeimage;
}
