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

set(MODULE fonts)

set(MODULE_QRC
    fonts_Leland.qrc
    fonts_Bravura.qrc
    fonts_Campania.qrc
    fonts_Edwin.qrc
    fonts_Free.qrc
    fonts_FreeSerif.qrc
    fonts_Gootville.qrc
    fonts_Leland.qrc
    fonts_MScore.qrc
    fonts_MuseJazz.qrc
    fonts_Smufl.qrc
    fonts_Tabulature.qrc
    fonts_Petaluma.qrc
    )

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/fontsmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/fontsmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontssetup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontssetup.h
    )

set(MODULE_USE_UNITY_NONE ON)
include(${PROJECT_SOURCE_DIR}/build/module.cmake)
