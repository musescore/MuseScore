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
import MuseScore.Project 1.0

FocusScope {
    id: root

    property string title: ""
    property alias timeSinceModified: timeSinceModified.text
    property alias thumbnail: loader.thumbnail
    property bool isAdd: false

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

        spacing: 16

        Item {
            id: scoreRect

            height: 224
            width: 172

            opacity: 0.9

            property int borderWidth: 0
            readonly property int radius: 3

            Loader {
                id: loader

                anchors.fill: parent

                property var thumbnail: undefined

                sourceComponent: root.isAdd ? addComp : thumbnailComp

                onLoaded: {
                    if (!root.isAdd) {
                        item.setThumbnail(root.thumbnail)
                    }
                }

                layer.enabled: true
                layer.effect: OpacityMask {
                    maskSource: Rectangle {
                        width: scoreRect.width
                        height: scoreRect.height
                        radius: scoreRect.radius
                    }
                }
            }

            Rectangle {
                anchors.top: parent.top

                height: parent.height + parent.borderWidth
                width: parent.width

                color: "transparent"
                radius: parent.radius

                NavigationFocusBorder { navigationCtrl: navCtrl }

                border.color: ui.theme.strokeColor
                border.width: parent.borderWidth
            }

            states: [
                State {
                    name: "NORMAL"
                    when: !mouseArea.containsMouse && !mouseArea.pressed

                    PropertyChanges {
                        target: scoreRect
                        borderWidth: ui.theme.borderWidth
                    }
                },

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

        Column {
            anchors.left: parent.left
            anchors.right: parent.right

            spacing: 4

            StyledTextLabel {
                anchors.horizontalCenter: parent.horizontalCenter

                text: root.title

                wrapMode: Text.WrapAnywhere
                maximumLineCount: 1
                width: parent.width

                font: ui.theme.largeBodyFont
            }

            StyledTextLabel {
                id: timeSinceModified

                anchors.horizontalCenter: parent.horizontalCenter

                font.capitalization: Font.AllUppercase

                visible: !root.isAdd
            }
        }
    }

    Component {
        id: thumbnailComp

        ScoreThumbnail {
            anchors.fill: parent
        }
    }

    Component {
        id: addComp

        Rectangle {
            anchors.fill: parent

            color: "white"

            StyledIconLabel {
                anchors.centerIn: parent

                iconCode: IconCode.PLUS

                font.pixelSize: 50
                color: "black"
            }
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
