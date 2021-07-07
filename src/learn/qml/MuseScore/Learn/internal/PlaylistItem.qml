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
import QtGraphicalEffects 1.0

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

FocusScope {
    id: root

    property string title: ""
    property string author: ""
    property string duration: ""
    property alias thumbnail: thumbnailItem.source

    property alias navigation: navCtrl

    signal clicked()

    NavigationControl {
        id: navCtrl
        name: root.title

        accessible.role: MUAccessible.Button
        accessible.name: root.title

        onActiveChanged: {
            if (active) {
                root.forceActiveFocus()
            }
        }

        onTriggered: root.clicked()
    }

    Column {
        anchors.fill: parent

        spacing: 8

        Item {
            id: scoreRect

            height: 150
            width: 250

            opacity: 0.9

            property int borderWidth: 0
            readonly property int radius: 3

            Image {
                id: thumbnailItem
                anchors.fill: parent
            }

            Rectangle {
                anchors.top: parent.top

                height: parent.height + parent.borderWidth
                width: parent.width

                color: "transparent"
                radius: parent.radius

                border.color: navCtrl.active ? ui.theme.focusColor : ui.theme.strokeColor
                border.width: navCtrl.active ? 2 : parent.borderWidth
            }

            states: [
                State {
                    name: "HOVERED"
                    when: mouseArea.containsMouse && !mouseArea.pressed

                    PropertyChanges {
                        target: scoreRect
                        opacity: 1
                        borderWidth: 1
                    }
                },

                State {
                    name: "PRESSED"
                    when: mouseArea.pressed

                    PropertyChanges {
                        target: scoreRect
                        opacity: 0.5
                    }
                }
            ]

            RectangularGlow {
                anchors.fill: scoreRect
                z: -1

                glowRadius: 20
                color: "#08000000"
                cornerRadius: scoreRect.radius + glowRadius
            }
        }

        StyledTextLabel {
            anchors.left: parent.left
            width: parent.width

            text: root.title

            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WrapAnywhere
            maximumLineCount: 1

            font: ui.theme.tabBoldFont
        }

        StyledTextLabel {
            anchors.left: parent.left
            width: parent.width

            text: root.author

            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WrapAnywhere
            maximumLineCount: 1

            font: ui.theme.bodyBoldFont
        }

        StyledTextLabel {
            anchors.left: parent.left
            width: parent.width

            text: root.duration

            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WrapAnywhere
            maximumLineCount: 1

            font: ui.theme.bodyFont
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        hoverEnabled: true

        onClicked: {
            root.clicked()
        }
    }
}
