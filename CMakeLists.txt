# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore BVBA and others
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

cmake_minimum_required(VERSION 3.16)

cmake_policy(SET CMP0091 OLD) # not set MSVC default args

project(mscore LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/build
    ${CMAKE_CURRENT_LIST_DIR}/build/cmake
    ${CMAKE_MODULE_PATH}
    )

###########################################
# Setup option and build settings
###########################################
include(GetPaths)

set(MUSESCORE_BUILD_CONFIGURATION "app" CACHE STRING "Build configuration")
# Possible MUSESCORE_BUILD_CONFIGURATION values:
# - app             - for desktop app
# - app-portable    - for desktop portable app (Windows build for PortableApps.com)
# - vtest           - for visual tests (for CI)
# - utest           - for unit tests (for CI)

set(MUSESCORE_BUILD_MODE "dev" CACHE STRING "Build mode")
# Possible MUSESCORE_BUILD_MODE values:
# - dev     - for development/nightly builds
# - testing - for testing versions (alpha, beta, RC)
# - release - for stable release builds

set(MUSESCORE_REVISION "" CACHE STRING "Build revision")

include(muse_framework/DeclareOptions)

# Modules (alphabetical order please)
option(MUE_BUILD_APPSHELL_MODULE "Build appshell module" ON)
option(MUE_BUILD_BRAILLE_MODULE "Build braille module" ON)
option(MUE_BUILD_BRAILLE_TESTS "Build braille tests" ON)
option(MUE_BUILD_CONVERTER_MODULE "Build converter module" ON)
option(MUE_BUILD_ENGRAVING_TESTS "Build engraving tests" ON)
option(MUE_BUILD_ENGRAVING_DEVTOOLS "Build engraving devtools" OFF)
option(MUE_BUILD_IMPORTEXPORT_MODULE "Build importexport module" ON)
option(MUE_BUILD_IMPORTEXPORT_TESTS "Build importexport tests" ON)
option(MUE_BUILD_VIDEOEXPORT_MODULE "Build videoexport module" OFF)
option(MUE_BUILD_IMAGESEXPORT_MODULE "Build imagesexport module" ON)
option(MUE_BUILD_INSPECTOR_MODULE "Build inspector module" ON)
option(MUE_BUILD_INSTRUMENTSSCENE_MODULE "Build instruments scene module" ON)
option(MUE_BUILD_NOTATION_MODULE "Build notation module" ON)
option(MUE_BUILD_NOTATION_TESTS "Build notation tests" ON)
option(MUE_BUILD_PALETTE_MODULE "Build palette module" ON)
option(MUE_BUILD_PLAYBACK_MODULE "Build playback module" ON)
option(MUE_BUILD_PLAYBACK_TESTS "Build playback tests" ON)
option(MUE_BUILD_PROJECT_MODULE "Build project module" ON)
option(MUE_BUILD_PROJECT_TESTS "Build project tests" ON)

# === Setup ===
option(MUE_DOWNLOAD_SOUNDFONT "Download the latest soundfont version as part of the build process" ON)

# === Pack ===
option(MUE_RUN_LRELEASE "Generate .qm files" ON)
option(MUE_INSTALL_SOUNDFONT "Install sound font" ON)

# === Tests ===
set(MUE_VTEST_MSCORE_REF_BIN "${CMAKE_CURRENT_LIST_DIR}/../MU_ORIGIN/MuseScore/build.debug/install/${INSTALL_SUBDIR}/mscore" CACHE PATH "Path to mscore ref bin")
option(MUE_BUILD_ASAN "Enable Address Sanitizer" OFF)
set(MUE_CRASH_REPORT_URL "" CACHE STRING "URL where to send crash reports")
option(MUE_CRASHPAD_HANDLER_PATH "Path to custom crashpad_handler executable (optional)" "")

# === Tools ===
option(MUE_ENABLE_CUSTOM_ALLOCATOR "Enable custom allocator (used for engraving)" OFF)

