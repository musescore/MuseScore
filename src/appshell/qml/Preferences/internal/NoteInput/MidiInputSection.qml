/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0

import "../../internal"

BaseSection {
    id: root

    title: qsTrc("appshell/preferences", "MIDI devices")

    property alias midiInputEnabled: enableMidiInputToggle.checked
    property alias startNoteInputWhenPressingKey: startNoteInputWhenPressingKeyBox.checked
    property bool advanceToNextNote: false
    property int delayBetweenNotes: 0

    signal midiInputEnabledChangeRequested(bool enabled)
    signal startNoteInputWhenPressingKeyChangeRequested(bool start)
    signal advanceToNextNoteChangeRequested(bool advance)
    signal delayBetweenNotesChangeRequested(int delay)

    Row {
        width: parent.width
        height: enableMidiInputToggle.height

        spacing: 6

        ToggleButton {
            id: enableMidiInputToggle

            navigation.name: "EnableMidiInputToggle"
            navigation.panel: root.navigation
            navigation.row: 0

            onToggled: {
                root.midiInputEnabledChangeRequested(!checked)
            }
        }

        StyledTextLabel {
            height: parent.height

            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter

            text: qsTrc("appshell/preferences", "Enable MIDI input")
        }
    }

    CheckBox {
        id: startNoteInputWhenPressingKeyBox
        width: parent.width

        enabled: root.midiInputEnabled
        text: qsTrc("appshell/preferences", "When pressing a key, begin note input at the selected measure, note, or rest")

        navigation.name: "StartNoteInputWhenPressingKeyBox"
        navigation.panel: root.navigation
        navigation.row: 1

        onClicked: {
            root.startNoteInputWhenPressingKeyChangeRequested(!checked)
        }
    }

    ExpandableBlank {
        width: parent.width

        enabled: root.midiInputEnabled
        title: qsTrc("appshell/preferences", "Real-time input modes")
        isExpanded: false

        contentItemComponent: Column {
            width: parent.width

            spacing: root.spacing

            CheckBox {
                id: advanceToNextNoteBox
                width: parent.width

                enabled: root.midiInputEnabled
                text: qsTrc("appshell/preferences", "Advance to next note on key release")

                checked: root.advanceToNextNote

                navigation.name: "AdvanceToNextNoteBox"
                navigation.panel: root.navigation
                navigation.row: 2

                onClicked: {
                    root.advanceToNextNoteChangeRequested(!checked)
                }
            }

            IncrementalPropertyControlWithTitle {
                id: delayBetweenNotesControl

                enabled: root.midiInputEnabled
                title: qsTrc("appshell/preferences", "Delay between notes:")

                currentValue: root.delayBetweenNotes

                columnWidth: root.columnWidth
                spacing: root.columnSpacing

                measureUnitsSymbol: qsTrc("global", "ms")

                navigation.name: "DelayBetweenNotesControl"
                navigation.panel: root.navigation
                navigation.row: 3

                onValueEdited: function(newValue) {
                    root.delayBetweenNotesChangeRequested(newValue)
                }
            }
        }
    }
}
