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

include(GetBuildType)
include(GetPlatformInfo)
include(version)

if (NOT MUSESCORE_BUILD_CONFIGURATION)
    set(MUSESCORE_BUILD_CONFIGURATION "app")
endif()

if (NOT MUSE_APP_BUILD_MODE)
    set(MUSE_APP_BUILD_MODE "dev")
endif()

# Set revision for local builds
# Before need run 'make revision' or 'msvc_build.bat revision'
include(TryUseLocalRevision)

message(STATUS "MUSESCORE_BUILD_CONFIGURATION: ${MUSESCORE_BUILD_CONFIGURATION}")
message(STATUS "MUSE_APP_BUILD_MODE: ${MUSE_APP_BUILD_MODE}")
message(STATUS "MUSESCORE_BUILD_NUMBER: ${CMAKE_BUILD_NUMBER}")

string(TOUPPER ${MUSESCORE_BUILD_CONFIGURATION} BUILD_CONFIGURATION)
string(TOUPPER ${MUSE_APP_BUILD_MODE} BUILD_MODE)

###########################################
# Setup by mode
###########################################
if(BUILD_MODE STREQUAL "DEV")
    set(MUSE_APP_UNSTABLE ON)
    set(MUSE_APP_RELEASE_CHANNEL "dev")
    set(MUSE_APP_NAME_VERSION "${MUSE_APP_NAME_VERSION} ${MUSE_APP_RELEASE_CHANNEL}")
    set(MUSE_APP_TITLE_VERSION "${MUSE_APP_TITLE_VERSION} ${MUSE_APP_RELEASE_CHANNEL}")
    set(MUSE_APP_IS_PRERELEASE ON)
    set(MUSESCORE_ALLOW_UPDATE_ON_PRERELEASE OFF)
endif()

if(BUILD_MODE STREQUAL "TESTING")
    set(MUSE_APP_UNSTABLE OFF)
    set(MUSE_APP_RELEASE_CHANNEL "Testing")
    set(MUSE_APP_NAME_VERSION "${MUSE_APP_NAME_VERSION} ${MUSE_APP_RELEASE_CHANNEL}")
    set(MUSE_APP_TITLE_VERSION "${MUSE_APP_TITLE_VERSION} ${MUSE_APP_RELEASE_CHANNEL}")
    set(MUSE_APP_IS_PRERELEASE ON)
    set(MUSESCORE_ALLOW_UPDATE_ON_PRERELEASE ON)
endif()

if(BUILD_MODE STREQUAL "RELEASE")
    set(MUSE_APP_UNSTABLE OFF)
    set(MUSE_APP_IS_PRERELEASE OFF)
    set(MUSESCORE_ALLOW_UPDATE_ON_PRERELEASE OFF)
endif()

###########################################
# Setup paths
###########################################
if (OS_IS_MAC)
    SET(Mscore_INSTALL_NAME    "Contents/Resources/")
    SET(Mscore_SHARE_NAME      "mscore.app/")
elseif (OS_IS_WIN)
    SET(Mscore_INSTALL_NAME  "")
    SET(Mscore_SHARE_NAME    "./")
else()
    SET(Mscore_INSTALL_NAME  "mscore${MUSE_APP_INSTALL_SUFFIX}-${MUSE_APP_VERSION_MAJ_MIN}/")
    SET(Mscore_SHARE_NAME    "share/")
endif()

###########################################
# CONFIGURE: Desktop App
###########################################
set(MUE_GENERAL_APP OFF)
set(MUE_ENABLE_SPLASHSCREEN ON)
if(BUILD_CONFIGURATION STREQUAL "APP")
    set(MUE_CONFIGURATION_IS_APP ON)
    set(MUE_GENERAL_APP ON)
endif()

if(BUILD_CONFIGURATION STREQUAL "APP-PORTABLE")
    set(MUE_CONFIGURATION_IS_APPPORTABLE ON)
    set(MUE_GENERAL_APP ON)
    set(WIN_PORTABLE ON)
endif()

if (WIN_PORTABLE)
    set(MUSE_MODULE_UPDATE OFF)
