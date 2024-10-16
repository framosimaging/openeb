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

#include <string>
#include <memory>
#include "metavision/hal/plugin/plugin_entrypoint.h"
#include "metavision/hal/utils/hal_software_info.h"

#include "framos_imx636_camera_discovery.h"
#include "framos_imx636_file_discovery.h"
#include "framos_imx636_hw_identification.h"

int FRAMOS_IMX636_PLUGIN_VERSION_MAJOR                 = 0;
int FRAMOS_IMX636_PLUGIN_VERSION_MINOR                 = 1;
int FRAMOS_IMX636_PLUGIN_VERSION_PATCH                 = 0;
std::string FRAMOS_IMX636_PLUGIN_VERSION_SUFFIX_TYPE   = "hal-framos-imx636-plugin-version-suffix-type";
std::string FRAMOS_IMX636_PLUGIN_VCS_BRANCH            = "hal-framos-imx636-plugin-vcs-branch";
std::string FRAMOS_IMX636_PLUGIN_VCS_COMMIT            = "hal-framos-imx636-plugin-vcs-commit";
std::string FRAMOS_IMX636_PLUGIN_VCS_COMMIT_DATE       = "hal-framos-imx636-plugin-vcs-commit-date";

namespace {
Metavision::SoftwareInfo get_framos_imx636_plugin_software_info() {
    return Metavision::SoftwareInfo(FRAMOS_IMX636_PLUGIN_VERSION_MAJOR, FRAMOS_IMX636_PLUGIN_VERSION_MINOR, FRAMOS_IMX636_PLUGIN_VERSION_PATCH,
                                    FRAMOS_IMX636_PLUGIN_VERSION_SUFFIX_TYPE, FRAMOS_IMX636_PLUGIN_VCS_BRANCH, FRAMOS_IMX636_PLUGIN_VCS_COMMIT,
                                    FRAMOS_IMX636_PLUGIN_VCS_COMMIT_DATE);
}
} // namespace

// The plugin name is the name of the library loaded without lib and the extension
void initialize_plugin(void *plugin_ptr) {
    Metavision::Plugin &plugin = Metavision::plugin_cast(plugin_ptr);

    plugin.set_integrator_name(FramosImx636HWIdentification::FRAMOS_IMX636_INTEGRATOR);
    plugin.set_plugin_info(get_framos_imx636_plugin_software_info());
    plugin.set_hal_info(Metavision::get_hal_software_info());

    plugin.add_camera_discovery(std::make_unique<FramosImx636CameraDiscovery>());
    plugin.add_file_discovery(std::make_unique<FramosImx636FileDiscovery>());
}
