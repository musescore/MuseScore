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

Rectangle {
    id: root

    property var padModel: null

    property int panelMode: -1
    property bool useNotationPreview: false

    radius: root.width / 6

    color: root.useNotationPreview ? "white" : ui.theme.backgroundSecondaryColor

    border.color: root.panelMode === PanelMode.EDIT_LAYOUT ? ui.theme.accentColor : "transparent"
    border.width: 2

    Item {
        id: contentArea

        anchors.fill: parent

        visible: Boolean(root.padModel) ? !root.padModel.isEmptySlot : false

        StyledTextLabel {
            id: instrumentNameLabel

            visible: !root.useNotationPreview

            anchors.centerIn: parent
            anchors.margins: 5

            width: parent.width

            wrapMode: Text.WordWrap

            text: Boolean(root.padModel) ? root.padModel.instrumentName : ""
        }

        StyledTextLabel {
            // placeholder component - reflects the current state of the pad/panel
            id: modeLabel

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.margins: 5

            width: parent.width

            color: root.useNotationPreview ? "black" : "white"

            wrapMode: Text.WordWrap
        }
    }

    states: [
        State {
            name: "WRITE"
            when: root.panelMode === PanelMode.WRITE
            PropertyChanges {
                target: modeLabel

                // See modeLabel - these are all placeholder strings
                text: "Write mode"
            }
        },
        State {
            name: "SOUND_PREVIEW"
            when: root.panelMode === PanelMode.SOUND_PREVIEW
            PropertyChanges {
                target: modeLabel
                text: "Sound preview mode"
            }
        },
        State {
            name: "EDIT_LAYOUT"
            when: root.panelMode === PanelMode.EDIT_LAYOUT
            PropertyChanges {
                target: modeLabel
                text: "Edit layout mode"
            }
        }
    ]
}
