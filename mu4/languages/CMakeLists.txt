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

set(MODULE languages)

set(MODULE_QRC languages.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/languagesmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/languagesmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/languagestypes.h
    ${CMAKE_CURRENT_LIST_DIR}/languageserrors.h
    ${CMAKE_CURRENT_LIST_DIR}/ilanguagesconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/ilanguagescontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/ilanguageunpacker.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/languagesconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/languagesconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/languagescontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/languagescontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/languageunpacker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/languageunpacker.h
    ${CMAKE_CURRENT_LIST_DIR}/view/languagelistmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/languagelistmodel.h
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

