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

message(STATUS "Build configure: ${MUSESCORE_BUILD_CONFIGURE}")
string(TOUPPER ${MUSESCORE_BUILD_CONFIGURE} _MUSESCORE_BUILD_CONFIGURE)

###########################################
# General
###########################################

set(QT_SUPPORT ON)

if (NOT BUILD_AUDIO_MODULE)
    set(BUILD_MUSESAMPLER_MODULE OFF)
    set(BUILD_VST_MODULE OFF)
endif()

if (NOT BUILD_IMPORTEXPORT_MODULE)
    set(BUILD_VIDEOEXPORT_MODULE OFF)
endif()

if (NOT BUILD_DIAGNOSTICS)
    set(BUILD_CRASHPAD_CLIENT OFF)
endif()

###########################################
# Desktop App
###########################################
if(_MUSESCORE_BUILD_CONFIGURE MATCHES "APP")

endif()

###########################################
# VTest
###########################################
if(_MUSESCORE_BUILD_CONFIGURE MATCHES "VTEST")
    set(BUILD_UNIT_TESTS OFF)
    set(BUILD_UPDATE_MODULE OFF)
    set(BUILD_SHORTCUTS_MODULE OFF)
    set(BUILD_NETWORK_MODULE OFF)
    set(BUILD_AUDIO_MODULE OFF)
    set(BUILD_MUSESAMPLER_MODULE OFF)
    set(BUILD_VST_MODULE OFF)
    set(BUILD_LEARN_MODULE OFF)
    set(BUILD_WORKSPACE_MODULE OFF)
    set(BUILD_CLOUD_MODULE OFF)
    set(BUILD_LANGUAGES_MODULE OFF)
    set(BUILD_PLUGINS_MODULE OFF)
    set(BUILD_PLAYBACK_MODULE OFF)
    set(BUILD_PALETTE_MODULE OFF)
    set(BUILD_INSTRUMENTSSCENE_MODULE OFF)
    set(BUILD_INSPECTOR_MODULE OFF)
    set(BUILD_MULTIINSTANCES_MODULE OFF)
    set(BUILD_VIDEOEXPORT_MODULE OFF)
    set(BUILD_IMPORTEXPORT_MODULE OFF)
endif()

###########################################
# UTest
###########################################
if(_MUSESCORE_BUILD_CONFIGURE MATCHES "UTEST")
    set(BUILD_UNIT_TESTS ON)
endif()
