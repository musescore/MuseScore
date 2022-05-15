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
        width: parent.width
        spacing: root.sectionsSpacing

        NoteInputSection {
            advanceToNextNote: noteInputModel.advanceToNextNoteOnKeyRelease
            colorNotes: noteInputModel.colorNotesOutsideOfUsablePitchRange
            delayBetweenNotes: noteInputModel.delayBetweenNotesInRealTimeModeMilliseconds

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 1

            onAdvanceToNextNoteChangeRequested: function(advance) {
                noteInputModel.advanceToNextNoteOnKeyRelease = advance
            }

            onColorNotesChangeRequested: function(color) {
                noteInputModel.colorNotesOutsideOfUsablePitchRange = color
            }

            onDelayBetweenNotesChangeRequested: function(delay) {
                noteInputModel.delayBetweenNotesInRealTimeModeMilliseconds = delay
            }
        }

        SeparatorLine {}

        NoteInputPlaySection {
            playNotesWhenEditing: noteInputModel.playNotesWhenEditing
            playChordWhenEditing: noteInputModel.playChordWhenEditing
            playChordSymbolWhenEditing: noteInputModel.playChordSymbolWhenEditing
            notePlayDurationMilliseconds: noteInputModel.notePlayDurationMilliseconds

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 2

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
    }
}
