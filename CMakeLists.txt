#=============================================================================
#  Mscore
#  Linux Music Score Editor
#
#  Copyright (C) 2002-2011 by Werner Schweer and others
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

#minimum tested, report if it works with older
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)

include (${PROJECT_SOURCE_DIR}/build/ECMQt4To5Porting.cmake)
include (${PROJECT_SOURCE_DIR}/build/FindQt5Transitional.cmake)

cmake_policy(SET CMP0003 NEW)

if(POLICY CMP0020)
      cmake_policy(SET CMP0020 OLD)
endif(POLICY CMP0020)

#include modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/build" ${CMAKE_MODULE_PATH})
include (UsePkgConfig1)
include (TargetDoc)
include (FindPulseAudio)
include (FindLame)
include (GetCompilerVersion)
include (CreatePrecompiledHeader)

# for debugging the make system uncomment next line:
#set(CMAKE_VERBOSE_MAKEFILE ON)

set(MSCORE_UNSTABLE  TRUE)          # mark as unstable

set(USE_SSE           TRUE)
set(SCRIPT_INTERFACE  TRUE)

option(AEOLUS        "enable pipe organ synthesizer"      ON)
option(ZERBERUS      "enable experimental sfz sampler"    ON)
option(OSC           "enable OSC remote control protocol" ON)
option(OMR           "enable PDF import"  OFF)                 # OMR - optical music recognition
# for installation see: http://ubuntuforums.org/showthread.php?t=1647350
option(OCR           "enable OCR, requires OMR" OFF)           # requires tesseract 3.0, needs work on mac/win
option(SOUNDFONT3    "ogg vorbis compressed fonts" ON)         # enable ogg vorbis compressed fonts, require ogg & vorbis
option(HAS_AUDIOFILE "enable audio export" ON)               # requires libsndfile
option(USE_SYSTEM_QTSINGLEAPPLICATION "Use system QtSingleApplication" OFF)

SET(JACK_LONGNAME "jack (jack audio connection kit)")
SET(JACK_MIN_VERSION "0.98.0")
option(BUILD_JACK    "Build with support for ${JACK_LONGNAME}. jack >= ${JACK_MIN_VERSION} will be needed." ON)

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
      # set(CMAKE_OSX_SYSROOT /Developer/SDKs/MacOSX10.6.sdk)

      set(CMAKE_OSX_DEPLOYMENT_TARGET 10.7)      #min version required
      set(HAS_AUDIOFILE TRUE)            # requires libsndfile

      find_library(AudioToolboxFW        NAMES AudioToolbox)
      find_library(AudioUnitFW           NAMES AudioUnit)
      find_library(CoreAudioFW           NAMES CoreAudio)
      find_library(CoreMidiFW            NAMES CoreMIDI)
      find_library(SystemConfigurationFW NAMES SystemConfiguration)
      find_library(CoreServicesFW        NAMES CoreServices)
      set(OsxFrameworks ${AudioToolboxFW} ${AudioUnitFW} ${CoreAudioFW} ${CoreMidiFW} ${SystemConfigurationFW} ${CoreServicesFW})
endif (APPLE)

#
#     Check for gcc compiler >= 4.7
#
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
      if (${CMAKE_CXX_COMPILER_MAJOR} LESS 4
        OR ((${CMAKE_CXX_COMPILER_MAJOR} EQUAL 4) AND (${CMAKE_CXX_COMPILER_MINOR} LESS 7)))
            message(FATAL_ERROR "bad gcc compiler version " ${CMAKE_CXX_COMPILER_VERSION}
                  " >= 4.7 required")
      endif()
endif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")

if (APPLE)
   set(CMAKE_CXX_FLAGS_DEBUG   "-std=c++0x -fPIC -stdlib=libc++ -g")
   set(CMAKE_CXX_FLAGS_RELEASE "-std=c++0x -fPIC -stdlib=libc++ -O2 -DNDEBUG")
