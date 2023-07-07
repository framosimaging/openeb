# Copyright (c) Prophesee S.A.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
# on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and limitations under the License.

# #########################################################
#
# Metavision EMBEDDED Base - debian packages information
#

# File and package name of the components are automatically set, just need to set the package description
# and eventual dependencies

# Runtime (library)
set(CPACK_COMPONENT_METAVISION-EMBEDDED-BASE-LIB_DESCRIPTION "Metavision EMBEDDED Base library.\n${OPEN_PACKAGE_LICENSE}")

# Runtime (apps)
set(CPACK_COMPONENT_METAVISION-EMBEDDED-BASE-BIN_DESCRIPTION "Binaries for the Metavision EMBEDDED Base applications.\n${OPEN_PACKAGE_LICENSE}")
set(CPACK_COMPONENT_METAVISION-EMBEDDED-BASE-BIN_DEPENDS metavision-embedded-base-bin)

# Development package
set(CPACK_COMPONENT_METAVISION-EMBEDDED-BASE-DEV_DESCRIPTION "Development (C++) files for Metavision EMBEDDED Base library.\n${OPEN_PACKAGE_LICENSE}")
set(CPACK_COMPONENT_METAVISION-EMBEDDED-BASE-DEV_DEPENDS metavision-embedded-base-lib)
string(REPLACE ";" ", " CPACK_DEBIAN_METAVISION-EMBEDDED-BASE-DEV_PACKAGE_DEPENDS "${CPACK_DEBIAN_METAVISION-EMBEDDED-BASE-DEV_PACKAGE_DEPENDS}")

# Samples
set(CPACK_COMPONENT_METAVISION-EMBEDDED-BASE-SAMPLES_DESCRIPTION "Samples for Metavision EMBEDDED Base library.\n${OPEN_PACKAGE_LICENSE}")
set(CPACK_COMPONENT_METAVISION-EMBEDDED-BASE-SAMPLES_DEPENDS metavision-embedded-base-dev)