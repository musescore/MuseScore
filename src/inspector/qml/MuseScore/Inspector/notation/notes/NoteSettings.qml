/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
import MuseScore.Inspector

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

    readonly property NoteheadSettingsModel headModel: model ? model.modelByType(AbstractInspectorModel.TYPE_NOTEHEAD) : null
    readonly property ChordSettingsModel chordModel: model ? model.modelByType(AbstractInspectorModel.TYPE_CHORD) : null
    readonly property StemSettingsModel stemModel: model ? model.modelByType(AbstractInspectorModel.TYPE_STEM) : null
    readonly property HookSettingsModel hookModel: model ? model.modelByType(AbstractInspectorModel.TYPE_HOOK) : null
    readonly property BeamSettingsModel beamModel: model ? model.modelByType(AbstractInspectorModel.TYPE_BEAM) : null
    
    InspectorTabBar {
        id: tabBar

        currentIndex: root.model ? indexByType(root.model.defaultSubModelType) : 0

        function indexByType(modelType) {
            switch (modelType) {
            case AbstractInspectorModel.TYPE_NOTE: return 0
            case AbstractInspectorModel.TYPE_NOTEHEAD: return 0
            case AbstractInspectorModel.TYPE_STEM: return 1
            case AbstractInspectorModel.TYPE_HOOK: return 1
            case AbstractInspectorModel.TYPE_BEAM: return 2
            }

            return 0
        }

        InspectorTabButton {
            text: root.headModel?.title ?? ""

            navigation.name: "HeadTab"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart
        }

        InspectorTabButton {
            visible: root.headModel ? !root.headModel.isTrillCueNote : true
            text: root.stemModel ? root.stemModel.title : ""

            navigation.name: "StemTab"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 1
        }

        InspectorTabButton {
            visible: root.headModel ? !root.headModel.isTrillCueNote : true
            text: root.beamModel ? root.beamModel.title : ""

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
