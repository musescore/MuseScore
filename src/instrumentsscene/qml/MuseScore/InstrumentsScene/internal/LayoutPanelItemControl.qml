/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

ListItemBlank {
    id: root

    required property string title
    required property int depth
    
    property int sideMargin: 12

    normalColor: ui.theme.textFieldColor
    navigation.column: 0
    navigation.accessible.name: titleLabel.text

    anchors.verticalCenter: parent ? parent.verticalCenter : undefined
    anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined

    height: parent ? parent.height : implicitHeight
    width: parent ? parent.width : implicitWidth

    implicitHeight: 38
    implicitWidth: 248

    background.border.width: 0

    RowLayout {
        anchors.fill: parent

        // 70 = 32+2+32+4 for the buttons and spacing in LayoutPanelItemDelegate
        // to make sure that the Add button aligns vertically with the text of the item above it
        anchors.leftMargin: root.sideMargin + 70 + 12 * root.depth
        spacing: 4

        FlatButton {
            id: addButton

            Layout.preferredWidth: 24
            Layout.preferredHeight: 24

            icon: IconCode.PLUS
            onClicked: root.clicked(null)
        }

        StyledTextLabel {
            id: titleLabel
            Layout.fillWidth: true

            text: root.title
            horizontalAlignment: Text.AlignLeft
        }
    }

    SeparatorLine { anchors.bottom: parent.bottom; }
}
