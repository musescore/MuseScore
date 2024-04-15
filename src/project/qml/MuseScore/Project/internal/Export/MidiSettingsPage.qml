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
import QtQuick 2.15

import Muse.UiComponents 1.0

ExportSettingsPage {
    id: root

    CheckBox {
        width: parent.width
        text: qsTrc("project/export", "Expand repeats")

        navigation.name: "ExpandRepeatsCheckbox"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationOrder + 1

        checked: root.model.midiExpandRepeats
        onClicked: {
            root.model.midiExpandRepeats = !checked
        }
    }

    CheckBox {
        width: parent.width
        text: qsTrc("project/export", "Export RPNs")

        navigation.name: "ExportRpnsCheckbox"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationOrder + 2

        checked: root.model.midiExportRpns
        onClicked: {
            root.model.midiExportRpns = !checked
        }
    }

    StyledTextLabel {
        width: parent.width
        text: qsTrc("project/export", "Each selected part will be exported as a separate MIDI file.")
        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.WordWrap
    }
}
