# Copyright (c) Prophesee S.A.
# Copyright (c) Framos GmbH
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
# on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and limitations under the License.

# Add FRAMOS IMX636 plugin object library
add_library(hal_framos_imx636_plugin_obj OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/src/framos_imx636_camera_discovery.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/framos_imx636_device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/framos_imx636_data_transfer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/framos_imx636_device_control.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/framos_imx636_file_discovery.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/framos_imx636_geometry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/framos_imx636_hw_identification.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/framos_imx636_ll_biases.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/framos_imx636_roi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/framos_imx636_plugin.cpp
)

# The following line is needed becase when linking a shared library to an object one,
# the object library needs to be compiled with -fPIC
# cf https://stackoverflow.com/questions/50600708/combining-cmake-object-libraries-with-shared-libraries
set_target_properties(hal_framos_imx636_plugin_obj
    PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)

if(NOT ${CMAKE_VERSION} VERSION_LESS "3.13.3" AND NOT APPLE AND NOT MSVC)
    target_link_options(hal_framos_imx636_plugin_obj PRIVATE "LINKER:-z,defs")
endif()

target_include_directories(hal_framos_imx636_plugin_obj
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(hal_framos_imx636_plugin_obj
    PUBLIC
        metavision_hal
)

# Add FRAMOS IMX636 plugin library
add_library(hal_framos_imx636_plugin SHARED $<TARGET_OBJECTS:hal_framos_imx636_plugin_obj>)
target_link_libraries(hal_framos_imx636_plugin PRIVATE hal_framos_imx636_plugin_obj)
# Instead of setting the RUNTIME/LIBRARY_OUTPUT_DIRECTORY property on the target, we manually copy
# the library : this will work for linux and windows and avoid the automatic copy of the DLLs the
# plugin depends on by MSVC
add_custom_command(TARGET hal_framos_imx636_plugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${HAL_BUILD_PLUGIN_PATH}/framos_imx636"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:hal_framos_imx636_plugin>" "${HAL_BUILD_PLUGIN_PATH}/framos_imx636")


# Install FRAMOS IMX636
install(FILES README.md
        DESTINATION share/metavision/hal/framos_imx636/metavision_hal_framos_imx636_plugin
        COMPONENT metavision-hal-framos_imx636
)
install(DIRECTORY include src
        DESTINATION share/metavision/hal/framos_imx636/metavision_hal_framos_imx636_plugin
        COMPONENT metavision-hal-framos_imx636
)
install(FILES CMakeLists.txt.install
        RENAME CMakeLists.txt
        DESTINATION share/metavision/hal/framos_imx636/metavision_hal_framos_imx636_plugin
        COMPONENT metavision-hal-framos_imx636
)

