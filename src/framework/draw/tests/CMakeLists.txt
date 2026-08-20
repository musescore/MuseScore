# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2022 MuseScore Limited and others
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

set(MODULE_TEST muse_draw_tests)

set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/painter_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/fontfacext_tests.cpp
)

if (OS_IS_LIN)
    set(MODULE_TEST_SRC
        ${MODULE_TEST_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/fontsprovider_qt_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/fontrendercache_tests.cpp
    )
endif()

set(MODULE_TEST_LINK muse_draw)

set(MODULE_TEST_INCLUDE
    ${MODULE_TEST_INCLUDE}
    ${CMAKE_CURRENT_LIST_DIR}/..
)

set(MODULE_TEST_DEF
    ${MODULE_TEST_DEF}
    MUSE_DRAW_TEST_FONTS_DIR="${CMAKE_CURRENT_LIST_DIR}/fonts"
)

include(SetupGTest)
