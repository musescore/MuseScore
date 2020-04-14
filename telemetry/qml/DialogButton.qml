//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

import QtQuick 2.0

FocusableItem {
    id: root

    property alias text: buttonLabel.text
    property alias pressed: clickableArea.pressed

    signal clicked

    height: 24
    width: 252

    Keys.onPressed: {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {

            root.clicked()

            event.accepted = true
        }
    }

    Accessible.role: Accessible.Button
    Accessible.name: root.text

    Rectangle {
        id: backgroundRect

        anchors.fill: parent

        border.color: "#a2a2a2"
        border.width: 1

        radius: 3

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: "#E5E9EF"
            }
            GradientStop {
                position: 1.0
                color: "#C4C9CD"
            }
        }
    }

    Text {
        id: buttonLabel

        anchors.centerIn: root

        font.pixelSize: 13

        color: globalStyle.buttonText
    }

    MouseArea {
        id: clickableArea

        anchors.fill: parent

        onClicked: {
            root.clicked()
        }
    }

    states: [
        State {
            name: "PRESSED"
            when: root.pressed

            PropertyChanges {
                target: backgroundRect

                color: "#C4C9CD"
                gradient: undefined
            }
        }
    ]
}
