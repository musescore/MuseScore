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
    ${CMAKE_CURRENT_LIST_DIR}/build
    ${CMAKE_CURRENT_LIST_DIR}/build/cmake
    ${CMAKE_MODULE_PATH}
    )

###########################################
# Setup option and build settings
###########################################

set(MUSESCORE_REVISION "" CACHE STRING "Build revision")
set(MUSESCORE_BUILD_CONFIG "dev" CACHE STRING "Build config")
# Possible MUSESCORE_BUILD_CONFIG values:
# - dev     - for development/nightly builds
# - testing - for testing versions (alpha, beta, RC)
# - release - for stable release builds

option(BUILD_64 "Build 64 bit version of editor" ON)
option(BUILD_PORTABLEAPPS "Windows build for PortableApps.com" OFF)
option(BUILD_FOR_WINSTORE "Build for the Windows Store." OFF)
option(BUILD_AUTOUPDATE "Build with autoupdate support" OFF)

option(BUILD_TELEMETRY_MODULE "Build with telemetry module" ON)
set(TELEMETRY_TRACK_ID "" CACHE STRING "Telemetry track id")
set(CRASH_REPORT_URL "" CACHE STRING "URL where to send crash reports")

option(BUILD_SHORTCUTS_MODULE "Build shortcuts module" ON)
option(BUILD_SYSTEM_MODULE "Build system module" ON)
option(BUILD_NETWORK_MODULE "Build network module" ON)
option(BUILD_AUDIO_MODULE "Build audio module" ON)
option(BUILD_LEARN_MODULE "Build learn module" ON)
option(BUILD_WORKSPACE_MODULE "Build workspace module" ON)
option(BUILD_CLOUD_MODULE "Build cloud module" ON)
option(BUILD_EXTENSIONS_MODULE "Build extensions module" ON)
option(BUILD_LANGUAGES_MODULE "Build languages module" ON)
option(BUILD_PLUGINS_MODULE "Build plugins module" ON)
option(BUILD_PLAYBACK_MODULE "Build playback module" ON)
option(BUILD_PALETTE_MODULE "Build palette module" ON)
option(BUILD_INSTRUMENTSSCENE_MODULE "Build instruments scene module" ON)
option(BUILD_INSPECTOR_MODULE "Build inspector module" ON)
option(BUILD_AUTOBOT_MODULE "Build autobot module" OFF)
option(BUILD_MULTIINSTANCES_MODULE "Build multiinstances module" ON)
option(BUILD_DIAGNOSTICS "Build diagnostic code" ON)
set(YOUTUBE_API_KEY "" CACHE STRING "YouTube API key")

option(ENGRAVING_BUILD_ACCESSIBLE_TREE "Build score accessible tree" OFF)
# Temporary flags for MU3 compatibility to make testing easier.
option(ENGRAVING_COMPAT_WRITESTYLE_302 "Write style to score xml file" ON)
option(ENGRAVING_COMPAT_WRITEEXCERPTS_302 "Write excerpts to score xml file" ON)
# -----

option(SOUNDFONT3    "Ogg Vorbis compressed fonts"        ON)  # Enable Ogg Vorbis compressed fonts, requires Ogg & Vorbis
option(DOWNLOAD_SOUNDFONT "Download the latest soundfont version as part of the build process" ON)

option(BUILD_UNIT_TESTS "Build gtest unit test" OFF)
option(PACKAGE_FILE_ASSOCIATION "File types association" OFF)

option(TRY_USE_CCACHE "Try use ccache" ON)
option(BUILD_PCH "Build using precompiled headers." ON)
option(BUILD_UNITY "Build using unity build." ON)
option(TRY_BUILD_SHARED_LIBS_IN_DEBUG "Build shared libs if possible in debug" OFF)
option(QML_LOAD_FROM_SOURCE "Load qml files from source (not resource)" OFF)
option(TRACE_DRAW_OBJ_ENABLED "Trace draw objects" OFF)

option(USE_SYSTEM_FREETYPE "Use system FreeType" OFF) # requires freetype >= 2.5.2, does not work on win
set(SCRIPT_INTERFACE  TRUE)

set(VST3_SDK_VERSION "3.7")
option(BUILD_VST "Build VST MODULE" OFF)
set(VST3_SDK_PATH "" CACHE PATH "Path to VST3_SDK. SDK version >= ${VST3_SDK_VERSION} required")

