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

cmake_minimum_required(VERSION 3.22) # Min. required by Qt 6.9

project(MuseScore LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/buildscripts
    ${CMAKE_CURRENT_LIST_DIR}/buildscripts/cmake
    ${CMAKE_MODULE_PATH}
    )

set(MUSE_FRAMEWORK_PATH ${CMAKE_CURRENT_LIST_DIR})
set(MUSE_FRAMEWORK_SRC_PATH ${CMAKE_CURRENT_LIST_DIR}/src/framework)

###########################################
# Setup option and build settings
###########################################
include(GetPaths)

set(MUSESCORE_BUILD_CONFIGURATION "app" CACHE STRING "Build configuration")
# Possible MUSESCORE_BUILD_CONFIGURATION values:
# - app             - for desktop app
# - app-portable    - for desktop portable app (Windows build for PortableApps.com)
# - app-web         - for web app (experimental for internal use)
# - vtest           - for visual tests (for CI)
# - utest           - for unit tests (for CI)

set(MUSE_APP_BUILD_MODE "dev" CACHE STRING "Build mode")
# Possible MUSE_APP_BUILD_MODE values:
# - dev     - for development/nightly builds
# - testing - for testing versions (alpha, beta, RC)
# - release - for stable release builds

set(MUSESCORE_REVISION "" CACHE STRING "Build revision")

include(${MUSE_FRAMEWORK_SRC_PATH}/cmake/MuseDeclareOptions.cmake)

# Modules (alphabetical order please)
option(MUE_BUILD_APPSHELL_MODULE "Build appshell module" ON)
option(MUE_BUILD_BRAILLE_MODULE "Build braille module" ON)
option(MUE_BUILD_BRAILLE_TESTS "Build braille tests" ON)
option(MUE_BUILD_CONVERTER_MODULE "Build converter module" ON)
option(MUE_BUILD_ENGRAVING_TESTS "Build engraving tests" ON)
option(MUE_BUILD_ENGRAVING_DEVTOOLS "Build engraving devtools" ON)
option(MUE_BUILD_ENGRAVING_PLAYBACK "Build engraving playback" ON)
option(MUE_BUILD_IMPORTEXPORT_MODULE "Build importexport module" ON)
option(MUE_BUILD_IMPORTEXPORT_TESTS "Build importexport tests" ON)
option(MUE_BUILD_VIDEOEXPORT_MODULE "Build videoexport module" OFF)
option(MUE_BUILD_IMAGESEXPORT_MODULE "Build imagesexport module" ON)
option(MUE_BUILD_INSPECTOR_MODULE "Build inspector module" ON)
option(MUE_BUILD_INSTRUMENTSSCENE_MODULE "Build instruments scene module" ON)
option(MUE_BUILD_MUSESOUNDS_MODULE "Build MuseSounds module" ON)
option(MUE_BUILD_NOTATION_MODULE "Build notation module" ON)
option(MUE_BUILD_NOTATION_TESTS "Build notation tests" ON)
option(MUE_BUILD_PALETTE_MODULE "Build palette module" ON)
option(MUE_BUILD_PLAYBACK_MODULE "Build playback module" ON)
option(MUE_BUILD_PLAYBACK_TESTS "Build playback tests" ON)
option(MUE_BUILD_PRINT_MODULE "Build print module" ON)
option(MUE_BUILD_PROJECT_MODULE "Build project module" ON)
option(MUE_BUILD_PROJECT_TESTS "Build project tests" ON)

# === Setup ===
option(MUE_DOWNLOAD_SOUNDFONT "Download the latest soundfont version as part of the build process" ON)

# === Pack ===
option(MUE_RUN_LRELEASE "Generate .qm files" ON)
option(MUE_INSTALL_SOUNDFONT "Install sound font" ON)

# === Tests ===
set(MUE_VTEST_MSCORE_REF_BIN "${CMAKE_CURRENT_LIST_DIR}/../MU_ORIGIN/MuseScore/build.debug/install/${INSTALL_SUBDIR}/mscore" CACHE PATH "Path to mscore ref bin")

# === Compile ===
option(MUE_COMPILE_INSTALL_QTQML_FILES "Whether to bundle qml files along with the installation (relevant on macOS only)" ON)
option(MUE_COMPILE_MACOS_PRECOMPILED_DEPS_PATH "Path to precompiled dependencies (macOS only; optional: if not specified, some libraries will be used from the system and others will be built from source)" "")

option(MUSE_COMPILE_USE_UNITY "Use unity build" ON)
option(MUSE_COMPILE_USE_CCACHE "Try use ccache" ON)
option(MUSE_COMPILE_USE_SHARED_LIBS_IN_DEBUG "Build shared libs if possible in debug" OFF)

# === System libraries ===
# Important for the maintainers of Linux distributions
option(MUE_COMPILE_USE_SYSTEM_FLAC "Try use system flac" OFF)
option(MUE_COMPILE_USE_SYSTEM_FREETYPE "Try use system freetype" OFF)
option(MUE_COMPILE_USE_SYSTEM_HARFBUZZ "Try use system harfbuzz" OFF)
option(MUE_COMPILE_USE_SYSTEM_OPUS "Try use system opus" OFF)
option(MUE_COMPILE_USE_SYSTEM_OPUSENC "Try use system libopusenc" OFF)
option(MUE_COMPILE_USE_SYSTEM_TINYXML "Try use system tinyxml" OFF)

# === Debug ===
option(MUE_ENABLE_LOAD_QML_FROM_SOURCE "Load qml files from source (not resource)" OFF)
option(MUE_ENABLE_ENGRAVING_RENDER_DEBUG "Enable rendering debug" OFF)
option(MUE_ENABLE_ENGRAVING_LD_ACCESS "Enable diagnostic engraving check layout data access" OFF)
option(MUE_ENABLE_ENGRAVING_LD_PASSES "Enable engraving layout by passes" OFF)


###########################################
# Setup Configure
###########################################

# default
set(QT_ADD_STATEMACHINE ON)
set(QT_ADD_LINGUISTTOOLS ON)
set(QT_ADD_CONCURRENT ON)
set(QT_ADD_WEBSOCKET OFF)
set(QT_QPROCESS_SUPPORTED ON)

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
if (MUSE_COMPILE_USE_CCACHE)
    include(TryUseCcache)
endif()


###########################################
# Setup external dependencies
###########################################

if (MUE_COMPILE_MACOS_PRECOMPILED_DEPS_PATH)
    list(PREPEND CMAKE_PREFIX_PATH ${MUE_COMPILE_MACOS_PRECOMPILED_DEPS_PATH})

    # These are included in the precompiled dependencies, so let's use them from there
    set(MUE_COMPILE_USE_SYSTEM_FLAC ON)
    set(MUE_COMPILE_USE_SYSTEM_OPUS ON)
endif()

include(SetupQt6)

if (MUE_DOWNLOAD_SOUNDFONT)
    include(DownloadSoundFont)
endif(MUE_DOWNLOAD_SOUNDFONT)

###########################################
# Add source tree
###########################################
if (MUSE_ENABLE_UNIT_TESTS)
    enable_testing()
    message(STATUS "Enabled testing")
endif()

add_subdirectory(share)
add_subdirectory(src)


###########################################
# Setup Packaging
###########################################

if (OS_IS_LIN)
    include(packaging/Linux+BSD/SetupAppImagePackaging)
endif(OS_IS_LIN)

if (OS_IS_WIN)
    include(packaging/Windows/SetupWindowsPackaging)
endif(OS_IS_WIN)

###########################################
# Add VTest
###########################################
add_subdirectory(vtest)
