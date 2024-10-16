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

#include "metavision/hal/utils/hal_exception.h"
#include "framos_imx636_device.h"
#include <algorithm>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

FramosImx636Device::FramosImx636Device()
    : initialized(false) {}

FramosImx636Device::~FramosImx636Device() {
    close();
}

void FramosImx636Device::initialize(std::string path) {

    // Already initialized
    if (initialized) {
        return;
    }

    open(path);

    // Set Width and Height
    struct v4l2_format set_fmt {};
    set_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    set_fmt.fmt.pix.width = width;
    set_fmt.fmt.pix.height = height;
    set_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_SRGGB8;
    set_fmt.fmt.pix.field = V4L2_FIELD_NONE;
    bool success = xioctl(VIDIOC_S_FMT, &set_fmt);
    if (!success) {
        throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
            "Setting width, height and pixel format");
    }

    std::vector<v4l2_ext_control> ctrls;

    v4l2_ext_control hblank_ctrl {};
    hblank_ctrl.id = get_control_id("Horizontal Blank");
    if (hblank_ctrl.id == 0) {
        throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
            "Horizontal Blank control not implemented");
    }
    hblank_ctrl.value = hblank;
    ctrls.push_back(hblank_ctrl);
    
    v4l2_ext_control vblank_ctrl {};
    vblank_ctrl.id = get_control_id("Vertical Blank");
    if (vblank_ctrl.id == 0) {
        throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
            "Vertical Blank control not implemented");
    }
    vblank_ctrl.value = vblank;
    ctrls.push_back(vblank_ctrl);

    v4l2_ext_controls ext_ctrls {};
    ext_ctrls.controls = ctrls.data();
    ext_ctrls.count = ctrls.size();
    success = xioctl(VIDIOC_S_EXT_CTRLS, &ext_ctrls);
    if (!success) {
        throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
            "Failed to set blanking");
        return;
    }

    initialized = true;
}

void FramosImx636Device::open(std::string path) {
    
    fd_ = ::open(path.c_str(), O_RDWR);
    if (fd_ == -1) {
        throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
            "Open failed " + path);
    }

    v4l2_capability cap {};
    bool success = xioctl(VIDIOC_QUERYCAP, &cap);
    if (!success) {
        throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
            "VIDIOC_QUERYCAP failed " + path);
    }
    std::string card((char*)cap.card);
    std::transform(card.begin(), card.end(), card.begin(),
        [](u_char c){ return std::tolower(c); });
    if (card.find("imx636") != std::string::npos) {
        type_ = FramosDeviceType::IMX636;
    } else {
        type_ = FramosDeviceType::UNKNOWN;
    }

    v4l2_query_ext_ctrl qctrl {};
    qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL | V4L2_CTRL_FLAG_NEXT_COMPOUND;
    while (xioctl(VIDIOC_QUERY_EXT_CTRL, &qctrl)) {
        control_map_[std::string((char*)qctrl.name)] = qctrl.id;
        qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL | V4L2_CTRL_FLAG_NEXT_COMPOUND;
    }
}

void FramosImx636Device::close() {
    initialized = false;
    ::close(fd_);
}

int FramosImx636Device::get_fd() {
    return fd_;
}

FramosDeviceType FramosImx636Device::get_type() {
    return type_;
}

std::string FramosImx636Device::get_serial_number() {
	
    std::string serial_num = "";
    v4l2_ext_control eeprom_data_control = {};
    char eeprom_data[513];
    eeprom_data_control.size = 513;
    eeprom_data_control.string = eeprom_data;
    eeprom_data_control.id = get_control_id("EEPROM Data");
    if (eeprom_data_control.id == 0) {
        throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
            "EEPROM Data not implemented");
    }

    v4l2_ext_controls eeprom_data_ext_ctrls {};
    eeprom_data_ext_ctrls.controls = &eeprom_data_control;
    eeprom_data_ext_ctrls.count = 1;
    if (!xioctl(VIDIOC_G_EXT_CTRLS, &eeprom_data_ext_ctrls))
    {
        throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
            "VIDIOC_G_CTRL Getting EEPROM Data failed");
    }
    // Convert charaters to hexadecimal serial number
    char serial_num_char[14*2];
    strncpy(serial_num_char, eeprom_data+179*2, 14*2);
    for(int i = 0; i < 14; ++i) {
        std::string tmp_str(&serial_num_char[i*2], 2);
        char c = stoi(tmp_str, 0, 16);
        serial_num += c;
    }
    
    return serial_num;
}

bool FramosImx636Device::xioctl(unsigned long request, void* arg) {
    static constexpr uint32_t max_retry = 2;
    uint32_t retry_attempts = 0;
    while (::ioctl(fd_, request, arg) == -1 && retry_attempts < max_retry) {
        ++retry_attempts;
    }
    return retry_attempts != max_retry;
}

unsigned int FramosImx636Device::get_control_id(std::string name) {
    if (control_map_.find(name) != control_map_.end()) {
        return control_map_[name];
    }
    return 0;
}

uint32_t FramosImx636Device::getInternalBufferSize() {
    return width*height;
}