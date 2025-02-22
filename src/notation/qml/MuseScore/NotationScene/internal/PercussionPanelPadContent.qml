/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.NotationScene 1.0

Column {
    id: root

    property var padModel: null

    property int panelMode: -1
    property bool useNotationPreview: false

    property alias footerHeight: footerArea.height

    property bool padSwapActive: false

    function openContextMenu(pos) {
        if (!root.padModel) {
            return
        }

        if (!pos) {
            pos = menuLoader.parent.mapFromItem(root, 0, root.height)
        }

        menuLoader.show(pos, root.padModel.contextMenuItems)
    }

    Item {
        id: mainContentArea

        width: parent.width
        height: parent.height - separator.height - footerArea.height

        MouseArea {
            id: mouseArea
            anchors.fill: parent

            enabled: mainContentArea.enabled
            hoverEnabled: true

            acceptedButtons: Qt.LeftButton | Qt.RightButton

            onPressed: function(event) {
                ui.tooltip.hide(root)

                if (!Boolean(root.padModel)) {
                    return
                }

                if (event.button === Qt.RightButton) {
                    let pos = menuLoader.parent.mapFromItem(mouseArea, event.x, event.y)
                    root.openContextMenu(pos)
                    return
                }

                root.padModel.triggerPad(event.modifiers)
            }

            onContainsMouseChanged: {
                if (!Boolean(root.padModel)) {
                    ui.tooltip.hide(root)
                    return
                }

                if (mouseArea.containsMouse && root.useNotationPreview) {
                    ui.tooltip.show(root, root.padModel.padName)
                } else {
                    ui.tooltip.hide(root)
                }
            }
        }

        Rectangle {
            id: padNameBackground

            visible: !root.useNotationPreview
            anchors.fill: parent

            color: Utils.colorWithAlpha(ui.theme.accentColor, ui.theme.buttonOpacityNormal)
        }

        StyledTextLabel {
            id: padNameLabel

            visible: !root.useNotationPreview

            anchors.centerIn: parent
            width: parent.width - 12

            wrapMode: Text.WordWrap
            maximumLineCount: 4
            font: ui.theme.bodyBoldFont

            text: Boolean(root.padModel) ? root.padModel.padName : ""
        }

        PaintedEngravingItem {
            id: notationPreview

            visible: root.useNotationPreview

            anchors.fill: parent

            engravingItem: Boolean(root.padModel) ? root.padModel.notationPreviewItem : null
            spatium: 6.25 // Value approximated visually (needs to accomodate "extreme ledger line" situations)

            opacity: 0.9
        }

        states: [
            State {
                name: "MOUSE_HOVERED"
                when: mouseArea.containsMouse && !mouseArea.pressed && !root.padSwapActive
                PropertyChanges {
                    target: padNameBackground
                    color: Utils.colorWithAlpha(ui.theme.accentColor, ui.theme.buttonOpacityHover)
                }
                PropertyChanges {
                    target: notationPreview
                    opacity: 0.7
                }
            },
            State {
                name: "MOUSE_HIT"
                when: mouseArea.pressed || root.padSwapActive
                PropertyChanges {
                    target: padNameBackground
                    color: Utils.colorWithAlpha(ui.theme.accentColor, ui.theme.buttonOpacityHit)
                }
                PropertyChanges {
                    target: notationPreview
                    opacity: 1.0
                }
            }
        ]
    }

    Rectangle {
        id: separator

        width: parent.width
        height: 1

        color: root.useNotationPreview ? ui.theme.strokeColor : ui.theme.accentColor
    }

    Rectangle {
        id: footerArea

        width: parent.width

        color: Utils.colorWithAlpha(ui.theme.buttonColor, ui.theme.buttonOpacityNormal)

        MouseArea {
            id: footerMouseArea

            anchors.fill: parent
            enabled: root.panelMode !== PanelMode.EDIT_LAYOUT
            hoverEnabled: true

            acceptedButtons: Qt.LeftButton | Qt.RightButton

            onPressed: function(event) {
                let pos = menuLoader.parent.mapFromItem(footerMouseArea, event.x, event.y)
                root.openContextMenu(pos)
            }
        }

        StyledTextLabel {
            id: shortcutLabel

            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.margins: 6

            font: ui.theme.bodyFont
            color: ui.theme.fontPrimaryColor

            text: Boolean(root.padModel) ? root.padModel.keyboardShortcut : ""
        }

        StyledIconLabel {
            id: midiNoteIcon

            anchors.verticalCenter: parent.verticalCenter
            anchors.right: midiNoteLabel.left

            color: ui.theme.fontPrimaryColor

            iconCode: IconCode.SINGLE_NOTE
        }

        StyledTextLabel {
            id: midiNoteLabel

            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.margins: 6

            font: ui.theme.bodyFont
            color: ui.theme.fontPrimaryColor

            text: Boolean(root.padModel) ? root.padModel.midiNote : ""
        }
    }

    ContextMenuLoader {
        id: menuLoader

        onHandleMenuItem: function(itemId) {
            root.padModel.handleMenuItem(itemId)
        }

        states: [
            State {
                name: "MOUSE_HOVERED"
                when: footerMouseArea.containsMouse && !footerMouseArea.pressed
                PropertyChanges {
                    target: footerArea
                    color: Utils.colorWithAlpha(ui.theme.buttonColor, ui.theme.buttonOpacityHover)
                }
            },
            State {
                name: "MOUSE_HIT"
                when: footerMouseArea.pressed
                PropertyChanges {
                    target: footerArea
                    color: Utils.colorWithAlpha(ui.theme.buttonColor, ui.theme.buttonOpacityHit)
                }
            }
        ]
    }
}