endif()
if (OS_IS_FBSD)
    message(WARNING "Not building unsupported chrashpad client on FreeBSD")
    set(MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT OFF)
endif()

###########################################
# CONFIGURE: Web App
###########################################
if(BUILD_CONFIGURATION STREQUAL "APP-WEB")
    set(MUE_CONFIGURATION_IS_APPWEB ON)

    set(QT_ADD_LINGUISTTOOLS OFF)
    set(QT_ADD_CONCURRENT OFF)
    set(QT_QPROCESS_SUPPORTED OFF)
    set(QT_CONCURRENT_SUPPORTED OFF)

    set(MUE_GENERAL_APP ON)
    set(MUE_BUILD_APPSHELL_MODULE ON)
    set(MUE_BUILD_PROJECT_MODULE ON)
    set(MUE_BUILD_NOTATION_MODULE ON)
    set(MUE_BUILD_INSPECTOR_MODULE ON)
    set(MUE_BUILD_INSTRUMENTSSCENE_MODULE ON)
    set(MUE_BUILD_PALETTE_MODULE ON)

    set(MUSE_ENABLE_UNIT_TESTS OFF)

    #set(MUSE_MODULE_DIAGNOSTICS OFF)
    set(MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT OFF)
    set(MUSE_MODULE_AUDIO OFF)
    set(MUSE_MODULE_AUDIOPLUGINS OFF)
    set(MUSE_MODULE_ACCESSIBILITY OFF)
    set(MUSE_MODULE_AUDIO OFF)
    set(MUSE_MODULE_AUDIOPLUGINS OFF)
    set(MUSE_MODULE_MIDI OFF)
    set(MUSE_MODULE_MPE OFF)
    set(MUSE_MODULE_MUSESAMPLER OFF)
    set(MUSE_MODULE_NETWORK OFF)
    set(MUSE_MODULE_SHORTCUTS OFF)
    set(MUSE_MODULE_VST OFF)
    set(MUSE_MODULE_AUTOBOT OFF)
    set(MUSE_MODULE_CLOUD OFF)
    set(MUSE_MODULE_LANGUAGES OFF)
    set(MUSE_MODULE_LEARN OFF)
    set(MUSE_MODULE_MULTIINSTANCES OFF)
    set(MUSE_MODULE_EXTENSIONS OFF)
    set(MUSE_MODULE_UPDATE OFF)
    set(MUSE_MODULE_WORKSPACE OFF)
    set(MUSE_MODULE_TOURS OFF)

    set(MUE_ENABLE_SPLASHSCREEN OFF)
    set(MUE_BUILD_CONVERTER_MODULE OFF)
    set(MUE_BUILD_PRINT_MODULE OFF)
    set(MUE_BUILD_MUSESOUNDS_MODULE OFF)
    set(MUE_BUILD_PLAYBACK_MODULE OFF)
    set(MUE_BUILD_BRAILLE_MODULE OFF)
    # IMPORT EXPORT MODULES
    set(MUE_BUILD_IMPEXP_BB_MODULE OFF)
    set(MUE_BUILD_IMPEXP_BWW_MODULE OFF)
    set(MUE_BUILD_IMPEXP_CAPELLA_MODULE OFF)
    set(MUE_BUILD_IMPEXP_MIDI_MODULE OFF)
    set(MUE_BUILD_IMPEXP_MUSEDATA_MODULE OFF)
    set(MUE_BUILD_IMPEXP_MUSICXML_MODULE OFF)
    set(MUE_BUILD_IMPEXP_OVE_MODULE OFF)
    set(MUE_BUILD_IMPEXP_AUDIOEXPORT_MODULE OFF)
    set(MUE_BUILD_IMPEXP_IMAGESEXPORT_MODULE OFF)
    set(MUE_BUILD_IMPEXP_GUITARPRO_MODULE OFF)
    set(MUE_BUILD_IMPEXP_MEI_MODULE OFF)
    set(MUE_BUILD_IMPEXP_VIDEOEXPORT_MODULE OFF)

    set(MUE_INSTALL_SOUNDFONT OFF)

endif()

