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

Item {
    id: root

    required property string title

    property int contentHeight: 30

    property alias navigation: addButton.navigation

    signal clicked()

    anchors.verticalCenter: parent ? parent.verticalCenter : undefined
    anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined

    height: parent ? parent.height : implicitHeight
    width: parent ? parent.width : implicitWidth

    implicitHeight: 38
    implicitWidth: 248

    // Last row of the expanded instrument's card (see LayoutPanelItemDelegate)
    Rectangle {
        id: cardBackground

        x: 4
        width: parent.width - 8
        height: root.contentHeight

        color: ui.theme.textFieldColor
        bottomLeftRadius: 3
        bottomRightRadius: 3

        FlatButton {
            id: addButton

            // 68 = 30 + 4 + 30 + 4 for the visibility and expand buttons and spacing in LayoutPanelItemDelegate,
            // to make sure that the Add button aligns vertically with the text of the item above it
            x: 68
            anchors.verticalCenter: parent.verticalCenter
            height: 26
            margins: 8

            orientation: Qt.Horizontal
            icon: IconCode.PLUS
            text: root.title

            navigation.column: 0
            navigation.accessible.name: root.title

            onClicked: root.clicked()
        }
    }
}
