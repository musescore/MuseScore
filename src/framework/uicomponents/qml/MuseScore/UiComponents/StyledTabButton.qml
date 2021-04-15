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
import QtQuick 2.9
import QtQuick.Controls 2.2

import MuseScore.UiComponents 1.0

TabButton {
    id: root

    property int sideMargin: 0
    property bool isCurrent: false
    property string backgroundColor: ui.theme.backgroundPrimaryColor

    width: implicitWidth + sideMargin * 2 - 8

    contentItem: StyledTextLabel {
        id: textLabel
        text: root.text
        font: ui.theme.largeBodyFont
        opacity: 0.75
    }

    background: Rectangle {
        implicitHeight: 28

        color: root.backgroundColor

        Rectangle {
            id: selectedRect

            anchors.left: parent.left
            anchors.leftMargin: sideMargin
            anchors.right: parent.right
            anchors.rightMargin: sideMargin
            anchors.bottom: parent.bottom

            height: 2

            visible: isCurrent
            color: ui.theme.accentColor
        }
    }

    states: [
        State {
            name: "HOVERED"
            when: root.hovered && !isCurrent

            PropertyChanges {
                target: contentItem
                opacity: 1
            }
        },

        State {
            name: "SELECTED"
            when: isCurrent

            PropertyChanges {
                target: textLabel
                opacity: 1
                font: ui.theme.largeBodyBoldFont
            }
        }
    ]
}

