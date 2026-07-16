/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) MuseScore Limited and others
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

 #pragma once

 #include "rcommand/commandtypes.h"

namespace mu::notation {
// global commands
inline static const muse::rcommand::Command CANCEL_COMMAND("command://notation/cancel");
inline static const muse::rcommand::Command UNDO_COMMAND("command://notation/undo");
inline static const muse::rcommand::Command REDO_COMMAND("command://notation/redo");

// navigation commands
inline static const muse::rcommand::Command GOTO_FIRST_ELEMENT_COMMAND("command://notation/goto-first-element");
inline static const muse::rcommand::Command GOTO_LAST_ELEMENT_COMMAND("command://notation/goto-last-element");
inline static const muse::rcommand::Command GOTO_NEXT_ELEMENT_COMMAND("command://notation/goto-next-element");
inline static const muse::rcommand::Command GOTO_PREV_ELEMENT_COMMAND("command://notation/goto-prev-element");
inline static const muse::rcommand::Command GOTO_NEXT_TRACK_COMMAND("command://notation/goto-next-track");
inline static const muse::rcommand::Command GOTO_PREV_TRACK_COMMAND("command://notation/goto-prev-track");
inline static const muse::rcommand::Command GOTO_NEXT_FRAME_COMMAND("command://notation/goto-next-frame");
inline static const muse::rcommand::Command GOTO_PREV_FRAME_COMMAND("command://notation/goto-prev-frame");
inline static const muse::rcommand::Command GOTO_NEXT_SYSTEM_COMMAND("command://notation/goto-next-system");
inline static const muse::rcommand::Command GOTO_PREV_SYSTEM_COMMAND("command://notation/goto-prev-system");
inline static const muse::rcommand::Command GOTO_UP_CHORD_COMMAND("command://notation/goto-up-chord");
inline static const muse::rcommand::Command GOTO_DOWN_CHORD_COMMAND("command://notation/goto-down-chord");
inline static const muse::rcommand::Command GOTO_TOP_CHORD_COMMAND("command://notation/goto-top-chord");
inline static const muse::rcommand::Command GOTO_BOTTOM_CHORD_COMMAND("command://notation/goto-bottom-chord");

inline static const muse::rcommand::Command EDIT_NEXT_WORD_COMMAND("command://notation/edit-next-word");
inline static const muse::rcommand::Command EDIT_NEXT_TEXT_ELEMENT_COMMAND("command://notation/edit-next-text-element");
inline static const muse::rcommand::Command EDIT_PREV_TEXT_ELEMENT_COMMAND("command://notation/edit-prev-text-element");

// note input commands
inline static const muse::rcommand::Command TOGGLE_NOTE_INPUT_COMMAND("command://notation/toggle-note-input");
inline static const muse::rcommand::Command TOGGLE_NOTE_INPUT_BY_NOTE_NAME_COMMAND("command://notation/toggle-note-input-by-note-name");
inline static const muse::rcommand::Command TOGGLE_NOTE_INPUT_BY_DURATION_COMMAND("command://notation/toggle-note-input-by-duration");
inline static const muse::rcommand::Command TOGGLE_NOTE_INPUT_RHYTHM_COMMAND("command://notation/toggle-note-input-rhythm");
inline static const muse::rcommand::Command TOGGLE_NOTE_INPUT_REPITCH_COMMAND("command://notation/toggle-note-input-repitch");
inline static const muse::rcommand::Command TOGGLE_NOTE_INPUT_REALTIME_AUTO_COMMAND("command://notation/toggle-note-input-realtime-auto");
inline static const muse::rcommand::Command TOGGLE_NOTE_INPUT_REALTIME_MANUAL_COMMAND("command://notation/toggle-note-input-realtime-manual");
inline static const muse::rcommand::Command TOGGLE_NOTE_INPUT_TIMEWISE_COMMAND("command://notation/toggle-note-input-timewise");
inline static const muse::rcommand::Command REALTIME_ADVANCE_COMMAND("command://notation/realtime-advance");

inline static const muse::rcommand::Command NOTE_LONGA_COMMAND("command://notation/note-longa");
inline static const muse::rcommand::Command NOTE_BREVE_COMMAND("command://notation/note-breve");
inline static const muse::rcommand::Command PAD_NOTE_1_COMMAND("command://notation/pad-note-1");
inline static const muse::rcommand::Command PAD_NOTE_2_COMMAND("command://notation/pad-note-2");
inline static const muse::rcommand::Command PAD_NOTE_4_COMMAND("command://notation/pad-note-4");
inline static const muse::rcommand::Command PAD_NOTE_8_COMMAND("command://notation/pad-note-8");
inline static const muse::rcommand::Command PAD_NOTE_16_COMMAND("command://notation/pad-note-16");
inline static const muse::rcommand::Command PAD_NOTE_32_COMMAND("command://notation/pad-note-32");
inline static const muse::rcommand::Command PAD_NOTE_64_COMMAND("command://notation/pad-note-64");
inline static const muse::rcommand::Command PAD_NOTE_128_COMMAND("command://notation/pad-note-128");
inline static const muse::rcommand::Command PAD_NOTE_256_COMMAND("command://notation/pad-note-256");
inline static const muse::rcommand::Command PAD_NOTE_512_COMMAND("command://notation/pad-note-512");
inline static const muse::rcommand::Command PAD_NOTE_1024_COMMAND("command://notation/pad-note-1024");
inline static const muse::rcommand::Command PAD_DOT_COMMAND("command://notation/pad-dot");
inline static const muse::rcommand::Command PAD_DOT2_COMMAND("command://notation/pad-dot2");
inline static const muse::rcommand::Command PAD_DOT3_COMMAND("command://notation/pad-dot3");
inline static const muse::rcommand::Command PAD_DOT4_COMMAND("command://notation/pad-dot4");
inline static const muse::rcommand::Command PAD_REST_COMMAND("command://notation/pad-rest");

inline static const muse::rcommand::Command TOGGLE_FLAT2_COMMAND("command://notation/toggle-flat2");
inline static const muse::rcommand::Command TOGGLE_FLAT_COMMAND("command://notation/toggle-flat");
inline static const muse::rcommand::Command TOGGLE_NAT_COMMAND("command://notation/toggle-nat");
inline static const muse::rcommand::Command TOGGLE_SHARP_COMMAND("command://notation/toggle-sharp");
inline static const muse::rcommand::Command TOGGLE_SHARP2_COMMAND("command://notation/toggle-sharp2");

inline static const muse::rcommand::Command ADD_TIE_COMMAND("command://notation/add-tie");
inline static const muse::rcommand::Command ADD_SLUR_COMMAND("command://notation/add-slur");
inline static const muse::rcommand::Command ADD_LV_COMMAND("command://notation/add-laissez-vibrer");
inline static const muse::rcommand::Command ADD_MARCATO_COMMAND("command://notation/add-marcato");
inline static const muse::rcommand::Command ADD_SFORZATO_COMMAND("command://notation/add-sforzato");
inline static const muse::rcommand::Command ADD_TENUTO_COMMAND("command://notation/add-tenuto");
inline static const muse::rcommand::Command ADD_STACCATO_COMMAND("command://notation/add-staccato");

inline static const muse::rcommand::Command USE_VOICE_1_COMMAND("command://notation/use-voice-1");
inline static const muse::rcommand::Command USE_VOICE_2_COMMAND("command://notation/use-voice-2");
inline static const muse::rcommand::Command USE_VOICE_3_COMMAND("command://notation/use-voice-3");
inline static const muse::rcommand::Command USE_VOICE_4_COMMAND("command://notation/use-voice-4");

inline static const muse::rcommand::Command FLIP_COMMAND("command://notation/flip");
inline static const muse::rcommand::Command FLIP_HORIZONTALLY_COMMAND("command://notation/flip-horizontally");

inline static const muse::rcommand::Command ADD_NOTE_COMMAND("command://notation/add-note"); // with params
inline static const muse::rcommand::Command ENTER_NOTE_C_COMMAND("command://notation/enter-note-c");
inline static const muse::rcommand::Command ENTER_NOTE_D_COMMAND("command://notation/enter-note-d");
inline static const muse::rcommand::Command ENTER_NOTE_E_COMMAND("command://notation/enter-note-e");
inline static const muse::rcommand::Command ENTER_NOTE_F_COMMAND("command://notation/enter-note-f");
inline static const muse::rcommand::Command ENTER_NOTE_G_COMMAND("command://notation/enter-note-g");
inline static const muse::rcommand::Command ENTER_NOTE_A_COMMAND("command://notation/enter-note-a");
inline static const muse::rcommand::Command ENTER_NOTE_B_COMMAND("command://notation/enter-note-b");
inline static const muse::rcommand::Command ADD_NOTE_C_COMMAND("command://notation/add-note-c");
inline static const muse::rcommand::Command ADD_NOTE_D_COMMAND("command://notation/add-note-d");
inline static const muse::rcommand::Command ADD_NOTE_E_COMMAND("command://notation/add-note-e");
inline static const muse::rcommand::Command ADD_NOTE_F_COMMAND("command://notation/add-note-f");
inline static const muse::rcommand::Command ADD_NOTE_G_COMMAND("command://notation/add-note-g");
inline static const muse::rcommand::Command ADD_NOTE_A_COMMAND("command://notation/add-note-a");
inline static const muse::rcommand::Command ADD_NOTE_B_COMMAND("command://notation/add-note-b");
inline static const muse::rcommand::Command INSERT_NOTE_C_COMMAND("command://notation/insert-note-c");
inline static const muse::rcommand::Command INSERT_NOTE_D_COMMAND("command://notation/insert-note-d");
inline static const muse::rcommand::Command INSERT_NOTE_E_COMMAND("command://notation/insert-note-e");
inline static const muse::rcommand::Command INSERT_NOTE_F_COMMAND("command://notation/insert-note-f");
inline static const muse::rcommand::Command INSERT_NOTE_G_COMMAND("command://notation/insert-note-g");
inline static const muse::rcommand::Command INSERT_NOTE_A_COMMAND("command://notation/insert-note-a");
inline static const muse::rcommand::Command INSERT_NOTE_B_COMMAND("command://notation/insert-note-b");

inline static const muse::rcommand::Command SHOW_TUPLET_CONFIGURE_COMMAND("command://notation/show-tuplet-configure");
inline static const muse::rcommand::Command ADD_TUPLET_COMMAND("command://notation/add-tuplet"); // with params
inline static const muse::rcommand::Command ADD_DUPLET_COMMAND("command://notation/add-duplet");
inline static const muse::rcommand::Command ADD_TRIPLET_COMMAND("command://notation/add-triplet");
inline static const muse::rcommand::Command ADD_QUADRUPLET_COMMAND("command://notation/add-quadruplet");
inline static const muse::rcommand::Command ADD_QUINTUPLET_COMMAND("command://notation/add-quintuplet");
inline static const muse::rcommand::Command ADD_SEXTUPLET_COMMAND("command://notation/add-sextuplet");
inline static const muse::rcommand::Command ADD_SEPTUPLET_COMMAND("command://notation/add-septuplet");
inline static const muse::rcommand::Command ADD_OCTUPLET_COMMAND("command://notation/add-octuplet");
inline static const muse::rcommand::Command ADD_NONUPLET_COMMAND("command://notation/add-nonuplet");

// editing commands
inline static const muse::rcommand::Command COPY_COMMAND("command://notation/copy");
inline static const muse::rcommand::Command CUT_COMMAND("command://notation/cut");
inline static const muse::rcommand::Command PASTE_COMMAND("command://notation/paste");
inline static const muse::rcommand::Command DELETE_COMMAND("command://notation/delete");

// move commands
inline static const muse::rcommand::Command MOVE_RIGHT_COMMAND("command://notation/move-right");
inline static const muse::rcommand::Command MOVE_LEFT_COMMAND("command://notation/move-left");
inline static const muse::rcommand::Command MOVE_RIGHT_QUICKLY_COMMAND("command://notation/move-right-quickly");
inline static const muse::rcommand::Command MOVE_LEFT_QUICKLY_COMMAND("command://notation/move-left-quickly");

inline static const muse::rcommand::Command PITCH_UP_COMMAND("command://notation/pitch-up");
inline static const muse::rcommand::Command PITCH_DOWN_COMMAND("command://notation/pitch-down");
inline static const muse::rcommand::Command PITCH_UP_OCTAVE_COMMAND("command://notation/pitch-up-octave");
inline static const muse::rcommand::Command PITCH_DOWN_OCTAVE_COMMAND("command://notation/pitch-down-octave");
}
