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

declare_module(muse_uicomponents)
set(MODULE_ALIAS muse::uicomponents)

set(MODULE_QRC uicomponents.qrc graphicaleffects.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )
set(MODULE_QMLAPI_IMPORT ${CMAKE_CURRENT_LIST_DIR}/api )

include(${CMAKE_CURRENT_LIST_DIR}/view/treeview/treeview.cmake)

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
    ${CMAKE_CURRENT_LIST_DIR}/view/modelutils.h
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
    ${CMAKE_CURRENT_LIST_DIR}/view/dropdownview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/dropdownview.h
    ${CMAKE_CURRENT_LIST_DIR}/view/menuview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/menuview.h
    ${CMAKE_CURRENT_LIST_DIR}/view/topleveldialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/topleveldialog.h
    ${CMAKE_CURRENT_LIST_DIR}/view/filepickermodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/filepickermodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/colorpickermodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/colorpickermodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/textinputmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/textinputmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/abstractmenumodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/abstractmenumodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/menuitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/menuitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/abstracttoolbarmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/abstracttoolbarmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/toolbaritem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/toolbaritem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/popupviewclosecontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/popupviewclosecontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/radiobuttongroupbox.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/widgets/radiobuttongroupbox.h
    ${CMAKE_CURRENT_LIST_DIR}/view/selectmultipledirectoriesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/selectmultipledirectoriesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/quickpaintedview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/quickpaintedview.h
    ${CMAKE_CURRENT_LIST_DIR}/view/widgetview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/widgetview.h
    ${CMAKE_CURRENT_LIST_DIR}/view/buttonboxmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/buttonboxmodel.cpp
    ${TREEVIEW_SRC}
    )

if (OS_IS_MAC)
    set(MODULE_SRC ${MODULE_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/view/internal/platform/macos/macospopupviewclosecontroller.mm
        ${CMAKE_CURRENT_LIST_DIR}/view/internal/platform/macos/macospopupviewclosecontroller.h
        )

    set_source_files_properties(
        ${CMAKE_CURRENT_LIST_DIR}/view/internal/platform/macos/macospopupviewclosecontroller.mm
        PROPERTIES
        SKIP_UNITY_BUILD_INCLUSION ON
        SKIP_PRECOMPILE_HEADERS ON
        )
endif(OS_IS_MAC)

if (OS_IS_WIN)
    set(MODULE_SRC ${MODULE_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/view/internal/platform/win/winpopupviewclosecontroller.cpp
        ${CMAKE_CURRENT_LIST_DIR}/view/internal/platform/win/winpopupviewclosecontroller.h
        )
endif(OS_IS_WIN)

if (MUSE_MODULE_UI_DISABLE_MODALITY)
    set(MODULE_DEF ${MODULE_DEF} -DMUSE_MODULE_UI_DISABLE_MODALITY)
endif()

if(QT_SUPPORT)
    list(APPEND MODULE_LINK Qt::Widgets Qt::Quick)
endif()

set(MODULE_USE_UNITY OFF)

setup_module()

if (MUSE_MODULE_UI_TESTS)
    add_subdirectory(tests)
endif()
