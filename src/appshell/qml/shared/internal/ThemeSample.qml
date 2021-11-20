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
import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    property color strokeColor
    property color backgroundPrimaryColor
    property color backgroundSecondaryColor
    property color fontPrimaryColor
    property color buttonColor
    property color accentColor

    signal clicked

    width: 112
    height: 84

    radius: 4
    color: backgroundPrimaryColor

    RoundedRectangle {
        anchors.fill: parent
        anchors.topMargin: 12
        anchors.leftMargin: 16

        color: root.backgroundSecondaryColor

        border.color: root.strokeColor
        border.width: 1

        bottomRightRadius: borderRect.radius

        Column {
            anchors.fill: parent
            anchors.topMargin: 6
            anchors.bottomMargin: 6
            anchors.leftMargin: 10
            anchors.rightMargin: 10

            spacing: 8

            Rectangle {
                height: 38
                width: parent.width

                radius: 3
                color: root.backgroundPrimaryColor

                border.color: root.strokeColor
                border.width: 1

                Column {
                    anchors.fill: parent
                    anchors.margins: 7

                    spacing: 6

                    Rectangle {
                        width: parent.width
                        height: 4

                        radius: 3
                        color: root.fontPrimaryColor
                    }

                    Rectangle {
                        width: parent.width
                        height: 4

                        radius: 3
                        color: root.fontPrimaryColor
                    }

                    Rectangle {
                        width: parent.width / 2
                        height: 4

                        radius: 3
                        color: root.fontPrimaryColor
                    }
                }
            }

            Row {
                width: parent.width
                height: 12

                spacing: 6

                Rectangle {
                    height: parent.height
                    width: 34

                    radius: 3
                    color: root.accentColor
                }

                Rectangle {
                    height: parent.height
                    width: 34

                    radius: 3
                    color: root.buttonColor
                }
            }
        }
    }

    Rectangle {
        id: borderRect
        anchors.fill: parent
        color: "transparent"
        radius: 4
        border.width: 1
        border.color: mouseArea.containsMouse ? root.accentColor : root.strokeColor
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
    }
}