###########################################
# CONFIGURE: VTest
###########################################
if(BUILD_CONFIGURATION STREQUAL "VTEST")
    set(MUE_CONFIGURATION_IS_VTEST ON)

    set(MUSE_ENABLE_UNIT_TESTS OFF)
    set(MUSE_MODULE_GLOBAL_LOGGER_DEBUGLEVEL ON)
    set(MUSE_COMPILE_ASAN ON)

    set(MUE_BUILD_CONVERTER_MODULE ON)
    set(MUE_BUILD_PROJECT_MODULE ON)
    set(MUE_BUILD_NOTATION_MODULE ON)
    set(MUSE_MODULE_UI ON)

    set(MUSE_MODULE_ACCESSIBILITY OFF)
    set(MUSE_MODULE_AUDIO OFF)
    set(MUSE_MODULE_AUDIOPLUGINS OFF)
    set(MUE_BUILD_BRAILLE_MODULE OFF)
    set(MUSE_MODULE_MIDI OFF)
    set(MUSE_MODULE_MPE OFF)
    set(MUSE_MODULE_MUSESAMPLER OFF)
    set(MUSE_MODULE_NETWORK OFF)
    set(MUSE_MODULE_SHORTCUTS OFF)
    set(MUSE_MODULE_VST OFF)
    set(MUE_BUILD_APPSHELL_MODULE OFF)
    set(MUSE_MODULE_AUTOBOT OFF)
    set(MUSE_MODULE_CLOUD OFF)
    set(MUE_BUILD_INSPECTOR_MODULE OFF)
    set(MUE_BUILD_INSTRUMENTSSCENE_MODULE OFF)
    set(MUSE_MODULE_LANGUAGES OFF)
    set(MUSE_MODULE_LEARN OFF)
    set(MUSE_MODULE_MULTIINSTANCES OFF)
    set(MUE_BUILD_MUSESOUNDS_MODULE OFF)
    set(MUE_BUILD_PALETTE_MODULE OFF)
    set(MUE_BUILD_PLAYBACK_MODULE OFF)
    set(MUSE_MODULE_EXTENSIONS OFF)
    set(MUSE_MODULE_UPDATE OFF)
    set(MUSE_MODULE_WORKSPACE OFF)

    # IMPORT EXPORT MODULES
    set(MUE_BUILD_IMPEXP_BB_MODULE OFF)
    set(MUE_BUILD_IMPEXP_BWW_MODULE OFF)
    set(MUE_BUILD_IMPEXP_CAPELLA_MODULE OFF)
    set(MUE_BUILD_IMPEXP_MIDI_MODULE OFF)
    set(MUE_BUILD_IMPEXP_MUSEDATA_MODULE OFF)
    set(MUE_BUILD_IMPEXP_MUSICXML_MODULE OFF)
    set(MUE_BUILD_IMPEXP_OVE_MODULE OFF)
    set(MUE_BUILD_IMPEXP_AUDIOEXPORT_MODULE OFF)
    set(MUE_BUILD_IMPEXP_IMAGESEXPORT_MODULE ON)
    set(MUE_BUILD_IMPEXP_GUITARPRO_MODULE ON)
    set(MUE_BUILD_IMPEXP_MEI_MODULE OFF)
    set(MUE_BUILD_IMPEXP_VIDEOEXPORT_MODULE OFF)

    set(MUE_INSTALL_SOUNDFONT OFF)

    set(MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT OFF)

endif()

