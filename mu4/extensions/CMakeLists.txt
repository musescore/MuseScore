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

set(MODULE extensions)

set(MODULE_QRC extensions.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/extensionsmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/extensionsmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iextensionscontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/iextensionsconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/iextensionunpacker.h
    ${CMAKE_CURRENT_LIST_DIR}/extensionstypes.h
    ${CMAKE_CURRENT_LIST_DIR}/extensionserrors.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/extensionscontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/extensionscontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/extensionsconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/extensionsconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/extensionunpacker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/extensionunpacker.h
    ${CMAKE_CURRENT_LIST_DIR}/view/extensionlistmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/extensionlistmodel.h
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

