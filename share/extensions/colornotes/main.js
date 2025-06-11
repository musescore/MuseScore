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

const colors = [ // "#rrggbb" with rr, gg, and bb being the hex values for red, green, and blue, respectively
                "#e21c48", // C
                "#f26622", // C#/Db
                "#f99d1c", // D
                "#ffcc33", // D#/Eb
                "#fff32b", // E
                "#bcd85f", // F
                "#62bc47", // F#/Gb
                "#009c95", // G
                "#0071bb", // G#/Ab
                "#5e50a1", // A
                "#8d5ba6", // A#/Bb
                "#cf3e96"  // B
             ]
const black = "#000000"

function main() {
    api.log.info("hello colornotes");

    applyToNotesInSelection(colorNote)
}

// Apply the given function to all notes (elements with pitch) in selection
// or, if nothing is selected, in the entire score

function applyToNotesInSelection(func) {
    var fullScore = !curScore.selection.elements.length
    if (fullScore) {
        cmd("select-all")
    }
    curScore.startCmd()
    for (var i in curScore.selection.elements)
        if (curScore.selection.elements[i].pitch)
            func(curScore.selection.elements[i])
    curScore.endCmd()
    if (fullScore) {
        cmd("escape")
    }
}

function colorNote(note) {
    // Get the base color for the note based on its pitch (modulo 12 for octave wrapping)
    let base_color = colors[note.pitch % 12];

    // Check if the note has an accidental (e.g., sharp or flat)
    if (note.accidental) {
        // If the accidental already has the same color as the base color, change to black
        if (note.accidental.color == base_color) {
            base_color = black;
        }
        // Set the accidental's color to the base color
        note.accidental.color = base_color;

    // If there is no accidental, but the note's color is equal to the base color, change it to black
    } else if (note.color == base_color) {
        base_color = black;
    }

    // Assign the final color to the note itself
    note.color = base_color;

    // If the note has dots (augmentation dots), set each dot's color to match the note's color
    if (note.dots) {
        for (var i = 0; i < note.dots.length; i++) {
            if (note.dots[i]) {
                note.dots[i].color = note.color;
            }
        }
    }
}
