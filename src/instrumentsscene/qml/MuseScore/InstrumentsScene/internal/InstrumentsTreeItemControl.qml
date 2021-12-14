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
import QtQuick
import QtQuick.Layouts

import MuseScore.Ui
import MuseScore.UiComponents

ListItemBlank {
    id: root

    required property QtObject item
    required property int depth

    property int sideMargin: 12

    isSelected: item ? item.isSelected : false
    normalColor: ui.theme.textFieldColor

    navigation.column: 0
    navigation.accessible.name: titleLabel.text

    anchors.fill: parent

    implicitHeight: 38
    implicitWidth: 248

    background.border.width: 0

    onClicked: {
        item.appendNewItem()
    }

    RowLayout {
        anchors.fill: parent

        // 70 = 32+2+32+4 for the buttons and spacing in InstrumentsTreeItemDelegate
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

            text: root.item ? root.item.title : ""
            horizontalAlignment: Text.AlignLeft
        }
    }

    SeparatorLine { anchors.bottom: parent.bottom; }
}
