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

Item {
    id: root

    property alias name: nameLabel.text
    property alias thumbnailUrl: thumbnail.source
    property bool selected: false

    signal clicked()

    property NavigationControl navigation: NavigationControl {
        accessible.role: MUAccessible.ListItem
        accessible.name: root.name
        enabled: root.enabled && root.visible

        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }

        onTriggered: root.clicked()
    }

    Rectangle {
        id: thumbnailRect

        anchors.top: parent.top
        width: parent.width
        height: 144

        color: "transparent"
        radius: 10

        border.color: ui.theme.fontPrimaryColor
        border.width: root.selected ? 2 : 0

        NavigationFocusBorder {
            navigationCtrl: root.navigation
            drawOutsideParent: false
        }

        Image {
            id: thumbnail

            anchors.fill: parent
            anchors.margins: 4 //! NOTE: it is necessary to simplify understanding of which element the user is on when navigating

            fillMode: Image.PreserveAspectCrop

            layer.enabled: true
            layer.effect: OpacityMask {
                maskSource: Rectangle {
                    width: thumbnail.width
                    height: thumbnail.height
                    radius: 10
                }
            }
        }
    }

    StyledTextLabel {
        id: nameLabel

        anchors.top: thumbnailRect.bottom
        anchors.topMargin: 16
        anchors.horizontalCenter: parent.horizontalCenter
    }

    states: [
        State {
            name: "HOVERED"
            when: mouseArea.containsMouse && !mouseArea.pressed

            PropertyChanges {
                target: thumbnail
                opacity: 0.7
            }
        },

        State {
            name: "PRESSED"
            when: mouseArea.pressed

            PropertyChanges {
                target: thumbnail
                opacity: 0.5
            }
        }
    ]

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        hoverEnabled: true

        onClicked: {
            root.clicked()
        }
    }
}
