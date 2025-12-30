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

muse_create_module(iex_midi)

include(${CMAKE_CURRENT_LIST_DIR}/internal/midishared/midishared.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/internal/midiimport/midiimport.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/internal/midiexport/midiexport.cmake)

target_sources(iex_midi PRIVATE
    ${MIDISHARED_SRC}
    ${MIDIIMPORT_SRC}
    ${MIDIEXPORT_SRC}
    midimodule.cpp
    midimodule.h
    imidiconfiguration.h
    internal/midiconfiguration.cpp
    internal/midiconfiguration.h
    internal/notationmidireader.cpp
    internal/notationmidireader.h
    internal/notationmidiwriter.cpp
    internal/notationmidiwriter.h
)

add_subdirectory(thirdparty/beatroot)

target_link_libraries(iex_midi PRIVATE
    beatroot
    engraving
    Qt::Core5Compat
)

if (MUE_BUILD_IMPORTEXPORT_TESTS)
    add_subdirectory(tests)
endif()
