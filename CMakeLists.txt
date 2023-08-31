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

# Minimum tested, report if it works with older
CMAKE_MINIMUM_REQUIRED(VERSION 3.5.0)

project(mscore)

include(build/functions.cmake) # library of CMake functions ("fn__" namespace)

# Print Qt version or fail the build if Qt (qmake) is not in PATH.
fn__require_program(QMAKE Qt --version "https://musescore.org/en/handbook/developers-handbook/compilation" qmake)

# Libraries linked via full path no longer produce linker search paths.
cmake_policy(SET CMP0003 NEW)

# Apparently needed on Mac only (?)
if (APPLE)
    # Issue no warning non-existent target argument to get_target_property()
    # and set the result variable to a -NOTFOUND value rather than issuing a FATAL_ERROR 40
    if(POLICY CMP0045)
        cmake_policy(SET CMP0045 OLD)
    endif(POLICY CMP0045)

    # Silently ignore non-existent dependencies (mops1, mops2)
    if(POLICY CMP0046)
        cmake_policy(SET CMP0046 OLD)
    endif(POLICY CMP0046)
endif (APPLE)

# RPATH settings on macOS do not affect install_name
if(POLICY CMP0068)
      cmake_policy(SET CMP0068 NEW)
endif(POLICY CMP0068)

# Don't process generated source files with AUTOMOC
if(POLICY CMP0071)
      cmake_policy(SET CMP0071 OLD)
endif(POLICY CMP0071)

# Define MINGW for VS, as it appears not to be defined
if (MSVC)
   set (MINGW false)
endif (MSVC)

# We need this early, before FindQt5
option(BUILD_WEBENGINE "Built in webengine support" ON)

if (BUILD_WEBENGINE)
   if (MINGW)
      SET (USE_WEBENGINE 0)
   else (MINGW)
      SET (USE_WEBENGINE 1)
   endif(MINGW)
else (BUILD_WEBENGINE)
   SET (USE_WEBENGINE 0)
endif (BUILD_WEBENGINE)

set(SCRIPT_INTERFACE  TRUE)
#  Look for Qt5
if (SCRIPT_INTERFACE)
SET(QT_MIN_VERSION    "5.8.0")
else (SCRIPT_INTERFACE)
SET(QT_MIN_VERSION    "5.7.0")
endif (SCRIPT_INTERFACE)
# Include modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/build" ${CMAKE_MODULE_PATH})
include (FindQt5)
include (UsePkgConfig1) # Defines MACRO(PKGCONFIG1 _package _minVersion _include_DIR _link_DIR _link_FLAGS _cflags)
include (FindPulseAudio)
include (GetCompilerVersion)
include (CreatePrecompiledHeader)

# For debugging the make system uncomment next line:
# set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_AUTOMOC     TRUE)
set(USE_SSE           TRUE)

set(MUSESCORE_REVISION "" CACHE STRING "Build revision")

# Set revision for local builds
# Before need run 'make revision' or 'msvc_build.bat revision'
if (MUSESCORE_REVISION STREQUAL "")
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/local_build_revision.env")
        file(READ "${CMAKE_CURRENT_LIST_DIR}/local_build_revision.env" MUSESCORE_REVISION)
        STRING(REGEX REPLACE "\n" "" MUSESCORE_REVISION "${MUSESCORE_REVISION}")
    endif()
endif(MUSESCORE_REVISION STREQUAL "")
message(STATUS "MUSESCORE_REVISION: ${MUSESCORE_REVISION}")

add_definitions(-DMUSESCORE_REVISION="${MUSESCORE_REVISION}")

# Setup version number and general build settings
SET(MUSESCORE_BUILD_CONFIG "dev" CACHE STRING "Build config")
# Possible MUSESCORE_BUILD_CONFIG values:
# - dev     - for development/nightly builds
# - testing - for testing versions (alpha, beta, RC)
# - release - for stable release builds
include("${CMAKE_CURRENT_LIST_DIR}/config.cmake")

if (MUSESCORE_LABEL)
  SET (MUSESCORE_NAME_VERSION "${MUSESCORE_NAME_VERSION} ${MUSESCORE_LABEL}")
endif (MUSESCORE_LABEL)

if (MSCORE_UNSTABLE)
  SET (MUSESCORE_NAME_VERSION "${MUSESCORE_NAME_VERSION} (${MUSESCORE_VERSION_FULL} unstable)")
endif (MSCORE_UNSTABLE)

# Disable components not supported on Windows
if (MSVC OR MINGW)
      set(WIN_NOT_AVAIL "Not available on Windows")
      option(BUILD_PULSEAUDIO ${WIN_NOT_AVAIL} OFF)
      option(BUILD_ALSA ${WIN_NOT_AVAIL} OFF)
endif (MSVC OR MINGW)

# Disable components not supported on Mac
if (APPLE)
      set(MAC_NOT_AVAIL "Not available on Mac")
      option(BUILD_PULSEAUDIO ${MAC_NOT_AVAIL} OFF)
      option(BUILD_ALSA ${MAC_NOT_AVAIL} OFF)
endif (APPLE)

# Disable components not supported on Linux/BSD
if (NOT APPLE AND NOT MINGW AND NOT MSVC)
      set(NIX_NOT_AVAIL "Not available on Linux/BSD")
      #option(BUILD_PORTMIDI "PortMidi disabled on Linux. (It uses ALSA but it's better to use ALSA directly)" OFF)
endif (NOT APPLE AND NOT MINGW AND NOT MSVC)

