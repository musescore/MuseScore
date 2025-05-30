# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited
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

declare_module(appshell)

set(MODULE_QRC appshell.qrc)
set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/appshellmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/appshellmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iappshellconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/appshelltypes.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/applicationuiactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/applicationuiactions.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/appshellconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/appshellconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/applicationactioncontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/applicationactioncontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/istartupscenario.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/startupscenario.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/startupscenario.h
    ${CMAKE_CURRENT_LIST_DIR}/view/appmenumodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/appmenumodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/notationpagemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/notationpagemodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/notationstatusbarmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/notationstatusbarmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/navigableappmenumodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/navigableappmenumodel.h
    )

set(MODULE_LINK
    muse::ui
    muse::uicomponents
    )

# we use QKeyMapper to fix https://github.com/musescore/MuseScore/issues/10181
set(MODULE_INCLUDE_PRIVATE ${Qt6Gui_PRIVATE_INCLUDE_DIRS} )

if (QT_SUPPORT)
    list(APPEND MODULE_LINK Qt::Quick)
endif()

setup_module()
