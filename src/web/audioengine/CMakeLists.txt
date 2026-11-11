# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021-2025 MuseScore Limited and others
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
cmake_minimum_required(VERSION 3.22)

# src/web/audioengine
set(MU_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../..)
set(MUSE_FRAMEWORK_PATH ${MU_ROOT}/muse)
set(MUSE_FRAMEWORK_SRC_PATH ${MUSE_FRAMEWORK_PATH}/framework)

if (NOT EXISTS ${MUSE_FRAMEWORK_SRC_PATH})
    message(FATAL_ERROR "muse framework not found: ${MUSE_FRAMEWORK_SRC_PATH}")
endif()

project(MuseAudio LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MODULE_PATH
    ${MUSE_FRAMEWORK_SRC_PATH}/cmake
    ${MUSE_FRAMEWORK_PATH}/buildscripts
    ${MUSE_FRAMEWORK_PATH}/buildscripts/cmake
    ${CMAKE_MODULE_PATH}
    )

option(MUSE_MODULE_AUDIO_WORKER "Enable audio worker" OFF)
option(MUSE_FIX_MUSEAUDIO_FILENAME "Fix MuseAudio.js file name" OFF)

# Framework configuration: only the audio engine, without Qt and threads
include(GetPaths)
include(${MUSE_FRAMEWORK_SRC_PATH}/cmake/MuseDeclareOptions.cmake)

set(MUSE_QT_SUPPORT OFF)
set(MUSE_THREADS_SUPPORT OFF)
set(MUSE_CONFIGURATION_IS_WEB ON)
set(MUSE_ENABLE_UNIT_TESTS OFF)
set(MUSE_MODULE_AUDIO_EXPORT OFF)
set(MUSE_COMPILE_USE_PCH OFF)
set(MUSE_MODULE_MULTIWINDOWS OFF)

include(${MUSE_FRAMEWORK_SRC_PATH}/cmake/MuseSetupConfiguration.cmake)

include(SetupBuildEnvironment)
include(GetPlatformInfo)

# The artifacts of the app and the audio engine are placed together
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MU_ROOT}/build.artifacts)

# Dependencies: only the source ones, needed by the global module
set(EXTDEPS_DIR ${MU_ROOT}/muse_deps)
set(LOCAL_ROOT_PATH ${PROJECT_BINARY_DIR}/_deps)
include(${EXTDEPS_DIR}/buildtools/manifest.cmake)
require_source_dep(picojson)
require_source_dep(pugixml)
require_source_dep(utfcpp)

add_subdirectory(${MUSE_FRAMEWORK_SRC_PATH}/global global)

set(AUDIO_ROOT ${MUSE_FRAMEWORK_SRC_PATH}/audio)
set(WEBENGINE_FACADE_MODE OFF) # the real engine
add_subdirectory(${AUDIO_ROOT}/common common)
add_subdirectory(${AUDIO_ROOT}/engine engine)

add_executable(MuseAudio
    ${CMAKE_CURRENT_LIST_DIR}/engine_export.cpp
    ${CMAKE_CURRENT_LIST_DIR}/webaudioengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/webaudioengine.h

    ${AUDIO_ROOT}/driver/platform/web/webaudiochannel.cpp
    ${AUDIO_ROOT}/driver/platform/web/webaudiochannel.h
)

target_include_directories(MuseAudio PRIVATE
    ${PROJECT_BINARY_DIR}
    ${MUSE_FRAMEWORK_PATH}
    ${MUSE_FRAMEWORK_SRC_PATH}
    ${MUSE_FRAMEWORK_SRC_PATH}/global
    ${AUDIO_ROOT}
)

target_link_libraries(MuseAudio PRIVATE
    embind
    muse::global
    muse_audio_common
    muse_audio_engine
)

#set_target_properties(MuseAudio PROPERTIES OUTPUT_NAME "MuseAudio.wasm")
#target_link_options(MuseAudio PRIVATE "SHELL:-o MuseAudio.wasm")
target_link_options(MuseAudio PRIVATE ${EMCC_LINKER_FLAGS})
target_link_options(MuseAudio PRIVATE "SHELL:-s EXPORT_NAME=MuseAudio_entry")
target_link_options(MuseAudio PRIVATE "SHELL:-s MODULARIZE=1")
target_link_options(MuseAudio PRIVATE "SHELL:-s FETCH=1")
target_link_options(MuseAudio PRIVATE "SHELL:-s INITIAL_MEMORY=50MB")
target_link_options(MuseAudio PRIVATE "SHELL:-s ALLOW_MEMORY_GROWTH=1")
target_link_options(MuseAudio PRIVATE "SHELL:-s EXPORTED_RUNTIME_METHODS=ccall,cwrap")
target_link_options(MuseAudio PRIVATE "SHELL:-s EXPORTED_FUNCTIONS=[_main,_malloc,_free,_Init,_process]")

if (MUSE_MODULE_AUDIO_WORKER)
    target_link_options(MuseAudio PRIVATE "SHELL:-s ENVIRONMENT=worker")
else()
    target_link_options(MuseAudio PRIVATE "SHELL:-s ENVIRONMENT=shell")
    target_link_options(MuseAudio PRIVATE "SHELL:-s BINARYEN_ASYNC_COMPILATION=0")
    target_link_options(MuseAudio PRIVATE "SHELL:-s SINGLE_FILE=1")
   # target_link_options(MuseAudio PRIVATE "SHELL:-s EXPORT_ES6=1")
endif()


if (MUSE_FIX_MUSEAUDIO_FILENAME)
    add_custom_command(
        TARGET MuseAudio POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rename
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MuseAudio"
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MuseAudio.js"
        COMMENT "Fix output js file name"
    )
endif()
