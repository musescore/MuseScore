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

project(mscore)

# Minimum tested, report if it works with older
CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)

set (CI $ENV{CI})
if (CI)
    SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif (CI)

# Libraries linked via full path no longer produce linker search paths.
cmake_policy(SET CMP0003 NEW)

# Don't to link executables to qtmain.lib automatically when they link to the QtCore IMPORTED target
#if(POLICY CMP0020)
#      cmake_policy(SET CMP0020 OLD)
#endif(POLICY CMP0020)

# Issue no warning non-existent target argument to get_targer_property() and set the result variable to a -NOTFOUND value rather than issuing a FATAL_ERROR
if(POLICY CMP0045)
      cmake_policy(SET CMP0045 OLD)
endif(POLICY CMP0045)

# Silently ignore non-existent dependencies (mops1, mops2)
if(POLICY CMP0046)
      cmake_policy(SET CMP0046 OLD)
endif(POLICY CMP0046)

# Honor the legacy behavior for variable references and escape sequences
if(POLICY CMP0053)
      cmake_policy(SET CMP0053 OLD)
endif(POLICY CMP0053)

# RPATH settings on macOS do not affect install_name
if(POLICY CMP0068)
      cmake_policy(SET CMP0068 NEW)
endif(POLICY CMP0068)

# Don't process generated source files with AUTOMOC
if(POLICY CMP0071)
      cmake_policy(SET CMP0071 OLD)
endif(POLICY CMP0071)

#  Look for Qt5
SET(QT_MIN_VERSION    "5.8.0")
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
set(MSCORE_UNSTABLE   TRUE)          # Mark as unstable
set(USE_SSE           TRUE)
set(SCRIPT_INTERFACE  TRUE)

# Disable components not supported on Windows
if (MINGW)
      set(WIN_NOT_AVAIL "Not available on Windows")
      option(BUILD_PULSEAUDIO ${WIN_NOT_AVAIL} OFF)
      option(BUILD_ALSA ${WIN_NOT_AVAIL} OFF)
endif (MINGW)

# Disable components not supported on Mac
if (APPLE)
      set(MAC_NOT_AVAIL "Not available on Mac")
      option(BUILD_PULSEAUDIO ${MAC_NOT_AVAIL} OFF)
      option(BUILD_ALSA ${MAC_NOT_AVAIL} OFF)
endif (APPLE)

# Disable components not supported on Linux/BSD
if (NOT APPLE AND NOT MINGW)
      set(NIX_NOT_AVAIL "Not available on Linux/BSD")
      #option(BUILD_PORTMIDI "PortMidi disabled on Linux. (It uses ALSA but it's better to use ALSA directly)" OFF)
endif (NOT APPLE AND NOT MINGW)

option(AEOLUS        "Enable pipe organ synthesizer"      OFF)
option(ZERBERUS      "Enable experimental SFZ sampler"    ON)
option(OSC           "Enable OSC remote control protocol" ON)
option(OMR           "Enable PDF import"  ON)                 # OMR - optical music recognition
# For installation see: http://ubuntuforums.org/showthread.php?t=1647350
option(OCR           "Enable OCR, requires OMR" OFF)           # Requires tesseract 3.0, needs work on mac/win
option(SOUNDFONT3    "Ogg Vorbis compressed fonts" ON)         # Enable Ogg Vorbis compressed fonts, requires Ogg & Vorbis
option(HAS_AUDIOFILE "Enable audio export" ON)                 # Requires libsndfile
option(USE_SYSTEM_QTSINGLEAPPLICATION "Use system QtSingleApplication" OFF)
option(USE_SYSTEM_FREETYPE "Use system FreeType" OFF)          # requires freetype >= 2.5.2, does not work on win
option(BUILD_LAME    "Enable MP3 export" ON)                   # Requires libmp3lame (non-free), call CMake with -DBUILD_LAME="OFF" to disable

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

