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
    ${CMAKE_CURRENT_LIST_DIR}/view/selectableitemlistmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/selectableitemlistmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/qmllistproperty.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/qmllistproperty.h
    ${CMAKE_CURRENT_LIST_DIR}/view/popupview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/popupview.h
    ${CMAKE_CURRENT_LIST_DIR}/view/popupwindow/ipopupwindow.h
    ${CMAKE_CURRENT_LIST_DIR}/view/popupwindow/popupwindow_qquickview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/popupwindow/popupwindow_qquickview.h
    ${CMAKE_CURRENT_LIST_DIR}/view/dialogview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/dialogview.h
    ${CMAKE_CURRENT_LIST_DIR}/view/filepickermodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/filepickermodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/radiobuttongroupbox.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/radiobuttongroupbox.h
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)


