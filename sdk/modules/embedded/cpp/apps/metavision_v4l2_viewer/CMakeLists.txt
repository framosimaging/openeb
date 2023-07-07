# Copyright (c) Prophesee S.A.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
# on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and limitations under the License.

add_executable(metavision_v4l2_viewer metavision_v4l2_viewer.cpp)

target_link_libraries(metavision_v4l2_viewer PRIVATE
    metavision_hal
    MetavisionSDK::base
    MetavisionSDK::embedded
    ${OpenCV_LIBS}
    Boost::program_options)

install(TARGETS metavision_v4l2_viewer
    RUNTIME DESTINATION bin
    COMPONENT metavision-sdk-embedded-bin
)

install(FILES metavision_v4l2_viewer.cpp
    DESTINATION share/metavision/sdk/embedded/apps/metavision_v4l2_viewer
    COMPONENT metavision-sdk-embedded-samples
)

install(FILES CMakeLists.txt.install
    RENAME CMakeLists.txt
    DESTINATION share/metavision/sdk/embedded/apps/metavision_v4l2_viewer
    COMPONENT metavision-sdk-embedded-samples
)
