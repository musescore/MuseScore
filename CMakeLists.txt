#=============================================================================
#  MuseScore
#  Linux Music Score Editor
#
#  Copyright (C) 2002-2016 by Werner Schweer and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

cmake_minimum_required(VERSION 3.16)

project(mscore LANGUAGES C CXX)

# Conflicted with PCH on Linux, will be resolve later
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_LIST_DIR}/build
    ${CMAKE_CURRENT_LIST_DIR}/build/cmake
    ${CMAKE_MODULE_PATH}
    )

# Setup version number and general build settings
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

option(SOUNDFONT3    "Ogg Vorbis compressed fonts"        ON)  # Enable Ogg Vorbis compressed fonts, requires Ogg & Vorbis
option(DOWNLOAD_SOUNDFONT "Download the latest soundfont version as part of the build process" ON)

option(BUILD_UNIT_TESTS "Build gtest unit test" OFF)
option(PACKAGE_FILE_ASSOCIATION "File types association" OFF)

option(BUILD_PCH "Build using precompiled headers." OFF)
option(BUILD_SHARED_LIB_IN_DEBUG "Build shared libs if possible in debug" ON)
option(QML_LOAD_FROM_SOURCE "Load qml files from source (not resource)" OFF)

option(USE_SYSTEM_FREETYPE "Use system FreeType"          OFF) # requires freetype >= 2.5.2, does not work on win
option(BUILD_WEBENGINE "Built in webengine support" ON)
set(SCRIPT_INTERFACE  TRUE)

set(VST3_SDK_VERSION "3.7")
option(BUILD_VST "Build VST MODULE" OFF)
set(VST3_SDK_PATH "" CACHE PATH "Path to VST3_SDK. SDK version >= ${VST3_SDK_VERSION} required")

set(JACK_LONGNAME "JACK (Jack Audio Connection Kit)")
set(JACK_MIN_VERSION "0.98.0")
option(BUILD_JACK    "Build with support for ${JACK_LONGNAME} audio backend. JACK >= ${JACK_MIN_VERSION} will be needed." OFF)

#include(SetupCMakePolicy)
include(UsePkgConfig1) # Defines MACRO(PKGCONFIG1 _package _minVersion _include_DIR _link_DIR _link_FLAGS _cflags)

include(SetupBuildEnvironment)
include(GetPlatformInfo)
if (PLATFORM_IS_LINUX)
    include(TryUseCcache)
endif(PLATFORM_IS_LINUX)

include(${CMAKE_CURRENT_LIST_DIR}/config.cmake)

# We need this early, before FindQt5
if (BUILD_WEBENGINE)
   if (COMPILER_IS_MINGW)
      set(USE_WEBENGINE 0)
   else (COMPILER_IS_MINGW)
      set(USE_WEBENGINE 1)
   endif(COMPILER_IS_MINGW)
else (BUILD_WEBENGINE)
   set(USE_WEBENGINE 0)
endif (BUILD_WEBENGINE)

include_directories(${CMAKE_CURRENT_LIST_DIR}/build/qtfix)
set(QT_MIN_VERSION    "5.15.0")
include(FindQt5)
include_directories(${CMAKE_CURRENT_LIST_DIR}/build/qtfix)

include(SetupDependencies)

if (BUILD_JACK)
    include(FindJack)
endif(BUILD_JACK)


if (BUILD_PORTABLEAPPS)
    set(WIN_PORTABLE 1)
endif (BUILD_PORTABLEAPPS)

if (BUILD_FOR_WINSTORE)
    set(FOR_WINSTORE 1)
endif(BUILD_FOR_WINSTORE)

#
#  Sparkle/WinSparkle routine
#
if (BUILD_AUTOUPDATE)

    if (APPLE)
        set(MAC_APPCAST_URL "")
    endif (APPLE)

    include(SetupSparkle)
endif(BUILD_AUTOUPDATE)


