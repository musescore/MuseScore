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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Item {
    id: root

    property bool isHighlighted: false

    property int keynavRow: 0
    property NavigationPanel navigationPanel: null

    Rectangle {
        anchors.fill: parent

        color: root.isHighlighted ? ui.theme.accentColor : ui.theme.textFieldColor
        opacity: root.isHighlighted ? 0.5 : 1

        border.color: ui.theme.focusColor
        border.width: keynavItem.active ? 2 : 0
    }

    signal clicked()

    NavigationControl {
        id: keynavItem
        name: "InstrumentsTreeItemControl"
        panel: root.navigationPanel
        row: root.keynavRow
        column: 0
        enabled: root.visible

        onActiveChanged: {
            if (active) {
                root.focusActived()
            }
        }
    }

    RowLayout {
        anchors.top: parent.top
        anchors.topMargin: 16
        anchors.bottom: separator.top
        anchors.bottomMargin: 16
        anchors.left: parent.left
        anchors.right: parent.right

        spacing: 2

        Item {
            Layout.fillWidth: true
            Layout.leftMargin: 90 * styleData.depth
            Layout.preferredHeight: childrenRect.height

            FlatButton {
                id: addButton

                anchors.left: parent.left

                width: 24
                height: width

                objectName: "InstrumentsAddStaffBtn"
                navigation.panel: root.navigationPanel
                navigation.row: root.keynavRow
                navigation.column: 1

                icon: IconCode.PLUS
            }

            StyledTextLabel {
                anchors.left: addButton.right
                anchors.leftMargin: 8
                anchors.right: parent.right
                anchors.rightMargin: 8
                anchors.verticalCenter: addButton.verticalCenter

                text: model ? model.itemRole.title : ""
                horizontalAlignment: Text.AlignLeft
            }
        }
    }

    SeparatorLine { id: separator; anchors.bottom: parent.bottom; }

    MouseArea {
        anchors.fill: parent

        onClicked: {
            root.clicked()
        }
    }
}
