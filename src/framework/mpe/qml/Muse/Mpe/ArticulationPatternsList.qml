/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

ListView {
    id: root

    property ArticulationsProfileEditorModel editorModel: null

    property var contextMenuModel: [
        { id: "copy", title: /*qsTrc*/ "Copy pattern data" }
    ]

    implicitWidth: 180
    implicitHeight: contentHeight

    spacing: 4
    clip: true

    ScrollBar.vertical: StyledScrollBar { }

    delegate: Item {
        id: delegateItem

        required property ArticulationPatternItem modelData

        height: 32
        width: root.width

        function selectCurrentItem() {
            modelData.isSelected = true
        }

        Rectangle {
            id: backgroundRect

            anchors.fill: parent

            radius: 2

            color: ui.theme.backgroundPrimaryColor
        }

        RowLayout {
            anchors.fill: delegateItem
            anchors.margins: 4

            StyledTextLabel {
                id: label

                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                Layout.fillWidth: true

                horizontalAlignment: Qt.AlignLeft

                text: delegateItem.modelData.title

                MouseArea {
                    id: mouseArea

                    anchors.fill: label

                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    enabled: label.enabled
                    hoverEnabled: true

                    onClicked: function(mouse) {
                        if (mouse.button === Qt.LeftButton) {
                            delegateItem.selectCurrentItem()
                            return
                        }

                        if (mouse.button === Qt.RightButton && root.editorModel && !delegateItem.modelData.isSelected ) {
                            menuLoader.toggleOpened(root.contextMenuModel, mouseX, mouseY)
                        }
                    }
                }

                StyledMenuLoader {
                    id: menuLoader

                    onHandleMenuItem: function(itemId) {
                        if (itemId !== "copy" || !root.editorModel) {
                            return
                        }

                        root.editorModel.copyPatternDataFromItem(delegateItem.modelData)
                    }
                }
            }

            CheckBox {
                id: checkBox

                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.rightMargin: 12

                checked: delegateItem.modelData.isActive

                onClicked: {
                    delegateItem.modelData.isActive = !delegateItem.modelData.isActive
                    delegateItem.selectCurrentItem()
                }
            }
        }

        states: [
            State {
                name: "SELECTED"

                when: delegateItem.modelData.isSelected

                PropertyChanges {
                    target: backgroundRect

                    color: ui.theme.accentColor
                }
            },

            State {
                name: "HOVERED"
                when: (mouseArea.containsMouse || checkBox.hovered) && !delegateItem.modelData.isSelected && !mouseArea.containsPress

                PropertyChanges {
                    target: backgroundRect
                    color: ui.theme.buttonColor
                    opacity: ui.theme.buttonOpacityHover
                }
            },

            State {
                name: "PRESSED"
                when: (mouseArea.containsPress || checkBox.pressed) && !delegateItem.modelData.isSelected

                PropertyChanges {
                    target: backgroundRect
                    color: ui.theme.buttonColor
                    opacity: ui.theme.buttonOpacityHit
                }
            }
        ]
    }
}
