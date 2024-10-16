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
#include "metavision/hal/utils/hal_log.h"
#include "framos_imx636_ll_biases.h"
#include "framos_imx636_camera_discovery.h"
#include <linux/videodev2.h>

class FramosImx636LLBias {
public:
    FramosImx636LLBias(bool modifiable, std::string bias_name, int sensor_offset, int current_value, int factory_default,
                 int min_offset, int max_offset) {
        bias_name_   = bias_name;
        modifiable_      = modifiable;
        current_offset_  = sensor_offset;
        factory_default_ = factory_default;
        current_value_   = factory_default;
        min_offset_      = min_offset;
        max_offset_      = max_offset;
        min_value_       = factory_default_ + min_offset_;
        max_value_       = factory_default_ + max_offset_;

        display_bias();
    }

    ~FramosImx636LLBias() {}
    bool is_modifiable() const {
        return modifiable_;
    }
    const std::string &get_bias_name() const {
        return bias_name_;
    }
    int get_current_offset() {
        return current_offset_;
    }
    void set_current_offset(const int val) {
        current_offset_ = val;
    }
    int get_current_value() {
        return current_value_;
    }
    void set_current_value(const int val) {
        current_value_ = val;
    }
    int get_factory_default_value() {
        return factory_default_;
    }
    void set_factory_default_value(const int val) {
        factory_default_ = val;
    }
    int get_min_offset() {
        return min_offset_;
    }
    int get_max_offset() {
        return max_offset_;
    }
    int get_min_value() {
        return min_value_;
    }
    int get_max_value() {
        return max_value_;
    }
    void display_bias() {
        MV_HAL_LOG_TRACE() << "bias name:" << bias_name_ << ", factory default:" << factory_default_
                           << ", current value:" << current_value_ << ", current offset:" << current_offset_
                           << ", min offset:" << min_offset_ << ", max offset:" << max_offset_
                           << ", min value:" << min_value_ << ", max value:" << max_value_;
    }

private:
    std::string bias_name_;
    bool modifiable_;
    int current_value_;
    int current_offset_;
    int factory_default_;
    int min_offset_;
    int max_offset_;
    int min_value_;
    int max_value_;
};

std::map<std::string, FramosImx636LLBias> &get_framos_imx636_biases_map() {
    static std::map<std::string, FramosImx636LLBias> biases_map_;
    return biases_map_;
}

FramosImx636_LL_Biases::FramosImx636_LL_Biases(FramosImx636Device& device) 
    : device_(device) {
    
    std::string bias_fo_name       = "bias_fo";
    std::string bias_hpf_name      = "bias_hpf";
    std::string bias_diff_on_name  = "bias_diff_on";
    std::string bias_diff_name     = "bias_diff";
    std::string bias_diff_off_name = "bias_diff_off";
    std::string bias_refr_name     = "bias_refr";

    int bias_fo_sensor_current_offset       = 0;
    int bias_hpf_sensor_current_offset      = 0;
    int bias_diff_on_sensor_current_offset  = 0;
    int bias_diff_sensor_current_offset     = 0;
    int bias_diff_off_sensor_current_offset = 0;
    int bias_refr_sensor_current_offset     = 0;

    int bias_fo_factory_default  = get_default(bias_fo_name);
    int bias_hpf_factory_default = get_default(bias_hpf_name);
    int bias_diff_on_factory_default = get_default(bias_diff_on_name);
    int bias_diff_factory_default = get_default(bias_diff_name);
    int bias_diff_off_factory_default = get_default(bias_diff_off_name);
    int bias_refr_factory_default = get_default(bias_refr_name);

    int bias_fo_current_value       = bias_fo_factory_default;
    int bias_hpf_current_value      = bias_hpf_factory_default;
    int bias_diff_on_current_value  = bias_diff_on_factory_default;
    int bias_diff_current_value     = bias_diff_factory_default;
    int bias_diff_off_current_value = bias_diff_off_factory_default;
    int bias_refr_current_value     = bias_refr_factory_default;

    int bias_fo_min_offset       = BIAS_FO_MIN_OFFSET;
    int bias_fo_max_offset       = BIAS_FO_MAX_OFFSET;
    int bias_hpf_min_offset      = BIAS_HPF_MIN_OFFSET;
    int bias_hpf_max_offset      = BIAS_HPF_MAX_OFFSET;
    int bias_diff_on_min_offset  = BIAS_DIFF_ON_MIN_OFFSET;
    int bias_diff_on_max_offset  = BIAS_DIFF_ON_MAX_OFFSET;
    int bias_diff_min_offset     = BIAS_DIFF_MIN_OFFSET;
    int bias_diff_max_offset     = BIAS_DIFF_MAX_OFFSET;
    int bias_diff_off_min_offset = BIAS_DIFF_OFF_MIN_OFFSET;
    int bias_diff_off_max_offset = BIAS_DIFF_OFF_MAX_OFFSET;
    int bias_refr_min_offset     = BIAS_REFR_MIN_OFFSET;
    int bias_refr_max_offset     = BIAS_REFR_MAX_OFFSET;

    bool bias_fo_modifiable       = true;
    bool bias_hpf_modifiable      = true;
    bool bias_diff_on_modifiable  = true;
    bool bias_diff_modifiable     = false;
    bool bias_diff_off_modifiable = true;
    bool bias_refr_modifiable     = true;

    FramosImx636LLBias fo(bias_fo_modifiable, bias_fo_name, bias_fo_sensor_current_offset, bias_fo_current_value,
                    bias_fo_factory_default, bias_fo_min_offset, bias_fo_max_offset);
    FramosImx636LLBias hpf(bias_hpf_modifiable, bias_hpf_name, bias_hpf_sensor_current_offset,
                     bias_hpf_current_value, bias_hpf_factory_default, bias_hpf_min_offset, bias_hpf_max_offset);
    FramosImx636LLBias diff_on(bias_diff_on_modifiable, bias_diff_on_name, bias_diff_on_sensor_current_offset,
                         bias_diff_on_current_value, bias_diff_on_factory_default, bias_diff_on_min_offset,
                         bias_diff_on_max_offset);
    FramosImx636LLBias diff(bias_diff_modifiable, bias_diff_name, bias_diff_sensor_current_offset,
                      bias_diff_current_value, bias_diff_factory_default, bias_diff_min_offset, bias_diff_max_offset);
    FramosImx636LLBias diff_off(bias_diff_off_modifiable, bias_diff_off_name, bias_diff_off_sensor_current_offset,
                          bias_diff_off_current_value, bias_diff_off_factory_default, bias_diff_off_min_offset,
                          bias_diff_off_max_offset);
    FramosImx636LLBias refr(bias_refr_modifiable, bias_refr_name, bias_refr_sensor_current_offset,
                      bias_refr_current_value, bias_refr_factory_default, bias_refr_min_offset, bias_refr_max_offset);

    // Init map with the values in the registers
    auto &framos_imx636_biases_map = get_framos_imx636_biases_map();
    framos_imx636_biases_map.clear();
    framos_imx636_biases_map.insert({bias_fo_name, fo});
    framos_imx636_biases_map.insert({bias_hpf_name, hpf});
    framos_imx636_biases_map.insert({bias_diff_on_name, diff_on});
    framos_imx636_biases_map.insert({bias_diff_name, diff});
    framos_imx636_biases_map.insert({bias_diff_off_name, diff_off});
    framos_imx636_biases_map.insert({bias_refr_name, refr});

    set(bias_fo_name, bias_fo_current_value);
    set(bias_hpf_name, bias_hpf_current_value);
    set(bias_diff_on_name, bias_diff_on_current_value);
    set(bias_diff_name, bias_diff_current_value);
    set(bias_diff_off_name, bias_diff_off_current_value);
    set(bias_refr_name, bias_refr_current_value);
}

