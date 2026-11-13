# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2024 MuseScore Limited and others
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

set(MODULE_TEST playback_test)

set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/mocks/playbackconfigurationmock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/playbackcontrollermock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/playbackstatemock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/playbackmock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/playermock.h
    ${CMAKE_CURRENT_LIST_DIR}/playbackcontrollertests.cpp

    # NOTE The playback module uses this utility, but it lives in notationscene,
    # and linking the whole notationscene would drag the UI stack into the tests
    ${CMAKE_CURRENT_LIST_DIR}/../../notationscene/utilities/percussionutilities.cpp
)

set(MODULE_TEST_INCLUDE
    ${CMAKE_CURRENT_LIST_DIR}/..
)

set(MODULE_TEST_LINK
    playback
    notation
    engraving
)

set(MODULE_TEST_DATA_ROOT ${CMAKE_CURRENT_LIST_DIR})

if (MUE_BUILD_PLAYBACK_QML)
    list(APPEND MODULE_TEST_SRC
        ${CMAKE_CURRENT_LIST_DIR}/environment.cpp
        ${CMAKE_CURRENT_LIST_DIR}/practiceemphasispanelmodel_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../qml/MuseScore/Playback/practiceemphasispanelmodel.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../qml/MuseScore/Playback/practiceemphasispanelmodel.h
        ${PROJECT_SOURCE_DIR}/src/engraving/tests/utils/scorerw.cpp
        ${PROJECT_SOURCE_DIR}/src/engraving/tests/utils/scorerw.h
    )
    list(APPEND MODULE_TEST_INCLUDE
        ${CMAKE_CURRENT_LIST_DIR}/../qml/MuseScore/Playback
        ${PROJECT_SOURCE_DIR}/src/engraving/tests
    )
    list(APPEND MODULE_TEST_LINK Qt::Quick)
    set(MODULE_TEST_DATA_ROOT ${PROJECT_SOURCE_DIR}/src/engraving/tests)
endif()

include(SetupGTest)
