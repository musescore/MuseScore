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

#include(TryUseCcache)
include(SetupCMakePolicy)
include(GetCompilerInfo)
include(GetUtilsFunctions) # library of CMake functions ("fn__" namespace)

# We need this early, before FindQt5
option(BUILD_WEBENGINE "Built in webengine support" ON)
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


include(UsePkgConfig1) # Defines MACRO(PKGCONFIG1 _package _minVersion _include_DIR _link_DIR _link_FLAGS _cflags)

# Setup version number and general build settings
set(MUSESCORE_REVISION "" CACHE STRING "Build revision")
set(MUSESCORE_BUILD_CONFIG "dev" CACHE STRING "Build config")
# Possible MUSESCORE_BUILD_CONFIG values:
# - dev     - for development/nightly builds
# - testing - for testing versions (alpha, beta, RC)
# - release - for stable release builds
include(${CMAKE_CURRENT_LIST_DIR}/config.cmake)


set(SCRIPT_INTERFACE  TRUE)
option(ZERBERUS      "Enable experimental SFZ sampler"    ON)
option(OSC           "Enable OSC remote control protocol" ON)
option(SOUNDFONT3    "Ogg Vorbis compressed fonts"        ON)  # Enable Ogg Vorbis compressed fonts, requires Ogg & Vorbis
option(USE_SYSTEM_QTSINGLEAPPLICATION "Use system QtSingleApplication" OFF)
option(USE_SYSTEM_FREETYPE "Use system FreeType"          OFF) # requires freetype >= 2.5.2, does not work on win
option(DOWNLOAD_SOUNDFONT "Download the latest soundfont version as part of the build process" ON)
option(BUILD_PCH "Build using precompiled headers." OFF)
option(BUILD_FOR_WINSTORE "Build for the Windows Store." OFF)
option(COVERAGE "Build with instrumentation to record code coverage." OFF)
option(BUILD_64 "Build 64 bit version of editor" ON)
option(BUILD_AUTOUPDATE "Build with autoupdate support" OFF)
set(CRASH_REPORT_URL "" CACHE STRING "URL where to send crash reports")
option(BUILD_TELEMETRY_MODULE "Build with telemetry module" ON)
set(TELEMETRY_TRACK_ID "" CACHE STRING "Telemetry track id")
option(BUILD_PORTABLEAPPS "Windows build for PortableApps.com" OFF)
option(BUILD_UNIT_TESTS "Build gtest unit test" OFF)
option(QML_LOAD_FROM_SOURCE "Load qml files from source (not resource)" OFF)
option(PACKAGE_FILE_ASSOCIATION "File types association" OFF)

# Temporary OFF
set(BUILD_PCH OFF)

set(VST3_SDK_VERSION "3.7")
option(BUILD_VST "Build VST MODULE" OFF)
set(VST3_SDK_PATH "" CACHE PATH "Path to VST3_SDK. SDK version >= ${VST3_SDK_VERSION} required")

set(JACK_LONGNAME "JACK (Jack Audio Connection Kit)")
set(JACK_MIN_VERSION "0.98.0")
option(BUILD_JACK    "Build with support for ${JACK_LONGNAME} audio backend. JACK >= ${JACK_MIN_VERSION} will be needed." OFF)
if (BUILD_JACK)
    include(FindJack)
endif(BUILD_JACK)

add_definitions(-DQT_QML_DEBUG)
add_definitions(-D_USE_MATH_DEFINES)
add_definitions(-DNOMINMAX)


#-DOS_WIN
#-DNOMINMAX
#-DUNICODE
#-DWIN32_LEAN_AND_MEAN
#-DWIN32

message("[cmake] CRASH_REPORT_URL: ${CRASH_REPORT_URL}")

message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")

if (MSCORE_UNSTABLE OR TELEMETRY_TRACK_ID STREQUAL "")
      message("Telemetry feature is disabled")
      if (TELEMETRY_TRACK_ID STREQUAL "")
            message("Telemetry track id is empty")
      else(TELEMETRY_TRACK_ID STREQUAL "")
            message("Telemetry track id isn't empty")
      endif(TELEMETRY_TRACK_ID STREQUAL "")

      add_definitions(-DTELEMETRY_DISABLED)