else (APPLE)
   if (MINGW)
      # -mno-ms-bitfields see #22048
      set(CMAKE_CXX_FLAGS_DEBUG   "-std=gnu++0x -mno-ms-bitfields -g")
      set(CMAKE_CXX_FLAGS_RELEASE "-std=gnu++0x -mno-ms-bitfields -O2 -DNDEBUG")
   else (MINGW)
      set(CMAKE_CXX_FLAGS_DEBUG   "-std=gnu++0x -fPIC -fPIE -g")
      set(CMAKE_CXX_FLAGS_RELEASE "-std=gnu++0x -fPIC -O2 -DNDEBUG")
   endif (MINGW)
endif(APPLE)

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_SKIP_RULE_DEPENDENCY TRUE)

# The Mscore version number.
SET(MUSESCORE_VERSION_MAJOR  "2")
SET(MUSESCORE_VERSION_MINOR  "0")
SET(MUSESCORE_VERSION_PATCH  "0")
SET(MUSESCORE_VERSION       "${MUSESCORE_VERSION_MAJOR}.${MUSESCORE_VERSION_MINOR}")
#
# version schema x.x.x is hardcoded in source
#
SET(MUSESCORE_VERSION_FULL  "${MUSESCORE_VERSION}.${MUSESCORE_VERSION_PATCH}")

if (MINGW OR APPLE)
  if(MINGW)
      SET(Mscore_INSTALL_NAME  "")
      SET(Mscore_SHARE_NAME    "./")
  else(MINGW)
      SET(Mscore_INSTALL_NAME  "Contents/Resources/")
      SET(Mscore_SHARE_NAME    "mscore.app/")
  endif(MINGW)
else (MINGW OR APPLE)
      SET(Mscore_INSTALL_NAME  "mscore-${MUSESCORE_VERSION}/")
      SET(Mscore_SHARE_NAME    "share/")
endif (MINGW OR APPLE)

##
##  look for Qt5
##
SET(QT_MIN_VERSION    "5.1.0")
SET(QT_USE_QTXML         TRUE)
SET(QT_USE_QTSVG         TRUE)
SET(QT_USE_QTNETWORK     TRUE)
SET(QT_USE_QTDESIGNER    TRUE)
SET(QT_USE_QTWEBKIT      TRUE)
SET(QT_USE_QTXMLPATTERNS TRUE)
SET(QT_USE_QTDECLARATIVE TRUE)

# find_package(Qt4 "4.8" REQUIRED QtCore QtGui QtXml QtSvg QtNetwork QtWebkit QtDeclarative)
# include(${QT_USE_FILE})


##
## alsa >= 1.0.0
##

if (APPLE OR MINGW)
	message("Disabling ALSA support due to OS X or MINGW build.")
      set (USE_ALSA 0)
      set (HAS_MIDI 1)
else (APPLE OR MINGW)
      PKGCONFIG (alsa 1.0.0 ALSA_INCDIR ALSA_LIBDIR ALSA_LIB ALSA_CPP )
      if (NOT ALSA_INCDIR)
            message(FATAL_ERROR "Fatal error: ALSA >= 1.0.0 required")
      else (NOT ALSA_INCDIR)
	      message("Alsa detected.")
            set (USE_ALSA 1)
            set (HAS_MIDI 1)
      endif (NOT ALSA_INCDIR)
endif (APPLE OR MINGW)

##
## pulseaudio
##

if (APPLE OR MINGW)
      set (USE_PULSEAUDIO 0)
else (APPLE OR MINGW)
      if (PULSEAUDIO_FOUND)
            set(USE_PULSEAUDIO 1)
            message("Pulseaudio found.")
      else (PULSEAUDIO_FOUND)
            set(USE_PULSEAUDIO 0)
            message("Pulseaudio not found.")
      endif (PULSEAUDIO_FOUND)
endif (APPLE OR MINGW)


##
## find jack >= JACK_MIN_VERSION
##

