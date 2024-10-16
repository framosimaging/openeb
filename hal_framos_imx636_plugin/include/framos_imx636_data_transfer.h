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

#ifndef METAVISION_HAL_FRAMOS_IMX636_DATA_TRANSFER_H
#define METAVISION_HAL_FRAMOS_IMX636_DATA_TRANSFER_H

#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <metavision/sdk/base/utils/timestamp.h>
#include <metavision/hal/utils/data_transfer.h>
#include "framos_imx636_device.h"

/// @brief Class for getting buffers from cameras or files.
///
/// This class is the implementation of HAL's facility @ref Metavision::DataTransfer
class FramosImx636DataTransfer : public Metavision::DataTransfer {
public:
    /// @brief Constructor
    ///
    FramosImx636DataTransfer(uint32_t raw_event_size_bytes, FramosImx636Device& device);

private:
    FramosImx636Device& device_;

    struct DataBuffer{
        Data *data;
        std::size_t size;
    };

    std::vector<DataBuffer> active_buffers_;
    std::mutex stream_mutex;

    std::condition_variable lifo_condition_;
    std::mutex buffers_mutex_;
    std::deque<uint32_t> free_buffers_lifo_;
    std::vector<uint32_t> used_buffers_;

    BufferPtr active_buffer_ptr_;

    std::thread acquisition_thread_;
    std::atomic<bool> acquisition_thread_stop_;
    std::atomic<bool> processing_stop_;

    std::atomic<uint32_t> acquired_frames_;
    std::atomic<uint32_t> dropped_frames_;

    void start_impl(BufferPtr buffer) override final;
    void stop_impl() override final;
    void run_impl() override final;

    void AcquisitionThread();
};

#endif // METAVISION_HAL_FRAMOS_IMX636_DATA_TRANSFER_H
