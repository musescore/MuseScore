# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2025 MuseScore BVBA and others
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

include(GetPlatformInfo)

if (OS_IS_WIN)
    set(AUDIO_DRIVER_SRC
        #${CMAKE_CURRENT_LIST_DIR}/platform/win/winmmdriver.cpp
        #${CMAKE_CURRENT_LIST_DIR}/platform/win/winmmdriver.h
        #${CMAKE_CURRENT_LIST_DIR}/platform/win/wincoreaudiodriver.cpp
        #${CMAKE_CURRENT_LIST_DIR}/platform/win/wincoreaudiodriver.h
        ${CMAKE_CURRENT_LIST_DIR}/platform/win/wasapiaudioclient.cpp
        ${CMAKE_CURRENT_LIST_DIR}/platform/win/wasapiaudioclient.h
        ${CMAKE_CURRENT_LIST_DIR}/platform/win/wasapitypes.h
        ${CMAKE_CURRENT_LIST_DIR}/platform/win/wasapiaudiodriver.cpp
        ${CMAKE_CURRENT_LIST_DIR}/platform/win/wasapiaudiodriver.h
        ${CMAKE_CURRENT_LIST_DIR}/platform/win/audiodeviceslistener.cpp
        ${CMAKE_CURRENT_LIST_DIR}/platform/win/audiodeviceslistener.h
    )
elseif(OS_IS_LIN OR OS_IS_FBSD)
    set(AUDIO_DRIVER_SRC
        ${CMAKE_CURRENT_LIST_DIR}/platform/lin/alsaaudiodriver.cpp
        ${CMAKE_CURRENT_LIST_DIR}/platform/lin/alsaaudiodriver.h
        ${CMAKE_CURRENT_LIST_DIR}/platform/lin/audiodeviceslistener.cpp
        ${CMAKE_CURRENT_LIST_DIR}/platform/lin/audiodeviceslistener.h
    )
    if (MUSE_PIPEWIRE_AUDIO_DRIVER)
        # this is conditionally added to module source if
        # pipewire is actually found on the system
        set(PW_AUDIO_DRIVER_SRC
            ${CMAKE_CURRENT_LIST_DIR}/platform/lin/pwaudiodriver.cpp
            ${CMAKE_CURRENT_LIST_DIR}/platform/lin/pwaudiodriver.h
        )
    endif()
    if (MUSE_MODULE_AUDIO_JACK)
        set(AUDIO_DRIVER_SRC
            ${AUDIO_DRIVER_SRC}
            ${CMAKE_CURRENT_LIST_DIR}/platform/jack/jackaudiodriver.cpp
            ${CMAKE_CURRENT_LIST_DIR}/platform/jack/jackaudiodriver.h
        )
    endif()
elseif(OS_IS_MAC)
    set(AUDIO_DRIVER_SRC
        ${CMAKE_CURRENT_LIST_DIR}/platform/osx/osxaudiodriver.mm
        ${CMAKE_CURRENT_LIST_DIR}/platform/osx/osxaudiodriver.h
    )

    set_source_files_properties(
        ${CMAKE_CURRENT_LIST_DIR}/platform/osx/osxaudiodriver.mm
        PROPERTIES
        SKIP_UNITY_BUILD_INCLUSION ON
        SKIP_PRECOMPILE_HEADERS ON
    )
elseif(OS_IS_WASM)
    set(AUDIO_DRIVER_SRC
        ${CMAKE_CURRENT_LIST_DIR}/platform/web/webaudiodriver.cpp
        ${CMAKE_CURRENT_LIST_DIR}/platform/web/webaudiodriver.h
    )
endif()
