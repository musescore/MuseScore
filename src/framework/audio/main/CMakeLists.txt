# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited
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
# along with this program. If not, see <https://www.gnu.org/licenses/>.

muse_create_module(muse_audio_main)

target_include_directories(muse_audio_main PRIVATE ..)

target_sources(muse_audio_main PRIVATE
    audiomodule.cpp
    audiomodule.h
    iaudioconfiguration.h
    istartaudiocontroller.h
    iplayback.h
    iplayer.h
    isoundfontcontroller.h

    internal/audioconfiguration.cpp
    internal/audioconfiguration.h
    internal/audiouiactions.cpp
    internal/audiouiactions.h
    internal/audioactionscontroller.cpp
    internal/audioactionscontroller.h
    internal/startaudiocontroller.cpp
    internal/startaudiocontroller.h
    internal/playback.cpp
    internal/playback.h
    internal/player.cpp
    internal/player.h
    internal/audiodrivercontroller.cpp
    internal/audiodrivercontroller.h
)

if (OS_IS_WASM)
    target_sources(muse_audio_main PRIVATE
        platform/web/websoundfontcontroller.cpp
        platform/web/websoundfontcontroller.h
    )
else()
    target_sources(muse_audio_main PRIVATE
        platform/general/generalsoundfontcontroller.cpp
        platform/general/generalsoundfontcontroller.h
    )
endif()

target_link_libraries(muse_audio_main PRIVATE muse_audio_common)
