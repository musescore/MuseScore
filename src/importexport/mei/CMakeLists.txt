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

set(MODULE iex_mei)

set(MODULE_INCLUDE
    ${CMAKE_CURRENT_LIST_DIR}/internal/
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/libmei/
    )

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/imeiconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/meimodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/meimodule.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/meiconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/meiconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/meiexporter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/meiexporter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/meiimporter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/meiimporter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/meireader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/meireader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/meiwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/meiwriter.h
    )

set(MODULE_LINK
    engraving
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

