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
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

StyledDialogView {

    id: root

    property color color: "#444444"
    property bool isApplyColor: false

    contentWidth: 400
    contentHeight: 400

    title: "Sample dialog"

    Rectangle {
        anchors.fill: parent
        color: root.isApplyColor ? root.color : "#666666"

        Column {
            anchors.centerIn: parent

            spacing: 50

            TextInputField {
                id: input
                anchors.horizontalCenter: parent.horizontalCenter

                property string value: ""
                width: 150
                onCurrentTextEdited: input.value = newTextValue
            }

            StyledTextLabel {
                anchors.horizontalCenter: parent.horizontalCenter

                text: "Use right click for showing context menu"
            }
        }

        Row {
            anchors.bottom: parent.bottom
            anchors.right: parent.right

            anchors.rightMargin: 16
            anchors.bottomMargin: 20
            spacing: 20

            FlatButton {
                text: "Cancel"
                onClicked: {
                    root.reject()
                }
            }

            FlatButton {
                text: "OK"
                onClicked: {
                    root.ret = {errcode: 0, value: input.value }
                    root.hide()
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            onClicked: menu.popup()
        }

        ContextMenu {
            id: menu

            StyledContextMenuItem {
                hintIcon: IconCode.UNDO

                text: "Undo"
                shortcut: "Ctrl+Z"
            }

            StyledContextMenuItem {
                hintIcon: IconCode.REDO

                text: "Redo"
                shortcut: "Shift+Ctrl+Z"

                enabled: false
            }

            SeparatorLine {}

            StyledContextMenuItem {
                hintIcon: IconCode.ZOOM_IN

                text: "Zoom in"
            }

            StyledContextMenuItem {
                hintIcon: IconCode.ZOOM_OUT

                text: "Zoom out"
            }

            SeparatorLine {}

            StyledContextMenuItem {
                text: "Checkable"

                checkable: true
                checked: false
            }
        }
    }
}
