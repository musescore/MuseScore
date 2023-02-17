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

set(MODULE plugins)

set(MODULE_QRC plugins.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/pluginsmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/pluginsmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/ipluginsservice.h
    ${CMAKE_CURRENT_LIST_DIR}/ipluginsconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/pluginstypes.h
    ${CMAKE_CURRENT_LIST_DIR}/pluginserrors.h
    ${CMAKE_CURRENT_LIST_DIR}/view/pluginsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/pluginsmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/pluginview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/pluginview.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/pluginsconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/pluginsconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/pluginsservice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/pluginsservice.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/pluginsuiactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/pluginsuiactions.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/pluginsactioncontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/pluginsactioncontroller.h

    ${CMAKE_CURRENT_LIST_DIR}/api/apitypes.h
    ${CMAKE_CURRENT_LIST_DIR}/api/cursor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/cursor.h
    ${CMAKE_CURRENT_LIST_DIR}/api/elements.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/elements.h
    ${CMAKE_CURRENT_LIST_DIR}/api/enums.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/enums.h
    ${CMAKE_CURRENT_LIST_DIR}/api/excerpt.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/excerpt.h
    ${CMAKE_CURRENT_LIST_DIR}/api/fraction.h
    ${CMAKE_CURRENT_LIST_DIR}/api/instrument.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/instrument.h
    ${CMAKE_CURRENT_LIST_DIR}/api/part.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/part.h
    ${CMAKE_CURRENT_LIST_DIR}/api/playevent.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/playevent.h
    ${CMAKE_CURRENT_LIST_DIR}/api/qmlplugin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/qmlplugin.h
    ${CMAKE_CURRENT_LIST_DIR}/api/qmlpluginapi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/qmlpluginapi.h
    ${CMAKE_CURRENT_LIST_DIR}/api/score.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/score.h
    ${CMAKE_CURRENT_LIST_DIR}/api/scoreelement.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/scoreelement.h
    ${CMAKE_CURRENT_LIST_DIR}/api/selection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/selection.h
    ${CMAKE_CURRENT_LIST_DIR}/api/style.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/style.h
    ${CMAKE_CURRENT_LIST_DIR}/api/tie.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/tie.h
    ${CMAKE_CURRENT_LIST_DIR}/api/util.cpp
    ${CMAKE_CURRENT_LIST_DIR}/api/util.h
    )

set(MODULE_LINK
    uicomponents
    engraving
    )

include(SetupModule)

if (MUE_BUILD_UNIT_TESTS)
    add_subdirectory(tests)
endif()
