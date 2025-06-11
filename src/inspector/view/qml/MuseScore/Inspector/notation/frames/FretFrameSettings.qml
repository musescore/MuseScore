/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
import QtQuick 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../common"
import "internal"

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "FretFrameSettings"

    height: implicitHeight
    spacing: 12

    function focusOnFirst() {
        tabBar.focusOnCurrentTab()
    }

    readonly property QtObject frameChordsModel: model ? model.modelByType(Inspector.TYPE_FRET_FRAME_CHORDS) : null
    readonly property QtObject frameSettingsModel: model ? model.modelByType(Inspector.TYPE_FRET_FRAME_SETTINGS) : null

    InspectorTabBar {
        id: tabBar

        property bool isChordsTabActive: currentIndex === 0

        InspectorTabButton {
            text: qsTrc("inspector", "Chords")

            navigation.name: "ChordsTab"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart
        }

        InspectorTabButton {
            text: qsTrc("inspector", "Frame")

            navigation.name: "FrameTab"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 1
        }
    }

    StackLayout {
        id: stackLayout

        readonly property int sideMargin: -12

        anchors.left: parent.left
        anchors.leftMargin: tabBar.isChordsTabActive ? sideMargin : 0
        anchors.right: parent.right
        anchors.rightMargin: tabBar.isChordsTabActive ? sideMargin : 0

        currentIndex: tabBar.currentIndex

        height: itemAt(currentIndex).implicitHeight

        FretFrameChordsTab {
            height: implicitHeight

            model: root.frameChordsModel

            sideMargin: -stackLayout.sideMargin

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 1000
        }

        FretFrameSettingsTab {
            height: implicitHeight

            model: root.frameSettingsModel

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 2000
        }
    }
}
