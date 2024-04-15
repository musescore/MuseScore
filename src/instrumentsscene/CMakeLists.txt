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

declare_module(instrumentsscene)

set(MODULE_QRC instrumentsscene.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/instrumentsscenemodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/instrumentsscenemodule.h
    ${CMAKE_CURRENT_LIST_DIR}/instrumentsscenetypes.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/selectinstrumentscenario.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/selectinstrumentscenario.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/instrumentsuiactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/instrumentsuiactions.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/instrumentsactionscontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/instrumentsactionscontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/view/instrumentlistmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/instrumentlistmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/abstractinstrumentspaneltreeitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/abstractinstrumentspaneltreeitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/roottreeitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/roottreeitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/parttreeitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/parttreeitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/stafftreeitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/stafftreeitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/staffcontroltreeitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/staffcontroltreeitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/instrumentspaneltreemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/instrumentspaneltreemodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/staffsettingsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/staffsettingsmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/instrumentsettingsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/instrumentsettingsmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/instrumentspanelcontextmenumodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/instrumentspanelcontextmenumodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/instrumentsonscorelistmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/instrumentsonscorelistmodel.h
    )

set(MODULE_LINK
    engraving
    )

setup_module()
