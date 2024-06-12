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

    objectName: "LineSettings"

    width: parent.width
    spacing: 12

    function focusOnFirst() {
        tabBar.focusOnCurrentTab()
    }

    InspectorTabBar {
        id: tabBar

        InspectorTabButton {
            text: qsTrc("inspector", "Position")

            navigation.name: "PositionTab"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart
        }

        InspectorTabButton {
            text: qsTrc("inspector", "Style")

            navigation.name: "StyleTab"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 1
        }

        InspectorTabButton {
            text: qsTrc("inspector", "Text")

            navigation.name: "TextTab"
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart + 2
        }
    }

    StackLayout {
        width: parent.width
        currentIndex: tabBar.currentIndex

        height: itemAt(currentIndex).implicitHeight

        GradualTempoChangePositionSettingsTab {
            height: implicitHeight

            model: root.model

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 3
        }

        GradualTempoChangeStyleTab {
            model: root.model

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 1000
        }

        LineTextSettingsTab {
            height: implicitHeight

            model: root.model

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 2000
        }
    }
}
