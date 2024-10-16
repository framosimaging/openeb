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
#include <metavision/hal/utils/file_data_transfer.h>

#include "framos_imx636_file_discovery.h"
#include "framos_imx636_hw_identification.h"
#include "framos_imx636_geometry.h"
#include "decoders/evt3/evt3_decoder.h"

bool FramosImx636FileDiscovery::discover(Metavision::DeviceBuilder &device_builder, std::unique_ptr<std::istream> &stream,
                                   const Metavision::RawFileHeader &header,
                                   const Metavision::RawFileConfig &stream_config) {
    // Add facilities to the device
    auto hw_identification = device_builder.add_facility(
        std::make_unique<FramosImx636HWIdentification>(device_builder.get_plugin_software_info(), "File", ""));
    device_builder.add_facility(std::make_unique<FramosImx636Geometry>());

    auto cd_event_decoder =
        device_builder.add_facility(std::make_unique<Metavision::I_EventDecoder<Metavision::EventCD>>());

    auto decoder = device_builder.add_facility(make_evt3_decoder(stream_config.do_time_shifting_, 720, 1280, cd_event_decoder));
    // Note that the current facility must take ownership of the stream instance (as it was the case in previous
    // versions).
    device_builder.add_facility(std::make_unique<Metavision::I_EventsStream>(
        std::make_unique<Metavision::FileDataTransfer>(std::move(stream), decoder->get_raw_event_size_bytes(),
                                                        stream_config), hw_identification));

    return true;
}