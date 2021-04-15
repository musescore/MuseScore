/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
import QtQuick 2.7
import QtGraphicalEffects 1.0

import MuseScore.UiComponents 1.0

Item {
    id: root

    property alias name: nameLabel.text
    property alias thumbnailUrl: thumbnail.source
    property bool selected: false

    signal clicked()

    Image {
        id: thumbnail

        anchors.top: parent.top

        width: parent.width
        height: 142

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

    Rectangle {
        anchors.fill: thumbnail

        color: "transparent"
        radius: 10

        border.color: ui.theme.fontPrimaryColor
        border.width: root.selected ? 2 : 0
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
            when: mouseArea.containsMouse && !mouseArea.pressed

            PropertyChanges {
                target: root
                opacity: 0.7
            }
        },

        State {
            name: "PRESSED"
            when: mouseArea.pressed

            PropertyChanges {
                target: root
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
