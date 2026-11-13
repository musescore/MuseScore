# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited and others
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

# Builds the web version of MuseScore Studio (the app and the MuseAudio engine)
# and serves it locally:
#
#   cmake [-D<option>=<value> ...] -P buildscripts/ci/wasm/run_web.cmake
#
# The options are of the framework script, e.g. -DJOBS=4 -DNO_BUILD=ON,
# see: cmake -DHELP=ON -P muse/buildscripts/ci/wasm/run_web.cmake

cmake_minimum_required(VERSION 3.24)

###########################################
# Arguments
###########################################
get_filename_component(_MU_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

if(NOT ROOT)
    set(ROOT "${_MU_ROOT}")
endif()
if(NOT APP)
    set(APP MuseScoreStudio)
endif()
if(NOT AUDIO_ENGINE)
    set(AUDIO_ENGINE src/web/audioengine)
endif()
# The defaults, unless they are given in CMAKE_ARGS
foreach(_arg -DMUSESCORE_BUILD_CONFIGURATION=app-web -DMUSE_APP_BUILD_MODE=dev)
    string(REGEX REPLACE "^-D([^=:]+).*$" "\\1" _key "${_arg}")
    set(_given ${CMAKE_ARGS})
    list(FILTER _given INCLUDE REGEX "^-D${_key}(:[A-Z]+)?=")
    if(NOT _given)
        list(PREPEND CMAKE_ARGS "${_arg}")
    endif()
endforeach()

###########################################
# Versions
###########################################
# The same as on CI
if(NOT EMSDK_VERSION)
    file(STRINGS "${_MU_ROOT}/buildscripts/ci/linux/setup.sh" _line REGEX "^EMSDK_VERSION=")
    string(REGEX REPLACE "^EMSDK_VERSION=\"([^\"]*)\".*" "\\1" EMSDK_VERSION "${_line}")
endif()
if(NOT QT_VERSION)
    file(READ "${_MU_ROOT}/.github/workflows/build_wasm.yml" _workflow)
    string(REGEX MATCH "version: *([0-9.]+)[ \r\n]+ *host: *[^\r\n]*[ \r\n]+ *target: *'wasm'" _ "${_workflow}")
    set(QT_VERSION "${CMAKE_MATCH_1}")
endif()

###########################################
# Run
###########################################
include("${_MU_ROOT}/muse/buildscripts/ci/wasm/run_web.cmake")
