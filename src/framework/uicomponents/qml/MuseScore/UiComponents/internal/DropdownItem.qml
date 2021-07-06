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

import ".."

Item {

    id: root

    property alias text: labelItem.text

    property bool selected: false

    property alias background: backgroundItem
    property alias label: labelItem

    property color hoveredColor: backgroundItem.color

    signal clicked()

    height: 30
    width: 126

    Rectangle {
        id: backgroundItem
        anchors.fill: parent
        color: "#CFD5DD"
        radius: 4
        opacity: 0.7
    }

    StyledTextLabel {
        id: labelItem
        anchors.fill: parent
        anchors.leftMargin: 12
        horizontalAlignment: Text.AlignLeft
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
    }

    states: [
        State {
            name: "HOVERED"
            when: mouseArea.containsMouse && !mouseArea.pressed

            PropertyChanges {
                target: backgroundItem
                opacity: ui.theme.buttonOpacityHover
                color: root.hoveredColor
            }
        },

        State {
            name: "PRESSED"
            when: mouseArea.pressed

            PropertyChanges {
                target: backgroundItem
                opacity: ui.theme.buttonOpacityHit
            }
        },

        State {
            name: "SELECTED"
            when: root.selected

            PropertyChanges {
                target: backgroundItem
                opacity: ui.theme.accentOpacityHit
                color: ui.theme.accentColor
            }
        }
    ]
}
