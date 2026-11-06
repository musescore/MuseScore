/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

import QtQuick

import Muse.Ui
import Muse.UiComponents

FlatButton {
    id: root

    required property string title
    required property int depth

    property int sideMargin: 12
    property int cardRadius: 0

    anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined

    height: parent ? parent.height : implicitHeight
    width: parent ? parent.width : implicitWidth

    icon: IconCode.PLUS
    text: root.title
    orientation: Qt.Horizontal

    navigation.column: 0

    backgroundItem: Rectangle {
        color: ui.theme.textFieldColor
        bottomLeftRadius: root.cardRadius
        bottomRightRadius: root.cardRadius
    }

    contentItem: Item {
        width: root.width
        height: root.height

        // Looks like a FlatButton's default background
        Rectangle {
            id: chipBackground

            // 70 = 32+2+32+4 for the buttons and spacing in LayoutPanelItemDelegate
            // to make sure that the Add button aligns vertically with the text of the item above it
            x: root.sideMargin + 70 + 12 * root.depth
            anchors.verticalCenter: parent.verticalCenter
            width: chipContent.implicitWidth + 2 * 8
            height: 24

            radius: 3
            color: ui.theme.buttonColor
            opacity: root.mouseArea.pressed ? ui.theme.buttonOpacityHit
                                            : root.mouseArea.containsMouse ? ui.theme.buttonOpacityHover
                                                                           : ui.theme.buttonOpacityNormal
            border.width: ui.theme.borderWidth
            border.color: ui.theme.strokeColor

            NavigationFocusBorder {
                navigationCtrl: root.navigation
            }
        }

        Row {
            id: chipContent

            anchors.centerIn: chipBackground
            spacing: 8

            StyledIconLabel {
                anchors.verticalCenter: parent.verticalCenter
                iconCode: root.icon
            }

            StyledTextLabel {
                anchors.verticalCenter: parent.verticalCenter
                text: root.title
            }
        }
    }
}
