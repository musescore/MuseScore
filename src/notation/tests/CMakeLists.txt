# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2025 MuseScore Limited
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

set(MODULE_TEST notation_tests)

set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/mocks/msczreadermock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/notationconfigurationmock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/notationinteractionmock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/notationselectionmock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/notationselectionrangemock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/controlledviewmock.h
)

set(MODULE_TEST_DATA_ROOT ${CMAKE_CURRENT_LIST_DIR})

# At the moment no own tests
# include(SetupGTest)
