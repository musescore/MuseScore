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
import QtQuick 2.15
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

Item {
    id: root

    property int sideMargin: 0
    property int buttonsMargin: 0

    property bool isRemovingAvailable: false

    signal createNewPartRequested()
    signal removeSelectedPartsRequested()

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "PartsControlPanel"
        enabled: root.enabled && root.visible
        direction: NavigationPanel.Horizontal
        accessible.name: qsTrc("notation", "Parts control")
        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }
    }

    StyledTextLabel {
        anchors.left: parent.left
        anchors.leftMargin: root.sideMargin

        text: qsTrc("notation", "Parts")
        font: ui.theme.headerBoldFont
    }

    FlatButton {
        text: qsTrc("notation", "Create new part")

        anchors.right: deleteButton.left
        anchors.rightMargin: 8

        navigation.name: "CreateNewPartButton"
        navigation.panel: root.navigationPanel
        navigation.column: 0

        onClicked: {
            root.createNewPartRequested()
        }
    }

    FlatButton {
        id: deleteButton

        anchors.right: parent.right
        anchors.rightMargin: root.buttonsMargin

        icon: IconCode.DELETE_TANK

        enabled: root.isRemovingAvailable

        navigation.name: "DeleteButton"
        navigation.panel: root.navigationPanel
        navigation.column: 1
        navigation.accessible.name: qsTrc("uicomponents", "Delete")

        onClicked: {
            root.removeSelectedPartsRequested()
        }
    }
}
