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

#include "framos_imx636_device_control.h"

void FramosImx636DeviceControl::reset() {}

void FramosImx636DeviceControl::start() {}

void FramosImx636DeviceControl::stop() {}

bool FramosImx636DeviceControl::set_mode_standalone() {
    return true;
}

bool FramosImx636DeviceControl::set_mode_master() {
    return false;
}

bool FramosImx636DeviceControl::set_mode_slave() {
    return false;
}

FramosImx636DeviceControl::SyncMode FramosImx636DeviceControl::get_mode() {
    return SyncMode::STANDALONE;
}