endif(MSCORE_UNSTABLE OR TELEMETRY_TRACK_ID STREQUAL "")

if (APPLE)
      set (CMAKE_CXX_COMPILER   clang++)
      set (CMAKE_CXX_COMPILER_ID "Clang")
      set(CMAKE_OSX_ARCHITECTURES x86_64)

      set(MACOSX_DEPLOYMENT_TARGET 10.10)
      set(CMAKE_OSX_DEPLOYMENT_TARGET 10.10)

      set(MAC_APPCAST_URL "")

      find_library(AudioToolboxFW        NAMES AudioToolbox)
      find_library(AudioUnitFW           NAMES AudioUnit)
      find_library(CoreAudioFW           NAMES CoreAudio)
      find_library(CoreMidiFW            NAMES CoreMIDI)
      find_library(SystemConfigurationFW NAMES SystemConfiguration)
      find_library(CoreServicesFW        NAMES CoreServices)
      find_library(AppKit                NAMES AppKit)
      set(OsxFrameworks ${AudioToolboxFW} ${AudioUnitFW} ${CoreAudioFW} ${CoreMidiFW} ${SystemConfigurationFW} ${CoreServicesFW} ${AppKit})
endif (APPLE)

#
# Set up Windows arch variables/paths
#

if (BUILD_64 STREQUAL "ON")
      SET (ARCH_TYPE "_x64")
      SET (DEPENDENCIES_DIR "${PROJECT_SOURCE_DIR}/dependencies/libx64")
else (BUILD_64 STREQUAL "ON")
      SET (ARCH_TYPE "_x86")
      SET (DEPENDENCIES_DIR "${PROJECT_SOURCE_DIR}/dependencies/libx86")
endif (BUILD_64 STREQUAL "ON")

link_directories(${DEPENDENCIES_DIR})

if (BUILD_PORTABLEAPPS STREQUAL "ON")
      SET (WIN_PORTABLE 1)
endif (BUILD_PORTABLEAPPS STREQUAL "ON")

#
#  Sparkle/WinSparkle routine
#
if (BUILD_AUTOUPDATE)
    include(SetupSparkle)
endif(BUILD_AUTOUPDATE)

if (NOT MSVC)
   set(CMAKE_CXX_FLAGS_DEBUG   "-g")
   set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -DQT_NO_DEBUG")
endif (NOT MSVC)

if (APPLE)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC -stdlib=libc++ -Wno-inconsistent-missing-override -Wno-deprecated-register")
   set(CMAKE_CXX_STANDARD 17)
   # This is necessary for genManual to be executed during the build phase,
   # it needs to be able to get the Qt libs.
   # TODO: is it still needed? genManual is removed.
   SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
   SET(CMAKE_INSTALL_RPATH "${QT_INSTALL_LIBS}")
