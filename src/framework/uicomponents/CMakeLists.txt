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

set(MODULE uicomponents)

set(MODULE_QRC uicomponents.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/uicomponentsmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/uicomponentsmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/view/sampleobject.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/sampleobject.h
    ${CMAKE_CURRENT_LIST_DIR}/view/validators/doubleinputvalidator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/validators/doubleinputvalidator.h
    ${CMAKE_CURRENT_LIST_DIR}/view/validators/intinputvalidator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/validators/intinputvalidator.h
    ${CMAKE_CURRENT_LIST_DIR}/view/iconview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/iconview.h
    ${CMAKE_CURRENT_LIST_DIR}/view/sortfilterproxymodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/sortfilterproxymodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/filtervalue.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/filtervalue.h
    ${CMAKE_CURRENT_LIST_DIR}/view/sortervalue.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/sortervalue.h
    ${CMAKE_CURRENT_LIST_DIR}/view/itemmultiselectionmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/itemmultiselectionmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/qmllistproperty.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/qmllistproperty.h
    ${CMAKE_CURRENT_LIST_DIR}/view/popupview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/popupview.h
    ${CMAKE_CURRENT_LIST_DIR}/view/filepickermodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/filepickermodel.h
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)


