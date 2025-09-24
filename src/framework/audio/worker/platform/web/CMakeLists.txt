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

cmake_minimum_required(VERSION 3.22)

project(MuseAudio LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(MUSE_MODULE_AUDIO_WORKER "Enable audio worker" OFF)

# src/framework/audio/worker/platform/web
set(MU_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../../../../..)
set(MUSE_FRAMEWORK_PATH ${MU_ROOT})
set(MUSE_FRAMEWORK_SRC_PATH ${MU_ROOT}/src/framework)

if (NOT EXISTS ${MU_ROOT}/buildscripts)
    message(FATAL_ERROR "not root: ${MU_ROOT}")
endif()

set(CMAKE_MODULE_PATH
    ${MU_ROOT}
    ${MU_ROOT}/buildscripts
    ${MU_ROOT}/buildscripts/cmake
    ${CMAKE_MODULE_PATH}
    )


include(DeclareModuleSetup)
include(SetupCompileWarnings)
include(GetBuildType)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MU_ROOT}/build.artifacts)

set(EMCC_COMPILE_FLAGS "-s USE_ZLIB=1 -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMCC_COMPILE_FLAGS}")

if (BUILD_IS_DEBUG)
    set(EMCC_LINKER_FLAGS -O0)
else()
    set(EMCC_LINKER_FLAGS -Os)
endif()


set(AUDIO_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../..)
include(${AUDIO_ROOT}/common/audio_common.cmake)
set(WEBWORKER_FACADE_MODE OFF)
include(${AUDIO_ROOT}/worker/audio_worker.cmake)

add_executable(MuseAudio
    ${AUDIO_COMMON_SRC}
    ${AUDIO_WORKER_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/workerapi_export.cpp
    ${CMAKE_CURRENT_LIST_DIR}/webworkerapi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/webworkerapi.h

    ${AUDIO_ROOT}/driver/platform/web/webaudiochannel.cpp
    ${AUDIO_ROOT}/driver/platform/web/webaudiochannel.h
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

add_definitions(-DNO_QT_SUPPORT)
add_definitions(-DQ_OS_WASM)
set(GLOBAL_NO_INTERNAL ON)

configure_file(${MU_ROOT}/src/framework/cmake/muse_framework_config.h.in muse_framework_config.h )

add_subdirectory(${MU_ROOT}/src/framework/global global)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../../thirdparty/fluidsynth fluidsynth)

target_link_libraries(MuseAudio PRIVATE
    embind
    muse::global
    fluidsynth
)

#set_target_properties(MuseAudio PROPERTIES OUTPUT_NAME "MuseAudio.js")
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
    target_link_options(MuseAudio PRIVATE "SHELL:-s EXPORT_ES6=1")
endif()

add_custom_command(
    TARGET MuseAudio POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rename
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MuseAudio"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MuseAudio.js"
    COMMENT "Fix output js file name"
)

