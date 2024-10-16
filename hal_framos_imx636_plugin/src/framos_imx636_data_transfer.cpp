/**********************************************************************************************************************
 * Copyright (c) Prophesee S.A.                                                                                       *
 * Copyright (c) Framos GmbH                                                                                          *
 *                                                                                                                    *
 * Licensed under the Apache License, Version 2.0 (the "License");                                                    *
 * you may not use this file except in compliance with the License.                                                   *
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0                                 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed   *
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.                      *
 * See the License for the specific language governing permissions and limitations under the License.                 *
 **********************************************************************************************************************/

#include <metavision/sdk/base/utils/get_time.h>

#include "metavision/hal/utils/hal_exception.h"

#include "framos_imx636_data_transfer.h"
#include "framos_imx636_camera_discovery.h"
#include "framos_imx636_geometry.h"

#include <sys/mman.h>
#include <linux/videodev2.h>

#define ANALYTICS

using namespace std::chrono;

static constexpr uint32_t buffer_num = 16;
static constexpr uint32_t lifo_size = 8;

FramosImx636DataTransfer::FramosImx636DataTransfer(uint32_t raw_event_size_bytes, FramosImx636Device& device) 
    : DataTransfer(raw_event_size_bytes, BufferPool::make_bounded()),
     device_(device) {}

void FramosImx636DataTransfer::start_impl(BufferPtr buffer) {

    uint32_t bufferSize = device_.getInternalBufferSize();
    struct v4l2_requestbuffers req {};
    req.count = buffer_num;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
 
    auto success = device_.xioctl(VIDIOC_REQBUFS, &req);
    if (!success) {
        throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
            "Requesting Buffer failed");
        return;
    }
    if (req.count < 1) {
        throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
            "Insufficient buffer memory");
        return;
    }

    // Init MMAP
    for (int i = 0; i < req.count; i++){
        struct v4l2_buffer buf {};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        success = device_.xioctl(VIDIOC_QUERYBUF, &buf);
        if(!success) {
            throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
                "VIDIOC_QUERYBUF failed");
            return;
        }
    
        void *v_addr = ::mmap (NULL, 
            bufferSize, 
            PROT_READ | PROT_WRITE, 
            MAP_SHARED, 
            device_.get_fd(), 
            buf.m.offset);
        if (v_addr == nullptr) {
            throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
                "mmap failed");
            return;
        }
        active_buffers_.push_back(DataBuffer{(Data*)v_addr, bufferSize});
    }

    // Start capture
    for (int i = 0; i < active_buffers_.size(); i++){
        struct v4l2_buffer buf {};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        success = device_.xioctl(VIDIOC_QBUF, &buf);
        if(!success) {
            throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
                "VIDIOC_QBUF failed");
            return;
        }
    }

    active_buffer_ptr_ = buffer;

    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    success = device_.xioctl(VIDIOC_STREAMON, &type);
    if(!success) {
        throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
            "VIDIOC_STREAMON failed");
        return;
    }

    acquisition_thread_ = std::thread(&FramosImx636DataTransfer::AcquisitionThread, this);
}

void FramosImx636DataTransfer::stop_impl() {

    acquisition_thread_stop_ = true;
    processing_stop_ = true;
    acquisition_thread_.join();

    std::lock_guard<std::mutex> lock(stream_mutex);

    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    auto success = device_.xioctl(VIDIOC_STREAMOFF, &type);
    if (!success) {
        throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
            "VIDIOC_STREAMOFF failed");
    }
    for (int i = 0; i < buffer_num; i++) {
        ::munmap(active_buffers_[i].data, active_buffers_[i].size);
    }
    active_buffers_.clear();
}

void FramosImx636DataTransfer::run_impl() {

    processing_stop_ = false;
    std::lock_guard<std::mutex> lock(stream_mutex);

#ifdef ANALYTICS
    uint64_t ts_loop = Metavision::get_system_time_us();
    uint32_t transferred_frames = 0;
#endif

    while (!should_stop() && !processing_stop_) {      

        int active_buffer;
        {
            std::unique_lock<std::mutex> lock(buffers_mutex_);
            lifo_condition_.wait(lock, [&]{ return !free_buffers_lifo_.empty(); });
            active_buffer = free_buffers_lifo_.front();
            free_buffers_lifo_.pop_front();
        }
        std::this_thread::yield();

        active_buffer_ptr_->assign((Data*)active_buffers_[active_buffer].data, 
            (Data*)(active_buffers_[active_buffer].data + active_buffers_[active_buffer].size));
        auto next_buffer = transfer_data(active_buffer_ptr_);
        active_buffer_ptr_ = next_buffer;
#ifdef ANALYTICS
        ++transferred_frames;
#endif

        {
            std::unique_lock<std::mutex> lock(buffers_mutex_);
            used_buffers_.push_back(active_buffer);
        }
        std::this_thread::yield();

#ifdef ANALYTICS // Calculate FPS every N-th frame
        if ((Metavision::get_system_time_us() - ts_loop) > 1000000){
            ts_loop = Metavision::get_system_time_us();

            printf("FPS acquired:%u transferred:%u dropped:%u\n", 
                acquired_frames_.exchange(0), 
                transferred_frames, 
                dropped_frames_.exchange(0));
            transferred_frames = 0;
        }
#endif
    }
}

void FramosImx636DataTransfer::AcquisitionThread()
{
    acquisition_thread_stop_ = false;
    acquired_frames_ = 0;
    dropped_frames_ = 0;
    while (!acquisition_thread_stop_) {
        struct v4l2_buffer buf {};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        auto success = device_.xioctl(VIDIOC_DQBUF, &buf);
        if(!success)
        {
            throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
                "VIDIOC_DQBUF failed");
        }
        ++acquired_frames_;

        std::vector<uint32_t> local_used_buffers;
        {
            std::unique_lock<std::mutex> lock(buffers_mutex_);
            if (free_buffers_lifo_.size() == lifo_size) 
            {
                used_buffers_.push_back(free_buffers_lifo_.front());
                free_buffers_lifo_.pop_front();
                ++dropped_frames_;
            }
            free_buffers_lifo_.push_back(buf.index);
            local_used_buffers = used_buffers_;
            used_buffers_.clear();
        }
        lifo_condition_.notify_all();
        std::this_thread::yield();

        struct v4l2_buffer active_buf {};
        active_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        active_buf.memory = V4L2_MEMORY_MMAP;
        for (auto i : local_used_buffers) {
            active_buf.index = i;
            auto success = device_.xioctl(VIDIOC_QBUF, &active_buf);
            if(!success) {
                throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
                    "VIDIOC_QBUF failed");
            }
        }
    }
}