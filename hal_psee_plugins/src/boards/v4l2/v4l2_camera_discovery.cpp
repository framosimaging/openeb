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

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <memory>
#include <string>
#include <sys/ioctl.h>

#include "boards/v4l2/v4l2_device.h"
#include "boards/v4l2/v4l2_camera_discovery.h"
#include "boards/v4l2/v4l2_data_transfer.h"

#include "metavision/hal/device/device_discovery.h"
#include "metavision/hal/facilities/i_camera_synchronization.h"
#include "metavision/hal/facilities/i_decoder.h"
#include "metavision/hal/facilities/i_events_stream.h"
#include "metavision/hal/facilities/i_hw_identification.h"
#include "metavision/hal/facilities/i_plugin_software_info.h"
#include "metavision/hal/plugin/plugin_entrypoint.h"
#include "metavision/hal/utils/camera_discovery.h"
#include "metavision/hal/utils/device_builder.h"
#include "metavision/hal/utils/hal_log.h"
#include "metavision/psee_hw_layer/utils/psee_format.h"
#include "metavision/psee_hw_layer/boards/v4l2/v4l2_board_command.h"

#include "utils/make_decoder.h"

namespace Metavision {

bool V4l2CameraDiscovery::is_for_local_camera() const {
    return true;
}

CameraDiscovery::SerialList V4l2CameraDiscovery::list() {
    return {"V4l2_device_serial_0"};
}

CameraDiscovery::SystemList V4l2CameraDiscovery::list_available_sources() {
    return {{"V4l2_device_serial_0", ConnectionType::MIPI_LINK, 000420}};
}

bool V4l2CameraDiscovery::discover(DeviceBuilder &device_builder, const std::string &serial,
                                   const DeviceConfig &config) {
    MV_HAL_LOG_TRACE() << "V4l2Discovery - Discovering...";

    std::vector<std::string> device_names = {
        // We might want to iterate over all /dev/mediaX files
        "/dev/media0",
        /* "/dev/media0", "/dev/media1", "/dev/media2", "/dev/media3", "/dev/media4", */
    };

    for (auto device_name : device_names) {
        try {
            devices_.emplace_back(std::make_shared<V4L2BoardCommand>(device_name));
        } catch (std::exception &e) {
            MV_HAL_LOG_TRACE() << "Cannot open V4L2 device '" << device_name << "' (err: " << e.what();
        }
    }

    if (devices_.empty()) {
        return false;
    }

    auto &main_device  = devices_[0];

    try {
        builder->build_device(main_device, device_builder, config);
    } catch (std::exception &e) { MV_HAL_LOG_ERROR() << "Failed to build streaming facilities :" << e.what(); }

    MV_HAL_LOG_INFO() << "V4l2 Discovery with great success +1";
    return true;
}

} // namespace Metavision
