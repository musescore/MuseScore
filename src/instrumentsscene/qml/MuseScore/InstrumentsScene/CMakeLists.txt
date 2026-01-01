# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

muse_create_qml_module(instrumentsscene_qml FOR instrumentsscene)

qt_add_qml_module(instrumentsscene_qml
    URI MuseScore.InstrumentsScene
    VERSION 1.0
    SOURCES
        internal/abstractlayoutpaneltreeitem.cpp
        internal/abstractlayoutpaneltreeitem.h
        internal/instrumentlistmodel.cpp
        internal/instrumentlistmodel.h
        internal/instrumentsettingsmodel.cpp
        internal/instrumentsettingsmodel.h
        internal/instrumentsonscorelistmodel.cpp
        internal/instrumentsonscorelistmodel.h
        internal/layoutpanelcontextmenumodel.cpp
        internal/layoutpanelcontextmenumodel.h
        internal/layoutpanelitemtype.h
        internal/layoutpaneltreemodel.cpp
        internal/layoutpaneltreemodel.h
        internal/layoutpanelutils.h
        internal/parttreeitem.cpp
        internal/parttreeitem.h
        internal/roottreeitem.cpp
        internal/roottreeitem.h
        internal/staffcontroltreeitem.cpp
        internal/staffcontroltreeitem.h
        internal/staffsettingsmodel.cpp
        internal/staffsettingsmodel.h
        internal/stafftreeitem.cpp
        internal/stafftreeitem.h
        internal/systemobjectslayersettingsmodel.cpp
        internal/systemobjectslayersettingsmodel.h
        internal/systemobjectslayertreeitem.cpp
        internal/systemobjectslayertreeitem.h
    QML_FILES
        ChooseInstrumentsPage.qml
        InstrumentsDialog.qml
        internal/FamilyView.qml
        internal/InstrumentSettingsPopup.qml
        internal/InstrumentsOnScoreView.qml
        internal/InstrumentsView.qml
        internal/LayoutControlPanel.qml
        internal/LayoutPanelAddButton.qml
        internal/LayoutPanelItemControl.qml
        internal/LayoutPanelItemDelegate.qml
        internal/StaffSettingsPopup.qml
        internal/SystemObjectsLayerSettingsPopup.qml
        LayoutPanel.qml
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
        TARGET muse_legacytreeview_qml
)

fixup_qml_module_dependencies(instrumentsscene_qml)

# Necessary for the auto-generated sources
target_include_directories(instrumentsscene_qml PRIVATE internal)
