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

#ifndef METAVISION_HAL_FRAMOS_IMX636_LL_BIASES_H
#define METAVISION_HAL_FRAMOS_IMX636_LL_BIASES_H

#include "metavision/hal/facilities/i_ll_biases.h"
#include "framos_imx636_device.h"

static constexpr int BIAS_FO_MIN_OFFSET       = -35;
static constexpr int BIAS_FO_MAX_OFFSET       = 55;
static constexpr int BIAS_HPF_MIN_OFFSET      = 0;
static constexpr int BIAS_HPF_MAX_OFFSET      = 120;
static constexpr int BIAS_DIFF_ON_MIN_OFFSET  = -85;
static constexpr int BIAS_DIFF_ON_MAX_OFFSET  = 140;
static constexpr int BIAS_DIFF_MIN_OFFSET     = -32;
static constexpr int BIAS_DIFF_MAX_OFFSET     = 16;
static constexpr int BIAS_DIFF_OFF_MIN_OFFSET = -35;
static constexpr int BIAS_DIFF_OFF_MAX_OFFSET = 190;
static constexpr int BIAS_REFR_MIN_OFFSET     = -20;
static constexpr int BIAS_REFR_MAX_OFFSET     = 235;

class FramosImx636_LL_Biases : public Metavision::I_LL_Biases {
public:
    FramosImx636_LL_Biases(FramosImx636Device& device);

    virtual bool set(const std::string &bias_name, int bias_value) override;
    virtual int get(const std::string &bias_name) override;
    virtual int get_default(const std::string &bias_name);
    virtual std::map<std::string, int> get_all_biases() override;

private:
    FramosImx636Device& device_;
    
};

#endif // METAVISION_HAL_FRAMOS_IMX636_LL_BIASES_H