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

#ifndef METAVISION_HAL_FRAMOS_IMX636_DEVICE_H
#define METAVISION_HAL_FRAMOS_IMX636_DEVICE_H

#include <string.h>
#include <unordered_map>

enum class FramosDeviceType { IMX636, UNKNOWN };

class FramosImx636Device {
public:
    FramosImx636Device();
    ~FramosImx636Device();
    void initialize(std::string path);
    void open(std::string path);
    void close();
    int get_fd();
    FramosDeviceType get_type();
    std::string get_serial_number();
    bool xioctl(unsigned long request, void* arg);
    unsigned int get_control_id(std::string name);
    uint32_t getInternalBufferSize();
private:
    int fd_;
    bool initialized;
    FramosDeviceType type_;
    std::unordered_map<std::string, unsigned int> control_map_;
    static constexpr uint32_t width = 1024;
    static constexpr uint32_t height = 64;
    static constexpr uint32_t hblank = 250;
    static constexpr uint32_t vblank = 250;
};

#endif // METAVISION_HAL_FRAMOS_IMX636_DEVICE_H