option(AEOLUS        "Enable pipe organ synthesizer"      OFF)
option(ZERBERUS      "Enable experimental SFZ sampler"    ON)
option(USE_ZITA_REVERB "Enable Zita Reverb audio effect"    ON)
option(OSC           "Enable OSC remote control protocol" ON)
option(AVSOMR        "Enable AVSOMR import"               OFF) # AVSOMR - optical music recognition by audirevis
option(OMR           "Enable PDF import"                  OFF) # OMR - optical music recognition
# For installation see: http://ubuntuforums.org/showthread.php?t=1647350
option(OCR           "Enable OCR, requires OMR"           OFF) # Requires tesseract 3.0, needs work on mac/win
option(SOUNDFONT3    "Ogg Vorbis compressed fonts"        ON)  # Enable Ogg Vorbis compressed fonts, requires Ogg & Vorbis
option(HAS_AUDIOFILE "Enable audio export"                ON)  # Requires libsndfile
option(USE_SYSTEM_QTSINGLEAPPLICATION "Use system QtSingleApplication" OFF)
option(USE_SYSTEM_FREETYPE "Use system FreeType"          OFF) # requires freetype >= 2.5.2, does not work on win
option(USE_SYSTEM_POPPLER  "Use system Poppler for OMR"   OFF)
option(BUILD_LAME    "Enable MP3 export"                  ON)  # Requires libmp3lame, call CMake with -DBUILD_LAME="OFF" to disable
option(DOWNLOAD_SOUNDFONT "Download the latest soundfont version as part of the build process" ON)

# licence incompatibility, must never be distributed
set(OCR OFF)

SET(JACK_LONGNAME "JACK (Jack Audio Connection Kit)")
SET(JACK_MIN_VERSION "0.98.0")
option(BUILD_JACK    "Build with support for ${JACK_LONGNAME} audio backend. JACK >= ${JACK_MIN_VERSION} will be needed." ON)
option(BUILD_PULSEAUDIO "Build with support for PulseAudio audio backend." ON)
option(BUILD_ALSA "Build with support for ALSA audio backend." ON)
option(BUILD_PORTAUDIO "Build with support for PortAudio audio backend." ON)
option(BUILD_PORTMIDI "Build with support for PortAudio's MIDI features." ${BUILD_PORTAUDIO}) # PortAudio required
option(BUILD_PCH "Build using precompiled headers." ON)
option(BUILD_FOR_WINSTORE "Build for the Windows Store." OFF)
option(COVERAGE "Build with instrumentation to record code coverage." OFF)
option(BUILD_64 "Build 64 bit version of editor" ON)
option(BUILD_AUTOUPDATE "Build with autoupdate support" OFF)
option(BUILD_CRASH_REPORTER "Build with crash reporter" OFF)
option(BUILD_PORTABLEAPPS "Windows build for PortableApps.com" OFF)
set(CRASH_REPORT_URL "http://127.0.0.1:1127/post" CACHE STRING "URL where to send crash reports (valid if BUILD_CRASH_REPORTER is set to ON)")
option(BUILD_TELEMETRY_MODULE "Build with telemetry module" ON)
set(TELEMETRY_TRACK_ID "" CACHE STRING "Telemetry track id")
option(PACKAGE_FILE_ASSOCIATION "File types association" OFF)

if (MSCORE_UNSTABLE OR TELEMETRY_TRACK_ID STREQUAL "")
      message("Telemetry feature is disabled")
      message("Build is unstable = ${MSCORE_UNSTABLE}")

      if (TELEMETRY_TRACK_ID STREQUAL "")
            message("Telemetry track id is empty")
      else(TELEMETRY_TRACK_ID STREQUAL "")
            message("Telemetry track id isn't empty")
      endif(TELEMETRY_TRACK_ID STREQUAL "")

      add_definitions(-DTELEMETRY_DISABLED)
endif(MSCORE_UNSTABLE OR TELEMETRY_TRACK_ID STREQUAL "")

if (BUILD_CRASH_REPORTER)
      message("Crash reporter enabled")
else(BUILD_CRASH_REPORTER)
      message("Crash reporter disabled")
endif(BUILD_CRASH_REPORTER)

if (APPLE)
      set (CMAKE_CXX_COMPILER   clang++)
      set (CMAKE_CXX_COMPILER_ID "Clang")
      set(CMAKE_OSX_ARCHITECTURES x86_64)

      set(MACOSX_DEPLOYMENT_TARGET 10.10)
      set(CMAKE_OSX_DEPLOYMENT_TARGET 10.10)

      set(HAS_AUDIOFILE TRUE)            # Requires libsndfile
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

if (BUILD_PORTABLEAPPS STREQUAL "ON")
      SET (WIN_PORTABLE 1)
endif (BUILD_PORTABLEAPPS STREQUAL "ON")

#
#  Sparkle/WinSparkle routine
#
if (BUILD_AUTOUPDATE STREQUAL "ON")
      if (APPLE)
             find_package(Sparkle) #needed for SPARKLE_FOUND variable
             if(SPARKLE_FOUND)
                   set(MAC_SPARKLE_ENABLED 1)
                   set(OsxFrameworks ${OsxFrameworks} ${SPARKLE_LIBRARY})
                   message("Current OSX version is ${CURRENT_OSX_VERSION}")
                   set(MAC_APPCAST_URL "https://sparkle.musescore.org/${MSCORE_RELEASE_CHANNEL}/3/macos/appcast.xml")
             endif(SPARKLE_FOUND)
      elseif (MSVC)
             if ((NOT MSCORE_UNSTABLE) AND (NOT DEFINED WIN_PORTABLE)) # do not include WinSparkle in unstable and portable builds
                   include(FindWinSparkle)
                   add_library(winsparkledll SHARED IMPORTED)
                   set_target_properties(winsparkledll PROPERTIES IMPORTED_IMPLIB ${WINSPARKLE_LIBRARY})
                   set(WIN_SPARKLE_ENABLED 1)
                   set(WIN_SPARKLE_APPCAST_URL "https://sparkle.musescore.org/${MSCORE_RELEASE_CHANNEL}/3/win/appcast${ARCH_TYPE}.xml")
                   message("Win Sparkle Url: " ${WIN_SPARKLE_APPCAST_URL})
             endif ((NOT MSCORE_UNSTABLE) AND (NOT DEFINED WIN_PORTABLE))
      else (APPLE)
             message("Sparkle is not supported on your system.")
      endif (APPLE)
