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

#include "hal_python_binder.h"
#include "metavision/hal/facilities/i_event_trail_filter_module.h"
#include "pb_doc_hal.h"

namespace Metavision {

namespace {
bool set_stc_cut_trail_filter_wrapper(I_EventTrailFilterModule &self) {
    if (self.get_available_types().count(I_EventTrailFilterModule::Type::STC_CUT_TRAIL) <= 0)
        return false;

    self.set_type(I_EventTrailFilterModule::Type::STC_CUT_TRAIL);
    return true;
}

bool set_trail_filter_wrapper(I_EventTrailFilterModule &self) {
    if (self.get_available_types().count(I_EventTrailFilterModule::Type::TRAIL) <= 0)
        return false;
    self.set_type(I_EventTrailFilterModule::Type::TRAIL);
    return true;
}
} // namespace

// TODO : remove before next major release
// Add depreciation message for get_i_eventrail_fiter_module typo -> get_i_event_trail_filter_module
static DeprecatedDeviceFacilityGetter<I_EventTrailFilterModule> getter_("get_i_eventrailfilter_module",
                                                                        "get_i_event_trail_filter_module", "4.0.1");
static DeviceFacilityGetter<I_EventTrailFilterModule> getter("get_i_event_trail_filter_module");

static HALFacilityPythonBinder<I_EventTrailFilterModule> bind(
    [](auto &module, auto &class_binding) {
        class_binding
            .def("enable", &I_EventTrailFilterModule::enable, py::arg("state"),
                 pybind_doc_hal["Metavision::I_EventTrailFilterModule::enable"])
            .def("is_enabled", &I_EventTrailFilterModule::is_enabled,
                 pybind_doc_hal["Metavision::I_EventTrailFilterModule::is_enabled"])
            .def("set_stc_cut_trail_filter", set_stc_cut_trail_filter_wrapper,
                 "Sets the EventTrailFilterModule filter mode to STC_CUT_TRAIL\n"
                 "\n"
                 "This filter keeps the second event within a burst of events with the same polarity\n"
                 "Returns true on success, false if filter type is not supported\n")
            .def("set_trail_filter", set_trail_filter_wrapper,
                 "Sets the EventTrailFilterModule filter mode to TRAIL\n"
                 "\n"
                 "This filter keeps the first event within a burst of events with the same polarity\n"
                 "Returns true on success, false if filter type is not supported\n")
            .def("set_threshold", &I_EventTrailFilterModule::set_threshold, py::arg("threshold"),
                 pybind_doc_hal["Metavision::I_EventTrailFilterModule::set_threshold"])
            .def("get_threshold", &I_EventTrailFilterModule::get_threshold,
                 pybind_doc_hal["Metavision::I_EventTrailFilterModule::get_threshold"])
            .def("get_min_supported_threshold", &I_EventTrailFilterModule::get_min_supported_threshold,
                 pybind_doc_hal["Metavision::I_EventTrailFilterModule::get_min_supported_threshold"])
            .def("get_max_supported_threshold", &I_EventTrailFilterModule::get_max_supported_threshold,
                 pybind_doc_hal["Metavision::I_EventTrailFilterModule::get_max_supported_threshold"]);
    },
    "I_EventTrailFilterModule", pybind_doc_hal["Metavision::I_EventTrailFilterModule"]);
} // namespace Metavision
