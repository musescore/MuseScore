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
import Muse.GraphicalEffects 1.0
import MuseScore.NotationScene 1.0

Rectangle {
    id: root

    property var padModel: null

    property bool isEmptySlot: Boolean(root.padModel) ? root.padModel.isEmptySlot : true

    property int panelMode: -1
    property bool useNotationPreview: false

    property alias totalBorderWidth: contentColumn.anchors.margins

    radius: root.width / 6

    color: ui.theme.backgroundPrimaryColor

    border.color: root.panelMode === PanelMode.EDIT_LAYOUT ? ui.theme.accentColor : "transparent"
    border.width: 2

    Column {
        id: contentColumn

        anchors.fill: parent
        anchors.margins: 2 + root.border.width // Defined as 1 in the spec, but causes some aliasing in practice...

        // Can't simply use clip as this won't take into account radius...
        layer.enabled: ui.isEffectsAllowed
        layer.effect: EffectOpacityMask {
            maskSource: Rectangle {
                width: contentColumn.width
                height: contentColumn.height
                radius: root.radius - contentColumn.anchors.margins
            }
        }

        Rectangle {
            id: mainContentArea

            width: parent.width
            height: parent.height - separator.height - footerArea.height

            color: root.isEmptySlot ? ui.theme.backgroundSecondaryColor : Utils.colorWithAlpha(ui.theme.accentColor, ui.theme.buttonOpacityNormal)

            StyledTextLabel {
                id: instrumentNameLabel

                visible: !root.useNotationPreview

                anchors.centerIn: parent
                width: parent.width - root.radius

                wrapMode: Text.WordWrap
                maximumLineCount: 4
                font: ui.theme.bodyBoldFont

                text: Boolean(root.padModel) ? root.padModel.instrumentName : ""
            }
        }

        Rectangle {
            id: separator

            width: parent.width
            height: 1

            color: root.isEmptySlot ? ui.theme.backgroundSecondaryColor : ui.theme.accentColor
        }

        Rectangle {
            id: footerArea

            width: parent.width
            height: 24

            color: ui.theme.backgroundSecondaryColor

            StyledTextLabel {
                id: shortcutLabel

                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.margins: 6

                font: ui.theme.bodyFont

                text: Boolean(root.padModel) ? root.padModel.keyboardShortcut : ""
            }

            StyledTextLabel {
                id: midiNoteLabel

                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.margins: 6

                font: ui.theme.bodyFont

                text: Boolean(root.padModel) ? root.padModel.midiNote : ""
            }
        }
    }
}