set(JACK_LONGNAME "JACK (Jack Audio Connection Kit)")
set(JACK_MIN_VERSION "0.98.0")
option(BUILD_JACK    "Build with support for ${JACK_LONGNAME} audio backend. JACK >= ${JACK_MIN_VERSION} will be needed." OFF)

if (BUILD_PORTABLEAPPS)
    set(WIN_PORTABLE 1)
endif (BUILD_PORTABLEAPPS)

if (BUILD_FOR_WINSTORE)
    set(FOR_WINSTORE 1)
endif(BUILD_FOR_WINSTORE)

set(APP_UPDATABLE 1)
if (OS_IS_MAC OR OS_IS_WIN)
    if (WIN_PORTABLE OR FOR_WINSTORE)
        set(APP_UPDATABLE 0)
    endif(WIN_PORTABLE OR FOR_WINSTORE)
endif(OS_IS_MAC OR OS_IS_WIN)

###########################################
# Setup compiler and build environment
###########################################

include(SetupBuildEnvironment)
include(GetPlatformInfo)
if (TRY_USE_CCACHE)
    include(TryUseCcache)
endif(TRY_USE_CCACHE)

###########################################
# Setup MuseScore config
###########################################

include(${CMAKE_CURRENT_LIST_DIR}/config.cmake)
configure_file (
      ${PROJECT_SOURCE_DIR}/build/config.h.in
      ${PROJECT_BINARY_DIR}/config.h
      )


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

include(SetupFreetype)

include(FindSndFile)

#if(SOUNDFONT3)
#    include(FindVorbisAndOgg)
#endif(SOUNDFONT3)

if (BUILD_JACK)
    include(FindJack)
endif(BUILD_JACK)

if (BUILD_AUTOUPDATE)
    include(SetupSparkle)
endif(BUILD_AUTOUPDATE)

if (DOWNLOAD_SOUNDFONT)
    include(DownloadSoundFont)
endif(DOWNLOAD_SOUNDFONT)


###########################################
# Add source tree
###########################################

if (BUILD_UNIT_TESTS)
    enable_testing()
    message(STATUS "Enabled testing")

    define_property(TARGET PROPERTY OUTPUT_XML
        BRIEF_DOCS "List XML files outputed by google test."
        FULL_DOCS "List XML files outputed by google test."
    )

    add_subdirectory(thirdparty/googletest)
endif(BUILD_UNIT_TESTS)

add_subdirectory(share)
add_subdirectory(src/framework/global) # should be first to work pch
add_subdirectory(thirdparty/qzip)
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


###########################################
# Custom target for translation generation
###########################################

# lupdate is used to generate the translation text files based off of the source code
add_custom_target(lupdate
   COMMAND ${PROJECT_SOURCE_DIR}/build/gen-qt-projectfile ${PROJECT_SOURCE_DIR} > mscore.pro
   COMMAND Qt5::lupdate ${PROJECT_BINARY_DIR}/mscore.pro
   COMMAND ${PROJECT_SOURCE_DIR}/build/gen-instruments-projectfile ${PROJECT_SOURCE_DIR}/share/instruments > instruments.pro
   COMMAND Qt5::lupdate ${PROJECT_BINARY_DIR}/instruments.pro
   COMMAND ${PROJECT_SOURCE_DIR}/build/gen-tours-projectfile ${PROJECT_SOURCE_DIR}/share/tours > tours.pro
   COMMAND Qt5::lupdate ${PROJECT_BINARY_DIR}/tours.pro
   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
   )

# lrelease is used to compile the translated text files generated by lupdate into binary resources
file(GLOB INSTRUMENTS_TS_FILES
   "share/locale/instruments*.ts"
)
file(GLOB MSCORE_TS_FILES
   "share/locale/mscore*.ts"
)
file(GLOB TOURS_TS_FILES
   "share/locale/tours*.ts"
)
file(GLOB QT_TS_FILES
   "share/locale/qt*.ts"
)

add_custom_target(lrelease
   COMMAND Qt5::lrelease ${INSTRUMENTS_TS_FILES}
   COMMAND Qt5::lrelease ${MSCORE_TS_FILES}
   COMMAND Qt5::lrelease ${TOURS_TS_FILES}
   COMMAND Qt5::lrelease ${QT_TS_FILES}
   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
)