# Download MuseScore SoundFont
if (DOWNLOAD_SOUNDFONT)
    include(DownloadSoundFont)
endif(DOWNLOAD_SOUNDFONT)

include(SetupFreetype)

if(SOUNDFONT3)
    include(SetupVorbisAndOgg)
endif(SOUNDFONT3)

##
## produce config.h file
##
configure_file (
      ${PROJECT_SOURCE_DIR}/build/config.h.in
      ${PROJECT_BINARY_DIR}/config.h
      )


#
#  Create precompiled header file
#
# Temporary OFF
set(BUILD_PCH OFF)
if (BUILD_PCH)

    include(CreatePrecompiledHeader)

    # all.h is expected in PROJECT_BINARY_DIR by subdirs, except for MSVC
    if (NOT MSVC)
       execute_process(
          COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/all.h ${PROJECT_BINARY_DIR}/all.h
          WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
          )
    endif (NOT MSVC)

    precompiled_header(QT_INCLUDES all ${BUILD_PCH})

    # MSVC does not require these targets, as all.h is not copied and the
    #   PCH generation is done per-project
    if (NOT MSVC)
       ADD_CUSTOM_TARGET(mops1 DEPENDS ${PROJECT_BINARY_DIR}/all.h)
       ADD_CUSTOM_TARGET(mops2 DEPENDS ${PCH})
    endif (NOT MSVC)

endif(BUILD_PCH)


##
##  Custom target for translation generation
##

## lupdate is used to generate the translation text files based off of the source code

add_custom_target(lupdate
   COMMAND ${PROJECT_SOURCE_DIR}/build/gen-qt-projectfile ${PROJECT_SOURCE_DIR} > mscore.pro
   COMMAND Qt5::lupdate ${PROJECT_BINARY_DIR}/mscore.pro
   COMMAND ${PROJECT_SOURCE_DIR}/build/gen-instruments-projectfile ${PROJECT_SOURCE_DIR}/share/instruments > instruments.pro
   COMMAND Qt5::lupdate ${PROJECT_BINARY_DIR}/instruments.pro
   COMMAND ${PROJECT_SOURCE_DIR}/build/gen-tours-projectfile ${PROJECT_SOURCE_DIR}/share/tours > tours.pro
   COMMAND Qt5::lupdate ${PROJECT_BINARY_DIR}/tours.pro
   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
   )

## lrelease is used to compile the translated text files generated by lupdate into binary resources

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

add_subdirectory(share)
add_subdirectory(thirdparty/qzip)
add_subdirectory(src)

if (BUILD_UNIT_TESTS)
    define_property(TARGET PROPERTY OUTPUT_XML
        BRIEF_DOCS "List XML files outputed by google test."
        FULL_DOCS "List XML files outputed by google test."
    )

    add_subdirectory(thirdparty/googletest)
    enable_testing()

endif(BUILD_UNIT_TESTS)


# ====== Packaging ===============

#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
#set(CMAKE_BUILD_WITH_INSTALL_RPATH ON) # Call CMake with option -DCMAKE_SKIP_RPATH to not set RPATH (Debian packaging requirement)
#set(CMAKE_SKIP_RULE_DEPENDENCY TRUE)

##
##  Include packaging
##
include(SetupAppImagePackaging)
include(Packaging)

#set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};${CMAKE_INSTALL_PREFIX}/bin")
##set(CMAKE_INSTALL_RPATH "$ORIGIN")

#message(STATUS "==========CMAKE_INSTALL_RPATH: ${CMAKE_INSTALL_RPATH}")

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_SKIP_RPATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/bin")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

## TEMP: Display all variables!
### message(STATUS "===========================================================")
### message(STATUS "VARIABLES:")
### message(STATUS "")
### get_cmake_property(_variableNames VARIABLES)
### list (SORT _variableNames)
### foreach (_variableName ${_variableNames})
###     message(STATUS "${_variableName}=${${_variableName}}")
### endforeach()
### message(STATUS "===========================================================")
