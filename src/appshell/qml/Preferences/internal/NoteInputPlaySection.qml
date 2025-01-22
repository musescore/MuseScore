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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Preferences 1.0

BaseSection {
    id: root

    title: qsTrc("appshell/preferences", "Note preview")

    property alias playNotesWhenEditing: playNotesBox.checked
    property alias notePlayDurationMilliseconds: notePlayDurationControl.currentValue
    property alias playChordWhenEditing: playChordBox.checked
    property alias playChordSymbolWhenEditing: playChordSymbolBox.checked
    property alias playNotesOnMidiInput: playNotesOnMidiInputBox.checked

    property alias playNotesOnMidiInputBoxEnabled: playNotesOnMidiInputBox.enabled

    //property alias chordOptions: playChordBox.model
    //property alias currentChordIndex: playChordBox.currentIndex

    signal playNotesWhenEditingChangeRequested(bool play)
    signal notePlayDurationChangeRequested(int duration)
    signal playChordWhenEditingChangeRequested(bool play)
    signal playChordSymbolWhenEditingChangeRequested(bool play)
    signal playNotesOnMidiInputChangeRequested(bool play)

    Row {
        id: playNotesBoxRow

        height: playNotesBox.height
        width: parent.width

        spacing: 6

        ToggleButton {
            id: playNotesBox

            navigation.name: "PlayNotesBox"
            navigation.panel: root.navigation
            navigation.row: 0

            onToggled: {
                root.playNotesWhenEditingChangeRequested(!checked)
            }
        }

        StyledTextLabel {
            id: playNotesBoxLabel

            height: parent.height

            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter

            wrapMode: Text.Wrap
            text: qsTrc("appshell/preferences", "Hear playback when adding, editing, and selecting notes")
        }
    }

    IncrementalPropertyControlWithTitle {
        id: notePlayDurationControl

        title: qsTrc("appshell/preferences", "Playback duration:")

        enabled: (root.playNotesWhenEditing || root.playNotesOnMidiInput)

        columnWidth: root.columnWidth
        spacing: root.columnSpacing

        //: Abbreviation of "milliseconds"
        measureUnitsSymbol: qsTrc("global", "ms")

        navigation.name: "NotePlayDurationControl"
        navigation.panel: root.navigation
        navigation.row: 4

        onValueEdited: function(newValue) {
            root.notePlayDurationChangeRequested(newValue)
        }
    }

    // ComboBoxWithTitle {
    //     id: playChordBox

    //     title: qsTrc("appshell/preferences", "When adding chords to notes, play:")
    //     columnWidth: root.columnWidth

    //     navigation.name: "PlayChordsBox"
    //     navigation.panel: root.navigation
    //     navigation.row: 5

    //     onValueEdited: function(newIndex, newValue) {
    //         root.playChordWhenEditingChangeRequested(newIndex)
    //     }
    // }

    CheckBox {
        id: playChordBox
        width: parent.width;

        text: qsTrc("appshell/preferences", "When adding chords to notes, play:")

        enabled: root.playNotesWhenEditing

        navigation.name: "PlayChordBox"
        navigation.panel: root.navigation
        navigation.row: 1

        onClicked: {
            root.playChordWhenEditingChangeRequested(!checked)
        }
    }

    CheckBox {
        id: playChordSymbolBox
        width: parent.width;

        text: qsTrc("appshell/preferences", "Play chord symbols and Nashville numbers when selected")

        enabled: root.playNotesWhenEditing

        navigation.name: "PlayChordSymbolBox"
        navigation.panel: root.navigation
        navigation.row: 2

        onClicked: {
            root.playChordSymbolWhenEditingChangeRequested(!checked)
        }
    }

    CheckBox {
        id: playNotesOnMidiInputBox
        width: parent.width

        text: qsTrc("appshell/preferences", "Play MIDI input")

        navigation.name: "PlayNotesOnMidiInputBox"
        navigation.panel: root.navigation
        navigation.row: 3

        onClicked: {
            root.playNotesOnMidiInputChangeRequested(!checked)
        }
    }
}
