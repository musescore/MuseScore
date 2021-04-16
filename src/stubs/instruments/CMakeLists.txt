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

set(MODULE instruments)

set(MODULE_QRC instruments.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/instrumentsstubmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/instrumentsstubmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/instrumentsconfigurationstub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/instrumentsconfigurationstub.h
    ${CMAKE_CURRENT_LIST_DIR}/instrumentsreaderstub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/instrumentsreaderstub.h
    ${CMAKE_CURRENT_LIST_DIR}/instrumentsrepositorystub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/instrumentsrepositorystub.h
    ${CMAKE_CURRENT_LIST_DIR}/selectinstrumentscenariostub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/selectinstrumentscenariostub.h
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
