# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021-2025 MuseScore Limited
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

set(WEBWORKER_FACADE_MODE OFF)
include(${CMAKE_CURRENT_LIST_DIR}/../../audio_worker.cmake)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../../thirdparty/fluidsynth fluidsynth)

qt_add_executable(MuseAudio
    ${AUDIO_WORKER_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/workerapi_export.cpp
    ${CMAKE_CURRENT_LIST_DIR}/webworkerapi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/webworkerapi.h
)

target_include_directories(MuseAudio PRIVATE
    ${PROJECT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}

    ${PROJECT_SOURCE_DIR}/src

    ${MUSE_FRAMEWORK_PATH}
    ${MUSE_FRAMEWORK_PATH}/framework
    ${MUSE_FRAMEWORK_PATH}/framework/global
    ${MUSE_FRAMEWORK_PATH}/framework/audio

    # compat
    ${MUSE_FRAMEWORK_PATH}/src
    ${MUSE_FRAMEWORK_PATH}/src/framework
    ${MUSE_FRAMEWORK_PATH}/src/framework/global
    ${MUSE_FRAMEWORK_PATH}/src/framework/audio
    # end compat
)

target_link_libraries(MuseAudio
    PRIVATE muse::global
            muse::audio
            fluidsynth
)

target_link_options(MuseAudio PRIVATE ${EMCC_LINKER_FLAGS})
target_link_options(MuseAudio PRIVATE "SHELL:-s EXPORTED_FUNCTIONS=[_main,_malloc,_free,_Init]")
target_link_options(MuseAudio PRIVATE "SHELL:-s ENVIRONMENT=worker")
