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

BaseSection {
    id: root

    title: qsTrc("appshell", "Note input")

    property alias advanceToNextNote: advanceToNextNoteBox.checked
    property alias colorNotes: colorNotesBox.checked
    property alias delayBetweenNotes: delayBetweenNotesControl.currentValue

    signal advanceToNextNoteChangeRequested(bool advance)
    signal colorNotesChangeRequested(bool color)
    signal delayBetweenNotesChangeRequested(int delay)

    CheckBox {
        id: advanceToNextNoteBox
        width: parent.width

        text: qsTrc("appshell", "Advance to next note on key release (MIDI)")

        navigation.name: "AdvanceToNextNoteBox"
        navigation.panel: root.navigation
        navigation.row: 0

        onClicked: {
            root.advanceToNextNoteChangeRequested(!checked)
        }
    }

    CheckBox {
        id: colorNotesBox
        width: parent.width

        text: qsTrc("appshell", "Colour notes outside of usable pitch range")

        navigation.name: "ColorNotesBox"
        navigation.panel: root.navigation
        navigation.row: 1

        onClicked: {
            root.colorNotesChangeRequested(!checked)
        }
    }

    IncrementalPropertyControlWithTitle {
        id: delayBetweenNotesControl

        title: qsTrc("appshell", "Delay between notes in automatic real time mode:")

        columnWidth: root.columnWidth
        spacing: root.columnSpacing

        measureUnitsSymbol: qsTrc("appshell", "ms")

        navigation.name: "DelayBetweenNotesControl"
        navigation.panel: root.navigation
        navigation.row: 2

        onValueEdited: function(newValue) {
            root.delayBetweenNotesChangeRequested(newValue)
        }
    }
}