###########################################
# CONFIGURE: UTest
###########################################
if(BUILD_CONFIGURATION STREQUAL "UTEST")
    set(MUE_CONFIGURATION_IS_UTEST ON)

    set(MUSE_ENABLE_UNIT_TESTS ON)
    set(MUSE_MODULE_GLOBAL_LOGGER_DEBUGLEVEL ON)
    set(MUSE_MODULE_AUDIO ON)
    set(MUSE_COMPILE_ASAN ON)

    message(STATUS "If you added tests to a module that didn't have them yet, make sure that this module is enabled, see SetupConfigure.cmake")
    set(MUSE_MODULE_MIDI OFF)
    set(MUSE_MODULE_MUSESAMPLER OFF)
    set(MUSE_MODULE_NETWORK OFF)
    set(MUSE_MODULE_SHORTCUTS OFF)

    set(MUE_BUILD_APPSHELL_MODULE OFF)
    set(MUSE_MODULE_AUTOBOT OFF)
    set(MUSE_MODULE_CLOUD OFF)
    set(MUE_BUILD_CONVERTER_MODULE OFF)
    set(MUE_BUILD_INSPECTOR_MODULE OFF)
    set(MUE_BUILD_INSTRUMENTSSCENE_MODULE OFF)
    set(MUSE_MODULE_LANGUAGES OFF)
    set(MUSE_MODULE_LEARN OFF)
    set(MUSE_MODULE_MULTIINSTANCES OFF)
    set(MUE_BUILD_PALETTE_MODULE OFF)
    set(MUE_BUILD_PLAYBACK_MODULE OFF)
    set(MUSE_MODULE_WORKSPACE OFF)
endif()

###########################################
# Subsystem
###########################################

set(QT_SUPPORT ON)

if (MUSE_MODULE_AUDIO_JACK)
    if (NOT (OS_IS_LIN OR CC_IS_MINGW))
        set(MUSE_MODULE_AUDIO_JACK OFF)
    endif()
endif()

if (MUSE_COMPILE_ASAN)
    set(MUSE_ENABLE_CUSTOM_ALLOCATOR OFF)
endif()

if (NOT MUE_BUILD_NOTATION_MODULE)
    set(MUE_BUILD_PROJECT_MODULE OFF) # hard dependency
    set(MUE_BUILD_PALETTE_MODULE OFF) # hard dependency
endif()

if (NOT MUSE_MODULE_UI)
    set(MUE_BUILD_APPSHELL_MODULE OFF) # hard dependency
endif()

if (NOT QT_ADD_LINGUISTTOOLS)
    set(MUE_RUN_LRELEASE OFF) # hard dependency
endif()

if (MUSE_MODULE_NETWORK_WEBSOCKET)
    set(QT_ADD_WEBSOCKET ON)
endif()

###########################################
# Unit tests
###########################################
if (NOT MUSE_ENABLE_UNIT_TESTS)

    set(MUE_BUILD_BRAILLE_TESTS OFF)
    set(MUE_BUILD_ENGRAVING_TESTS OFF)
    set(MUE_BUILD_IMPORTEXPORT_TESTS OFF)
    set(MUE_BUILD_NOTATION_TESTS OFF)
    set(MUE_BUILD_PLAYBACK_TESTS OFF)
    set(MUE_BUILD_PROJECT_TESTS OFF)

endif()

###########################################
# Configure framework
###########################################
set(MUSE_APP_REVISION ${MUSESCORE_REVISION})
set(MUSE_APP_BUILD_NUMBER ${CMAKE_BUILD_NUMBER})
set(MUSE_APP_INSTALL_PREFIX "\"${CMAKE_INSTALL_PREFIX}\"")
set(MUSE_APP_INSTALL_NAME "\"${Mscore_INSTALL_NAME}\"")

include(${MUSE_FRAMEWORK_SRC_PATH}/cmake/MuseSetupConfiguration.cmake)

###########################################
# Global definitions
###########################################
if (QT_SUPPORT)
    add_compile_definitions(QT_SUPPORT)
    add_compile_definitions(SCRIPT_INTERFACE)
else()
    add_compile_definitions(NO_QT_SUPPORT)
endif()

if (MUE_GENERAL_APP)
    if (BUILD_IS_DEBUG)
        set(MUSE_MODULE_GLOBAL_LOGGER_DEBUGLEVEL ON)
    else()
        set(MUSE_MODULE_GLOBAL_LOGGER_DEBUGLEVEL OFF)
    endif()
endif()

add_compile_definitions(KORS_PROFILER_ENABLED)

if (MUE_ENABLE_LOAD_QML_FROM_SOURCE)
    add_compile_definitions(MUE_ENABLE_LOAD_QML_FROM_SOURCE)
endif()

configure_file(${CMAKE_CURRENT_LIST_DIR}/src/app/app_config.h.in app_config.h )
