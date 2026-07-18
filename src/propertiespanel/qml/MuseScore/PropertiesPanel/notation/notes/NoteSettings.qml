/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick
import QtQuick.Layouts

import Muse.UiComponents
import Muse.Ui
import MuseScore.PropertiesPanel

import "../../common"

Column {
    id: root

    required property NoteSettingsProxyModel model

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "NoteSettings"

    width: parent.width
    spacing: 12

    function focusOnFirst() {
        tabBar.focusOnCurrentTab()
    }

    readonly property NoteheadSettingsModel headModel: model ? model.modelByType(PropertiesPanelAbstractModel.TYPE_NOTEHEAD) : null
    readonly property ChordSettingsModel chordModel: model ? model.modelByType(PropertiesPanelAbstractModel.TYPE_CHORD) : null
    readonly property StemSettingsModel stemModel: model ? model.modelByType(PropertiesPanelAbstractModel.TYPE_STEM) : null
    readonly property HookSettingsModel hookModel: model ? model.modelByType(PropertiesPanelAbstractModel.TYPE_HOOK) : null
    readonly property BeamSettingsModel beamModel: model ? model.modelByType(PropertiesPanelAbstractModel.TYPE_BEAM) : null
    
    PropertiesPanelTabBar {
        id: tabBar

        currentIndex: root.model ? indexByType(root.model.defaultSubModelType) : 0

        function indexByType(modelType) {
            switch (modelType) {
            case PropertiesPanelAbstractModel.TYPE_NOTE: return 0
            case PropertiesPanelAbstractModel.TYPE_NOTEHEAD: return 0
            case PropertiesPanelAbstractModel.TYPE_STEM: return 1
            case PropertiesPanelAbstractModel.TYPE_HOOK: return 1
            case PropertiesPanelAbstractModel.TYPE_BEAM: return 2
            }

            return 0
        }

        PropertiesPanelTabButton {
            text: root.headModel?.title ?? ""
            maxWidth: tabBar.truncatedItemWidth

            navigation.name: "HeadTab"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart
        }

        PropertiesPanelTabButton {
            visible: root.headModel ? !root.headModel.isTrillCueNote : true
            text: root.stemModel ? root.stemModel.title : ""
            maxWidth: tabBar.truncatedItemWidth

            navigation.name: "StemTab"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 1
        }

        PropertiesPanelTabButton {
            visible: root.headModel ? !root.headModel.isTrillCueNote : true
            text: root.beamModel ? root.beamModel.title : ""
            maxWidth: tabBar.truncatedItemWidth

            navigation.name: "BeamTab"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 2
        }
    }

    StackLayout {
        width: parent.width
        currentIndex: tabBar.currentIndex

        height: itemAt(currentIndex).implicitHeight

        HeadSettings {
            height: implicitHeight

            headModel: root.headModel
            chordModel: root.chordModel

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 1000
        }

        StemSettings {
            height: implicitHeight

            chordModel: root.chordModel
            stemModel: root.stemModel
            hookModel: root.hookModel

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 2000
        }

        BeamSettings {
            height: implicitHeight

            model: root.beamModel

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 3000
        }
    }
}
