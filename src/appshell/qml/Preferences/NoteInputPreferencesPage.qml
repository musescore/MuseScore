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
            addAccidentalDotsArticulationsToNextNoteEntered: noteInputModel.addAccidentalDotsArticulationsToNextNoteEntered
            useNoteInputCursorInInputByDuration: noteInputModel.useNoteInputCursorInInputByDuration

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 1

            onDefaultNoteInputMethodChangeRequested: function(method) {
                noteInputModel.defaultNoteInputMethod = method
            }

            onAddAccidentalDotsArticulationsToNextNoteEnteredChangeRequested: function(add) {
                noteInputModel.addAccidentalDotsArticulationsToNextNoteEntered = add
            }

            onUseNoteInputCursorInInputByDurationChangeRequested: function(use) {
                noteInputModel.useNoteInputCursorInInputByDuration = use
            }
        }

        SeparatorLine {}

        MidiInputSection {
            midiInputEnabled: noteInputModel.midiInputEnabled
            startNoteInputWhenPressingKey: noteInputModel.startNoteInputAtSelectedNoteRestWhenPressingMidiKey
            advanceToNextNote: noteInputModel.advanceToNextNoteOnKeyRelease
            delayBetweenNotes: noteInputModel.delayBetweenNotesInRealTimeModeMilliseconds

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 2

            onMidiInputEnabledChangeRequested: function(enabled) {
                noteInputModel.midiInputEnabled = enabled
            }

            onStartNoteInputWhenPressingKeyChangeRequested: function(start) {
                noteInputModel.startNoteInputAtSelectedNoteRestWhenPressingMidiKey = start
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
            id: notePreviewSection

            playNotesWhenEditing: noteInputModel.playNotesWhenEditing
            playPreviewNotesInInputByDuration: noteInputModel.playPreviewNotesInInputByDuration
            playChordWhenEditing: noteInputModel.playNotesWhenEditing ? noteInputModel.playChordWhenEditing : false
            playChordSymbolWhenEditing: noteInputModel.playNotesWhenEditing ? noteInputModel.playChordSymbolWhenEditing : false
            notePlayDurationMilliseconds: noteInputModel.notePlayDurationMilliseconds
            playNotesWithScoreDynamics: noteInputModel.playPreviewNotesWithScoreDynamics

            playNotesOnMidiInputBoxEnabled: noteInputModel.midiInputEnabled && noteInputModel.playNotesWhenEditing
            playNotesOnMidiInput: notePreviewSection.playNotesOnMidiInputBoxEnabled && noteInputModel.playNotesOnMidiInput
            useMidiVelocityAndDurationDuringNoteInput: notePreviewSection.playNotesOnMidiInputBoxEnabled && noteInputModel.useMidiVelocityAndDurationDuringNoteInput

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 3

            onPlayNotesWhenEditingChangeRequested: function(play) {
                noteInputModel.playNotesWhenEditing = play
            }

            onPlayPreviewNotesInInputByDurationChangeRequested: function(play) {
                noteInputModel.playPreviewNotesInInputByDuration = play
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

            onPlayNotesWithScoreDynamicsChangeRequested: function(play) {
                noteInputModel.playPreviewNotesWithScoreDynamics = play
            }

            onPlayNotesOnMidiInputChangeRequested: function(play) {
                noteInputModel.playNotesOnMidiInput = play
            }

            onUseMidiVelocityAndDurationDuringNoteInputChangeRequested: function(use) {
                noteInputModel.useMidiVelocityAndDurationDuringNoteInput = use
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

        FretboardDiagramsSection {
            width: parent.width

            autoUpdateFretboardDiagrams: noteInputModel.autoUpdateFretboardDiagrams

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 5

            onAutoUpdateFretboardDiagramsChangeRequested: function(update) {
                noteInputModel.autoUpdateFretboardDiagrams = update
            }
        }

        SeparatorLine {}

        NoteColorsSection {
            width: parent.width

            colorNotes: noteInputModel.colorNotesOutsideOfUsablePitchRange
            warnGuitarBends: noteInputModel.warnGuitarBends

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 6

            onColorNotesChangeRequested: function(color) {
                noteInputModel.colorNotesOutsideOfUsablePitchRange = color
            }

            onWarnGuitarBendsChangeRequested: function(warn) {
                noteInputModel.warnGuitarBends = warn
            }
        }
    }
}
