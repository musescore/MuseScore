/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

//! NOTE Command URIs used by the test cases.
//! The names are the same as the C++ constants, see the *commands.h files

module.exports = {
    // project (src/project/projectcommands.h)
    PROJECT_CLOSE_COMMAND: "command://project/close",
    PROJECT_EXPORT_COMMAND: "command://project/export",
    PROJECT_PROPERTIES_COMMAND: "command://project/properties",

    // playback (src/playback/playbackcommands.h)
    PLAY_COMMAND: "command://playback/play",
    PAUSE_COMMAND: "command://playback/pause",

    // notation (src/notationscene/notationcommands.h)
    TOGGLE_NOTE_INPUT_BY_NOTE_NAME_COMMAND: "command://notation/toggle-note-input/by-note-name",
    SET_DURATION_WHOLE_COMMAND: "command://notation/set-duration/whole",
    SET_DURATION_HALF_COMMAND: "command://notation/set-duration/half",
    SET_DURATION_QUARTER_COMMAND: "command://notation/set-duration/quarter",
    SET_DURATION_EIGHTH_COMMAND: "command://notation/set-duration/eighth",
    SET_DURATION_16TH_COMMAND: "command://notation/set-duration/16th",
    SET_DURATION_32ND_COMMAND: "command://notation/set-duration/32nd",
    SET_DURATION_64TH_COMMAND: "command://notation/set-duration/64th",
    TOGGLE_DOT_COMMAND: "command://notation/toggle-dots/1",
    TOGGLE_REST_COMMAND: "command://notation/toggle-rest",
    TOGGLE_FLAT2_COMMAND: "command://notation/toggle-flat2",
    TOGGLE_FLAT_COMMAND: "command://notation/toggle-flat",
    TOGGLE_NAT_COMMAND: "command://notation/toggle-nat",
    TOGGLE_SHARP_COMMAND: "command://notation/toggle-sharp",
    TOGGLE_SHARP2_COMMAND: "command://notation/toggle-sharp2",
    TOGGLE_TIE_COMMAND: "command://notation/toggle-tie",
    ADD_SLUR_COMMAND: "command://notation/add-slur",
    TOGGLE_MARCATO_COMMAND: "command://notation/toggle-marcato",
    TOGGLE_SFORZATO_COMMAND: "command://notation/toggle-sforzato",
    TOGGLE_TENUTO_COMMAND: "command://notation/toggle-tenuto",
    TOGGLE_STACCATO_COMMAND: "command://notation/toggle-staccato",
    ENTER_NOTE_C_COMMAND: "command://notation/add-note/next-chord/c",
    ENTER_NOTE_D_COMMAND: "command://notation/add-note/next-chord/d",
    ENTER_NOTE_E_COMMAND: "command://notation/add-note/next-chord/e",
    ENTER_NOTE_F_COMMAND: "command://notation/add-note/next-chord/f",
    ENTER_NOTE_G_COMMAND: "command://notation/add-note/next-chord/g",
    ENTER_NOTE_A_COMMAND: "command://notation/add-note/next-chord/a",
    ENTER_NOTE_B_COMMAND: "command://notation/add-note/next-chord/b"
}
