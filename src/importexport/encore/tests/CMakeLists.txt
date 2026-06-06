# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited and others
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

set(MODULE_TEST iex_encore_tests)

set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/environment.cpp
    ${CMAKE_CURRENT_LIST_DIR}/testbase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/testbase.h
    ${CMAKE_CURRENT_LIST_DIR}/tst_smoke.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_parser_ticks.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_parser_chord.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_instruments.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_staff_size.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_structure.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_notes.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_notes_mrest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_tempo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_notes_tuplets.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tst_notes_ties.cpp
)

set(MODULE_TEST_LINK
    engraving
    iex_encore
)

set(MODULE_TEST_DATA_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(SetupGTest)
