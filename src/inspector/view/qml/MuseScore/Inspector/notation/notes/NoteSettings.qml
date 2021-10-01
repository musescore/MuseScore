/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
import QtQuick 2.9
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../common"

TabPanel {
    id: root

    property QtObject model: null

    property int navigationRowOffset: 1

    objectName: "NoteSettings"

    implicitHeight: Math.max(beamTab.visible ? beamTab.implicitHeight : 0,
                             headTab.visible ? headTab.implicitHeight : 0,
                             stemTab.visible ? stemTab.implicitHeight : 0) + tabBarHeight + 24
    width: parent ? parent.width : 0

    currentIndex: root.model ? indexByType(root.model.preferedSubModelType) : 0

    function indexByType(modelType) {
        switch (modelType) {
        case Inspector.TYPE_NOTE: return 0
        case Inspector.TYPE_NOTEHEAD: return 0
        case Inspector.TYPE_STEM: return 1
        case Inspector.TYPE_HOOK: return 1
        case Inspector.TYPE_BEAM: return 2
        }

        return 0
    }

    function focusOnFirst() {
        headTab.navigation.requestActive()
    }

    TabItem {
        id: headTab

        readonly property QtObject headModel: root.model ? root.model.modelByType(Inspector.TYPE_NOTEHEAD) : null

        width: root.width

        title: headModel ? headModel.title : ""
        checked: root.currentIndex === 0

        navigation.name: "HeadTab"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRowOffset
        onNavigationTriggered: root.currentIndex = 0

        HeadSettings {
            id: headSettings

            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width

            model: headTab.headModel

            enabled: headTab.checked

            navigationPanel: root.navigationPanel
            navigationRowOffset: root.navigationRowOffset + 1000
        }
    }

    TabItem {
        id: stemTab

        readonly property QtObject stemModel: root.model ? root.model.modelByType(Inspector.TYPE_STEM) : null
        readonly property QtObject hookModel: root.model ? root.model.modelByType(Inspector.TYPE_HOOK) : null
        readonly property QtObject beamModel: root.model ? root.model.modelByType(Inspector.TYPE_BEAM) : null

        height: implicitHeight
        width: root.width

        title: stemModel ? stemModel.title : ""
        checked: root.currentIndex === 1

        navigation.name: "StemTab"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRowOffset + 1
        onNavigationTriggered: root.currentIndex = 1

        StemSettings {
            id: stemSettings

            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width

            stemModel: stemTab.stemModel
            hookModel: stemTab.hookModel
            beamModel: stemTab.beamModel

            enabled: stemTab.checked

            navigationPanel: root.navigationPanel
            navigationRowOffset: root.navigationRowOffset + 2000
        }
    }

    TabItem {
        id: beamTab

        readonly property QtObject beamModel: root.model ? root.model.modelByType(Inspector.TYPE_BEAM) : null

        width: root.width

        title: beamModel ? beamModel.title : ""
        checked: root.currentIndex === 2

        navigation.name: "BeamTab"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRowOffset + 2
        onNavigationTriggered: root.currentIndex = 2

        BeamSettings {
            id: beamSettings

            anchors.top: parent.top
            anchors.topMargin: 24

            width: root.width

            model: beamTab.beamModel

            enabled: beamTab.checked

            navigationPanel: root.navigationPanel
            navigationRowOffset: root.navigationRowOffset + 3000
        }
    }
}
