# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2025 MuseScore BVBA and others
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

declare_module(muse_tours)
set(MODULE_ALIAS muse::tours)

set(MODULE_QRC tours.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/toursmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/toursmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/tourstypes.h
    ${CMAKE_CURRENT_LIST_DIR}/itoursconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/itoursservice.h
    ${CMAKE_CURRENT_LIST_DIR}/itoursprovider.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/toursconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/toursconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/toursservice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/toursservice.h

    ${CMAKE_CURRENT_LIST_DIR}/view/toursprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/toursprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/view/toursprovidermodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/toursprovidermodel.h
    )

setup_module()

if (MUSE_MODULE_TOURS_TESTS)
    # add_subdirectory(tests) # todo
endif()
