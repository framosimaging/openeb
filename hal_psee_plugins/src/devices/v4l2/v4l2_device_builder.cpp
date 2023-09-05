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
#include "metavision/hal/utils/detail/hal_log_impl.h"
#include "metavision/hal/facilities/i_events_stream.h"

#include "metavision/psee_hw_layer/boards/treuzell/board_command.h"
#include "metavision/psee_hw_layer/boards/v4l2/v4l2_board_command.h"
#include "metavision/psee_hw_layer/boards/utils/psee_libusb_data_transfer.h"
#include "metavision/psee_hw_layer/utils/psee_format.h"
#include "metavision/psee_hw_layer/utils/register_map.h"
#include "metavision/psee_hw_layer/facilities/psee_hw_register.h"
#include "metavision/hal/utils/device_builder.h"

#include "metavision/psee_hw_layer/devices/gen41/gen41_erc.h"
#include "metavision/psee_hw_layer/devices/gen41/gen41_digital_event_mask.h"
#include "metavision/psee_hw_layer/devices/gen41/gen41_digital_crop.h"
#include "metavision/psee_hw_layer/devices/gen41/gen41_roi_command.h"
#include "metavision/psee_hw_layer/devices/imx636/imx636_tz_trigger_event.h"
#include "metavision/psee_hw_layer/devices/imx636/imx636_ll_biases.h"

#include "metavision/psee_hw_layer/devices/genx320/genx320_tz_trigger_event.h"
#include "metavision/psee_hw_layer/devices/genx320/genx320_ll_roi.h"
#include "metavision/psee_hw_layer/devices/genx320/genx320_ll_biases.h"
#include "metavision/psee_hw_layer/devices/genx320/genx320_erc.h"
#include "metavision/psee_hw_layer/devices/genx320/genx320_nfl.h"
#include "devices/genx320/register_maps/genx320es_registermap.h"
#include "devices/imx636/register_maps/imx636_registermap.h"

#include "geometries/hd_geometry.h"

#include "metavision/psee_hw_layer/devices/common/antiflicker_filter.h"
#include "metavision/psee_hw_layer/devices/common/event_trail_filter.h"

#include "boards/v4l2/v4l2_device.h"
#include "boards/v4l2/v4l2_hardware_identification.h"

#include "devices/v4l2/v4l2_device_builder.h"
#include "utils/make_decoder.h"
#include "devices/common/sensor_descriptor.h"

