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

#include <metavision/hal/facilities/i_events_stream.h>
#include <metavision/hal/utils/device_builder.h>

#include <algorithm>
#include <dirent.h>
#include <fnmatch.h>

#include "framos_imx636_camera_discovery.h"
#include "framos_imx636_data_transfer.h"
#include "framos_imx636_hw_identification.h"
#include "framos_imx636_device_control.h"
#include "framos_imx636_geometry.h"
#include "framos_imx636_ll_biases.h"
#include "framos_imx636_roi.h"
#include "decoders/evt3/evt3_decoder.h"

FramosImx636CameraDiscovery::FramosImx636CameraDiscovery() {

    std::vector<std::string> cameras = get_all_cameras();

    if (cameras.empty()) {
        throw Metavision::HalException(Metavision::HalErrorCode::CameraNotFound,
            "No cameras detected.");
    }

    for (auto camera : cameras) {
        FramosImx636Device framosImx636Device;
        framosImx636Device.open(camera);
        if (framosImx636Device.get_type() == FramosDeviceType::IMX636){
            serial_number_map_[framosImx636Device.get_serial_number()] = camera;
        }
    }

    if (serial_number_map_.empty()) {
        throw Metavision::HalException(Metavision::HalErrorCode::CameraNotFound,
            "No IMX636 cameras detected.");
    }
}

FramosImx636CameraDiscovery::~FramosImx636CameraDiscovery() {}

Metavision::CameraDiscovery::SerialList FramosImx636CameraDiscovery::list() {

    SerialList ret;
    for (auto const &pair: serial_number_map_) {
        ret.push_back(pair.first);
    }
    
    return ret;
}

Metavision::CameraDiscovery::SystemList FramosImx636CameraDiscovery::list_available_sources() {

    SystemList systems;
    for (auto const &pair: serial_number_map_) {
        Metavision::PluginCameraDescription description;
        description.serial_     = pair.first;
        description.system_id_  = FramosImx636HWIdentification::FRAMOS_IMX636_SYSTEM_ID;
        description.connection_ = Metavision::MIPI_LINK;

        systems.push_back(description);
    }

    return systems;
}

bool FramosImx636CameraDiscovery::discover(Metavision::DeviceBuilder &device_builder, const std::string &serial,
                                   const Metavision::DeviceConfig &config) {

    // Check if serial number is empty (choose first device in that case) or check if serial number exists and initialize
    if(serial.empty()) {
        device.initialize("/dev/video0");
    } else if(serial_number_map_.find(serial) == serial_number_map_.end()) {
        throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
            "Camera with serial " + serial + " has not been found.");
    } else {
        device.initialize(serial_number_map_.at(serial));
    }
    // Add facilities to the device builder
    auto hw_identification = device_builder.add_facility(
        std::make_unique<FramosImx636HWIdentification>(device_builder.get_plugin_software_info(), "EMBEDDED", serial));
    device_builder.add_facility(std::make_unique<FramosImx636Geometry>());
    device_builder.add_facility(std::make_unique<FramosImx636DeviceControl>());

    auto cd_event_decoder = device_builder.add_facility(
        std::make_unique<Metavision::I_EventDecoder<Metavision::EventCD>>());
    auto decoder = device_builder.add_facility(make_evt3_decoder(false, 720, 1280, cd_event_decoder));
    device_builder.add_facility(std::make_unique<FramosImx636_LL_Biases>(device));
    device_builder.add_facility(std::make_unique<Metavision::I_EventsStream>(
        std::make_unique<FramosImx636DataTransfer>(decoder->get_raw_event_size_bytes(), device), hw_identification));
    device_builder.add_facility(std::make_unique<FramosImx636Roi>(device));

    return true;
}

bool FramosImx636CameraDiscovery::is_for_local_camera() const {
    return true;
}

std::vector<std::string> FramosImx636CameraDiscovery::get_all_cameras() {

	std::vector<std::string> cameras;
	auto directory = ::opendir("/dev");
    if (directory) {
        auto file = ::readdir(directory);
        while (file) {
            if (::fnmatch("video*", file->d_name, 0) == 0) {
                cameras.push_back("/dev/" + std::string(file->d_name));
            }
            file = ::readdir(directory);
        }
        ::closedir(directory);
    }
    std::sort(cameras.begin(), cameras.end(), 
    [] (std::string first, std::string second) {return first < second;} );
    return cameras;
}