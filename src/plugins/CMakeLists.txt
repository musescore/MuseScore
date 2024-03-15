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
    )

if (MUE_DISABLE_PLUGINS_TYPES)
    set(MODULE_DEF -DMUE_DISABLE_PLUGINS_TYPES)
endif()

set(MODULE_LINK
    uicomponents
    engraving
    )

include(SetupModule)

if (MUE_BUILD_PLUGINS_TESTS)
    add_subdirectory(tests)
endif()
