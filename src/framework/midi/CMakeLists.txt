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

muse_create_module(muse_midi ALIAS muse::midi)

target_sources(muse_midi PRIVATE
    midimodule.cpp
    midimodule.h
    imidiconfiguration.h
    imidiinport.h
    imidioutport.h
    midievent.h
    miditypes.h
    midierrors.h

    internal/midiconfiguration.cpp
    internal/midiconfiguration.h
    internal/dummymidioutport.cpp
    internal/dummymidioutport.h
    internal/dummymidiinport.cpp
    internal/dummymidiinport.h
    internal/midideviceslistener.cpp
    internal/midideviceslistener.h
)

include(GetPlatformInfo)
if (OS_IS_MAC)
    target_sources(muse_midi PRIVATE
        internal/platform/osx/coremidioutport.cpp
        internal/platform/osx/coremidioutport.h
        internal/platform/osx/coremidiinport.cpp
        internal/platform/osx/coremidiinport.h
    )

    find_library(CoreMIDI NAMES CoreMIDI)
    find_library(CoreAudio NAMES CoreAudio)
    target_link_libraries(muse_midi PRIVATE ${CoreMIDI} ${CoreAudio})
elseif (OS_IS_WIN)
    target_sources(muse_midi PRIVATE 
        internal/platform/win/winmidioutport.cpp
        internal/platform/win/winmidioutport.h
        internal/platform/win/winmidiinport.cpp
        internal/platform/win/winmidiinport.h
    )

    target_link_libraries(muse_midi PRIVATE winmm)
elseif (OS_IS_LIN OR OS_IS_FBSD)
    target_sources(muse_midi PRIVATE 
        internal/platform/lin/alsamidioutport.cpp
        internal/platform/lin/alsamidioutport.h
        internal/platform/lin/alsamidiinport.cpp
        internal/platform/lin/alsamidiinport.h
    )

    find_package(ALSA REQUIRED)
    target_include_directories(muse_midi PRIVATE ${ALSA_INCLUDE_DIRS})
    target_link_libraries(muse_midi PRIVATE ${ALSA_LIBRARIES} pthread)
endif()
