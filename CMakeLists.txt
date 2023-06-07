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

# Modules framework (alphabetical order please)
option(MUE_BUILD_ACCESSIBILITY_MODULE "Build accessibility module" ON)
option(MUE_BUILD_AUDIO_MODULE "Build audio module" ON)
option(MUE_ENABLE_AUDIO_EXPORT "Enable audio export" ON)
option(MUE_BUILD_MIDI_MODULE "Build midi module" ON)
option(MUE_BUILD_MPE_MODULE "Build mpe module" ON)
option(MUE_BUILD_MUSESAMPLER_MODULE "Build MuseSampler module" ON)
set(MUSESAMPLER_SRC_PATH "" CACHE PATH "Path to MuseSampler sources")
option(MUE_BUILD_NETWORK_MODULE "Build network module" ON)
option(MUE_BUILD_SHORTCUTS_MODULE "Build shortcuts module" ON)
option(MUE_BUILD_UI_MODULE "Build ui module" ON)
set(VST3_SDK_VERSION "3.7")
option(MUE_BUILD_VST_MODULE "Build VST module" OFF)
set(VST3_SDK_PATH "" CACHE PATH "Path to VST3_SDK. SDK version >= ${VST3_SDK_VERSION} required")

# Modules (alphabetical order please)
option(MUE_BUILD_APPSHELL_MODULE "Build appshell module" ON)
option(MUE_BUILD_AUTOBOT_MODULE "Build autobot module" ON)
option(MUE_BUILD_BRAILLE_MODULE "Build braille module (not yet implemented)" OFF)
option(MUE_BUILD_CLOUD_MODULE "Build cloud module" ON)
option(MUE_BUILD_CONVERTER_MODULE "Build converter module" ON)
option(MUE_BUILD_DIAGNOSTICS_MODULE "Build diagnostic code" ON)
option(MUE_BUILD_IMPORTEXPORT_MODULE "Build importexport module" ON)
option(MUE_BUILD_VIDEOEXPORT_MODULE "Build videoexport module" OFF)
option(MUE_BUILD_IMAGESEXPORT_MODULE "Build imagesexport module" ON)
option(MUE_BUILD_INSPECTOR_MODULE "Build inspector module" ON)
option(MUE_BUILD_INSTRUMENTSSCENE_MODULE "Build instruments scene module" ON)
option(MUE_BUILD_LANGUAGES_MODULE "Build languages module" ON)
option(MUE_BUILD_LEARN_MODULE "Build learn module" ON)
set(MUE_LEARN_YOUTUBE_API_KEY "" CACHE STRING "YouTube API key")
option(MUE_BUILD_MULTIINSTANCES_MODULE "Build multiinstances module" ON)
option(MUE_BUILD_NOTATION_MODULE "Build notation module" ON)
option(MUE_BUILD_PALETTE_MODULE "Build palette module" ON)
option(MUE_BUILD_PLAYBACK_MODULE "Build playback module" ON)
option(MUE_BUILD_PLUGINS_MODULE "Build plugins module" ON)
option(MUE_BUILD_PROJECT_MODULE "Build project module" ON)
option(MUE_BUILD_UPDATE_MODULE "Build update module" ON)
option(MUE_BUILD_WORKSPACE_MODULE "Build workspace module" ON)

# === Setup ===
option(MUE_DOWNLOAD_SOUNDFONT "Download the latest soundfont version as part of the build process" ON)

# === Pack ===
option(MUE_RUN_LRELEASE "Generate .qm files" ON)
option(MUE_ENABLE_FILE_ASSOCIATION "File types association (using for package)" OFF)
option(MUE_INSTALL_SOUNDFONT "Install sound font" ON)

# === Tests ===
option(MUE_BUILD_UNIT_TESTS "Build unit tests" ON)
option(MUE_BUILD_ASAN "Enable Address Sanitizer" OFF)
option(MUE_BUILD_CRASHPAD_CLIENT "Build crashpad client" ON)
set(MUE_CRASH_REPORT_URL "" CACHE STRING "URL where to send crash reports")
option(MUE_CRASHPAD_HANDLER_PATH "Path to custom crashpad_handler executable (optional)" "")

