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

BaseSection {
    id: root

    title: qsTrc("appshell/preferences", "Note preview")

    property alias playNotesWhenEditing: playNotesBox.checked
    property alias playChordWhenEditing: playChordBox.checked
    property alias playChordSymbolWhenEditing: playChordSymbolBox.checked
    property alias notePlayDurationMilliseconds: notePlayDurationControl.currentValue
    property alias playNotesOnMidiInput: playNotesOnMidiInputBox.checked

    signal playNotesWhenEditingChangeRequested(bool play)
    signal playChordWhenEditingChangeRequested(bool play)
    signal playChordSymbolWhenEditingChangeRequested(bool play)
    signal playNotesOnMidiInputChangeRequested(bool play)
    signal notePlayDurationChangeRequested(int duration)


    CheckBox {
        id: playNotesBox
        width: parent.width

        text: qsTrc("appshell/preferences", "Play notes when editing")

        navigation.name: "PlayNotesBox"
        navigation.panel: root.navigation
        navigation.row: 0

        onClicked: {
            root.playNotesWhenEditingChangeRequested(!checked)
        }
    }

    CheckBox {
        id: playChordBox
        anchors.left: parent.left;
        anchors.leftMargin: 24;
        anchors.right: parent.right;

        text: qsTrc("appshell/preferences", "Play chord when editing")

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
        anchors.left: parent.left;
        anchors.leftMargin: 24;
        anchors.right: parent.right;

        text: qsTrc("appshell/preferences", "Play chord symbol when editing")

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

    IncrementalPropertyControlWithTitle {
        id: notePlayDurationControl

        title: qsTrc("appshell/preferences", "Default duration:")

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
}
