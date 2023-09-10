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

declare_module(muse_midi)
set(MODULE_ALIAS muse::midi)

include(GetPlatformInfo)

set(JACK_SRC "")
if (MUSE_MODULE_AUDIO_JACK)
    set(JACK_SRC
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/jack/jackmidioutport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/jack/jackmidioutport.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/jack/jackmidiinport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/jack/jackmidiinport.h
        )
endif()

if (OS_IS_WIN)
    set(DRIVER_SRC
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/winmidioutport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/winmidioutport.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/winmidiinport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/win/winmidiinport.h
    )
elseif(OS_IS_LIN)
    set(DRIVER_SRC
        ${JACK_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/alsa/alsamidioutport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/alsa/alsamidioutport.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/alsa/alsamidiinport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/alsa/alsamidiinport.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/linuxmidioutport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/linuxmidioutport.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/linuxmidiinport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/linuxmidiinport.h
    )
elseif(OS_IS_FBSD)
    set(DRIVER_SRC
        ${JACK_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/alsa/alsamidioutport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/alsa/alsamidioutport.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/alsa/alsamidiinport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/alsa/alsamidiinport.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/linuxmidioutport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/linuxmidioutport.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/linuxmidiinport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/lin/linuxmidiinport.h
    )
elseif(OS_IS_MAC)
    set(DRIVER_SRC
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/osx/coremidioutport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/osx/coremidioutport.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/osx/coremidiinport.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/osx/coremidiinport.h
    )
endif()


set(MODULE_SRC
    ${DRIVER_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/midimodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/midimodule.h
    ${CMAKE_CURRENT_LIST_DIR}/imidiconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/imidiinport.h
    ${CMAKE_CURRENT_LIST_DIR}/imidioutport.h
    ${CMAKE_CURRENT_LIST_DIR}/midievent.h
    ${CMAKE_CURRENT_LIST_DIR}/miditypes.h
    ${CMAKE_CURRENT_LIST_DIR}/midierrors.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/midiconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/midiconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/dummymidioutport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/dummymidioutport.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/dummymidiinport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/dummymidiinport.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/midideviceslistener.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/midideviceslistener.h
    ${CMAKE_CURRENT_LIST_DIR}/view/devtools/midiportdevmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/devtools/midiportdevmodel.h
    )

if (OS_IS_MAC)
    find_library(CoreMIDI NAMES CoreMIDI)
    find_library(CoreAudio NAMES CoreAudio)
    set(MODULE_LINK ${MODULE_LINK} ${CoreMIDI} ${CoreAudio})
elseif (OS_IS_WIN)
    set(MODULE_LINK ${MODULE_LINK} winmm)
elseif (OS_IS_LIN)
    find_package(ALSA REQUIRED)
    set(MODULE_INCLUDE_PRIVATE ${MODULE_INCLUDE_PRIVATE} ${ALSA_INCLUDE_DIRS} )
    set(MODULE_LINK ${MODULE_LINK} ${ALSA_LIBRARIES} pthread )
endif()

set(MODULE_QRC midi.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

setup_module()
