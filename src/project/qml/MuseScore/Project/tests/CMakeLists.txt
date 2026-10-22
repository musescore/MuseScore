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

set(MODULE_TEST project_qml_tests)

set(MODULE_TEST_SRC
    ${PROJECT_SOURCE_DIR}/src/project/tests/mocks/projectconfigurationmock.h
    ${PROJECT_SOURCE_DIR}/src/project/tests/mocks/convertfiletoscorescenariomock.h
    ${PROJECT_SOURCE_DIR}/src/project/tests/mocks/deletecloudscorescenariomock.h
    ${PROJECT_SOURCE_DIR}/src/project/tests/mocks/convertfiletoscoreservicemock.h
    ${PROJECT_SOURCE_DIR}/src/project/tests/mocks/notationprojectmock.h
    ${PROJECT_SOURCE_DIR}/src/project/tests/mocks/recentfilescontrollermock.h

    ${CMAKE_CURRENT_LIST_DIR}/filelistmodel_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/convertfiletoscoremodel_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/reviewscorequalitymodel_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cloudscoresmodel_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/recentscoresmodel_tests.cpp
)

set(MODULE_TEST_LINK
    project_qml
    Qt::Qml
)

include(SetupGTest)
