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
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import "../../../common"

Column {
    id: root

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 0
    property int navigationRowEnd: toBackButton.navigation.row

    signal pushBackwardsRequested()
    signal pushForwardsRequested()
    signal pushToBackRequested()
    signal pushToFrontRequested()

    height: implicitHeight
    width: parent.width

    spacing: 12

    Column {
        width: parent.width

        spacing: 8

        StyledTextLabel {
            width: parent.width
            text: qsTrc("inspector", "Arrange")
            horizontalAlignment: Text.AlignLeft
        }

        Item {
            height: childrenRect.height
            width: parent.width

            FlatButton {
                id: forwardsButton
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 4

                navigation.name: "Forwards"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRowStart + 1

                text: qsTrc("inspector", "Forwards")

                onClicked: {
                    root.pushForwardsRequested()
                }
            }

            FlatButton {
                id: toFrontButton
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 4
                anchors.right: parent.right

                navigation.name: "To front"
                navigation.panel: root.navigationPanel
                navigation.row: forwardsButton.navigation.row + 1

                text: qsTrc("inspector", "To front")

                onClicked: {
                    root.pushToFrontRequested()
                }
            }
        }

        Item {
            height: childrenRect.height
            width: parent.width

            FlatButton {
                id: backwardsButton
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 4

                navigation.name: "Backwards"
                navigation.panel: root.navigationPanel
                navigation.row: toFrontButton.navigation.row + 1

                text: qsTrc("inspector", "Backwards")

                onClicked: {
                    root.pushBackwardsRequested()
                }
            }

            FlatButton {
                id: toBackButton
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 4
                anchors.right: parent.right

                navigation.name: "To back"
                navigation.panel: root.navigationPanel
                navigation.row: backwardsButton.navigation.row + 1

                text: qsTrc("inspector", "To back")

                onClicked: {
                    root.pushToBackRequested()
                }
            }
        }
    }
}
