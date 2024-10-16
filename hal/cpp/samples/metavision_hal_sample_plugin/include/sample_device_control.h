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

#ifndef METAVISION_HAL_SAMPLE_DEVICE_CONTROL_H
#define METAVISION_HAL_SAMPLE_DEVICE_CONTROL_H

#include <metavision/hal/facilities/i_device_control.h>

/// @brief Interface for controlling live camera
///
/// This class is the implementation of HAL's facility @ref Metavision::I_DeviceControl.
/// In this sample is just an empty class, but for a real camera you'll need to implement
/// the methods that allows to start, reset and stop the camera.
class SampleDeviceControl : public Metavision::I_DeviceControl {
public:
    /// @brief Restarts the device and the connection with it
    void reset() override final;

    /// @brief Starts the generation of events from the camera side
    void start() override final;

    /// @brief Stops the generation of events from the camera side
    void stop() override final;

    /// @brief Sets the camera in standalone mode.
    ///
    /// The camera does not interact with other devices.
    ///
    /// @return true on success
    bool set_mode_standalone() override final;

    /// @brief Sets the camera as master
    ///
    /// The camera sends clock signal to another device
    ///
    /// @return true on success
    bool set_mode_master() override final;

    /// @brief Sets the camera as slave
    ///
    /// The camera receives the clock from another device
    ///
    /// @return true on success
    bool set_mode_slave() override final;

    /// @brief Retrieves Synchronization mode
    /// @return synchronization mode
    SyncMode get_mode() override final;
};

#endif // METAVISION_HAL_SAMPLE_DEVICE_CONTROL_H