# === Compile ===
option(MUE_COMPILE_QT5_COMPAT "Build with Qt5" OFF)
option(MUE_COMPILE_BUILD_64 "Build 64 bit version of editor" ON)
option(MUE_COMPILE_BUILD_MACOS_APPLE_SILICON "Build for Apple Silicon architecture. Only applicable on Macs with Apple Silicon, and requires suitable Qt version." OFF)
option(MUE_COMPILE_INSTALL_QTQML_FILES "Whether to bundle qml files along with the installation (relevant on MacOS only)" ON)
option(MUE_COMPILE_USE_PCH "Use precompiled headers." ON)
option(MUE_COMPILE_USE_UNITY "Use unity build." ON)
option(MUE_COMPILE_USE_CCACHE "Try use ccache" ON)
option(MUE_COMPILE_USE_SHARED_LIBS_IN_DEBUG "Build shared libs if possible in debug" OFF)
option(MUE_COMPILE_USE_SYSTEM_FREETYPE "Try use system freetype" OFF) # Important for the maintainer of Linux distributions
option(MUE_COMPILE_USE_SYSTEM_OPUS "Try use system opus" OFF) # Important for the maintainers of Linux distributions

# === Debug ===


option(MUE_ENABLE_LOAD_QML_FROM_SOURCE "Load qml files from source (not resource)" OFF)
option(MUE_ENABLE_ENGRAVING_RENDER_DEBUG "Enable rendering debug" OFF)
option(MUE_ENABLE_ENGRAVING_LD_ACCESS "Enable diagnostic engraving check layout data access" OFF)
option(MUE_ENABLE_ENGRAVING_LD_PASSES "Enable engraving layout by passes" OFF)
option(MUE_ENABLE_STRING_DEBUG_HACK "Enable string debug hack (only clang)" ON)


###########################################
# Setup Configure
###########################################

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/SetupConfigure.local.cmake")
    include(${CMAKE_CURRENT_LIST_DIR}/SetupConfigure.local.cmake)
else()
    include(SetupConfigure)
endif()

set(MUSE_FRAMEWORK_SRC_PATH ${PROJECT_SOURCE_DIR}/src/framework)
set(THIRDPARTY_DIR ${PROJECT_SOURCE_DIR}/thirdparty)

###########################################
# Setup compiler and build environment
###########################################

include(SetupBuildEnvironment)
include(GetPlatformInfo)
if (MUE_COMPILE_USE_CCACHE)
    include(TryUseCcache)
endif(MUE_COMPILE_USE_CCACHE)


###########################################
# Setup external dependencies
###########################################
if (MUE_COMPILE_QT5_COMPAT)
    set(QT_MIN_VERSION "5.15.0")
    include(FindQt5)
else()
    set(QT_MIN_VERSION "6.2.4")
    include(FindQt6)
endif()

if (OS_IS_WIN)
    include(FetchContent)
    FetchContent_Declare(
      musescore_prebuild_win_deps
      GIT_REPOSITORY https://github.com/musescore/musescore_prebuild_win_deps.git
      GIT_TAG        HEAD
    )
    FetchContent_MakeAvailable(musescore_prebuild_win_deps)
    set(DEPENDENCIES_DIR ${musescore_prebuild_win_deps_SOURCE_DIR})
    set(DEPENDENCIES_LIB_DIR ${DEPENDENCIES_DIR}/libx64)
    set(DEPENDENCIES_INC ${DEPENDENCIES_DIR}/include)
endif(OS_IS_WIN)

include(FindSndFile)

if (MUE_DOWNLOAD_SOUNDFONT)
    include(DownloadSoundFont)
endif(MUE_DOWNLOAD_SOUNDFONT)


###########################################
# Add source tree
###########################################
add_subdirectory(share)
add_subdirectory(src)


###########################################
# Setup Packaging
###########################################

if (OS_IS_LIN)
    include(SetupAppImagePackaging)
endif(OS_IS_LIN)

if (OS_IS_WIN)
    include(SetupWindowsPackaging)
endif(OS_IS_WIN)

###########################################
# Add VTest
###########################################
add_subdirectory(vtest)