IF(BUILD_JACK)
     IF(MINGW)
           set (USE_JACK 1)
           IF("$ENV{PROCESSOR_ARCHITEW6432}" STREQUAL "")
              IF($ENV{PROCESSOR_ARCHITECTURE} STREQUAL "x86")
                 # "pure" 32-bit environment
                 set (JACK_INCDIR "$ENV{PROGRAMFILES}/Jack/includes")
                 set (JACK_LIB "$ENV{PROGRAMFILES}/Jack/lib/libjack.a")
              ELSE($ENV{PROCESSOR_ARCHITECTURE} STREQUAL "x86")
                 # "pure" 64-bit environment
                 set (JACK_INCDIR "$ENV{PROGRAMFILES(x86)}/Jack/includes")
                 set (JACK_LIB "$ENV{PROGRAMFILES(x86)}/Jack/lib/libjack.a")
              ENDIF($ENV{PROCESSOR_ARCHITECTURE} STREQUAL "x86")
           ELSE("$ENV{PROCESSOR_ARCHITEW6432}" STREQUAL "")
              IF("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "x86")
                 # 32-bit program running with an underlying 64-bit environment
                 set (JACK_INCDIR "$ENV{PROGRAMFILES(x86)}/Jack/includes")
                 set (JACK_LIB "$ENV{PROGRAMFILES(x86)}/Jack/lib/libjack.a")
              ELSE("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "x86")
                 # theoretically impossible case...
              ENDIF("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "x86")
           ENDIF("$ENV{PROCESSOR_ARCHITEW6432}" STREQUAL "")

     ELSE(MINGW)
           PKGCONFIG(jack ${JACK_MIN_VERSION} JACK_INCDIR JACK_LIBDIR JACK_LIB JACK_CPP)
           IF(JACK_INCDIR)
                 MESSAGE(STATUS "${JACK_LONGNAME} >= ${JACK_MIN_VERSION} found")
                 SET(USE_JACK 1)
           ELSE(JACK_INCDIR)
                 MESSAGE(STATUS "${JACK_LONGNAME} >= ${JACK_MIN_VERSION} not found")
                 IF(NOT BUILD_JACK STREQUAL AUTO)
                       MESSAGE(SEND_ERROR "Error: jack support requested but not found (BUILD_JACK=${BUILD_JACK})")
                 ENDIF()
           ENDIF(JACK_INCDIR)
     ENDIF(MINGW)
ELSE(BUILD_JACK)
     MESSAGE(STATUS "${JACK_LONGNAME} support disabled")
ENDIF(BUILD_JACK)


##
## portaudio
##

if (MINGW)
    set ( USE_PORTAUDIO 1 )
    set ( USE_PORTMIDI  1 )
else (MINGW)
    PKGCONFIG (portaudio-2.0 19 PORTAUDIO_INCDIR PORTAUDIO_LIBDIR PORTAUDIO_LIB PORTAUDIO_CPP)
    if (PORTAUDIO_INCDIR)
        message("portaudio detected ${PORTAUDIO_INCDIR} ${PORTAUDIO_LIBDIR} ${PORTAUDIO_LIB}")
          set ( USE_PORTAUDIO 1 )
    else (PORTAUDIO_INCDIR)
          message("optional package portaudio-2.0 Version 19 not found (package portaudio19-dev)\n")
          set ( USE_PORTAUDIO 0 )
    endif (PORTAUDIO_INCDIR)
    if (APPLE)
      set (USE_PORTMIDI   1)
    else (APPLE)
      set (USE_PORTMIDI   0)
    endif (APPLE)
endif (MINGW)


if (APPLE)
   if(SOUNDFONT3)
      ##
      ## libvorbis
      ##

      PKGCONFIG (vorbis 1.3.3 VORBIS_INCDIR VORBIS_LIBDIR VORBIS_LIB VORBIS_CPP)
      if (VORBIS_INCDIR)
          message("libvorbis detected ${VORBIS_INCDIR} ${VORBIS_LIBDIR} ${VORBIS_LIB}")
      else (VORBIS_INCDIR)
          message("libvorbis not found\n")
      endif (VORBIS_INCDIR)


      ##
      ## libogg
      ##

      PKGCONFIG (ogg 1.3.0 OGG_INCDIR OGG_LIBDIR OGG_LIB OGG_CPP)
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

      PKGCONFIG (sndfile 1.0.25 SNDFILE_INCDIR SNDFILE_LIBDIR SNDFILE_LIB SNDFILE_CPP)
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
configure_file (
      ${PROJECT_SOURCE_DIR}/build/Doxyfile.in
      ${PROJECT_BINARY_DIR}/Doxyfile
      )
configure_file (
      ${PROJECT_SOURCE_DIR}/build/Doxyfile-LibMscore.in
      ${PROJECT_BINARY_DIR}/Doxyfile-LibMscore
      )

if (NOT MINGW AND NOT APPLE)
    #
    #     set library search path for runtime linker to load the same
    #     qt libraries as we used at compile time
    #
    set (CMAKE_EXE_LINKER_FLAGS "-Wl,-rpath=${_qt5Core_install_prefix}/lib")
    # install desktop file
    install( FILES build/mscore.desktop DESTINATION share/applications)
endif (NOT MINGW AND NOT APPLE)

#
#  create precompiled header file
#

# all.h is expected in PROJECT_BINARY_DIR by subdirs
add_custom_command(
    OUTPUT ${PROJECT_BINARY_DIR}/all.h
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy ${PROJECT_SOURCE_DIR}/all.h ${PROJECT_BINARY_DIR}/all.h
    DEPENDS ${PROJECT_SOURCE_DIR}/all.h
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    )

precompiled_header(QT_INCLUDES all)
ADD_CUSTOM_TARGET(mops1 DEPENDS ${PROJECT_BINARY_DIR}/all.h)
ADD_CUSTOM_TARGET(mops2 DEPENDS ${PCH})

##
## add subdirs
##
subdirs(
      mscore  awl bww2mxml share midi audiofile fluid mstyle libmscore synthesizer
      effects thirdparty/rtf2html thirdparty/diff
      )

add_subdirectory(mtest EXCLUDE_FROM_ALL)
add_subdirectory(miditools EXCLUDE_FROM_ALL)
add_subdirectory(manual)

if (USE_PORTMIDI)
      subdirs (thirdparty/portmidi)
endif (USE_PORTMIDI)

if (AEOLUS)
      subdirs (aeolus)
endif (AEOLUS)

if (ZERBERUS)
      subdirs (zerberus)
endif (ZERBERUS)

if (OMR)
      subdirs (omr)
endif (OMR)

if (OSC)
      subdirs (thirdparty/ofqf)
endif (OSC)


##
## includes
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

##
##  Include packaging
##

include(Packaging)

##
##  custom target for translation generation
##

add_custom_target(lupdate
   COMMAND ${PROJECT_SOURCE_DIR}/build/gen-qt-projectfile ${PROJECT_SOURCE_DIR} > mscore.pro
   COMMAND ${QT_LUPDATE_EXECUTABLE} -noobsolete ${PROJECT_BINARY_DIR}/mscore.pro
   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
   )

add_custom_target(lrelease
   COMMAND ${PROJECT_SOURCE_DIR}/build/gen-qt-projectfile ${PROJECT_SOURCE_DIR} > mscore.pro
   COMMAND ${QT_LRELEASE_EXECUTABLE} ${PROJECT_SOURCE_DIR}/share/locale/*.ts
   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
   )


##
##  create and install the plugin framework manual
##

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/plugins
   COMMAND genManual ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR}
   DEPENDS genManual
   )

add_custom_target(install_plugins_manual
   COMMAND ${PROJECT_BINARY_DIR}/manual/${CMAKE_CFG_INTDIR}/genManual ${PROJECT_SOURCE_DIR} ${CMAKE_INSTALL_PREFIX}/${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}manual
   )

if (MINGW)
   install(CODE "execute_process(COMMAND cmd /c \"mingw32-make install_plugins_manual\")")
else (MINGW)
   install(CODE "execute_process(COMMAND make install_plugins_manual)")
endif (MINGW)

