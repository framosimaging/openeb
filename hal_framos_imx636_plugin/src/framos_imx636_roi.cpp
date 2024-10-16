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
#include "framos_imx636_roi.h"
#include "framos_imx636_geometry.h"
#include <limits>
#include <linux/videodev2.h>

static constexpr uint32_t bitword_width_ = 40;
static constexpr uint32_t bitword_height_ = 23;
static constexpr uint32_t bitword_step_ = 32;
static constexpr uint32_t bitword_on_ = 0;
static constexpr uint32_t bitword_off_ = std::numeric_limits<uint32_t>::max();

FramosImx636Roi::FramosImx636Roi(FramosImx636Device& device) 
    : device_(device),
     bitword_(bitword_width_ + bitword_height_, bitword_on_) {}

void FramosImx636Roi::enable(bool state) {
    if (state) {
        write_ROIs_from_bitword(bitword_);
    } else {
        write_ROIs_from_bitword(std::vector<uint32_t>(bitword_width_ + bitword_height_, bitword_on_));
    }
}

void FramosImx636Roi::set_ROIs_from_bitword(const std::vector<uint32_t> &vroiparams, bool enable) {
    bitword_ = vroiparams;
    this->enable(enable);
}

bool FramosImx636Roi::set_ROIs(const std::vector<bool> &cols_to_enable, const std::vector<bool> &rows_to_enable, bool enable) {
    if (cols_to_enable.size() != FramosImx636Geometry::WIDTH_
        || rows_to_enable.size() != FramosImx636Geometry::HEIGHT_) {
        return false;
    }
    auto bitword = cols_rows_to_bitword(cols_to_enable, rows_to_enable);;
    set_ROIs_from_bitword(bitword, enable);
    return true;
}

std::vector<uint32_t> FramosImx636Roi::create_ROIs(const std::vector<Metavision::DeviceRoi> &vroi) {
    std::vector<bool> cols_to_enable(FramosImx636Geometry::WIDTH_, false);
    std::vector<bool> rows_to_enable(FramosImx636Geometry::HEIGHT_, false);
    for (const auto& roi : vroi) {
        for (auto i = roi.x_; i < roi.x_ + roi.width_; ++i) {
            cols_to_enable[i] = true;
        }
        for (auto i = roi.y_; i < roi.y_ + roi.height_; ++i) {
            rows_to_enable[i] = true;
        }
    }
    return cols_rows_to_bitword(cols_to_enable, rows_to_enable);
}

std::vector<uint32_t> FramosImx636Roi::cols_rows_to_bitword(const std::vector<bool> &cols_to_enable, const std::vector<bool> &rows_to_enable) {
    std::vector<uint32_t> bitword(bitword_width_ + bitword_height_, bitword_off_);
    for (auto i = 0; i < cols_to_enable.size(); ++i) {
        if (cols_to_enable[i]) {
            bitword[i / bitword_step_] &= ~(1 << i % bitword_step_);
        }
    }
    for (auto i = 0; i < rows_to_enable.size(); ++i) {
        if (rows_to_enable[i]) {
            bitword[bitword_width_ + i / bitword_step_] &= ~(1 << i % bitword_step_);
        }
    }
    bitword[bitword_width_ + bitword_height_ - 1] &= 0x00FFFFFF;
    bitword[bitword_width_ + bitword_height_ - 1] |= 0x00FF0000;
    return bitword;
}

void FramosImx636Roi::write_ROIs_from_bitword(const std::vector<uint32_t> &vroiparams) {

    std::vector<v4l2_ext_control> ctrls;

    v4l2_ext_control roi_ctrl {};
    roi_ctrl.id = device_.get_control_id("Analog Roi");
    if (roi_ctrl.id == 0) {
        throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
            "Analog Roi control not implemented");
        return;
    }
    roi_ctrl.p_u32 = (unsigned int*)vroiparams.data();
    roi_ctrl.size = vroiparams.size() * sizeof(uint32_t);
    ctrls.push_back(roi_ctrl);

    v4l2_ext_control crop_ctrl {};
    crop_ctrl.id = device_.get_control_id("Digital Crop");
    if (crop_ctrl.id == 0) {
        throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
            "Digital Crop control not implemented");
        return;
    }
    auto crop_registers = get_crop_registers(vroiparams);
    crop_ctrl.p_u32 = (unsigned int*)crop_registers.data();
    crop_ctrl.size = crop_registers.size() * sizeof(uint32_t);
    ctrls.push_back(crop_ctrl);
    
    v4l2_ext_controls ext_ctrls {};
    ext_ctrls.controls = ctrls.data();
    ext_ctrls.count = ctrls.size();
    auto success = device_.xioctl(VIDIOC_S_EXT_CTRLS, &ext_ctrls);
    if (!success) {
        throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
            "VIDIOC_S_EXT_CTRLS Failed to set Roi");
        return;
    }
}

std::vector<uint32_t> FramosImx636Roi::get_crop_registers(const std::vector<uint32_t> &vroiparams) {

    static constexpr uint32_t crop_mask_x = 0x7FF;
    static constexpr uint32_t crop_mask_y = 0x3FF;
    static constexpr uint32_t crop_shift_y = 16;

    uint32_t x_start = 0, x_end = 0;
    bool x_found = false;
    for (auto i = 0; i < bitword_width_; ++i) {
        if (vroiparams[i] != bitword_off_) {
            if (!x_found) {
                x_start = i * bitword_step_;
                x_found = true;
            }
            x_end = (i + 1) * bitword_step_ - 1;
        }
    }

    uint32_t y_start = 0, y_end = 0;
    bool y_found = false;
    for (auto i = 0; i < bitword_height_; ++i) {
        auto step = bitword_step_;
        if (i == bitword_height_ - 1) {
            step /= 2;
        }
        for (auto j = 0; j < step; ++j) {
            if (~vroiparams[bitword_width_ + i] & (1 << j)) {
                if (!y_found) {
                    y_start = i * bitword_step_ + j;
                    y_found = true;
                }
                y_end = i * bitword_step_ + j;
            }
        }
    }

    bool all_pixels_active = x_start == 0 
        && x_end == FramosImx636Geometry::WIDTH_ - 1
        && y_start == 0
        && y_end == FramosImx636Geometry::HEIGHT_ - 1;
    if (all_pixels_active || !x_found || !y_found) {
        x_start = 0;
        y_start = 0;
        x_end = 0;
        y_end = 0;
    } else if (y_start == y_end) {
        if (y_start == 0) {
            ++y_end;
        } else {
            --y_start;
        }
    }

    std::vector<uint32_t> crop_registers(2, 0);
    crop_registers[0] |= x_start & crop_mask_x;
    crop_registers[0] |= (y_start & crop_mask_y) << crop_shift_y;
    crop_registers[1] |= x_end & crop_mask_x;
    crop_registers[1] |= (y_end & crop_mask_y) << crop_shift_y;

    return crop_registers;
}