endif(BUILD_AUTOUPDATE STREQUAL "ON")

#
#     Check for gcc compiler >= 4.8
#
if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
      if (${CMAKE_CXX_COMPILER_MAJOR} LESS 4
        OR ((${CMAKE_CXX_COMPILER_MAJOR} EQUAL 4) AND (${CMAKE_CXX_COMPILER_MINOR} LESS 8)))
            message(FATAL_ERROR "bad gcc compiler version " ${CMAKE_CXX_COMPILER_VERSION}
                  " >= 4.8 required")
      endif()
endif (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")

if (NOT MSVC)
   set(CMAKE_CXX_FLAGS_DEBUG   "-g")
   set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -DQT_NO_DEBUG")
endif (NOT MSVC)

if (APPLE)
   if (XCODE)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -stdlib=libc++ -Wno-inconsistent-missing-override -Wno-deprecated-register")
   else (XCODE)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -fPIC -stdlib=libc++ -Wno-inconsistent-missing-override -Wno-deprecated-register")
   endif (XCODE)
   # This is necessary for genManual to be executed during the build phase,
   # it needs to be able to get the Qt libs.
   # TODO: is it still needed? genManual is removed.
   SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
   SET(CMAKE_INSTALL_RPATH "${QT_INSTALL_LIBS}")
else (APPLE)
   if (MSVC)
      if (BUILD_64 STREQUAL "ON")
         set(USE_SSE FALSE)
      endif(BUILD_64 STREQUAL "ON")
   # Set compiler options for VS2017/19 toolchain.
   # Note: /D_CRT_SECURE_NO WARNINGS disables warnings when using "non-secure" library functions like sscanf...
      set(CMAKE_CXX_FLAGS                "/MP /DWIN32 /D_WINDOWS /GR /EHsc /D_UNICODE /DUNICODE /D_CRT_SECURE_NO_WARNINGS /execution-charset:utf-8 /source-charset:utf-8")
      set(CMAKE_C_FLAGS                  "/MP /DWIN32 /D_WINDOWS /D_CRT_SECURE_NO_WARNINGS")
      if (USE_SSE)
         # If USE_SSE is set, enable use of SSE instructions to compiler.
         set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /arch:SSE")
         set(CMAKE_C_FLAGS    "${CMAKE_C_FLAGS} /arch:SSE")
      endif (USE_SSE)
      set(CMAKE_CXX_FLAGS_DEBUG          "/MT /permissive- /std:c++14 /W4 /Zi /Ob0 /Od /RTC1 /wd4996")
      set(CMAKE_CXX_FLAGS_RELEASE        "/MT /permissive- /std:c++14 /W4 /O2 /Ob2 /DNDEBUG /DQT_NO_DEBUG /DQT_NO_DEBUG_OUTPUT")
      set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MT /permissive- /std:c++14 /W4 /Zi /O2 /Ob1 /DNDEBUG /DQT_NO_DEBUG /DQT_NO_DEBUG_OUTPUT /wd4996")
      set(CMAKE_C_FLAGS_DEBUG            "/MT /W4 /Zi /Ob0 /Od /RTC1")
      set(CMAKE_C_FLAGS_RELEASE          "/MT /W4 /O2 /Ob2 /DNDEBUG")
      set(CMAKE_C_FLAGS_RELWITHDEBINFO   "/MT /W4 /Zi /O2 /Ob1 /DNDEBUG")
      set(CMAKE_EXE_LINKER_FLAGS         "/DYNAMICBASE:NO")
   else (MSVC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -D_UNICODE -DUNICODE")
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
      set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -Wno-deprecated-declarations -Wno-cast-function-type") # don't want to see those in 3.x when using Qt 5.15
   endif (MSVC)
endif(APPLE)

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON) # Call CMake with option -DCMAKE_SKIP_RPATH to not set RPATH (Debian packaging requirement)
set(CMAKE_SKIP_RULE_DEPENDENCY TRUE)

if (MINGW OR MSVC OR APPLE)
  if(MINGW OR MSVC)
      # Option for MINGW and MSVC
      SET(Mscore_INSTALL_NAME  "")
      SET(Mscore_SHARE_NAME    "./")
  else(MINGW OR MSVC)
       # Option for Apple
       SET(Mscore_INSTALL_NAME  "Contents/Resources/")
       SET(Mscore_SHARE_NAME    "mscore.app/")
  endif(MINGW OR MSVC)
else (MINGW OR MSVC OR APPLE)
      SET(Mscore_INSTALL_NAME  "mscore${MSCORE_INSTALL_SUFFIX}-${MUSESCORE_VERSION}/")
      SET(Mscore_SHARE_NAME    "share/")
endif (MINGW OR MSVC OR APPLE)

# Download MuseScore SoundFont
if (DOWNLOAD_SOUNDFONT)
  set (SF_VERSION_REMOTE_FILE ${PROJECT_BINARY_DIR}/SF_VERSION)
  set (SF_VERSION_LOCAL_FILE ${PROJECT_SOURCE_DIR}/share/sound/VERSION)
  set (DOWNLOAD_URL_ROOT "https://ftp.osuosl.org/pub/musescore/soundfont/MuseScore_General")
  set (SOUND_DIRECTORY ${PROJECT_SOURCE_DIR}/share/sound)

  file(DOWNLOAD ${DOWNLOAD_URL_ROOT}/VERSION ${SF_VERSION_REMOTE_FILE} STATUS SF_VERSION_DL_STATUS_LIST)
  list(GET SF_VERSION_DL_STATUS_LIST 0 SF_VERSION_DL_STATUS)

  if(EXISTS ${SF_VERSION_REMOTE_FILE} AND SF_VERSION_DL_STATUS EQUAL 0)
    file (STRINGS ${SF_VERSION_REMOTE_FILE} SF_VERSION_REMOTE)

    set(SF_VERSION_LOCAL "0.0")
    if (EXISTS ${SF_VERSION_LOCAL_FILE})
      file (STRINGS ${SF_VERSION_LOCAL_FILE} SF_VERSION_LOCAL)
    endif ()

    string(COMPARE LESS ${SF_VERSION_LOCAL} ${SF_VERSION_REMOTE} DO_DOWNLOAD)
    if (DO_DOWNLOAD)
      message("Version ${SF_VERSION_LOCAL} of the MuseScore SoundFont is outdated, downloading version ${SF_VERSION_REMOTE}.")
      # delete soundfont and download new version
      ## TODO check STATUS of downloads
      file (REMOVE ${SOUND_DIRECTORY}/MuseScore_General.sf3
                   ${SOUND_DIRECTORY}/MuseScore_General_License.md
                   ${SOUND_DIRECTORY}/MuseScore_General_Changelog.md
                   ${SOUND_DIRECTORY}/MuseScore_General_Readme.md)
      file(DOWNLOAD ${DOWNLOAD_URL_ROOT}/MuseScore_General.sf3 ${SOUND_DIRECTORY}/MuseScore_General.sf3 SHOW_PROGRESS STATUS SF_DL_STATUS)
      list(GET SF_DL_STATUS 0 HTTP_STATUS)
      if (NOT HTTP_STATUS EQUAL 0)
        list(GET SF_DL_STATUS 1 HTTP_STATUS_TEXT)
        message(FATAL_ERROR "Cannot download new soundfont. Error: ${HTTP_STATUS_TEXT}")
      endif()
      file(DOWNLOAD ${DOWNLOAD_URL_ROOT}/MuseScore_General_License.md ${SOUND_DIRECTORY}/MuseScore_General_License.md)
      file(DOWNLOAD ${DOWNLOAD_URL_ROOT}/MuseScore_General_Changelog.md ${SOUND_DIRECTORY}/MuseScore_General_Changelog.txt)
      file(DOWNLOAD ${DOWNLOAD_URL_ROOT}/MuseScore_General_Readme.md ${SOUND_DIRECTORY}/MuseScore_General_Readme.md)
      # replace VERSION file (in any case, delete VERSION file)
      file (COPY ${SF_VERSION_REMOTE_FILE} DESTINATION ${SOUND_DIRECTORY})
      file (REMOVE ${SF_VERSION_REMOTE_FILE})
    else(DO_DOWNLOAD)
      message("MuseScore SoundFont is up to date.")
    endif(DO_DOWNLOAD)
  endif()
endif(DOWNLOAD_SOUNDFONT)


if (BUILD_FOR_WINSTORE)
  set(FOR_WINSTORE 1)
endif(BUILD_FOR_WINSTORE)

if (USE_ZITA_REVERB)
    add_definitions(-DZITA_REVERB)
endif(USE_ZITA_REVERB)

##
## freetype2 >= 2.5.2
##

if (USE_SYSTEM_FREETYPE)
      if (APPLE)
            PKGCONFIG1 (freetype2 2.5.2 FREETYPE_INCLUDE_DIRS FREETYPE_LIBDIR FREETYPE_LIBRARIES FREETYPE_CPP)
            if (FREETYPE_INCLUDE_DIRS)
                  STRING(REGEX REPLACE  "\"" "" FREETYPE_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIRS})
                  STRING(REGEX REPLACE  "\"" "" FREETYPE_LIBDIR ${FREETYPE_LIBDIR})
                  message("freetype2 detected ${FREETYPE_INCLUDE_DIRS} ${FREETYPE_LIBDIR} ${FREETYPE_LIBRARIES}")
            else (FREETYPE_INCLUDE_DIRS)
                  message(FATAL_ERROR "freetype >= 2.5.2 is required\n")
            endif (FREETYPE_INCLUDE_DIRS)
      else (APPLE)
            find_package(Freetype REQUIRED)
      endif (APPLE)
