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

include(GetCompilerInfo)
include(GetBuildType)

# Unity build defaults
if(NOT DEFINED CMAKE_UNITY_BUILD_BATCH_SIZE)
    set(CMAKE_UNITY_BUILD_BATCH_SIZE 12)
endif()

# Debug/no-debug options
add_compile_definitions($<$<CONFIG:Debug>:QT_QML_DEBUG>)

add_compile_definitions($<$<NOT:$<CONFIG:Debug>>:NDEBUG>)
add_compile_definitions($<$<NOT:$<CONFIG:Debug>>:QT_NO_DEBUG>)

# String debug hack
if(MUSE_COMPILE_STRING_DEBUG_HACK AND CC_IS_CLANG)
    add_compile_definitions($<$<CONFIG:Debug>:MUSE_STRING_DEBUG_HACK>)
endif()

# Shared libraries
set(BUILD_SHARED_LIBS OFF)
set(SHARED_LIBS_INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

if(MUE_COMPILE_USE_SHARED_LIBS_IN_DEBUG AND BUILD_IS_DEBUG)
    if(CC_IS_GCC OR CC_IS_MINGW)
        set(BUILD_SHARED_LIBS ON)
    endif()
endif()

# Address Sanitizer
if(MUSE_COMPILE_ASAN)
    if(CC_IS_CLANG OR CC_IS_GCC OR CC_IS_MINGW)
        add_compile_options("-fsanitize=address")
        add_compile_options("-fno-omit-frame-pointer")
        add_link_options("-fsanitize=address")
    elseif(CC_IS_MSVC)
        add_compile_options("/fsanitize=address")
    endif()
endif()

# Mac-specific
if(OS_IS_MAC)
    set(MACOSX_DEPLOYMENT_TARGET 10.14)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14)
endif(OS_IS_MAC)

# MSVC-specific
if(CC_IS_MSVC)
    # TODO: remove this
    # It is necessary because of the following line in src/app/CMakeLists.txt:
    # `set_target_properties(${QtLibrary} PROPERTIES MAP_IMPORTED_CONFIG_DEBUG "RELEASE")`
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL") # i.e. not MultiThreaded$<$<CONFIG:Debug>:Debug>DLL

    add_compile_options("/EHsc")
    add_compile_options("/utf-8")

    add_compile_definitions(WIN32 _WINDOWS)
    add_compile_definitions(_UNICODE UNICODE)
    add_compile_definitions(_USE_MATH_DEFINES)
    add_compile_definitions(NOMINMAX)
endif()

# MinGW-specific
if(CC_IS_MINGW)
    # https://musescore.org/node/22048
    add_compile_options(-mno-ms-bitfields)

    if(NOT MUSE_COMPILE_BUILD_64)
        add_link_options("-Wl,--large-address-aware")
    endif()

    add_compile_definitions(_UNICODE UNICODE)
endif()

# Wasm-specific
if(CC_IS_EMSCRIPTEN)
    set(EMCC_CMAKE_TOOLCHAIN "" CACHE FILEPATH "Path to EMCC CMake Emscripten.cmake")
    set(EMCC_INCLUDE_PATH "." CACHE PATH "Path to EMCC include dir")
    set(EMCC_COMPILE_FLAGS "--bind -o .html --preload-file ../../files")

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/public_html)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMCC_COMPILE_FLAGS}")
    set(CMAKE_TOOLCHAIN_FILE ${EMCC_CMAKE_TOOLCHAIN})
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    # for QtCreator
    include_directories(AFTER
        ${EMCC_INCLUDE_PATH}
        ${EMCC_INCLUDE_PATH}/libcxx
        ${EMCC_INCLUDE_PATH}/libc
    )
endif(CC_IS_EMSCRIPTEN)

# Warnings
include(SetupCompileWarnings)
