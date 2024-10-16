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

#ifndef METAVISION_HAL_FRAMOS_IMX636_HW_IDENTIFICATION_H
#define METAVISION_HAL_FRAMOS_IMX636_HW_IDENTIFICATION_H

#include <metavision/hal/facilities/i_hw_identification.h>

/// @brief Facility to provide information about the available system
///
/// This class is the implementation of HAL's facility @ref Metavision::I_HW_Identification
class FramosImx636HWIdentification : public Metavision::I_HW_Identification {
public:
    /// @brief Constructor
    ///
    /// @param plugin_sw_info Information about the plugin software version
    /// @param connection_type Type of connection with the device
    /// @param serial Serial number of the device
    FramosImx636HWIdentification(const std::shared_ptr<Metavision::I_PluginSoftwareInfo> &plugin_sw_info,
                         const std::string &connection_type, std::string serial);

    /// @brief Returns the serial number of the camera
    ///
    /// @return Serial number as a string
    std::string get_serial() const override final;

    /// @brief Returns the system ID of the camera
    ///
    /// @return The system id as an integer
    long get_system_id() const override final;

    /// @brief Returns the detail about the available sensor
    ///
    /// @return The sensor information
    I_HW_Identification::SensorInfo get_sensor_info() const override final;

    /// @brief Returns the version number for this system
    ///
    /// @return System version as an integer
    long get_system_version() const override final;

    /// @brief Returns the name of the available RAW format
    ///
    /// @return The available format
    std::vector<std::string> get_available_raw_format() const override final;

    /// @brief Returns the integrator name
    ///
    /// @return Name of the integrator
    std::string get_integrator() const override final;

    /// @brief Returns the connection with the camera as a string
    ///
    /// @return A string providing the type of connection with the available camera
    std::string get_connection_type() const override final;

    static constexpr long FRAMOS_IMX636_SYSTEM_ID      = 01;
    static constexpr long FRAMOS_IMX636_SYSTEM_VERSION = 1;
    static constexpr auto FRAMOS_IMX636_INTEGRATOR     = "Framos GmbH";

private:
    std::string serial;
    std::string connection_type_; 
};

#endif // METAVISION_HAL_FRAMOS_IMX636_HW_IDENTIFICATION_H