bool FramosImx636_LL_Biases::set(const std::string &bias_name, int bias_value) {
    auto it = get_framos_imx636_biases_map().find(bias_name);
    if (it == get_framos_imx636_biases_map().end()) {
        return false;
    }
    if (it->second.is_modifiable() == false) {
        return false;
    }

    // Display old bias settings
    it->second.display_bias();

    // Check bounds
    if (bias_value < it->second.get_min_value()) {
        MV_HAL_LOG_WARNING() << "Attempted to set " << bias_name << " lower than min value of"
                                << it->second.get_min_value();
        return false;
    } else if (bias_value > it->second.get_max_value()) {
        MV_HAL_LOG_WARNING() << "Attempted to set " << bias_name << " greater than max value of"
                                << it->second.get_max_value();
        return false;
    }

    // Update the value
    it->second.set_current_value(bias_value);
    // Tracking the total offset for later
    it->second.set_current_offset(bias_value - it->second.get_factory_default_value());
    it->second.display_bias();

    v4l2_control bias_ctrl = {0};
    bias_ctrl.id = device_.get_control_id(bias_name);
    if (bias_ctrl.id == 0) {
        throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
            "Bias control not implemented");
    }
    bias_ctrl.value = bias_value;
    if (!device_.xioctl(VIDIOC_S_CTRL, &bias_ctrl))
    {
        throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
            "VIDIOC_S_CTRL Setting bias failed");
    }

    return true;
}

int FramosImx636_LL_Biases::get(const std::string &bias_name) {
    auto it = get_framos_imx636_biases_map().find(bias_name);
    if (it == get_framos_imx636_biases_map().end()) {
        return -1;
    }

    v4l2_control bias_ctrl = {0};
    bias_ctrl.id = device_.get_control_id(bias_name);
    if (bias_ctrl.id == 0) {
        throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
            "Bias control not implemented");
    }
    if (!device_.xioctl(VIDIOC_G_CTRL, &bias_ctrl))
    {
        throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
            "VIDIOC_G_CTRL Getting bias failed");
    }

    auto r = bias_ctrl.value;

    return r;
}

int FramosImx636_LL_Biases::get_default(const std::string &bias_name) {

    v4l2_queryctrl bias_qctrl = {0};
    bias_qctrl.id = device_.get_control_id(bias_name);
    if (bias_qctrl.id == 0) {
        throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
            "Bias control not implemented");
    }
    if (!device_.xioctl(VIDIOC_QUERYCTRL, &bias_qctrl))
    {
        throw Metavision::HalException(Metavision::HalErrorCode::FailedInitialization,
            "VIDIOC_G_CTRL Getting bias failed");
    }

    auto r = bias_qctrl.default_value;

    return r;
}

std::map<std::string, int> FramosImx636_LL_Biases::get_all_biases() {
    std::map<std::string, int> ret;
    for (auto &b : get_framos_imx636_biases_map()) {
        ret[b.first] = get(b.first);
    }
    return ret;
}