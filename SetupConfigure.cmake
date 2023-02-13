#=============================================================================
#  MuseScore
#  Linux Music Score Editor
#
#  Copyright (C) 2023 MuseScore BVBA and others
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

include(GetBuildType)

message(STATUS "MUSESCORE_BUILD_CONFIGURE: ${MUSESCORE_BUILD_CONFIGURE}")
message(STATUS "MUSESCORE_BUILD_MODE: ${MUSESCORE_BUILD_MODE}")

string(TOUPPER ${MUSESCORE_BUILD_CONFIGURE} BUILD_CONFIGURE)

###########################################
# General
###########################################

set(QT_SUPPORT ON)

if (NOT MUE_BUILD_AUDIO_MODULE)
    set(MUE_BUILD_MUSESAMPLER_MODULE OFF)
    set(MUE_BUILD_VST_MODULE OFF)
endif()

if (NOT MUE_BUILD_IMPORTEXPORT_MODULE)
    set(MUE_BUILD_VIDEOEXPORT_MODULE OFF)
endif()

if (NOT MUE_BUILD_DIAGNOSTICS_MODULE)
    set(MUE_BUILD_CRASHPAD_CLIENT OFF)
endif()

if (MUE_BUILD_ASAN)
    set(MUE_ENABLE_CUSTOM_ALLOCATOR OFF)
endif()

###########################################
# Desktop App
###########################################
set(MUE_GENERAL_APP OFF)
if(BUILD_CONFIGURE MATCHES "APP")
    set(MUE_GENERAL_APP ON)
endif()

if(BUILD_CONFIGURE MATCHES "APP-PORTABLE")
    set(MUE_GENERAL_APP ON)
    set(WIN_PORTABLE ON)
endif()

if (MUE_GENERAL_APP)
    if (BUILD_IS_DEBUG)
        set(MUE_ENABLE_LOGGER_DEBUGLEVEL ON)
    else()
        set(MUE_ENABLE_LOGGER_DEBUGLEVEL OFF)
    endif()
endif()

if (OS_IS_MAC OR OS_IS_WIN)
    if (WIN_PORTABLE)
        set(MUE_BUILD_UPDATE_MODULE OFF)
    endif()
endif()

###########################################
# VTest
###########################################
if(BUILD_CONFIGURE MATCHES "VTEST")
    set(MUE_BUILD_AUDIO_MODULE OFF)
    set(MUE_BUILD_MUSESAMPLER_MODULE OFF)
    set(MUE_BUILD_NETWORK_MODULE OFF)
    set(MUE_BUILD_SHORTCUTS_MODULE OFF)
    set(MUE_BUILD_VST_MODULE OFF)
    set(MUE_BUILD_CLOUD_MODULE OFF)
    set(MUE_BUILD_IMPORTEXPORT_MODULE OFF)
    set(MUE_BUILD_VIDEOEXPORT_MODULE OFF)
    set(MUE_BUILD_INSPECTOR_MODULE OFF)
    set(MUE_BUILD_INSTRUMENTSSCENE_MODULE OFF)
    set(MUE_BUILD_LANGUAGES_MODULE OFF)
    set(MUE_BUILD_LEARN_MODULE OFF)
    set(MUE_BUILD_MULTIINSTANCES_MODULE OFF)
    set(MUE_BUILD_PALETTE_MODULE OFF)
    set(MUE_BUILD_PLAYBACK_MODULE OFF)
    set(MUE_BUILD_PLUGINS_MODULE OFF)
    set(MUE_BUILD_UPDATE_MODULE OFF)
    set(MUE_BUILD_WORKSPACE_MODULE OFF)

    set(MUE_BUILD_UNIT_TESTS OFF)
    set(MUE_ENABLE_LOGGER_DEBUGLEVEL ON)
endif()

###########################################
# UTest
###########################################
if(BUILD_CONFIGURE MATCHES "UTEST")
    set(MUE_BUILD_UNIT_TESTS ON)
    set(MUE_ENABLE_LOGGER_DEBUGLEVEL ON)
endif()

###########################################
# Global definitions
###########################################
function(def_opt name val)
    if (${val})
        add_definitions(-D${name})
    endif()
endfunction()

# framework
def_opt(MUE_BUILD_AUDIO_MODULE ${MUE_BUILD_AUDIO_MODULE})
def_opt(MUE_ENABLE_AUDIO_EXPORT ${MUE_ENABLE_AUDIO_EXPORT})
def_opt(MUE_BUILD_MUSESAMPLER_MODULE ${MUE_BUILD_MUSESAMPLER_MODULE})
def_opt(MUE_BUILD_NETWORK_MODULE ${MUE_BUILD_NETWORK_MODULE})
def_opt(MUE_BUILD_SHORTCUTS_MODULE ${MUE_BUILD_SHORTCUTS_MODULE})
# modules
def_opt(MUE_BUILD_AUTOBOT_MODULE ${MUE_BUILD_AUTOBOT_MODULE})
def_opt(MUE_BUILD_CLOUD_MODULE ${MUE_BUILD_CLOUD_MODULE})
def_opt(MUE_BUILD_DIAGNOSTICS_MODULE ${MUE_BUILD_DIAGNOSTICS_MODULE})
def_opt(MUE_BUILD_IMPORTEXPORT_MODULE ${MUE_BUILD_IMPORTEXPORT_MODULE})
def_opt(MUE_BUILD_VIDEOEXPORT_MODULE ${MUE_BUILD_VIDEOEXPORT_MODULE})
def_opt(MUE_BUILD_INSPECTOR_MODULE ${MUE_BUILD_INSPECTOR_MODULE})
def_opt(MUE_BUILD_INSTRUMENTSSCENE_MODULE ${MUE_BUILD_INSTRUMENTSSCENE_MODULE})
def_opt(MUE_BUILD_LANGUAGES_MODULE ${MUE_BUILD_LANGUAGES_MODULE})
def_opt(MUE_BUILD_LEARN_MODULE ${MUE_BUILD_LEARN_MODULE})
def_opt(MUE_BUILD_MULTIINSTANCES_MODULE ${MUE_BUILD_MULTIINSTANCES_MODULE})
def_opt(MUE_BUILD_PALETTE_MODULE ${MUE_BUILD_PALETTE_MODULE})
def_opt(MUE_BUILD_PLAYBACK_MODULE ${MUE_BUILD_PLAYBACK_MODULE})
def_opt(MUE_BUILD_PLUGINS_MODULE ${MUE_BUILD_PLUGINS_MODULE})
def_opt(MUE_BUILD_UPDATE_MODULE ${MUE_BUILD_UPDATE_MODULE})
def_opt(MUE_BUILD_WORKSPACE_MODULE ${MUE_BUILD_WORKSPACE_MODULE})

if (QT_SUPPORT)
    add_definitions(-DQT_SUPPORT)
    add_definitions(-DHAW_LOGGER_QT_SUPPORT)
    add_definitions(-DSCRIPT_INTERFACE)
else()
    add_definitions(-DNO_QT_SUPPORT)
endif()

if (WIN_PORTABLE)
    add_definitions(-DWIN_PORTABLE)
endif()

add_definitions(-DMUSESCORE_REVISION="${MUSESCORE_REVISION}")
add_definitions(-DHAW_PROFILER_ENABLED)

if (MUE_ENABLE_LOAD_QML_FROM_SOURCE)
    add_definitions(-DMUE_ENABLE_LOAD_QML_FROM_SOURCE)
endif()