if (APPLE)
      set (CMAKE_CXX_COMPILER   clang++)
      set (CMAKE_CXX_COMPILER_ID "Clang")
      # Currently, just 'ppc', 'i386' or 'ppc i386' are useful architectures,
      # because the Flash-Player is not yet available as a 64-bit version.
      # Flash is required for displaying the videos of MuseScore-Connect.
      # See http://qt.gitorious.org/qt/pages/Qt470KnownIssues
      # Since 10.6+ only runs on intel, just set it to i386.
      set(CMAKE_OSX_ARCHITECTURES x86_64)

      # Adjust and uncomment this variable, if you target a different version
      # of MacOSX.
      #set(CMAKE_OSX_SYSROOT /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk)

      set(CMAKE_OSX_DEPLOYMENT_TARGET 10.7)      # Min version required
      set(HAS_AUDIOFILE TRUE)            # Requires libsndfile

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
#     Check for GCC compiler >= 4.7
#
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
      if (${CMAKE_CXX_COMPILER_MAJOR} LESS 4
        OR ((${CMAKE_CXX_COMPILER_MAJOR} EQUAL 4) AND (${CMAKE_CXX_COMPILER_MINOR} LESS 7)))
            message(FATAL_ERROR "Bad GCC compiler version " ${CMAKE_CXX_COMPILER_VERSION}
                  " >= 4.7 required")
      endif()
endif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")

if (APPLE)
   set(CMAKE_CXX_FLAGS_DEBUG   "-std=c++11 -fPIC -stdlib=libc++ -g -Wno-inconsistent-missing-override")
   set(CMAKE_CXX_FLAGS_RELEASE "-std=c++11 -fPIC -stdlib=libc++ -O2 -DNDEBUG -DQT_NO_DEBUG -Wno-inconsistent-missing-override")
   # This is necessary for genManual to be executed during the build phase,
   # it needs to be able to get the Qt libs.
   SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
   SET(CMAKE_INSTALL_RPATH "${QT_INSTALL_PREFIX}/lib")
else (APPLE)
   if (MINGW)
      # -mno-ms-bitfields see #22048
      set(CMAKE_CXX_FLAGS_DEBUG   "-std=gnu++11 -mno-ms-bitfields -g")
      set(CMAKE_CXX_FLAGS_RELEASE "-std=gnu++11 -mno-ms-bitfields -O2 -DNDEBUG -DQT_NO_DEBUG")
      set(CMAKE_EXE_LINKER_FLAGS "-Wl,--large-address-aware")
   else (MINGW)
      set(CMAKE_CXX_FLAGS_DEBUG   "-std=gnu++11 -fPIC -g -Wall -Wextra -Woverloaded-virtual")
      set(CMAKE_CXX_FLAGS_RELEASE "-std=gnu++11 -fPIC -O2 -DNDEBUG -DQT_NO_DEBUG -DQT_NO_DEBUG_OUTPUT")
   endif (MINGW)
endif(APPLE)

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON) # Call CMake with option -DCMAKE_SKIP_RPATH to not set RPATH (Debian packaging requirement)
set(CMAKE_SKIP_RULE_DEPENDENCY TRUE)

# The MuseScore version number.
SET(MUSESCORE_NAME "MuseScore")
SET(MUSESCORE_VERSION_MAJOR  "3")
SET(MUSESCORE_VERSION_MINOR  "0")
SET(MUSESCORE_VERSION_PATCH  "0")
SET(MUSESCORE_VERSION       "${MUSESCORE_VERSION_MAJOR}.${MUSESCORE_VERSION_MINOR}")
SET(MUSESCORE_NAME_VERSION "${MUSESCORE_NAME} ${MUSESCORE_VERSION_MAJOR}")
#
# Version schema x.x.x is hardcoded in source
#
SET(MUSESCORE_VERSION_FULL  "${MUSESCORE_VERSION}.${MUSESCORE_VERSION_PATCH}")

if (MUSESCORE_LABEL)
  SET (MUSESCORE_NAME_VERSION "${MUSESCORE_NAME_VERSION} ${MUSESCORE_LABEL}")
endif (MUSESCORE_LABEL)

if (MSCORE_UNSTABLE)
  SET (MUSESCORE_NAME_VERSION "${MUSESCORE_NAME_VERSION} (${MUSESCORE_VERSION_FULL} unstable)")
endif (MSCORE_UNSTABLE)

