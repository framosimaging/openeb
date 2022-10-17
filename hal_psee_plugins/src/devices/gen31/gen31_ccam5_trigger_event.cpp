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

#include "devices/gen31/gen31_ccam5_trigger_event.h"
#include "utils/register_map.h"

namespace Metavision {

Gen31Ccam5TriggerEvent::Gen31Ccam5TriggerEvent(const std::shared_ptr<RegisterMap> &register_map,
                                               const std::shared_ptr<TzDevice> &device) :
    tzDev_(device), register_map_(register_map) {
    for (uint32_t i = 0; i < 8; ++i) {
        disable(i);
    }
}

bool Gen31Ccam5TriggerEvent::enable(uint32_t channel) {
    if (channel != 0 && channel != 6) {
        return false;
    }

    (*register_map_)["SYSTEM_MONITOR/EXT_TRIGGERS/ENABLE"]["TRIGGER_" + std::to_string(channel)] = true;
    return true;
}

bool Gen31Ccam5TriggerEvent::disable(uint32_t channel) {
    if (channel != 0 && channel != 6) {
        return false;
    }

    (*register_map_)["SYSTEM_MONITOR/EXT_TRIGGERS/ENABLE"]["TRIGGER_" + std::to_string(channel)] = false;
    return true;
}

bool Gen31Ccam5TriggerEvent::is_enabled(uint32_t channel) {
    if (channel != 0 && channel != 6) {
        return false;
    }

    return (*register_map_)["SYSTEM_MONITOR/EXT_TRIGGERS/ENABLE"]["TRIGGER_" + std::to_string(channel)].read_value();
}

} // namespace Metavision
