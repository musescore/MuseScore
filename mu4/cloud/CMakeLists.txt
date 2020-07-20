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

set(MODULE cloud)

set(MODULE_QRC cloud.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/cloudmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cloudmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iaccountcontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/imp3exporter.h
    ${CMAKE_CURRENT_LIST_DIR}/cloudtypes.h
    ${CMAKE_CURRENT_LIST_DIR}/view/accountmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/accountmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/accountcontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/accountcontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/cloudmanager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/cloudmanager.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/cloudmanager_p.h
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