if (MINGW OR APPLE)
  if(MINGW)
      SET(Mscore_INSTALL_NAME  "")
      SET(Mscore_SHARE_NAME    "./")
  else(MINGW)
      SET(Mscore_INSTALL_NAME  "Contents/Resources/")
      SET(Mscore_SHARE_NAME    "mscore.app/")
  endif(MINGW)
else (MINGW OR APPLE)
      SET(Mscore_INSTALL_NAME  "mscore${MSCORE_INSTALL_SUFFIX}-${MUSESCORE_VERSION}/")
      SET(Mscore_SHARE_NAME    "share/")
endif (MINGW OR APPLE)

if (BUILD_FOR_WINSTORE)
  set(FOR_WINSTORE 1)
endif(BUILD_FOR_WINSTORE)

##
## freetype2 >= 2.5.2
##

if (USE_SYSTEM_FREETYPE)
      if (APPLE)
            PKGCONFIG (freetype2 2.5.2 FREETYPE_INCLUDE_DIRS FREETYPE_LIBDIR FREETYPE_LIBRARIES FREETYPE_CPP)
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

if (APPLE OR MINGW)
      set (HAS_MIDI 1)
else (APPLE OR MINGW)
      if (USE_ALSA)
            set (HAS_MIDI 1)
      endif (USE_ALSA)
endif (APPLE OR MINGW)

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
      IF (BUILD_LAME)
            include (FindLame)
            set (USE_LAME 1)
      ENDIF (BUILD_LAME)
else (APPLE OR MINGW)
      IF (BUILD_LAME)
            include (FindLame)
            IF (LAME_FOUND)
                  set(USE_LAME 1)
                  MESSAGE("LAME found.")
            ELSE (LAME_FOUND)
                  set(USE_LAME 0)
                  MESSAGE("LAME not found.")
            ENDIF (LAME_FOUND)
      ELSE (BUILD_LAME)
            MESSAGE(STATUS "LAME MP3 support disabled")
      ENDIF (BUILD_LAME)
endif (APPLE OR MINGW)

##
## Find JACK >= JACK_MIN_VERSION
##