# === Tools ===
option(MUE_ENABLE_CUSTOM_ALLOCATOR "Enable custom allocator (used for engraving)" OFF)

# === Compile ===
option(MUE_COMPILE_BUILD_64 "Build 64 bit version of editor" ON)
option(MUE_COMPILE_BUILD_MACOS_APPLE_SILICON "Build for Apple Silicon architecture. Only applicable on Macs with Apple Silicon, and requires suitable Qt version." OFF)
option(MUE_COMPILE_USE_PCH "Use precompiled headers." ON)
option(MUE_COMPILE_USE_UNITY "Use unity build." ON)
option(MUE_COMPILE_USE_CCACHE "Try use ccache" ON)
option(MUE_COMPILE_USE_SHARED_LIBS_IN_DEBUG "Build shared libs if possible in debug" OFF)

# === Debug ===
option(MUE_ENABLE_LOGGER_DEBUGLEVEL "Enable logging debug level" ON)
option(MUE_ENABLE_ACCESSIBILITY_TRACE "Enable accessibility logging" OFF)
option(MUE_ENABLE_DRAW_TRACE "Trace draw objects" ON)
option(MUE_DISABLE_UI_MODALITY "Disable dialogs modality for testing purpose" OFF)
option(MUE_ENABLE_LOAD_QML_FROM_SOURCE "Load qml files from source (not resource)" OFF)
option(MUE_ENABLE_ENGRAVING_PAINT_DEBUGGER "Enable diagnostic engraving paint debugger" OFF)
option(MUE_ENABLE_STRING_DEBUG_HACK "Enable string debug hack (only clang)" ON)

###########################################
# Setup Configure
###########################################
include(SetupConfigure)

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

set(QT_MIN_VERSION    "5.15.0")
include(FindQt5)

if (OS_IS_WIN)
    set(DEPENDENCIES_DIR "${PROJECT_SOURCE_DIR}/dependencies/libx64")
    link_directories(${DEPENDENCIES_DIR})
    include_directories(${PROJECT_SOURCE_DIR}/dependencies/include)
endif(OS_IS_WIN)

include(FindSndFile)

if (MUE_DOWNLOAD_SOUNDFONT)
    include(DownloadSoundFont)
endif(MUE_DOWNLOAD_SOUNDFONT)


###########################################
# Add source tree
###########################################

if (MUE_BUILD_UNIT_TESTS)
    enable_testing()
    message(STATUS "Enabled testing")

    define_property(TARGET PROPERTY OUTPUT_XML
        BRIEF_DOCS "List XML files outputted by google test."
        FULL_DOCS "List XML files outputted by google test."
    )

    add_subdirectory(thirdparty/googletest)
endif(MUE_BUILD_UNIT_TESTS)

add_subdirectory(share)
add_subdirectory(src/framework/global) # should be first to work pch
add_subdirectory(src)

###########################################
# Setup the KDDockWidgets lib
###########################################
if (NOT BUILD_SHARED_LIBS)
    set(KDDockWidgets_STATIC ON CACHE BOOL "Build static versions of the libraries" FORCE)
endif(NOT BUILD_SHARED_LIBS)

set(KDDockWidgets_QTQUICK ON CACHE BOOL "Build for QtQuick instead of QtWidgets" FORCE)
set(KDDockWidgets_EXAMPLES OFF CACHE BOOL "Build the examples" FORCE)
add_subdirectory(thirdparty/KDDockWidgets)

###########################################
# Setup Packaging
###########################################

if (OS_IS_LIN)
    include(SetupAppImagePackaging)
endif(OS_IS_LIN)

if (OS_IS_WIN)
    include(Packaging)
endif(OS_IS_WIN)
