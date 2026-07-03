# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2024 MuseScore Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

include(GetPlatformInfo)

set(EXTDEPS_DIR "${CMAKE_SOURCE_DIR}/muse_deps")
if (NOT EXISTS "${EXTDEPS_DIR}/buildtools/manifest.cmake")
    message(FATAL_ERROR "muse_deps submodule missing, run: git submodule update --init muse_deps")
endif()

set(LOCAL_ROOT_PATH ${FETCHCONTENT_BASE_DIR})
include(${EXTDEPS_DIR}/buildtools/manifest.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/DependencyManifest.cmake)
include(${MUSE_FRAMEWORK_PATH}/buildscripts/cmake/ExtDepsManifest.cmake)

extdeps_install_consumed(MACOS_BUNDLE mscore.app)

add_custom_target(prepare_deps_sources
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_LIST_DIR}/PrepareDepsSources.cmake"
    COMMENT "Pre-fetching dependency sources into the cache"
    VERBATIM
)
