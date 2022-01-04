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

set(MODULE_TEST engraving_tests)

set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/environment.cpp
    ${CMAKE_CURRENT_LIST_DIR}/testbase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/testbase.h
    # ${CMAKE_CURRENT_LIST_DIR}/tst_compat114.cpp # fail
    # ${CMAKE_CURRENT_LIST_DIR}/tst_compat206.cpp # fail
    # ${CMAKE_CURRENT_LIST_DIR}/tst_links.cpp # fail
    # ${CMAKE_CURRENT_LIST_DIR}/tst_midi.cpp not ported
    # ${CMAKE_CURRENT_LIST_DIR}/tst_midimapping.cpp not ported
    #${CMAKE_CURRENT_LIST_DIR}/tst_parts.cpp # won't compile
    # ${CMAKE_CURRENT_LIST_DIR}/tst_repeat.cpp # fail
    # ${CMAKE_CURRENT_LIST_DIR}/tst_text.cpp not actual, not compile
)

set(MODULE_TEST_LINK
    qzip
    engraving
    fonts
    )

set(MODULE_TEST_DATA_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(${PROJECT_SOURCE_DIR}/src/framework/testing/qtest.cmake)