IF(BUILD_JACK)
     IF(MINGW)
           set (USE_JACK 1)
           IF("$ENV{PROCESSOR_ARCHITEW6432}" STREQUAL "")
              IF("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "x86")
                 # "pure" 32-bit environment
                 set (JACK_INCDIR "$ENV{PROGRAMFILES}/Jack/includes")
                 set (JACK_LIB "$ENV{PROGRAMFILES}/Jack/lib/libjack.a")
              ELSE("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "x86")
                 # "pure" 64-bit environment
                 set (JACK_INCDIR "$ENV{PROGRAMFILES(x86)}/Jack/includes")
                 set (JACK_LIB "$ENV{PROGRAMFILES(x86)}/Jack/lib/libjack.a")
              ENDIF("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "x86")
           ELSE("$ENV{PROCESSOR_ARCHITEW6432}" STREQUAL "")
              IF("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "x86")
                 # 32-bit program running with an underlying 64-bit environment
                 set (JACK_INCDIR "$ENV{PROGRAMFILES(x86)}/Jack/includes")
                 set (JACK_LIB "$ENV{PROGRAMFILES(x86)}/Jack/lib/libjack.a")
              ELSE("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "x86")
                 # Theoretically impossible case...
                 MESSAGE(SEND_ERROR "Error: Impossible program/environment bitness combination deduced: 64-bit program running in 32-bit environment. This is a programming error. PROCESSOR_ARCHITEW6432=$ENV{PROCESSOR_ARCHITEW6432}. PROCESSOR_ARCHITECTURE=$ENV{PROCESSOR_ARCHITECTURE}")
              ENDIF("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "x86")
           ENDIF("$ENV{PROCESSOR_ARCHITEW6432}" STREQUAL "")
           MESSAGE("JACK support enabled.")
     ELSE(MINGW)
           PKGCONFIG1 (jack ${JACK_MIN_VERSION} JACK_INCDIR JACK_LIBDIR JACK_LIB JACK_CPP)
           IF(JACK_INCDIR)
                 MESSAGE(STATUS "${JACK_LONGNAME} >= ${JACK_MIN_VERSION} found. jack support enabled.")
                 SET(USE_JACK 1)
           ELSE(JACK_INCDIR)
                 MESSAGE(STATUS "${JACK_LONGNAME} >= ${JACK_MIN_VERSION} not found")
                 MESSAGE(SEND_ERROR "Error: JACK support requested (BUILD_JACK=${BUILD_JACK}), but JACK was not found")
           ENDIF(JACK_INCDIR)
     ENDIF(MINGW)
ELSE(BUILD_JACK)
     MESSAGE(STATUS "${JACK_LONGNAME} support disabled")
ENDIF(BUILD_JACK)


##
## PortAudio
##

if (BUILD_PORTAUDIO)
    if (MINGW)
        set ( USE_PORTAUDIO 1 )
    else (MINGW)
        PKGCONFIG1 (portaudio-2.0 19 PORTAUDIO_INCDIR PORTAUDIO_LIBDIR PORTAUDIO_LIB PORTAUDIO_CPP)
        if (PORTAUDIO_INCDIR)
              message("PortAudio found. PortAudio support enabled. INCDIR ${PORTAUDIO_INCDIR}, LIBDIR ${PORTAUDIO_LIBDIR}, LIB ${PORTAUDIO_LIB}")
              set ( USE_PORTAUDIO 1 )
        else (PORTAUDIO_INCDIR)
              message(SEND_ERROR "Error: PortAudio support requested (BUILD_PORTAUDIO=${BUILD_PORTAUDIO}), but portaudio-2.0 version 19 was not found (package portaudio19-dev)")
        endif (PORTAUDIO_INCDIR)
    endif (MINGW)
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
   if(MINGW)
      set(SNDFILE_LIB sndfile-1)
   else(MINGW)
      set(SNDFILE_LIB sndfile)
   endif(MINGW)
   set(OGG_LIB ogg)
   set(VORBIS_LIB vorbis)
endif(APPLE)

##
## QtSingleApplication
##

if (USE_SYSTEM_QTSINGLEAPPLICATION)
      find_path(QTSINGLEAPPLICATION_INCLUDE_DIRS qtsingleapplication.h PATH_SUFFIXES QtSolutions)
      find_library(QTSINGLEAPPLICATION_LIBRARIES QtSolutions_SingleApplication-2.6)
else(USE_SYSTEM_QTSINGLEAPPLICATION)
      subdirs (thirdparty/singleapp)
      set(QTSINGLEAPPLICATION_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/singleapp/src)
      set(QTSINGLEAPPLICATION_LIBRARIES qtsingleapp)
endif(USE_SYSTEM_QTSINGLEAPPLICATION)

##
## produce config.h file
##
configure_file (
      ${PROJECT_SOURCE_DIR}/build/config.h.in
      ${PROJECT_BINARY_DIR}/config.h
      )

if (NOT MINGW AND NOT APPLE)
    #### PACKAGING for Linux and BSD based systems (more in mscore/CMakeLists.txt) ####
    #
    #     set library search path for runtime linker to load the same
    #     qt libraries as we used at compile time
    #
    SET(CMAKE_INSTALL_RPATH "${_qt5Core_install_prefix}/lib") # Ignored if CMAKE_SKIP_RPATH="TRUE"
    string(TOUPPER "mscore${MSCORE_INSTALL_SUFFIX}" MAN_MSCORE_UPPER) # Command name shown in uppercase in man pages by convention
    if (${MSCORE_INSTALL_SUFFIX} MATCHES "portable") # Note: "-portable-anything" would match
      # Build portable AppImage as per https://github.com/probonopd/AppImageKit
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
      set(SCRIPT_PERMS PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ WORLD_READ)
      install(PROGRAMS ${PROJECT_BINARY_DIR}/AppRun     DESTINATION .   COMPONENT portable ${SCRIPT_PERMS})
      install(PROGRAMS ${PROJECT_BINARY_DIR}/portable-utils
                       build/Linux+BSD/portable/ldd-recursive
                       build/rm-empty-dirs              DESTINATION bin COMPONENT portable ${SCRIPT_PERMS})
      install(FILES    build/Linux+BSD/portable/qt.conf DESTINATION bin COMPONENT portable)
    else (${MSCORE_INSTALL_SUFFIX} MATCHES "portable")
      set(MAN_PORTABLE '.\"') # Comment out lines in man page that are only relevent to the portable version
    endif (${MSCORE_INSTALL_SUFFIX} MATCHES "portable")
    # Install desktop file (perform variable substitution first)
    configure_file(build/Linux+BSD/mscore.desktop.in mscore${MSCORE_INSTALL_SUFFIX}.desktop)
    install( FILES ${PROJECT_BINARY_DIR}/mscore${MSCORE_INSTALL_SUFFIX}.desktop DESTINATION share/applications)
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
endif (NOT MINGW AND NOT APPLE)

#
#  Create precompiled header file
#

# all.h is expected in PROJECT_BINARY_DIR by subdirs
add_custom_command(
    OUTPUT ${PROJECT_BINARY_DIR}/all.h
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy ${PROJECT_SOURCE_DIR}/all.h ${PROJECT_BINARY_DIR}/all.h
    DEPENDS ${PROJECT_SOURCE_DIR}/all.h
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    )

precompiled_header(QT_INCLUDES all ${BUILD_PCH})

ADD_CUSTOM_TARGET(mops1 DEPENDS ${PROJECT_BINARY_DIR}/all.h)
ADD_CUSTOM_TARGET(mops2 DEPENDS ${PCH})

##
## Add subdirs
##
subdirs(
      mscore awl bww2mxml share midi audiofile fluid libmscore synthesizer
      effects thirdparty/rtf2html thirdparty/diff thirdparty/beatroot
      thirdparty/qzip thirdparty/kQOAuth
      )
#      thirdparty/xmlstream

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
if (APPLE AND CMAKE_BUILD_TYPE MATCHES "DEBUG")
# With xcode, we need to have all the targets in the same project
add_subdirectory(mtest)
else(APPLE AND CMAKE_BUILD_TYPE MATCHES "DEBUG")
add_subdirectory(mtest EXCLUDE_FROM_ALL)
endif(APPLE AND CMAKE_BUILD_TYPE MATCHES "DEBUG")

add_subdirectory(rdoc EXCLUDE_FROM_ALL)
add_subdirectory(miditools EXCLUDE_FROM_ALL)
add_subdirectory(fonttools EXCLUDE_FROM_ALL)
add_subdirectory(manual)
add_subdirectory(demos)

if (USE_PORTMIDI AND (MINGW OR APPLE))
      subdirs (thirdparty/portmidi)
endif (USE_PORTMIDI AND (MINGW OR APPLE))

if (AEOLUS)
      subdirs (aeolus)
endif (AEOLUS)

if (ZERBERUS)
      subdirs (zerberus)
endif (ZERBERUS)

if (OMR)
      subdirs (omr thirdparty/poppler)
endif (OMR)

if (OSC)
      subdirs (thirdparty/ofqf)
endif (OSC)

if (NOT USE_SYSTEM_FREETYPE)
      subdirs (thirdparty/freetype)
endif (NOT USE_SYSTEM_FREETYPE)

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

if (USE_SYSTEM_FREETYPE)
      include_directories(${FREETYPE_INCLUDE_DIRS})
else (USE_SYSTEM_FREETYPE)
      include_directories(${PROJECT_SOURCE_DIR}/thirdparty/freetype/include)
endif (USE_SYSTEM_FREETYPE)

if (OMR)
   include_directories(${PROJECT_SOURCE_DIR}/thirdparty/poppler)
   include_directories(${PROJECT_SOURCE_DIR}/thirdparty/poppler/poppler)
endif (OMR)

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
   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
   )

## lrelease is used to compile the translated text files generated by lupdate into binary resources

add_custom_target(lrelease
   COMMAND Qt5::lrelease ${PROJECT_SOURCE_DIR}/share/locale/*.ts
   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
   )


##
##  Create and install the plugin framework manual
##

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/plugins
   COMMAND genManual ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR}
   DEPENDS genManual
   )
