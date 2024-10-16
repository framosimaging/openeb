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

#ifndef METAVISION_HAL_FRAMOS_IMX636_ROI_H
#define METAVISION_HAL_FRAMOS_IMX636_ROI_H

#include <metavision/hal/facilities/i_roi.h>
#include <metavision/hal/utils/device_roi.h>
#include "framos_imx636_device.h"

class FramosImx636Roi : public Metavision::I_ROI {

public:
    FramosImx636Roi(FramosImx636Device& device);
    virtual void enable(bool state) override final;
    virtual void set_ROIs_from_bitword(const std::vector<uint32_t> &vroiparams, bool enable = true) override final;
    virtual bool set_ROIs(const std::vector<bool> &cols_to_enable, const std::vector<bool> &rows_to_enable,
                          bool enable = true) override final;
    virtual std::vector<uint32_t> create_ROIs(const std::vector<Metavision::DeviceRoi> &vroi) override final;

private:
    FramosImx636Device& device_;
    std::vector<uint32_t> bitword_;
    std::vector<uint32_t> cols_rows_to_bitword(const std::vector<bool> &cols_to_enable, const std::vector<bool> &rows_to_enable);
    void write_ROIs_from_bitword(const std::vector<uint32_t> &vroiparams);
    std::vector<uint32_t> get_crop_registers(const std::vector<uint32_t> &vroiparams);
};

#endif // METAVISION_HAL_FRAMOS_IMX636_ROI_H