else (APPLE)
   if (MSVC)
   # Set compiler options for VS2017/19 toolchain.
   # Note: /D_CRT_SECURE_NO WARNINGS disables warnings when using "non-secure" library functions like sscanf...
      set(CMAKE_CXX_FLAGS                "/MP /DWIN32 /D_WINDOWS /GR /EHsc /D_UNICODE /DUNICODE /D_CRT_SECURE_NO_WARNINGS /execution-charset:utf-8 /source-charset:utf-8")
      set(CMAKE_C_FLAGS                  "/MP /DWIN32 /D_WINDOWS /D_CRT_SECURE_NO_WARNINGS")
      set(CMAKE_CXX_FLAGS_DEBUG          "/MT /permissive- /std:c++17 /W4 /Zi /Ob0 /Od /RTC1")
      set(CMAKE_CXX_FLAGS_RELEASE        "/MT /permissive- /std:c++17 /W4 /O2 /Ob2 /DNDEBUG /DQT_NO_DEBUG /DQT_NO_DEBUG_OUTPUT")
      set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MT /permissive- /std:c++17 /W4 /Zi /O2 /Ob1 /DNDEBUG /DQT_NO_DEBUG /DQT_NO_DEBUG_OUTPUT")
      set(CMAKE_C_FLAGS_DEBUG            "/MT /W4 /Zi /Ob0 /Od /RTC1")
      set(CMAKE_C_FLAGS_RELEASE          "/MT /W4 /O2 /Ob2 /DNDEBUG")
      set(CMAKE_C_FLAGS_RELWITHDEBINFO   "/MT /W4 /Zi /O2 /Ob1 /DNDEBUG")
      set(CMAKE_EXE_LINKER_FLAGS         "/DYNAMICBASE:NO")
   else (MSVC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
      if (MINGW)
         # -mno-ms-bitfields see #22048
         set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -mno-ms-bitfields")
         if (NOT BUILD_64)
             set(CMAKE_EXE_LINKER_FLAGS "-Wl,--large-address-aware")
         endif (NOT BUILD_64)
      else (MINGW)
         set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -fPIC")
         set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Woverloaded-virtual")
         set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DQT_NO_DEBUG_OUTPUT")
      endif (MINGW)
   endif (MSVC)
endif(APPLE)

set(CMAKE_BUILD_WITH_INSTALL_RPATH ON) # Call CMake with option -DCMAKE_SKIP_RPATH to not set RPATH (Debian packaging requirement)
set(CMAKE_SKIP_RULE_DEPENDENCY TRUE)

# Download MuseScore SoundFont
if (DOWNLOAD_SOUNDFONT)
    include(DownloadSoundFont)
endif(DOWNLOAD_SOUNDFONT)


if (BUILD_FOR_WINSTORE)
  set(FOR_WINSTORE 1)
endif(BUILD_FOR_WINSTORE)

include(SetupFreetype)

if(SOUNDFONT3)
    include(SetupVorbisAndOgg)
endif(SOUNDFONT3)

##
##  Include packaging
##
include(SetupAppImagePackaging)
include(Packaging)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};${CMAKE_INSTALL_PREFIX}/bin")
#set(CMAKE_INSTALL_RPATH "$ORIGIN")

#
#  Create precompiled header file
#

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

## Includes
##
include_directories(
   ${PROJECT_SOURCE_DIR}
   ${PROJECT_BINARY_DIR}
   ${ALSA_INCDIR}
   ${OGG_INCDIR}
   ${VORBIS_INCDIR}
   ${SNDFILE_INCDIR}
)

if (MSVC)
   include_directories(${PROJECT_SOURCE_DIR}/dependencies)
   include_directories(${PROJECT_SOURCE_DIR}/dependencies/include)
   include_directories(${PROJECT_SOURCE_DIR}/dependencies/include/vorbis)
   include_directories(${SNDFILE_INCLUDE_DIR})
   include_directories(${VORBIS_INCLUDE_DIR})
   include_directories(${OGG_INCLUDE_DIR})
endif (MSVC)

if (MINGW)
    include_directories(${SNDFILE_INCLUDE_DIR})
endif(MINGW)


##
## produce config.h file
##
configure_file (
      ${PROJECT_SOURCE_DIR}/build/config.h.in
      ${PROJECT_BINARY_DIR}/config.h
      )

add_subdirectory(share)
add_subdirectory(thirdparty/qzip)


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

define_property(TARGET PROPERTY OUTPUT_XML
    BRIEF_DOCS "List XML files outputed by google test."
    FULL_DOCS "List XML files outputed by google test."
)

if (BUILD_UNIT_TESTS)
    add_subdirectory(thirdparty/googletest)
    enable_testing()
endif(BUILD_UNIT_TESTS)

add_subdirectory(thirdparty/singleapp)
set(QTSINGLEAPPLICATION_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/singleapp/src)
set(QTSINGLEAPPLICATION_LIBRARIES qtsingleapp)

add_subdirectory(src)

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
