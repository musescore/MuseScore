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
import MuseScore.Preferences 1.0

import "internal"

PreferencesPage {
    id: root

    NoteInputPreferencesModel {
        id: noteInputModel
    }

    Column {
        anchors.fill: parent

        spacing: 22

        StyledTextLabel {
            text: qsTrc("appshell", "Note Input")
            font: ui.theme.bodyBoldFont
        }

        CheckBox {
            text: qsTrc("appshell", "Advance to next note on key release (MIDI)")

            checked: noteInputModel.advanceToNextNoteOnKeyRelease

            onClicked: {
                noteInputModel.advanceToNextNoteOnKeyRelease = !checked
            }
        }

        CheckBox {
            width: 170
            text: qsTrc("appshell", "Colour notes outside of usable pitch range")

            checked: noteInputModel.colorNotesOusideOfUsablePitchRange

            onClicked: {
                noteInputModel.colorNotesOusideOfUsablePitchRange = !checked
            }
        }

        IncrementalPropertyControlWithTitle {
            title: qsTrc("appshell", "Delay between notes in automatic real time mode:")

            titleWidth: 173
            spacing: 46

            currentValue: noteInputModel.delayBetweenNotesInRealTimeModeMilliseconds
            measureUnitsSymbol: qsTrc("appshell", "ms")

            onValueEdited: {
                noteInputModel.delayBetweenNotesInRealTimeModeMilliseconds = newValue
            }
        }

        SeparatorLine {}

        CheckBox {
            text: qsTrc("appshell", "Play Notes When Editing")
            font: ui.theme.bodyBoldFont

            checked: noteInputModel.playNotesWhenEditing

            onClicked: {
                noteInputModel.playNotesWhenEditing = !checked
            }
        }

        IncrementalPropertyControlWithTitle {
            title: qsTrc("appshell", "Default duration:")

            spacing: 126

            currentValue: noteInputModel.notePlayDurationMilliseconds
            measureUnitsSymbol: qsTrc("appshell", "ms")

            onValueEdited: {
                noteInputModel.notePlayDurationMilliseconds = newValue
            }
        }

        CheckBox {
            text: qsTrc("appshell", "Play chord when editing")

            checked: noteInputModel.playChordWhenEditing
            enabled: noteInputModel.playNotesWhenEditing

            onClicked: {
                noteInputModel.playChordWhenEditing = !checked
            }
        }

        CheckBox {
            text: qsTrc("appshell", "Play chord symbol when editing")

            checked: noteInputModel.playChordSymbolWhenEditing
            enabled: noteInputModel.playNotesWhenEditing

            onClicked: {
                noteInputModel.playChordSymbolWhenEditing = !checked
            }
        }
    }
}
