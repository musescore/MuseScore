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

set(MODULE scores)

set(MODULE_QRC scores.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/scoresmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/scoresmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iopenscorecontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/iscoresconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/view/recentscoresmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/recentscoresmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/newscoremodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/newscoremodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/scorethumbnail.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/scorethumbnail.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/openscorecontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/openscorecontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/scoresconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/scoresconfiguration.h
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

