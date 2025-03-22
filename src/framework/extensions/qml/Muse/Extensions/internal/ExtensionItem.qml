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
import QtQuick.Window 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.GraphicalEffects 1.0

Item {
    id: root

    property alias name: nameLabel.text
    property alias thumbnailUrl: thumbnailImage.source
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

    Item {
        id: thumbnail

        anchors.top: parent.top
        width: parent.width
        height: 144

        property real radius: 10

        Image {
            id: thumbnailImage

            anchors.fill: parent

            fillMode: Image.PreserveAspectCrop

            sourceSize: Qt.size(width * Screen.devicePixelRatio, height * Screen.devicePixelRatio)

            layer.enabled: ui.isEffectsAllowed
            layer.effect: EffectOpacityMask {
                maskSource: Rectangle {
                    width: thumbnail.width
                    height: thumbnail.height
                    radius: thumbnail.radius
                }
            }
        }

        Rectangle {
            id: selectionBorder

            readonly property real padding: 2 // add some padding between image and border, to make border better distinguishable

            anchors.fill: parent
            anchors.margins: -border.width - padding

            visible: root.selected

            color: "transparent"

            border.color: ui.theme.fontPrimaryColor
            border.width: 2
            radius: thumbnail.radius - anchors.margins
        }

        NavigationFocusBorder {
            navigationCtrl: root.navigation

            padding: 2
        }
    }

    StyledTextLabel {
        id: nameLabel

        anchors.top: thumbnail.bottom
        anchors.topMargin: 16
        anchors.horizontalCenter: parent.horizontalCenter
    }

    states: [
        State {
            name: "HOVERED"
            when: mouseArea.containsMouse && !mouseArea.pressed && root.enabled

            PropertyChanges {
                target: thumbnail
                opacity: 0.7
            }
        },

        State {
            name: "PRESSED"
            when: mouseArea.pressed && root.enabled

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
