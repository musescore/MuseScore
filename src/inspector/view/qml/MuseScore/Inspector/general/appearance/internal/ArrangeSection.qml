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
    property int navigationRowOffset: 0

    signal pushBackRequested()
    signal pushFrontRequested()

    height: implicitHeight
    width: parent.width

    spacing: 16

    Column {
        width: parent.width

        spacing: 8

        StyledTextLabel {
            text: qsTrc("inspector", "Arrange")
        }

        Item {
            height: childrenRect.height
            width: parent.width

            FlatButton {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 4

                navigation.name: "Backwards"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRowOffset + 1

                text: qsTrc("inspector", "Backwards")

                onClicked: {
                    root.pushBackRequested()
                }
            }

            FlatButton {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 4
                anchors.right: parent.right

                navigation.name: "Forwards"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRowOffset + 2

                text: qsTrc("inspector", "Forwards")

                onClicked: {
                    root.pushFrontRequested()
                }
            }
        }
    }
}
