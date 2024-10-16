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

#include "framos_imx636_hw_identification.h"

constexpr long FramosImx636HWIdentification::FRAMOS_IMX636_SYSTEM_ID;
constexpr long FramosImx636HWIdentification::FRAMOS_IMX636_SYSTEM_VERSION;
constexpr const char *const FramosImx636HWIdentification::FRAMOS_IMX636_INTEGRATOR;

FramosImx636HWIdentification::FramosImx636HWIdentification(const std::shared_ptr<Metavision::I_PluginSoftwareInfo> &plugin_sw_info,
                                           const std::string &connection_type, std::string serial) :
    Metavision::I_HW_Identification(plugin_sw_info), connection_type_(connection_type), serial(serial) {}

std::string FramosImx636HWIdentification::get_serial() const {
    return serial;
}
long FramosImx636HWIdentification::get_system_id() const {
    return FRAMOS_IMX636_SYSTEM_ID;
}

FramosImx636HWIdentification::SensorInfo FramosImx636HWIdentification::get_sensor_info() const {
    return SensorInfo({4, 1});
}

long FramosImx636HWIdentification::get_system_version() const {
    return FRAMOS_IMX636_SYSTEM_VERSION;
}

std::vector<std::string> FramosImx636HWIdentification::get_available_raw_format() const {
    return {"FRAMOS_IMX636-FORMAT-1.0"};
}

std::string FramosImx636HWIdentification::get_integrator() const {
    return FRAMOS_IMX636_INTEGRATOR;
}

std::string FramosImx636HWIdentification::get_connection_type() const {
    return connection_type_;
}