endif (USE_SYSTEM_FREETYPE)

##
## ALSA >= 1.0.0
##

if (BUILD_ALSA)
      PKGCONFIG1 (alsa 1.0.0 ALSA_INCDIR ALSA_LIBDIR ALSA_LIB ALSA_CPP )
      if (NOT ALSA_INCDIR)
            message(SEND_ERROR "Error: ALSA support requested (BUILD_ALSA=${BUILD_ALSA}), but ALSA >= 1.0.0 was not found.")
      else (NOT ALSA_INCDIR)
            message("ALSA >= 1.0.0 found. ALSA support enabled. INCDIR ${ALSA_INCDIR}, LIBDIR ${ALSA_LIBDIR}, LIB ${ALSA_LIB}, CPP ${ALSA_CPP}")
            set (USE_ALSA 1)
      endif (NOT ALSA_INCDIR)
else (BUILD_ALSA)
     message(STATUS "ALSA support disabled")
endif (BUILD_ALSA)

##
## MIDI
##

if (APPLE OR MINGW OR MSVC)
      set (HAS_MIDI 1)
else (APPLE OR MINGW OR MSVC)
      if (USE_ALSA)
            set (HAS_MIDI 1)
      endif (USE_ALSA)
endif (APPLE OR MINGW OR MSVC)

##
## PulseAudio
##

if (BUILD_PULSEAUDIO)
      if (PULSEAUDIO_FOUND)
            set(USE_PULSEAUDIO 1)
            message("PulseAudio found. PulseAudio support enabled.")
      else (PULSEAUDIO_FOUND)
            message(SEND_ERROR "Error: PulseAudio support requested (BUILD_PULSEAUDIO=${BUILD_PULSEAUDIO}), but PulseAudio was not found.")
      endif (PULSEAUDIO_FOUND)
else (BUILD_PULSEAUDIO)
      message(STATUS "PulseAudio support disabled")
endif (BUILD_PULSEAUDIO)


##
## LAME
##

