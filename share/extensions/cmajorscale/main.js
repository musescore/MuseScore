/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025
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

// C Major scale MIDI pitches (C4 to C5)
const cMajorScale = [60, 62, 64, 65, 67, 69, 71, 72] // C D E F G A B C

function main() {
    api.log.info("C Major Scale Generator started");

    if (typeof curScore === 'undefined' || !curScore) {
        api.log.error("No score opened");
        quit();
        return;
    }

    var cursor = curScore.newCursor();
    if (!cursor) {
        api.log.error("Could not create cursor");
        quit();
        return;
    }

    curScore.startCmd()

    // Check if there's a selection, otherwise start from beginning
    cursor.rewind(Cursor.SELECTION_START);
    if (!cursor.segment) {
        // No selection, start from the beginning of the score
        cursor.rewind(Cursor.SCORE_START);
    }

    // Verify we have a valid position
    if (!cursor.segment) {
        api.log.error("Could not position cursor");
        curScore.endCmd();
        quit();
        return;
    }

    // Get the current track (staff and voice)
    var startTrack = cursor.track;

    // Set note duration to quarter notes (1/4)
    cursor.setDuration(1, 4);

    // Generate the C major scale
    for (var i = 0; i < cMajorScale.length; i++) {
        cursor.track = startTrack;
        cursor.addNote(cMajorScale[i], false);

        if (i < cMajorScale.length - 1) {
            cursor.next();
        }
    }

    curScore.endCmd()

    api.log.info("C Major scale generated successfully!");
    quit();
}
