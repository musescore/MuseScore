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

set(COMPAT_MIDI_SRC
    ${CMAKE_CURRENT_LIST_DIR}/event.cpp
    ${CMAKE_CURRENT_LIST_DIR}/event.h
    ${CMAKE_CURRENT_LIST_DIR}/midicoreevent.h
    ${CMAKE_CURRENT_LIST_DIR}/midiinstrumenteffects.h
    ${CMAKE_CURRENT_LIST_DIR}/midipatch.h
    ${CMAKE_CURRENT_LIST_DIR}/midirender.cpp
    ${CMAKE_CURRENT_LIST_DIR}/midirender.h
    ${CMAKE_CURRENT_LIST_DIR}/pitchwheelrenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/pitchwheelrenderer.h
    )

if (NOT MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 9.0)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-copy")
endif (NOT MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 9.0)
