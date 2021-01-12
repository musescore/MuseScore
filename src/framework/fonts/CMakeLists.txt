#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2020 MuseScore BVBA and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

set(MODULE fonts)

set(MODULE_QRC
    fonts_Leland.qrc
    fonts_Bravura.qrc
    fonts_Campania.qrc
    fonts_Free.qrc
    fonts_FreeSerif.qrc
    fonts_Gootville.qrc
    fonts_MScore.qrc
    fonts_MuseJazz.qrc
    fonts_Smufl.qrc
    fonts_Tabulature.qrc
    fonts_Petaluma.qrc
    fonts_FiraSans.qrc
    )

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/fontsmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/fontsmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontscontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/fontscontroller.h
    )

set(MODULE_USE_UNITY_NONE ON)
include(${PROJECT_SOURCE_DIR}/build/module.cmake)
