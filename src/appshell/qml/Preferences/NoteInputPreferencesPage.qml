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

import "internal/NoteInput"

PreferencesPage {
    id: root

    Component.onCompleted: {
        noteInputModel.load()
    }

    NoteInputPreferencesModel {
        id: noteInputModel
    }

    Column {
        width: parent.width
        spacing: root.sectionsSpacing

        NoteInputSection {
            noteInputMethods: noteInputModel.noteInputMethods()
            defaultNoteInputMethod: noteInputModel.defaultNoteInputMethod

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 1

            onDefaultNoteInputMethodChangeRequested: function(method) {
                noteInputModel.defaultNoteInputMethod = method
            }
        }

        SeparatorLine {}

        MidiInputSection {
            midiInputEnabled: noteInputModel.midiInputEnabled
            startNoteInputWhenPressingKey: noteInputModel.startNoteInputAtSelectionWhenPressingMidiKey
            advanceToNextNote: noteInputModel.advanceToNextNoteOnKeyRelease
            delayBetweenNotes: noteInputModel.delayBetweenNotesInRealTimeModeMilliseconds

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 2

            onMidiInputEnabledChangeRequested: function(enabled) {
                noteInputModel.midiInputEnabled = enabled
            }

            onStartNoteInputWhenPressingKeyChangeRequested: function(start) {
                noteInputModel.startNoteInputAtSelectionWhenPressingMidiKey = start
            }

            onAdvanceToNextNoteChangeRequested: function(advance) {
                noteInputModel.advanceToNextNoteOnKeyRelease = advance
            }

            onDelayBetweenNotesChangeRequested: function(delay) {
                noteInputModel.delayBetweenNotesInRealTimeModeMilliseconds = delay
            }
        }

        SeparatorLine {}

        NotePreviewSection {
            playNotesWhenEditing: noteInputModel.playNotesWhenEditing
            playChordWhenEditing: noteInputModel.playChordWhenEditing
            playChordSymbolWhenEditing: noteInputModel.playChordSymbolWhenEditing
            notePlayDurationMilliseconds: noteInputModel.notePlayDurationMilliseconds

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 3

            onPlayNotesWhenEditingChangeRequested: function(play) {
                noteInputModel.playNotesWhenEditing = play
            }

            onPlayChordWhenEditingChangeRequested: function(play) {
                noteInputModel.playChordWhenEditing = play
            }

            onPlayChordSymbolWhenEditingChangeRequested: function(play) {
                noteInputModel.playChordSymbolWhenEditing = play
            }

            onNotePlayDurationChangeRequested: function(duration) {
                noteInputModel.notePlayDurationMilliseconds = duration
            }
        }

        SeparatorLine {}

        VoiceAssignmentSection {
            width: parent.width

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 4

            dynamicsApplyToAllVoices: noteInputModel.dynamicsApplyToAllVoices

            onDynamicsApplyToAllVoicesChangeRequested: function(value) {
                noteInputModel.dynamicsApplyToAllVoices = value
            }
        }

        SeparatorLine {}

        NoteColorsSection {
            width: parent.width

            colorNotes: noteInputModel.colorNotesOutsideOfUsablePitchRange
            warnGuitarBends: noteInputModel.warnGuitarBends

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 5

            onColorNotesChangeRequested: function(color) {
                noteInputModel.colorNotesOutsideOfUsablePitchRange = color
            }

            onWarnGuitarBendsChangeRequested: function(warn) {
                noteInputModel.warnGuitarBends = warn
            }

        }
    }
}