namespace Metavision {

static bool match(std::shared_ptr<BoardCommand> cmd, std::vector<MatchPattern> match_list) {
    for (auto match: match_list) {
        if ((cmd->read_device_register(0, match.addr, 1)[0] & match.mask) != (match.value))
            return false;
    }
    return true;
}

// TODO: these functions should be registered automatically to the builder,
// each sensor description should provide a spawn facility function
// -> define a sensor descriptor (a set of regmap + a facility function)
//
// example: GenX320SensorDescriptor desc(GenX320ESRegisterMap, Genx320ESRegisterMapSize, genx320_spawn_facilities);
// this would register everyting to the builder. And should avoid including all sensors from here...
//
static void genx320_spawn_facilities(DeviceBuilder &device_builder,
                                     const DeviceConfig &device_config,
                                     I_HW_Identification::SensorInfo sensor_info,
                                     std::shared_ptr<RegisterMap> register_map) {
    device_builder.add_facility(
        std::make_unique<GenX320TzTriggerEvent>(register_map, ""));
    device_builder.add_facility(std::make_unique<GenX320LowLevelRoi>(device_config, register_map, ""));
    device_builder.add_facility(std::make_unique<GenX320LLBiases>(register_map, device_config));
    device_builder.add_facility(std::make_unique<AntiFlickerFilter>(
        register_map, sensor_info, ""));
    device_builder.add_facility(std::make_unique<EventTrailFilter>(
        register_map, sensor_info, ""));
    device_builder.add_facility(std::make_unique<GenX320Erc>(register_map));
    device_builder.add_facility(std::make_unique<GenX320NoiseFilter>(register_map));
}

static void imx636_spawn_facilities(DeviceBuilder &device_builder,
                                    const DeviceConfig &device_config,
                                    I_HW_Identification::SensorInfo sensor_info,
                                    std::shared_ptr<RegisterMap> register_map) {
    device_builder.add_facility(std::make_unique<EventTrailFilter>(
        register_map, sensor_info, ""));
    device_builder.add_facility(std::make_unique<AntiFlickerFilter>(
        register_map, sensor_info, ""));

    auto erc = device_builder.add_facility(
        std::make_unique<Gen41Erc>(register_map, "erc/"));
    erc->initialize();

    auto geometry = HDGeometry();

    auto hw_register = std::make_shared<PseeHWRegister>(register_map);
    device_builder.add_facility(
        std::make_unique<Imx636_LL_Biases>(device_config, hw_register, ""));

    device_builder.add_facility(
        std::make_unique<Gen41ROICommand>(geometry.get_width(), geometry.get_height(), register_map, ""));

    device_builder.add_facility(
        std::make_unique<Imx636TzTriggerEvent>(register_map, ""));

    device_builder.add_facility(
        std::make_unique<Gen41DigitalEventMask>(register_map, "ro/digital_mask_pixel_"));

    device_builder.add_facility(std::make_unique<Gen41DigitalCrop>(register_map, ""));
}


// TODO: automatically register regmaps and facilities methods (at regmap deefinition time)
std::vector<SensorDescriptor> supported_sensors = {
    {
        GenX320ESRegisterMap, GenX320ESRegisterMapSize,
        genx320_spawn_facilities,
        {
            {.addr = 0x14, .value = 0x30501C01, .mask = 0xFFFFFFFF} ,
        },
        {320, 0, "GenX320"},
        "EVT21;height=320;width=320",
    },
    {
        Imx636RegisterMap, Imx636RegisterMapSize,
        imx636_spawn_facilities,
        {
            {.addr = 0x14, .value = 0xA0401806, .mask = 0xFFFFFFFF} ,
            {.addr = 0xF128, .value = 0b00, .mask = 0x00000003}
        },
        {4, 2, "IMX636"},
        "EVT3;height=720;width=1280",
    },
    {
        Imx636RegisterMap, Imx636RegisterMapSize,
        imx636_spawn_facilities,
        {
            {.addr = 0x14, .value = 0xA0401806, .mask = 0xFFFFFFFF} ,
            {.addr = 0xF128, .value = 0b10, .mask = 0x00000003} ,
        },
        {4, 2, "IMX646"},
        "EVT3;height=720;width=1280",
    },
};

static SensorDescriptor* get_sensor_descriptor(std::shared_ptr<BoardCommand>cmd , uint32_t chip_id) {
    auto it = std::find_if(supported_sensors.begin(), supported_sensors.end(), [chip_id, cmd](const SensorDescriptor& config) {
        return match(cmd, config.opt_match_list);
    });

    if (it != supported_sensors.end()) {
        return &(*it);
    } else {
        return nullptr;
    }
}

bool V4L2DeviceBuilder::build_device(std::shared_ptr<BoardCommand> cmd,
                                    Metavision::DeviceBuilder &device_builder, const Metavision::DeviceConfig &config) {

    auto chip_id = cmd->read_device_register(0, 0x14)[0];
    auto sensor_descriptor = get_sensor_descriptor(cmd, chip_id);
    if (sensor_descriptor == nullptr || !match(cmd, sensor_descriptor->opt_match_list)) {
        return false;
    }

    auto regmap_data = RegisterMap::RegmapData(1 ,std::make_tuple(sensor_descriptor->regmap, sensor_descriptor->size, "", 0));

    auto register_map = std::make_shared<RegisterMap>(regmap_data);
    register_map->set_read_cb([cmd](uint32_t address) {
        return cmd->read_device_register(0, address, 1)[0];
    });
    register_map->set_write_cb([cmd](uint32_t address, uint32_t v) { cmd->write_device_register(0, address, {v}); });

    auto v4l2cmd = std::dynamic_pointer_cast<V4L2BoardCommand>(cmd);

    auto ctrl = v4l2cmd->get_device_control();
    auto cap = ctrl->get_capability();
    auto software_info = device_builder.get_plugin_software_info();
    auto hw_identification = device_builder.add_facility(
        std::make_unique<V4l2HwIdentification>(cap, software_info, *sensor_descriptor));

    try {
        size_t raw_size_bytes = 0;
        auto format           = StreamFormat(hw_identification->get_current_data_encoding_format());
        auto decoder          = make_decoder(device_builder, format, raw_size_bytes, false);
        device_builder.add_facility(std::make_unique<Metavision::I_EventsStream>(
            v4l2cmd->build_data_transfer(raw_size_bytes), hw_identification, decoder, ctrl));
    } catch (std::exception &e) { MV_HAL_LOG_WARNING() << "System can't stream:" << e.what(); }

    sensor_descriptor->spawn_facilities(device_builder, config, hw_identification->get_sensor_info(), register_map);

    return true;
}

} // namespace Metavision
