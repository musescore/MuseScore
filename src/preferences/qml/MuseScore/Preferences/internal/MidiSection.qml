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

import Muse.UiComponents

BaseSection {
    id: root

    title: qsTrc("preferences", "MIDI")

    property alias shortestNotes: shortestNotesBox.model
    property int currentShortestNote: 0

    property alias roundTempo: roundTempoBox.checked

    signal currentShortestNoteChangeRequested(int note)
    signal roundTempoChangeRequested(bool round)

    ComboBoxWithTitle {
        id: shortestNotesBox

        title: qsTrc("preferences", "Shortest note")
        columnWidth: root.columnWidth

        currentIndex: control.indexOfValue(root.currentShortestNote)

        control.textRole: "title"
        control.valueRole: "value"

        navigation.name: "ShortestNoteBox"
        navigation.panel: root.navigation
        navigation.row: 0

        onValueEdited: function(newIndex, newValue) {
            root.currentShortestNoteChangeRequested(newValue)
        }
    }

    CheckBox {
        id: roundTempoBox
        width: parent.width

        text: qsTrc("appshell/preferences", "Round tempos to nearest whole number")

        navigation.name: "RoundTempoBox"
        navigation.panel: root.navigation
        navigation.row: 1

        onClicked: {
            root.roundTempoChangeRequested(!checked)
        }
    }
}
