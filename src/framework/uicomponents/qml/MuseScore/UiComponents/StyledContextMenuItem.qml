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
import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0

MenuItem {
    id: root

    implicitHeight: 30
    implicitWidth: 220

    property var hintIcon: checkable && checked ? IconCode.TICK : IconCode.NONE
    property string shortcut: ""

    background: Rectangle {
        id: background

        anchors.fill: parent

        color: "transparent"
        opacity: 1

        states: [
            State {
                name: "HOVERED"
                when: root.hovered && !root.pressed

                PropertyChanges {
                    target: background
                    color: ui.theme.accentColor
                    opacity: ui.theme.buttonOpacityHover
                }
            },

            State {
                name: "PRESSED"
                when: root.pressed

                PropertyChanges {
                    target: background
                    color: ui.theme.accentColor
                    opacity: ui.theme.buttonOpacityHit
                }
            }
        ]
    }

    indicator: Item {}

    contentItem: RowLayout {
        id: contentRow

        implicitHeight: root.implicitHeight
        implicitWidth: root.implicitWidth

        spacing: 0

        Item {
            readonly property int size: 16

            Layout.preferredWidth: size
            Layout.preferredHeight: size
            Layout.leftMargin: 6
            Layout.rightMargin: 10

            StyledIconLabel {
                iconCode: root.hintIcon
            }
        }

        StyledTextLabel {
            Layout.fillWidth: true
            Layout.fillHeight: true

            text: root.text
            horizontalAlignment: Text.AlignLeft
        }

        StyledTextLabel {
            Layout.alignment: Qt.AlignRight
            Layout.rightMargin: 14

            text: root.shortcut

            visible: Boolean(text)
        }
    }
}