if (APPLE OR MINGW)
      if (BUILD_LAME)
            include (FindLame)
            set (USE_LAME 1)
      endif (BUILD_LAME)
else (APPLE OR MINGW)
      if (BUILD_LAME)
            if (MSVC)
               include(FindLameMSVC)
            else (MSVC)
               include (FindLame)
            endif (MSVC)
            if (LAME_FOUND)
                  set(USE_LAME 1)
                  MESSAGE("LAME found.")
            else (LAME_FOUND)
                  set(USE_LAME 0)
                  MESSAGE("LAME not found.")
            endif (LAME_FOUND)
      else (BUILD_LAME)
            MESSAGE(STATUS "LAME MP3 support disabled")
      endif (BUILD_LAME)
endif (APPLE OR MINGW)

##
## Find JACK >= JACK_MIN_VERSION
##


IF(BUILD_JACK)
     IF(MINGW OR MSVC)
           set (USE_JACK 1)
           IF("$ENV{PROCESSOR_ARCHITEW6432}" STREQUAL "")
              IF("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "x86")
                 # "pure" 32-bit environment
                 set (progenv "PROGRAMFILES")
              ELSE("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "x86")
                 # "pure" 64-bit environment
                 set (progenv "PROGRAMFILES(x86)")
              ENDIF("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "x86")
           ELSE("$ENV{PROCESSOR_ARCHITEW6432}" STREQUAL "")
              IF("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "x86")
                 # 32-bit program running with an underlying 64-bit environment
                 set (progenv "PROGRAMFILES(x86)")
              ELSE("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "x86")
                 # Theoretically impossible case...
                 MESSAGE(SEND_ERROR "Error: Impossible program/environment bitness combination deduced: 64-bit program running in 32-bit environment. This is a programming error. PROCESSOR_ARCHITEW6432=$ENV{PROCESSOR_ARCHITEW6432}. PROCESSOR_ARCHITECTURE=$ENV{PROCESSOR_ARCHITECTURE}")
              ENDIF("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "x86")
           ENDIF("$ENV{PROCESSOR_ARCHITEW6432}" STREQUAL "")
           set (JACK_INCDIR "$ENV{${progenv}}/Jack/includes")
           set (JACK_LIB "$ENV{${progenv}}/Jack/lib/libjack.a")
           MESSAGE("JACK support enabled.")
     ELSE(MINGW OR MSVC)
           PKGCONFIG1 (jack ${JACK_MIN_VERSION} JACK_INCDIR JACK_LIBDIR JACK_LIB JACK_CPP)
           IF(JACK_INCDIR)
                 MESSAGE(STATUS "${JACK_LONGNAME} >= ${JACK_MIN_VERSION} found. jack support enabled.")
                 SET(USE_JACK 1)
           ELSE(JACK_INCDIR)
                 MESSAGE(STATUS "${JACK_LONGNAME} >= ${JACK_MIN_VERSION} not found")
                 MESSAGE(SEND_ERROR "Error: JACK support requested (BUILD_JACK=${BUILD_JACK}), but JACK was not found")
           ENDIF(JACK_INCDIR)
     ENDIF(MINGW OR MSVC)
ELSE(BUILD_JACK)
     MESSAGE(STATUS "${JACK_LONGNAME} support disabled")
ENDIF(BUILD_JACK)


##
## PortAudio
##

if (BUILD_PORTAUDIO)
    if (MINGW OR MSVC)
        set ( USE_PORTAUDIO 1 )
        include(FindPortAudio)
        add_library(portaudiodll SHARED IMPORTED)
        set_target_properties(portaudiodll PROPERTIES IMPORTED_IMPLIB ${PORTAUDIO_LIBRARY})
    else (MINGW OR MSVC)
        PKGCONFIG1 (portaudio-2.0 19 PORTAUDIO_INCDIR PORTAUDIO_LIBDIR PORTAUDIO_LIB PORTAUDIO_CPP)
        if (PORTAUDIO_INCDIR)
              message("PortAudio found. PortAudio support enabled. INCDIR ${PORTAUDIO_INCDIR}, LIBDIR ${PORTAUDIO_LIBDIR}, LIB ${PORTAUDIO_LIB}")
              set ( USE_PORTAUDIO 1 )
        else (PORTAUDIO_INCDIR)
              message(SEND_ERROR "Error: PortAudio support requested (BUILD_PORTAUDIO=${BUILD_PORTAUDIO}), but portaudio-2.0 version 19 was not found (package portaudio19-dev)")
        endif (PORTAUDIO_INCDIR)
    endif (MINGW OR MSVC)
else (BUILD_PORTAUDIO)
     message(STATUS "PortAudio support disabled")
endif (BUILD_PORTAUDIO)

##
## PortMidi
##

if (BUILD_PORTMIDI)
    if (NOT BUILD_PORTAUDIO)
        message(SEND_ERROR "Error: PortMidi support requested (BUILD_PORTMIDI=${BUILD_PORTMIDI}), but PortAudio support is disabled (BUILD_PORTAUDIO=${BUILD_PORTAUDIO}). PortMidi requires PortAudio.")
    endif (NOT BUILD_PORTAUDIO)
    set (USE_PORTMIDI 1)
else (BUILD_PORTMIDI)
    message(STATUS "PortMidi support disabled")
endif (BUILD_PORTMIDI)

if (APPLE)
   if(SOUNDFONT3)
      ##
      ## libvorbis
      ##

      PKGCONFIG1 (vorbis 1.3.3 VORBIS_INCDIR VORBIS_LIBDIR VORBIS_LIB VORBIS_CPP)
      if (VORBIS_INCDIR)
          message("libvorbis detected ${VORBIS_INCDIR} ${VORBIS_LIBDIR} ${VORBIS_LIB}")
      else (VORBIS_INCDIR)
          message("libvorbis not found\n")
      endif (VORBIS_INCDIR)


      ##
      ## libogg
      ##

      PKGCONFIG1 (ogg 1.3.0 OGG_INCDIR OGG_LIBDIR OGG_LIB OGG_CPP)
      if (OGG_INCDIR)
          message("libogg detected ${OGG_INCDIR} ${OGG_LIBDIR} ${OGG_LIB}")
      else (OGG_INCDIR)
          message("libogg not found\n")
      endif (OGG_INCDIR)
   endif(SOUNDFONT3)

   if(HAS_AUDIOFILE)
      ##
      ## libsndfile
      ##

      PKGCONFIG1 (sndfile 1.0.25 SNDFILE_INCDIR SNDFILE_LIBDIR SNDFILE_LIB SNDFILE_CPP)
      if (SNDFILE_INCDIR)
          message("libsndfile detected ${SNDFILE_INCDIR} ${SNDFILE_LIBDIR} ${SNDFILE_LIB}")
      else (SNDFILE_INCDIR)
         message("libsndfile not found\n")
      endif (SNDFILE_INCDIR)
   endif(HAS_AUDIOFILE)
else(APPLE)
   if (MINGW OR MSVC)
      set(SNDFILE_LIB sndfile-1)
      include(FindVorbis)
      add_library(vorbisdll SHARED IMPORTED)
      set_target_properties(vorbisdll PROPERTIES IMPORTED_IMPLIB ${VORBIS_LIBRARY})

      include(FindSndFile)
      add_library(sndfiledll SHARED IMPORTED)
      set_target_properties(sndfiledll PROPERTIES IMPORTED_IMPLIB ${SNDFILE_LIBRARY})

      include(FindVorbisfile)
      add_library(vorbisfiledll SHARED IMPORTED)
      set_target_properties(vorbisfiledll PROPERTIES IMPORTED_IMPLIB ${VORBISFILE_LIBRARY})

      include(FindOgg)
      add_library(oggdll SHARED IMPORTED)
      set_target_properties(oggdll PROPERTIES IMPORTED_IMPLIB ${OGG_LIBRARY})
   else (MINGW OR MSVC)
      set(SNDFILE_LIB sndfile)
   endif (MINGW OR MSVC)
   set(OGG_LIB ogg)

   set(VORBIS_LIB vorbis)
endif(APPLE)

##
## QtSingleApplication
##

if (USE_SYSTEM_QTSINGLEAPPLICATION)
    if (USE_PATH_WITH_EXPLICIT_QT_VERSION)
      find_path(QTSINGLEAPPLICATION_INCLUDE_DIRS qtsingleapplication.h PATH_SUFFIXES qt5/QtSolutions)
      find_library(QTSINGLEAPPLICATION_LIBRARIES Qt5Solutions_SingleApplication-2.6)
    else (USE_PATH_WITH_EXPLICIT_QT_VERSION)
      find_path(QTSINGLEAPPLICATION_INCLUDE_DIRS qtsingleapplication.h PATH_SUFFIXES QtSolutions)
      find_library(QTSINGLEAPPLICATION_LIBRARIES QtSolutions_SingleApplication-2.6)
    endif (USE_PATH_WITH_EXPLICIT_QT_VERSION)
else(USE_SYSTEM_QTSINGLEAPPLICATION)
      subdirs (thirdparty/singleapp)
      set(QTSINGLEAPPLICATION_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/singleapp/src)
      set(QTSINGLEAPPLICATION_LIBRARIES qtsingleapp)
endif(USE_SYSTEM_QTSINGLEAPPLICATION)

if (NOT MINGW AND NOT MSVC AND NOT APPLE)
    #### PACKAGING for Linux and BSD based systems (more in mscore/CMakeLists.txt) ####
    #
    #     set library search path for runtime linker to load the same
    #     qt libraries as we used at compile time
    #
    SET(CMAKE_INSTALL_RPATH "${_qt5Core_install_prefix}/lib") # Ignored if CMAKE_SKIP_RPATH="TRUE"
    string(TOUPPER "mscore${MSCORE_INSTALL_SUFFIX}" MAN_MSCORE_UPPER) # Command name shown in uppercase in man pages by convention
    if (${MSCORE_INSTALL_SUFFIX} MATCHES "portable") # Note: "-portable-anything" would match
      # Build portable AppImage as per https://github.com/probonopd/AppImageKit
      add_subdirectory(build/Linux+BSD/portable)
      if (NOT DEFINED ARCH)
        execute_process(COMMAND arch OUTPUT_VARIABLE ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)# Get architecture (strip trailing newline)
      endif (NOT DEFINED ARCH)
      get_filename_component(PORTABLE_INSTALL_PATH ${CMAKE_INSTALL_PREFIX} PATH)# Get path (dirname)
      get_filename_component(PORTABLE_INSTALL_NAME ${CMAKE_INSTALL_PREFIX} NAME)# Strip path (basename)
      if (NOT MSCORE_UNSTABLE)
        set(PORTABLE_INSTALL_NAME "${PORTABLE_INSTALL_NAME}-${MUSESCORE_VERSION_FULL}") # Append version info.
      endif (NOT MSCORE_UNSTABLE)
      set(PORTABLE_INSTALL_NAME "${PORTABLE_INSTALL_NAME}-${ARCH}") # Append system architecture.
      set(CMAKE_INSTALL_PREFIX ${PORTABLE_INSTALL_PATH}/${PORTABLE_INSTALL_NAME}.AppDir) # E.g. "MuseScore-X.Y.Z-x86_64.AppDir"
      execute_process(COMMAND echo ${CMAKE_INSTALL_PREFIX} OUTPUT_FILE PREFIX.txt)
      # Prepare portable scripts:
      configure_file(build/Linux+BSD/portable/AppRun.in          AppRun         @ONLY)
      configure_file(build/Linux+BSD/portable/portable-utils.in  portable-utils @ONLY)
      install(PROGRAMS ${PROJECT_BINARY_DIR}/AppRun     DESTINATION .   COMPONENT portable)
      install(PROGRAMS ${PROJECT_BINARY_DIR}/portable-utils
                       build/Linux+BSD/portable/ldd-recursive
                       build/rm-empty-dirs              DESTINATION bin COMPONENT portable)
      install(FILES    build/Linux+BSD/portable/qt.conf DESTINATION bin COMPONENT portable)
    else (${MSCORE_INSTALL_SUFFIX} MATCHES "portable")
      set(MAN_PORTABLE .\\\") # comment out lines in man page that are only relevant to the portable version
    endif (${MSCORE_INSTALL_SUFFIX} MATCHES "portable")
    # Install desktop file (perform variable substitution first)
    configure_file(build/Linux+BSD/mscore.desktop.in mscore${MSCORE_INSTALL_SUFFIX}.desktop)
    install( FILES ${PROJECT_BINARY_DIR}/mscore${MSCORE_INSTALL_SUFFIX}.desktop DESTINATION share/applications)
    # Install appdata file (perform variable substitution first)
    configure_file(build/Linux+BSD/org.musescore.MuseScore.appdata.xml.in org.musescore.MuseScore${MSCORE_INSTALL_SUFFIX}.appdata.xml)
    install( FILES ${PROJECT_BINARY_DIR}/org.musescore.MuseScore${MSCORE_INSTALL_SUFFIX}.appdata.xml DESTINATION share/metainfo)
    # Substitute variables within man pages
    set(MAN_NAME mscore)
    set(MAN_ALIAS musescore)
    set(MAN_EXTENSION .1)
    set(MAN_FULL_NAME ${MAN_NAME}${MSCORE_INSTALL_SUFFIX}${MAN_EXTENSION})
    set(MAN_FULL_ALIAS ${MAN_ALIAS}${MSCORE_INSTALL_SUFFIX}${MAN_EXTENSION})
    set(MAN_TARGET ${PROJECT_SOURCE_DIR}/build/Linux+BSD/${MAN_NAME}${MAN_EXTENSION}.in)
    set(MAN_BUILD ${PROJECT_BINARY_DIR}/${MAN_FULL_NAME})
    configure_file(${MAN_TARGET} ${MAN_BUILD})
    # Compress man pages if gzip is installed (don't on OpenBSD)
    # Note: Compressing man pages is normal on Linux but not OpenBSD
    find_program( GZIP_EXECUTABLE gzip DOC "A tool for compressing manpages (optional)." )
    if (GZIP_EXECUTABLE AND NOT CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
        message(STATUS "Found 'gzip'. Man pages will be compressed.")
        set(MAN_TARGET ${MAN_BUILD})
        set(MAN_EXTENSION ${MAN_EXTENSION}.gz)
        set(MAN_FULL_NAME ${MAN_NAME}${MSCORE_INSTALL_SUFFIX}${MAN_EXTENSION})
        set(MAN_FULL_ALIAS ${MAN_ALIAS}${MSCORE_INSTALL_SUFFIX}${MAN_EXTENSION})
        set(MAN_BUILD ${PROJECT_BINARY_DIR}/${MAN_FULL_NAME})
        add_custom_command(
            OUTPUT ${MAN_BUILD}
            DEPENDS ${MAN_TARGET}
            COMMAND ${GZIP_EXECUTABLE} -9 < ${MAN_TARGET} > ${MAN_BUILD}
            )
        add_custom_target(manpages ALL
            DEPENDS ${MAN_BUILD}
            COMMAND echo "Man pages have been compressed ready for installation."
            VERBATIM
            )
    else (GZIP_EXECUTABLE AND NOT CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
        if (CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
            message(STATUS "System is OpenBSD: Man pages will not be compressed.")
        else (CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
            message(STATUS "'gzip' not found (it is optional). Man pages will not be compressed.")
        endif (CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
        add_custom_target(manpages ALL
            COMMAND echo "Man pages will be installed uncompressed."
            VERBATIM
            )
    endif (GZIP_EXECUTABLE AND NOT CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
    # Install man pages in either compressed or uncompressed form
    install( FILES ${MAN_BUILD} DESTINATION share/man/man1 COMPONENT doc)
    # Create symlink alias for man pages so `man musescore` = `man mscore`
    find_program( LN_EXECUTABLE ln DOC "A tool for creating symbolic link aliases (optional)." )
    if (LN_EXECUTABLE)
        message(STATUS "Found 'ln'. Symlink aliases will be created for MuseScore executable and the man pages.")
        add_custom_command(
            TARGET manpages
            COMMAND echo "Creating symlink alias for man pages."
            COMMAND ${LN_EXECUTABLE} -sf "${MAN_FULL_NAME}" "${MAN_FULL_ALIAS}"
            COMMAND echo 'Symlink alias: ${MAN_FULL_ALIAS} -> ${MAN_FULL_NAME}'
            )
        install( FILES ${PROJECT_BINARY_DIR}/${MAN_FULL_ALIAS} DESTINATION share/man/man1 COMPONENT doc)
     else (LN_EXECUTABLE)
         message(STATUS "'ln' not found (it is optional). No symlink aliases will be created.")
     endif (LN_EXECUTABLE)
    # Add .MSCZ and .MSCX to MIME database (informs system that filetypes .MSCZ & .MSCX are MuseScore files)
    configure_file(build/Linux+BSD/musescore.xml.in musescore${MSCORE_INSTALL_SUFFIX}.xml)
    install( FILES ${PROJECT_BINARY_DIR}/musescore${MSCORE_INSTALL_SUFFIX}.xml DESTINATION share/mime/packages COMPONENT doc)
    # Note: Must now run "update-mime-database" to apply changes. This is done in the Makefile.
endif (NOT MINGW AND NOT MSVC AND NOT APPLE)

#
#  Create precompiled header file
#

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

##

## Includes
##
include_directories(
   ${PROJECT_SOURCE_DIR}
   ${PROJECT_BINARY_DIR}
   ${ALSA_INCDIR}
   ${JACK_INCDIR}
   ${PORTAUDIO_INCDIR}
   ${OGG_INCDIR}
   ${VORBIS_INCDIR}
   ${SNDFILE_INCDIR}
   ${LAME_INCLUDE_DIR}
)

if (MSVC)
   include_directories(${PROJECT_SOURCE_DIR}/dependencies)
   include_directories(${PROJECT_SOURCE_DIR}/dependencies/include)
   include_directories(${PROJECT_SOURCE_DIR}/dependencies/include/vorbis)
   include_directories(${SNDFILE_INCLUDE_DIR})
   include_directories(${VORBIS_INCLUDE_DIR})
   include_directories(${PORTAUDIO_INCLUDE_DIR})
   include_directories(${OGG_INCLUDE_DIR})
endif (MSVC)

if (USE_SYSTEM_FREETYPE)
      include_directories(${FREETYPE_INCLUDE_DIRS})
else (USE_SYSTEM_FREETYPE)
      include_directories(${PROJECT_SOURCE_DIR}/thirdparty/freetype/include)
endif (USE_SYSTEM_FREETYPE)

if (OMR)
 if (NOT USE_SYSTEM_POPPLER)
   include_directories(${PROJECT_SOURCE_DIR}/thirdparty/poppler)
   include_directories(${PROJECT_SOURCE_DIR}/thirdparty/poppler/poppler)
 else (NOT USE_SYSTEM_POPPLER)
   include_directories(/usr/include/poppler)
   include_directories(/usr/include/poppler/qt5)
 endif (NOT USE_SYSTEM_POPPLER)
endif (OMR)

if (BUILD_TELEMETRY_MODULE)
      add_subdirectory(telemetry)
      add_subdirectory(thirdparty/qt-google-analytics)
endif (BUILD_TELEMETRY_MODULE)

add_subdirectory(global)
add_subdirectory(importexport)
add_subdirectory(mscore)
add_subdirectory(main)

if (BUILD_CRASH_REPORTER)
      set (CRASHREPORTER_EXECUTABLE_NAME "${MSCORE_EXECUTABLE_NAME}-crash-reporter")
      add_subdirectory(crashreporter)
      add_dependencies(mscore mscore-crash-reporter)
      set (CRASHREPORTER_EXECUTABLE "${CRASHREPORTER_EXECUTABLE_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
endif (BUILD_CRASH_REPORTER)

##
## produce config.h file
##
configure_file (
      ${PROJECT_SOURCE_DIR}/build/config.h.in
      ${PROJECT_BINARY_DIR}/config.h
      )

##
## Add subdirs
##
add_subdirectory(effects)
add_subdirectory(audio)
add_subdirectory(audiofile)

subdirs(
      awl bww2mxml share libmscore
      thirdparty/qzip
      )

if (BUILD_CRASH_REPORTER)
      set(ENABLE_CRASH_REPORTER OFF CACHE BOOL "Disable libcrashreporter-qt GUI component")
      add_subdirectory(thirdparty/libcrashreporter-qt)
endif (BUILD_CRASH_REPORTER)

string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE)
if (XCODE AND CMAKE_BUILD_TYPE MATCHES "DEBUG")
# With xcode, we need to have all the targets in the same project
add_subdirectory(mtest)
else(XCODE AND CMAKE_BUILD_TYPE MATCHES "DEBUG")
add_subdirectory(mtest EXCLUDE_FROM_ALL)
endif(XCODE AND CMAKE_BUILD_TYPE MATCHES "DEBUG")

add_subdirectory(rdoc EXCLUDE_FROM_ALL)
add_subdirectory(miditools EXCLUDE_FROM_ALL)
add_subdirectory(fonttools EXCLUDE_FROM_ALL)
add_subdirectory(demos)

if (USE_PORTMIDI AND (MINGW OR APPLE OR MSVC))
      add_subdirectory(thirdparty/portmidi)
endif (USE_PORTMIDI AND (MINGW OR APPLE OR MSVC))

if (AEOLUS)
      add_subdirectory(aeolus)
endif (AEOLUS)

if (AVSOMR)
    add_subdirectory(avsomr)
endif (AVSOMR)

if (OMR)
      add_subdirectory(omr)
      if (NOT USE_SYSTEM_POPPLER)
            add_subdirectory(thirdparty/poppler)
      endif (NOT USE_SYSTEM_POPPLER)
endif (OMR)

if (OSC)
      add_subdirectory(thirdparty/ofqf)
endif (OSC)

if (NOT USE_SYSTEM_FREETYPE)
      add_subdirectory(thirdparty/freetype)
endif (NOT USE_SYSTEM_FREETYPE)

##
##  Include packaging
##

include(Packaging)

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

##
## Miscellaneous Microsoft Visual Studio settings
##
if (MSVC)

   # Force the "install" and "package" targets not to depend on the "all" target.
   set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)
   set(CMAKE_SKIP_PACKAGE_ALL_DEPENDENCY true)

   # Set the startup project to "mscore".
   if (NOT ${CMAKE_VERSION} VERSION_LESS "3.6.0")
      set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT mscore)
   endif ()

   # Set the debugging properties for the "mscore" project.
   file(TO_NATIVE_PATH "${CMAKE_INSTALL_PREFIX}/bin" VS_DEBUGGER_WORKING_DIRECTORY)
   if (NOT ${CMAKE_VERSION} VERSION_LESS "3.12.0")
      set_target_properties(mscore PROPERTIES VS_DEBUGGER_COMMAND "${VS_DEBUGGER_WORKING_DIRECTORY}\\${MSCORE_EXECUTABLE_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
   endif ()
   if (NOT ${CMAKE_VERSION} VERSION_LESS "3.8.0")
      set_target_properties(mscore PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${VS_DEBUGGER_WORKING_DIRECTORY}")
   endif ()
   if (NOT ${CMAKE_VERSION} VERSION_LESS "3.13.0")
      set_target_properties(mscore PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "--debug")
   endif ()

endif (MSVC)

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
