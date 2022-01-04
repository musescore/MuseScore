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

set(MODULE iex_midi)

include(${CMAKE_CURRENT_LIST_DIR}/internal/midiimport/midiimport.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/internal/midiexport/midiexport.cmake)

set(MODULE_SRC
    ${MIDIIMPORT_SRC}
    ${MIDIEXPORT_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/midimodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/midimodule.h
    ${CMAKE_CURRENT_LIST_DIR}/imidiconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/midiconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/midiconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationmidireader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationmidireader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationmidiwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationmidiwriter.h
    )

set(MODULE_LINK
    beatroot    # for midi
    engraving
    qzip
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

