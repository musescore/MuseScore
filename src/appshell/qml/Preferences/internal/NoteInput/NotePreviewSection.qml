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

import Muse.Ui 1.0
import Muse.UiComponents 1.0

import "../../internal"

BaseSection {
    id: root

    property alias playNotesWhenEditing: playNotesToggle.checked
    property alias playChordWhenEditing: playChordBox.checked
    property alias playChordSymbolWhenEditing: playChordSymbolBox.checked
    property alias playPreviewNotesInInputByDuration: playPreviewNotesInInputByDurationBox.checked
    property alias notePlayDurationMilliseconds: notePlayDurationControl.currentValue
    property alias playNotesWithScoreDynamics: playNotesWithScoreDynamicsBox.checked

    property alias playNotesOnMidiInputBoxEnabled: playNotesOnMidiInputBox.enabled
    property alias playNotesOnMidiInput: playNotesOnMidiInputBox.checked
    property alias useMidiVelocityAndDurationDuringNoteInput: useMidiVelocityAndDurationDuringNoteInputBox.checked

    signal playNotesWhenEditingChangeRequested(bool play)
    signal playChordWhenEditingChangeRequested(bool play)
    signal playChordSymbolWhenEditingChangeRequested(bool play)
    signal playPreviewNotesInInputByDurationChangeRequested(bool play)
    signal notePlayDurationChangeRequested(int duration)
    signal playNotesWithScoreDynamicsChangeRequested(bool play)

    signal playNotesOnMidiInputChangeRequested(bool play)
    signal useMidiVelocityAndDurationDuringNoteInputChangeRequested(bool use)

    title: qsTrc("appshell/preferences", "Note preview")

    Row {
        width: parent.width
        height: playNotesToggle.height

        spacing: 6

        ToggleButton {
            id: playNotesToggle

            navigation.name: "PlayNotesToggle"
            navigation.panel: root.navigation
            navigation.row: 0

            navigation.accessible.name: playNotesBoxLabel.text

            onToggled: {
                root.playNotesWhenEditingChangeRequested(!checked)
            }
        }

        StyledTextLabel {
            id: playNotesBoxLabel

            anchors.verticalCenter: parent.verticalCenter
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.Wrap

            text: qsTrc("appshell/preferences", "Hear playback when adding, editing, and selecting notes")
        }
    }

    IncrementalPropertyControlWithTitle {
        id: notePlayDurationControl

        title: qsTrc("appshell/preferences", "Playback duration:")

        enabled: root.playNotesWhenEditing

        columnWidth: root.columnWidth
        spacing: root.columnSpacing

        //: Abbreviation of "milliseconds"
        measureUnitsSymbol: qsTrc("global", "ms")

        navigation.name: "NotePlayDurationControl"
        navigation.panel: root.navigation
        navigation.row: 1

        onValueEdited: function(newValue) {
            root.notePlayDurationChangeRequested(newValue)
        }
    }

    CheckBox {
        id: playChordBox
        width: parent.width

        text: qsTrc("appshell/preferences", "Play chord when editing")

        enabled: root.playNotesWhenEditing

        navigation.name: "PlayChordBox"
        navigation.panel: root.navigation
        navigation.row: 2

        onClicked: {
            root.playChordWhenEditingChangeRequested(!checked)
        }
    }

    CheckBox {
        id: playChordSymbolBox
        width: parent.width

        text: qsTrc("appshell/preferences", "Play chord symbols and Nashville numbers")

        enabled: root.playNotesWhenEditing

        navigation.name: "PlayChordSymbolBox"
        navigation.panel: root.navigation
        navigation.row: 3

        onClicked: {
            root.playChordSymbolWhenEditingChangeRequested(!checked)
        }
    }

    CheckBox {
        id: playPreviewNotesInInputByDurationBox
        width: parent.width

        text: qsTrc("appshell/preferences", "Play when setting pitch (input by duration mode only)")

        enabled: root.playNotesWhenEditing

        navigation.name: "PlayPreviewNotesInInputByDurationBox"
        navigation.panel: root.navigation
        navigation.row: 4

        onClicked: {
            root.playPreviewNotesInInputByDurationChangeRequested(!checked)
        }
    }

    CheckBox {
        id: playNotesWithScoreDynamicsBox
        width: parent.width

        text: qsTrc("appshell/preferences", "Play preview notes with score dynamics")

        enabled: root.playNotesWhenEditing

        navigation.name: "PlayNotesWithScoreDynamics"
        navigation.panel: root.navigation
        navigation.row: 5

        onClicked: {
            root.playNotesWithScoreDynamicsChangeRequested(!checked)
        }
    }

    CheckBox {
        id: playNotesOnMidiInputBox
        width: parent.width

        text: qsTrc("appshell/preferences", "Play MIDI input")

        navigation.name: "PlayNotesOnMidiInputBox"
        navigation.panel: root.navigation
        navigation.row: 6

        onClicked: {
            root.playNotesOnMidiInputChangeRequested(!checked)
        }
    }

    CheckBox {
        id: useMidiVelocityAndDurationDuringNoteInputBox
        width: parent.width

        text: qsTrc("appshell/preferences", "Play MIDI notes with velocity and duration during note input")

        enabled: root.playNotesWhenEditing && playNotesOnMidiInputBox.checked

        navigation.name: "UseMidiVelocityAndDurationDuringNoteInputBox"
        navigation.panel: root.navigation
        navigation.row: 7

        onClicked: {
            root.useMidiVelocityAndDurationDuringNoteInputChangeRequested(!checked)
        }
    }
}
