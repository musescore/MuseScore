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

    Rectangle {
        id: mainContentArea

        width: parent.width
        height: parent.height - separator.height - footerArea.height

        color: Utils.colorWithAlpha(ui.theme.accentColor, ui.theme.buttonOpacityNormal)

        MouseArea {
            id: mouseArea

            anchors.fill: parent
            hoverEnabled: true

            onPressed: {
                if (!Boolean(root.padModel)) {
                    return
                }
                root.padModel.triggerPad()
            }
        }

        StyledTextLabel {
            id: instrumentNameLabel

            visible: !root.useNotationPreview

            anchors.centerIn: parent
            width: parent.width - 12

            wrapMode: Text.WordWrap
            maximumLineCount: 4
            font: ui.theme.bodyBoldFont

            text: Boolean(root.padModel) ? root.padModel.instrumentName : ""
        }

        PaintedEngravingItem {
            id: notationPreview

            visible: root.useNotationPreview

            anchors.fill: parent

            engravingItem: Boolean(root.padModel) ? root.padModel.notationPreviewItem : null
        }

        states: [
            State {
                name: "MOUSE_HOVERED"
                when: mouseArea.containsMouse && !mouseArea.pressed && !root.padSwapActive
                PropertyChanges {
                    target: mainContentArea
                    color: Utils.colorWithAlpha(ui.theme.accentColor, ui.theme.buttonOpacityHover)
                }
            },
            State {
                name: "MOUSE_HIT"
                when: mouseArea.pressed || root.padSwapActive
                PropertyChanges {
                    target: mainContentArea
                    color: Utils.colorWithAlpha(ui.theme.accentColor, ui.theme.buttonOpacityHit)
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
}
