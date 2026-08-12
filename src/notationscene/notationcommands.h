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

// navigation and selection commands
inline static const muse::rcommand::Command SELECT_COMMAND("command://notation/select"); // with params
inline static const muse::rcommand::Command OPEN_SELECTION_OPTIONS_COMMAND("command://notation/open-selection-options");
// aliases for select command
inline static const muse::rcommand::Command GOTO_FIRST_ELEMENT_COMMAND("command://notation/goto-first-element");
inline static const muse::rcommand::Command GOTO_LAST_ELEMENT_COMMAND("command://notation/goto-last-element");
inline static const muse::rcommand::Command GOTO_NEXT_ELEMENT_COMMAND("command://notation/goto-next-element");
inline static const muse::rcommand::Command GOTO_PREV_ELEMENT_COMMAND("command://notation/goto-prev-element");
inline static const muse::rcommand::Command GOTO_NEXT_SEGMENT_ELEMENT_COMMAND("command://notation/goto-next-segment-element");
inline static const muse::rcommand::Command GOTO_PREV_SEGMENT_ELEMENT_COMMAND("command://notation/goto-prev-segment-element");
inline static const muse::rcommand::Command GOTO_NEXT_TRACK_COMMAND("command://notation/goto-next-track");
inline static const muse::rcommand::Command GOTO_PREV_TRACK_COMMAND("command://notation/goto-prev-track");
inline static const muse::rcommand::Command GOTO_NEXT_FRAME_COMMAND("command://notation/goto-next-frame");
inline static const muse::rcommand::Command GOTO_PREV_FRAME_COMMAND("command://notation/goto-prev-frame");
inline static const muse::rcommand::Command GOTO_NEXT_SYSTEM_COMMAND("command://notation/goto-next-system");
inline static const muse::rcommand::Command GOTO_PREV_SYSTEM_COMMAND("command://notation/goto-prev-system");
inline static const muse::rcommand::Command GOTO_UPNOTE_IN_CHORD_COMMAND("command://notation/goto-upnote-in-chord");
inline static const muse::rcommand::Command GOTO_DOWNNOTE_IN_CHORD_COMMAND("command://notation/goto-downnote-in-chord");
inline static const muse::rcommand::Command GOTO_TOPNOTE_IN_CHORD_COMMAND("command://notation/goto-topnote-in-chord");
inline static const muse::rcommand::Command GOTO_BOTTOMNOTE_IN_CHORD_COMMAND("command://notation/goto-bottomnote-in-chord");
inline static const muse::rcommand::Command GOTO_TOP_STAFF_COMMAND("command://notation/goto-top-staff");
inline static const muse::rcommand::Command GOTO_EMPTY_TRAILING_MEASURE_COMMAND("command://notation/goto-empty-trailing-measure");
inline static const muse::rcommand::Command SELECT_SIMILAR_COMMAND("command://notation/select-similar");
inline static const muse::rcommand::Command SELECT_SIMILAR_IN_STAFF_COMMAND("command://notation/select-similar-in-staff");
inline static const muse::rcommand::Command SELECT_SIMILAR_IN_RANGE_COMMAND("command://notation/select-similar-in-range");
inline static const muse::rcommand::Command SELECT_NOTES_IN_CHORD_COMMAND("command://notation/select-notes-in-chord");
inline static const muse::rcommand::Command SELECT_ALL_COMMAND("command://notation/select-all");
inline static const muse::rcommand::Command SELECT_SECTION_COMMAND("command://notation/select-section");
// ------------------------------------------------------------
inline static const muse::rcommand::Command GET_LOCATION_COMMAND("command://notation/get-location");

inline static const muse::rcommand::Command ADD_TO_SELECTION_NEXT_CHORD_COMMAND("command://notation/add-to-selection-next-chord");
inline static const muse::rcommand::Command ADD_TO_SELECTION_PREV_CHORD_COMMAND("command://notation/add-to-selection-prev-chord");
inline static const muse::rcommand::Command ADD_TO_SELECTION_NEXT_MEASURE_COMMAND("command://notation/add-to-selection-next-measure");
inline static const muse::rcommand::Command ADD_TO_SELECTION_PREV_MEASURE_COMMAND("command://notation/add-to-selection-prev-measure");
inline static const muse::rcommand::Command ADD_TO_SELECTION_ABOVE_STAFF_COMMAND("command://notation/add-to-selection-above-staff");
inline static const muse::rcommand::Command ADD_TO_SELECTION_BELOW_STAFF_COMMAND("command://notation/add-to-selection-below-staff");
inline static const muse::rcommand::Command ADD_TO_SELECTION_BEGIN_SYSTEM_COMMAND("command://notation/add-to-selection-begin-system");
inline static const muse::rcommand::Command ADD_TO_SELECTION_END_SYSTEM_COMMAND("command://notation/add-to-selection-end-system");
inline static const muse::rcommand::Command ADD_TO_SELECTION_BEGIN_SCORE_COMMAND("command://notation/add-to-selection-begin-score");
inline static const muse::rcommand::Command ADD_TO_SELECTION_END_SCORE_COMMAND("command://notation/add-to-selection-end-score");

// text navigation commands
inline static const muse::rcommand::Command EDITTEXT_NEXT_WORD_COMMAND("command://notation/edittext-next-word");
inline static const muse::rcommand::Command EDITTEXT_NEXT_ELEMENT_COMMAND("command://notation/edittext-next-element");
inline static const muse::rcommand::Command EDITTEXT_PREV_ELEMENT_COMMAND("command://notation/edittext-prev-element");
inline static const muse::rcommand::Command EDITTEXT_NEXT_BEAT_COMMAND("command://notation/edittext-next-beat");
inline static const muse::rcommand::Command EDITTEXT_PREV_BEAT_COMMAND("command://notation/edittext-prev-beat");

inline static const muse::rcommand::Command EDITTEXT_ADVANCE_LONGA_COMMAND("command://notation/edittext-advance-longa");
inline static const muse::rcommand::Command EDITTEXT_ADVANCE_BREVE_COMMAND("command://notation/edittext-advance-breve");
inline static const muse::rcommand::Command EDITTEXT_ADVANCE_1_COMMAND("command://notation/edittext-advance-1");
inline static const muse::rcommand::Command EDITTEXT_ADVANCE_2_COMMAND("command://notation/edittext-advance-2");
inline static const muse::rcommand::Command EDITTEXT_ADVANCE_4_COMMAND("command://notation/edittext-advance-4");
inline static const muse::rcommand::Command EDITTEXT_ADVANCE_8_COMMAND("command://notation/edittext-advance-8");
inline static const muse::rcommand::Command EDITTEXT_ADVANCE_16_COMMAND("command://notation/edittext-advance-16");
inline static const muse::rcommand::Command EDITTEXT_ADVANCE_32_COMMAND("command://notation/edittext-advance-32");
inline static const muse::rcommand::Command EDITTEXT_ADVANCE_64_COMMAND("command://notation/edittext-advance-64");

// lyrics editing commands
inline static const muse::rcommand::Command EDITLYRIC_NEXT_VERSE_COMMAND("command://notation/editlyrics-next-verse");
inline static const muse::rcommand::Command EDITLYRIC_PREV_VERSE_COMMAND("command://notation/editlyrics-prev-verse");
inline static const muse::rcommand::Command EDITLYRIC_NEXT_SYLLABLE_COMMAND("command://notation/editlyrics-next-syllable");
inline static const muse::rcommand::Command EDITLYRIC_ADD_MELISMA_COMMAND("command://notation/editlyrics-add-melisma");
inline static const muse::rcommand::Command EDITLYRIC_ADD_VERSE_COMMAND("command://notation/editlyrics-add-verse");

// text editing commands
inline static const muse::rcommand::Command EDITTEXT_TOGGLE_BOLD_COMMAND("command://notation/edittext-toggle-bold");
inline static const muse::rcommand::Command EDITTEXT_TOGGLE_ITALIC_COMMAND("command://notation/edittext-toggle-italic");
inline static const muse::rcommand::Command EDITTEXT_TOGGLE_UNDERLINE_COMMAND("command://notation/edittext-toggle-underline");
inline static const muse::rcommand::Command EDITTEXT_TOGGLE_STRIKE_COMMAND("command://notation/edittext-toggle-strike");
inline static const muse::rcommand::Command EDITTEXT_TOGGLE_SUBSCRIPT_COMMAND("command://notation/edittext-toggle-subscript");
inline static const muse::rcommand::Command EDITTEXT_TOGGLE_SUPERSCRIPT_COMMAND("command://notation/edittext-toggle-superscript");

// note input commands
inline static const muse::rcommand::Command TOGGLE_NOTE_INPUT_COMMAND("command://notation/toggle-note-input");
inline static const muse::rcommand::Command TOGGLE_NOTE_INPUT_BY_NOTE_NAME_COMMAND("command://notation/toggle-note-input-by-note-name");
inline static const muse::rcommand::Command TOGGLE_NOTE_INPUT_BY_DURATION_COMMAND("command://notation/toggle-note-input-by-duration");
inline static const muse::rcommand::Command TOGGLE_NOTE_INPUT_RHYTHM_COMMAND("command://notation/toggle-note-input-rhythm");
inline static const muse::rcommand::Command TOGGLE_NOTE_INPUT_REPITCH_COMMAND("command://notation/toggle-note-input-repitch");
inline static const muse::rcommand::Command TOGGLE_NOTE_INPUT_REALTIME_AUTO_COMMAND("command://notation/toggle-note-input-realtime-auto");
inline static const muse::rcommand::Command TOGGLE_NOTE_INPUT_REALTIME_MANUAL_COMMAND(
    "command://notation/toggle-note-input-realtime-manual");
inline static const muse::rcommand::Command TOGGLE_NOTE_INPUT_TIMEWISE_COMMAND("command://notation/toggle-note-input-timewise");
inline static const muse::rcommand::Command TOGGLE_INSERT_MODE_COMMAND("command://notation/toggle-insert-mode");
inline static const muse::rcommand::Command REALTIME_ADVANCE_COMMAND("command://notation/realtime-advance");

inline static const muse::rcommand::Command SET_DURATION_LONGA_COMMAND("command://notation/set-duration-longa");
inline static const muse::rcommand::Command SET_DURATION_BREVE_COMMAND("command://notation/set-duration-breve");
inline static const muse::rcommand::Command SET_DURATION_WHOLE_COMMAND("command://notation/set-duration-whole");
inline static const muse::rcommand::Command SET_DURATION_HALF_COMMAND("command://notation/set-duration-half");
inline static const muse::rcommand::Command SET_DURATION_QUARTER_COMMAND("command://notation/set-duration-quarter");
inline static const muse::rcommand::Command SET_DURATION_EIGHTH_COMMAND("command://notation/set-duration-eighth");
inline static const muse::rcommand::Command SET_DURATION_16TH_COMMAND("command://notation/set-duration-16th");
inline static const muse::rcommand::Command SET_DURATION_32ND_COMMAND("command://notation/set-duration-32nd");
inline static const muse::rcommand::Command SET_DURATION_64TH_COMMAND("command://notation/set-duration-64th");
inline static const muse::rcommand::Command SET_DURATION_128TH_COMMAND("command://notation/set-duration-128th");
inline static const muse::rcommand::Command SET_DURATION_256TH_COMMAND("command://notation/set-duration-256th");
inline static const muse::rcommand::Command SET_DURATION_512TH_COMMAND("command://notation/set-duration-512th");
inline static const muse::rcommand::Command SET_DURATION_1024TH_COMMAND("command://notation/set-duration-1024th");

inline static const muse::rcommand::Command SET_DOUBLE_DURATION_COMMAND("command://notation/set-double-duration");
inline static const muse::rcommand::Command SET_HALVE_DURATION_COMMAND("command://notation/set-halve-duration");
inline static const muse::rcommand::Command SET_DOUBLE_DURATION_DOTTED_COMMAND("command://notation/set-double-duration-dotted");
inline static const muse::rcommand::Command SET_HALVE_DURATION_DOTTED_COMMAND("command://notation/set-halve-duration-dotted");

inline static const muse::rcommand::Command EXTEND_TO_NEXT_NOTE_COMMAND("command://notation/extend-to-next-note");

inline static const muse::rcommand::Command TOGGLE_DOT_COMMAND("command://notation/toggle-dot");
inline static const muse::rcommand::Command TOGGLE_DOT2_COMMAND("command://notation/toggle-dot2");
inline static const muse::rcommand::Command TOGGLE_DOT3_COMMAND("command://notation/toggle-dot3");
inline static const muse::rcommand::Command TOGGLE_DOT4_COMMAND("command://notation/toggle-dot4");

inline static const muse::rcommand::Command TOGGLE_REST_COMMAND("command://notation/toggle-rest");

inline static const muse::rcommand::Command TOGGLE_FLAT2_COMMAND("command://notation/toggle-flat2");
inline static const muse::rcommand::Command TOGGLE_FLAT_COMMAND("command://notation/toggle-flat");
inline static const muse::rcommand::Command TOGGLE_NAT_COMMAND("command://notation/toggle-nat");
inline static const muse::rcommand::Command TOGGLE_SHARP_COMMAND("command://notation/toggle-sharp");
inline static const muse::rcommand::Command TOGGLE_SHARP2_COMMAND("command://notation/toggle-sharp2");

inline static const muse::rcommand::Command ADD_SHARP2_COMMAND("command://notation/add-sharp2");
inline static const muse::rcommand::Command ADD_SHARP_COMMAND("command://notation/add-sharp");
inline static const muse::rcommand::Command ADD_NAT_COMMAND("command://notation/add-nat");
inline static const muse::rcommand::Command ADD_FLAT_COMMAND("command://notation/add-flat");
inline static const muse::rcommand::Command ADD_FLAT2_COMMAND("command://notation/add-flat2");

inline static const muse::rcommand::Command TOGGLE_TIE_COMMAND("command://notation/toggle-tie");
inline static const muse::rcommand::Command ADD_SLUR_COMMAND("command://notation/add-slur");
inline static const muse::rcommand::Command TOGGLE_LV_COMMAND("command://notation/toggle-laissez-vibrer");
inline static const muse::rcommand::Command TOGGLE_MARCATO_COMMAND("command://notation/toggle-marcato");
inline static const muse::rcommand::Command TOGGLE_SFORZATO_COMMAND("command://notation/toggle-sforzato");
inline static const muse::rcommand::Command TOGGLE_TENUTO_COMMAND("command://notation/toggle-tenuto");
inline static const muse::rcommand::Command TOGGLE_STACCATO_COMMAND("command://notation/toggle-staccato");

// voice commands
inline static const muse::rcommand::Command USE_VOICE_1_COMMAND("command://notation/use-voice-1");
inline static const muse::rcommand::Command USE_VOICE_2_COMMAND("command://notation/use-voice-2");
inline static const muse::rcommand::Command USE_VOICE_3_COMMAND("command://notation/use-voice-3");
inline static const muse::rcommand::Command USE_VOICE_4_COMMAND("command://notation/use-voice-4");
inline static const muse::rcommand::Command SWAP_VOICE_X12_COMMAND("command://notation/swap-voice-x12");
inline static const muse::rcommand::Command SWAP_VOICE_X13_COMMAND("command://notation/swap-voice-x13");
inline static const muse::rcommand::Command SWAP_VOICE_X14_COMMAND("command://notation/swap-voice-x14");
inline static const muse::rcommand::Command SWAP_VOICE_X23_COMMAND("command://notation/swap-voice-x23");
inline static const muse::rcommand::Command SWAP_VOICE_X24_COMMAND("command://notation/swap-voice-x24");
inline static const muse::rcommand::Command SWAP_VOICE_X34_COMMAND("command://notation/swap-voice-x34");

inline static const muse::rcommand::Command FLIP_COMMAND("command://notation/flip");
inline static const muse::rcommand::Command FLIP_HORIZONTALLY_COMMAND("command://notation/flip-horizontally");

inline static const muse::rcommand::Command ADD_NOTE_COMMAND("command://notation/add-note"); // with params
inline static const muse::rcommand::Command ADD_DRUM_NOTE_COMMAND("command://notation/add-drum-note"); // with params

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

inline static const muse::rcommand::Command ENTER_REST_COMMAND("command://notation/enter-rest");

// add commands
inline static const muse::rcommand::Command OPEN_TUPLET_CONFIGURE_COMMAND("command://notation/open-tuplet-configure");
inline static const muse::rcommand::Command ADD_TUPLET_COMMAND("command://notation/add-tuplet"); // with params
inline static const muse::rcommand::Command ADD_DUPLET_COMMAND("command://notation/add-duplet");
inline static const muse::rcommand::Command ADD_TRIPLET_COMMAND("command://notation/add-triplet");
inline static const muse::rcommand::Command ADD_QUADRUPLET_COMMAND("command://notation/add-quadruplet");
inline static const muse::rcommand::Command ADD_QUINTUPLET_COMMAND("command://notation/add-quintuplet");
inline static const muse::rcommand::Command ADD_SEXTUPLET_COMMAND("command://notation/add-sextuplet");
inline static const muse::rcommand::Command ADD_SEPTUPLET_COMMAND("command://notation/add-septuplet");
inline static const muse::rcommand::Command ADD_OCTUPLET_COMMAND("command://notation/add-octuplet");
inline static const muse::rcommand::Command ADD_NONUPLET_COMMAND("command://notation/add-nonuplet");

inline static const muse::rcommand::Command INSERT_HBOX_COMMAND("command://notation/insert-hbox");
inline static const muse::rcommand::Command INSERT_VBOX_COMMAND("command://notation/insert-vbox");
inline static const muse::rcommand::Command INSERT_TEXTFRAME_COMMAND("command://notation/insert-textframe");
inline static const muse::rcommand::Command INSERT_FRETFRAME_COMMAND("command://notation/insert-fretframe");
inline static const muse::rcommand::Command APPEND_HBOX_COMMAND("command://notation/append-hbox");
inline static const muse::rcommand::Command APPEND_VBOX_COMMAND("command://notation/append-vbox");
inline static const muse::rcommand::Command APPEND_TEXTFRAME_COMMAND("command://notation/append-textframe");
inline static const muse::rcommand::Command APPEND_FRETFRAME_COMMAND("command://notation/append-fretframe");

inline static const muse::rcommand::Command ADD_FRETBOARD_DIAGRAM_COMMAND("command://notation/add-fretboard-diagram");

inline static const muse::rcommand::Command ADD_OTTAVA_8VA_COMMAND("command://notation/add-ottava-8va");
inline static const muse::rcommand::Command ADD_OTTAVA_8VB_COMMAND("command://notation/add-ottava-8vb");

inline static const muse::rcommand::Command ADD_DYNAMIC_COMMAND("command://notation/add-dynamic");
inline static const muse::rcommand::Command ADD_HAIRPIN_COMMAND("command://notation/add-hairpin");
inline static const muse::rcommand::Command ADD_HAIRPIN_REVERSE_COMMAND("command://notation/add-hairpin-reverse");
inline static const muse::rcommand::Command ADD_NOTELINE_COMMAND("command://notation/add-noteline");

inline static const muse::rcommand::Command ADD_IMAGE_COMMAND("command://notation/add-image");

inline static const muse::rcommand::Command ADD_UP_BOW_COMMAND("command://notation/add-up-bow");
inline static const muse::rcommand::Command ADD_DOWN_BOW_COMMAND("command://notation/add-down-bow");

// add text commands
inline static const muse::rcommand::Command ADD_TITLE_TEXT_COMMAND("command://notation/add-title-text");
inline static const muse::rcommand::Command ADD_SUBTITLE_TEXT_COMMAND("command://notation/add-subtitle-text");
inline static const muse::rcommand::Command ADD_COMPOSER_TEXT_COMMAND("command://notation/add-composer-text");
inline static const muse::rcommand::Command ADD_LYRICIST_TEXT_COMMAND("command://notation/add-lyricist-text");
inline static const muse::rcommand::Command ADD_PART_TEXT_COMMAND("command://notation/add-part-text");
inline static const muse::rcommand::Command ADD_FRAME_TEXT_COMMAND("command://notation/add-frame-text");
inline static const muse::rcommand::Command ADD_SYSTEM_TEXT_COMMAND("command://notation/add-system-text");
inline static const muse::rcommand::Command ADD_STAFF_TEXT_COMMAND("command://notation/add-staff-text");
inline static const muse::rcommand::Command ADD_EXPRESSION_TEXT_COMMAND("command://notation/add-expression-text");
inline static const muse::rcommand::Command ADD_REHEARSALMARK_TEXT_COMMAND("command://notation/add-rehearsalmark-text");
inline static const muse::rcommand::Command ADD_INSTRUMENT_CHANGE_TEXT_COMMAND("command://notation/add-instrument-change-text");
inline static const muse::rcommand::Command ADD_FINGERING_TEXT_COMMAND("command://notation/add-fingering-text");
inline static const muse::rcommand::Command ADD_STICKING_TEXT_COMMAND("command://notation/add-sticking-text");
inline static const muse::rcommand::Command ADD_CHORD_TEXT_COMMAND("command://notation/add-chord-text");
inline static const muse::rcommand::Command ADD_ROMAN_NUMERAL_TEXT_COMMAND("command://notation/add-roman-numeral-text");
inline static const muse::rcommand::Command ADD_NASHVILLE_NUMBER_TEXT_COMMAND("command://notation/add-nashville-number-text");
inline static const muse::rcommand::Command ADD_LYRICS_COMMAND("command://notation/add-lyrics");
inline static const muse::rcommand::Command ADD_TEMPO_COMMAND("command://notation/add-tempo");
inline static const muse::rcommand::Command ADD_FIGURED_BASS_COMMAND("command://notation/add-figured-bass");

// add grace notes commands
inline static const muse::rcommand::Command ADD_ACCIACCATURA_COMMAND("command://notation/add-acciaccatura");
inline static const muse::rcommand::Command ADD_APPOGGIATURA_COMMAND("command://notation/add-appoggiatura");
inline static const muse::rcommand::Command ADD_GRACE4_COMMAND("command://notation/add-grace4");
inline static const muse::rcommand::Command ADD_GRACE16_COMMAND("command://notation/add-grace16");
inline static const muse::rcommand::Command ADD_GRACE32_COMMAND("command://notation/add-grace32");
inline static const muse::rcommand::Command ADD_GRACE8_AFTER_COMMAND("command://notation/add-grace8-after");
inline static const muse::rcommand::Command ADD_GRACE16_AFTER_COMMAND("command://notation/add-grace16-after");
inline static const muse::rcommand::Command ADD_GRACE32_AFTER_COMMAND("command://notation/add-grace32-after");

// add beam commands
inline static const muse::rcommand::Command ADD_BEAM_AUTO_COMMAND("command://notation/add-beam-auto");
inline static const muse::rcommand::Command ADD_BEAM_NONE_COMMAND("command://notation/add-beam-none");
inline static const muse::rcommand::Command ADD_BEAM_BEGIN_COMMAND("command://notation/add-beam-begin");
inline static const muse::rcommand::Command ADD_BEAM_BEGIN16_COMMAND("command://notation/add-beam-begin16");
inline static const muse::rcommand::Command ADD_BEAM_BEGIN32_COMMAND("command://notation/add-beam-begin32");
inline static const muse::rcommand::Command ADD_BEAM_MID_COMMAND("command://notation/add-beam-mid");
inline static const muse::rcommand::Command ADD_BEAM_SELECTED_RANGE_COMMAND("command://notation/add-beam-selected-range");

// add brackets commands
inline static const muse::rcommand::Command ADD_BRACKETS_COMMAND("command://notation/add-brackets");
inline static const muse::rcommand::Command ADD_PARENTHESES_COMMAND("command://notation/add-parentheses");
inline static const muse::rcommand::Command ADD_BRACES_COMMAND("command://notation/add-braces");

// add ornament commands
inline static const muse::rcommand::Command ADD_TURN_COMMAND("command://notation/add-turn");
inline static const muse::rcommand::Command ADD_TURN_INVERTED_COMMAND("command://notation/add-turn-inverted");
inline static const muse::rcommand::Command ADD_TURN_SLASH_COMMAND("command://notation/add-turn-slash");
inline static const muse::rcommand::Command ADD_TURN_UP_COMMAND("command://notation/add-turn-up");
inline static const muse::rcommand::Command ADD_TURN_INVERTED_UP_COMMAND("command://notation/add-turn-inverted-up");
inline static const muse::rcommand::Command ADD_TRILL_COMMAND("command://notation/add-trill");
inline static const muse::rcommand::Command ADD_SHORT_TRILL_COMMAND("command://notation/add-short-trill");
inline static const muse::rcommand::Command ADD_MORDENT_COMMAND("command://notation/add-mordent");
inline static const muse::rcommand::Command ADD_HAYDN_COMMAND("command://notation/add-haydn");
inline static const muse::rcommand::Command ADD_TREMBLEMENT_COMMAND("command://notation/add-tremblement");
inline static const muse::rcommand::Command ADD_PRALL_MORDENT_COMMAND("command://notation/add-prall-mordent");
inline static const muse::rcommand::Command ADD_SHAKE_COMMAND("command://notation/add-shake");
inline static const muse::rcommand::Command ADD_SHAKE_MUFFAT_COMMAND("command://notation/add-shake-muffat");
inline static const muse::rcommand::Command ADD_TREMBLEMENT_COUPERIN_COMMAND("command://notation/add-tremblement-couperin");

// clef commands
inline static const muse::rcommand::Command ADD_CLEF_VIOLIN_COMMAND("command://notation/add-clef-violin");
inline static const muse::rcommand::Command ADD_CLEF_BASS_COMMAND("command://notation/add-clef-bass");

inline static const muse::rcommand::Command ADD_FULL_MEASURE_REST_COMMAND("command://notation/add-full-measure-rest");

// interval commands
inline static const muse::rcommand::Command ADD_INTERVAL_PLUS_1_COMMAND("command://notation/add-interval-plus1");
inline static const muse::rcommand::Command ADD_INTERVAL_PLUS_2_COMMAND("command://notation/add-interval-plus2");
inline static const muse::rcommand::Command ADD_INTERVAL_PLUS_3_COMMAND("command://notation/add-interval-plus3");
inline static const muse::rcommand::Command ADD_INTERVAL_PLUS_4_COMMAND("command://notation/add-interval-plus4");
inline static const muse::rcommand::Command ADD_INTERVAL_PLUS_5_COMMAND("command://notation/add-interval-plus5");
inline static const muse::rcommand::Command ADD_INTERVAL_PLUS_6_COMMAND("command://notation/add-interval-plus6");
inline static const muse::rcommand::Command ADD_INTERVAL_PLUS_7_COMMAND("command://notation/add-interval-plus7");
inline static const muse::rcommand::Command ADD_INTERVAL_PLUS_8_COMMAND("command://notation/add-interval-plus8");
inline static const muse::rcommand::Command ADD_INTERVAL_PLUS_9_COMMAND("command://notation/add-interval-plus9");
inline static const muse::rcommand::Command ADD_INTERVAL_PLUS_10_COMMAND("command://notation/add-interval-plus10");
inline static const muse::rcommand::Command ADD_INTERVAL_MINUS_2_COMMAND("command://notation/add-interval-minus2");
inline static const muse::rcommand::Command ADD_INTERVAL_MINUS_3_COMMAND("command://notation/add-interval-minus3");
inline static const muse::rcommand::Command ADD_INTERVAL_MINUS_4_COMMAND("command://notation/add-interval-minus4");
inline static const muse::rcommand::Command ADD_INTERVAL_MINUS_5_COMMAND("command://notation/add-interval-minus5");
inline static const muse::rcommand::Command ADD_INTERVAL_MINUS_6_COMMAND("command://notation/add-interval-minus6");
inline static const muse::rcommand::Command ADD_INTERVAL_MINUS_7_COMMAND("command://notation/add-interval-minus7");
inline static const muse::rcommand::Command ADD_INTERVAL_MINUS_8_COMMAND("command://notation/add-interval-minus8");
inline static const muse::rcommand::Command ADD_INTERVAL_MINUS_9_COMMAND("command://notation/add-interval-minus9");
inline static const muse::rcommand::Command ADD_INTERVAL_MINUS_10_COMMAND("command://notation/add-interval-minus10");

// editing commands
inline static const muse::rcommand::Command COPY_COMMAND("command://notation/copy");
inline static const muse::rcommand::Command COPY_PASTE_SWAP_COMMAND("command://notation/copy-paste-swap");
inline static const muse::rcommand::Command COPY_LYRICS_COMMAND("command://notation/copy-lyrics");
inline static const muse::rcommand::Command CUT_COMMAND("command://notation/cut");
inline static const muse::rcommand::Command PASTE_COMMAND("command://notation/paste");
inline static const muse::rcommand::Command PASTE_HALF_COMMAND("command://notation/paste-half");
inline static const muse::rcommand::Command PASTE_DOUBLE_COMMAND("command://notation/paste-double");
inline static const muse::rcommand::Command PASTE_SPECIAL_COMMAND("command://notation/paste-special");
inline static const muse::rcommand::Command DELETE_COMMAND("command://notation/delete");

// move commands
inline static const muse::rcommand::Command MOVE_RIGHT_COMMAND("command://notation/move-right");
inline static const muse::rcommand::Command MOVE_LEFT_COMMAND("command://notation/move-left");
inline static const muse::rcommand::Command MOVE_RIGHT_QUICKLY_COMMAND("command://notation/move-right-quickly");
inline static const muse::rcommand::Command MOVE_LEFT_QUICKLY_COMMAND("command://notation/move-left-quickly");
inline static const muse::rcommand::Command MOVE_UP_COMMAND("command://notation/move-up");
inline static const muse::rcommand::Command MOVE_DOWN_COMMAND("command://notation/move-down");
inline static const muse::rcommand::Command SWAP_LEFT_COMMAND("command://notation/swap-left");
inline static const muse::rcommand::Command SWAP_RIGHT_COMMAND("command://notation/swap-right");

inline static const muse::rcommand::Command PITCH_UP_COMMAND("command://notation/pitch-up");
inline static const muse::rcommand::Command PITCH_DOWN_COMMAND("command://notation/pitch-down");
inline static const muse::rcommand::Command PITCH_UP_OCTAVE_COMMAND("command://notation/pitch-up-octave");
inline static const muse::rcommand::Command PITCH_DOWN_OCTAVE_COMMAND("command://notation/pitch-down-octave");
inline static const muse::rcommand::Command PITCH_UP_DIATONIC_COMMAND("command://notation/pitch-up-diatonic");
inline static const muse::rcommand::Command PITCH_DOWN_DIATONIC_COMMAND("command://notation/pitch-down-diatonic");
inline static const muse::rcommand::Command PITCH_UP_DIATONIC_ALTERATIONS_COMMAND("command://notation/pitch-up-diatonic-alterations");
inline static const muse::rcommand::Command PITCH_DOWN_DIATONIC_ALTERATIONS_COMMAND("command://notation/pitch-down-diatonic-alterations");

// properties commands
inline static const muse::rcommand::Command TOGGLE_VISIBLE_COMMAND("command://notation/toggle-visible");

// snap commands
inline static const muse::rcommand::Command TOGGLE_SNAP_TO_PREV_COMMAND("command://notation/toggle-snap-to-prev");
inline static const muse::rcommand::Command TOGGLE_SNAP_TO_NEXT_COMMAND("command://notation/toggle-snap-to-next");

// layout commands

// layout commands: measure
inline static const muse::rcommand::Command MOVE_MEASURE_TO_PREV_SYSTEM_COMMAND("command://notation/move-measure-to-prev-system");
inline static const muse::rcommand::Command MOVE_MEASURE_TO_NEXT_SYSTEM_COMMAND("command://notation/move-measure-to-next-system");
inline static const muse::rcommand::Command SPLIT_MEASURE_COMMAND("command://notation/split-measure");
inline static const muse::rcommand::Command JOIN_MEASURES_COMMAND("command://notation/join-measures");
inline static const muse::rcommand::Command INSERT_MEASURE_COMMAND("command://notation/insert-measure");
inline static const muse::rcommand::Command INSERT_MEASURES_COMMAND("command://notation/insert-measures"); // with params
inline static const muse::rcommand::Command INSERT_MEASURES_AFTER_SELECTION_COMMAND("command://notation/insert-measures-after-selection"); // with params
inline static const muse::rcommand::Command INSERT_MEASURES_AT_START_OF_SCORE_COMMAND(
    "command://notation/insert-measures-at-start-of-score"); // with params
inline static const muse::rcommand::Command APPEND_MEASURE_COMMAND("command://notation/append-measure");
inline static const muse::rcommand::Command APPEND_MEASURES_COMMAND("command://notation/append-measures"); // with params

// layout commands: section
inline static const muse::rcommand::Command TOGGLE_SECTION_BREAK_COMMAND("command://notation/toggle-section-break");

// layout commands: system
inline static const muse::rcommand::Command TOGGLE_SYSTEM_BREAK_COMMAND("command://notation/toggle-system-break");
inline static const muse::rcommand::Command APPLY_SYSTEM_LOCK_COMMAND("command://notation/apply-system-lock");
inline static const muse::rcommand::Command TOGGLE_SYSTEM_LOCK_COMMAND("command://notation/toggle-system-lock");
inline static const muse::rcommand::Command MAKE_INTO_SYSTEM_COMMAND("command://notation/make-into-system");
inline static const muse::rcommand::Command MOVE_SYSTEM_TO_PREV_PAGE_COMMAND("command://notation/move-system-to-prev-page");
inline static const muse::rcommand::Command MOVE_SYSTEM_TO_NEXT_PAGE_COMMAND("command://notation/move-system-to-next-page");

// layout commands: page
inline static const muse::rcommand::Command TOGGLE_PAGE_BREAK_COMMAND("command://notation/toggle-page-break");
inline static const muse::rcommand::Command APPLY_PAGE_LOCK_COMMAND("command://notation/apply-page-lock");
inline static const muse::rcommand::Command TOGGLE_PAGE_LOCK_COMMAND("command://notation/toggle-page-lock");
inline static const muse::rcommand::Command MAKE_INTO_PAGE_COMMAND("command://notation/make-into-page");

// layout commands: score
inline static const muse::rcommand::Command TOGGLE_SCORE_LOCK_COMMAND("command://notation/toggle-score-lock");

// layout commands: stretch
inline static const muse::rcommand::Command STRETCH_DECREASE_COMMAND("command://notation/stretch-decrease");
inline static const muse::rcommand::Command STRETCH_INCREASE_COMMAND("command://notation/stretch-increase");
inline static const muse::rcommand::Command STRETCH_RESET_COMMAND("command://notation/stretch-reset");

// open properties
inline static const muse::rcommand::Command OPEN_PAGE_SETTINGS_COMMAND("command://notation/open-page-settings");
inline static const muse::rcommand::Command OPEN_EDIT_STRINGS_COMMAND("command://notation/open-edit-strings");
inline static const muse::rcommand::Command OPEN_BREAKS_COMMAND("command://notation/open-breaks");
inline static const muse::rcommand::Command OPEN_STAFF_PROPERTIES_COMMAND("command://notation/open-staff-properties");
inline static const muse::rcommand::Command OPEN_STAFF_TEXT_PROPERTIES_COMMAND("command://notation/open-staff-text-properties");
inline static const muse::rcommand::Command OPEN_SYSTEM_TEXT_PROPERTIES_COMMAND("command://notation/open-system-text-properties");
inline static const muse::rcommand::Command OPEN_MEASURE_PROPERTIES_COMMAND("command://notation/open-measure-properties");
inline static const muse::rcommand::Command OPEN_TRANSPOSE_COMMAND("command://notation/open-transpose");
inline static const muse::rcommand::Command OPEN_PARTS_COMMAND("command://notation/open-parts");
inline static const muse::rcommand::Command OPEN_EDITGRIDSIZE_COMMAND("command://notation/open-editgridsize");
inline static const muse::rcommand::Command OPEN_REALIZECHORDSYMBOLS_COMMAND("command://notation/open-realizechordsymbols");

// style commands
inline static const muse::rcommand::Command LOAD_STYLE_COMMAND("command://notation/load-style");
inline static const muse::rcommand::Command SAVE_STYLE_COMMAND("command://notation/save-style");
inline static const muse::rcommand::Command OPEN_EDIT_STYLE_COMMAND("command://notation/open-edit-style");
inline static const muse::rcommand::Command TOGGLE_CONCERT_PITCH_COMMAND("command://notation/toggle-concert-pitch");

// reset commands
inline static const muse::rcommand::Command RESET_TEXT_STYLE_OVERRIDES_COMMAND("command://notation/reset-text-style-overrides");
inline static const muse::rcommand::Command RESET_BEAMS_COMMAND("command://notation/reset-beams");
inline static const muse::rcommand::Command RESET_SHAPES_AND_POSITIONS_COMMAND("command://notation/reset-shapes-and-positions");
inline static const muse::rcommand::Command RESET_TO_DEFAULT_LAYOUT_COMMAND("command://notation/reset-to-default-layout");

// show commands
inline static const muse::rcommand::Command SHOW_INVISIBLE_COMMAND("command://notation/show-invisible");
inline static const muse::rcommand::Command SHOW_UNPRINTABLE_COMMAND("command://notation/show-unprintable");
inline static const muse::rcommand::Command SHOW_FRAMES_COMMAND("command://notation/show-frames");
inline static const muse::rcommand::Command SHOW_PAGEBORDERS_COMMAND("command://notation/show-pageborders");
inline static const muse::rcommand::Command SHOW_SOUNDFLAGS_COMMAND("command://notation/show-soundflags");
inline static const muse::rcommand::Command SHOW_IRREGULAR_COMMAND("command://notation/show-irregular");

// staff commands
inline static const muse::rcommand::Command STAFF_EXPLODE_COMMAND("command://notation/staff-explode");
inline static const muse::rcommand::Command STAFF_IMPLODE_COMMAND("command://notation/staff-implode");

// remove commands
inline static const muse::rcommand::Command REMOVE_SELECTED_RANGE_COMMAND("command://notation/remove-selected-range");
inline static const muse::rcommand::Command REMOVE_EMPTY_TRAILING_MEASURES_COMMAND("command://notation/remove-empty-trailing-measures");

// slash commands
inline static const muse::rcommand::Command SLASH_FILL_COMMAND("command://notation/slash-fill");
inline static const muse::rcommand::Command SLASH_RHYTHM_COMMAND("command://notation/slash-rhythm");

// spelling commands
inline static const muse::rcommand::Command PITCH_SPELL_COMMAND("command://notation/pitch-spell");
inline static const muse::rcommand::Command PITCH_SPELL_SHARPS_COMMAND("command://notation/pitch-spell-sharps");
inline static const muse::rcommand::Command PITCH_SPELL_FLATS_COMMAND("command://notation/pitch-spell-flats");
inline static const muse::rcommand::Command ENHARMONIC_SPELL_BOTH_COMMAND("command://notation/enharmonic-spell-both");
inline static const muse::rcommand::Command ENHARMONIC_SPELL_CURRENT_COMMAND("command://notation/enharmonic-spell-current");

// screen commands
inline static const muse::rcommand::Command SCREEN_PUT_NOTE_COMMAND("command://notation/screen-put-note"); // with params
inline static const muse::rcommand::Command SCREEN_REMOVE_NOTE_COMMAND("command://notation/screen-remove-note"); // with params
inline static const muse::rcommand::Command SCREEN_EDIT_TEXT_COMMAND("command://notation/screen-edit-text"); // with params
inline static const muse::rcommand::Command SCREEN_EDIT_ELEMENT_COMMAND("command://notation/screen-edit-element"); // with params

// others commands
inline static const muse::rcommand::Command REGROUP_RHYTHMS_COMMAND("command://notation/regroup-rhythms");
inline static const muse::rcommand::Command RESEQUENCE_REHEARSAL_MARKS_COMMAND("command://notation/resequence-rehearsal-marks");
inline static const muse::rcommand::Command UNROLL_REPEATS_COMMAND("command://notation/unroll-repeats");
inline static const muse::rcommand::Command REPEAT_SELECTION_COMMAND("command://notation/repeat-selection");
inline static const muse::rcommand::Command TRANSPOSE_UP_COMMAND("command://notation/transpose-up");
inline static const muse::rcommand::Command TRANSPOSE_DOWN_COMMAND("command://notation/transpose-down");
inline static const muse::rcommand::Command TOGGLE_MMREST_COMMAND("command://notation/toggle-mmrest");
inline static const muse::rcommand::Command TOGGLE_HIDE_EMPTY_COMMAND("command://notation/toggle-hide-empty");
inline static const muse::rcommand::Command MIRROR_NOTEHEAD_COMMAND("command://notation/mirror-notehead");
inline static const muse::rcommand::Command SET_VISIBLE_COMMAND("command://notation/set-visible");
inline static const muse::rcommand::Command UNSET_VISIBLE_COMMAND("command://notation/unset-visible");
inline static const muse::rcommand::Command TOGGLE_AUTOPLACE_COMMAND("command://notation/toggle-autoplace");
inline static const muse::rcommand::Command AUTOPLACE_ENABLED_COMMAND("command://notation/autoplace-enabled");
inline static const muse::rcommand::Command VOICE_ASSIGNMENT_ALL_IN_INSTR_COMMAND("command://notation/voice-assignment-all-in-instrument");
inline static const muse::rcommand::Command VOICE_ASSIGNMENT_ALL_IN_STAFF_COMMAND("command://notation/voice-assignment-all-in-staff");
inline static const muse::rcommand::Command TOGGLE_AUTOMATION_COMMAND("command://notation/toggle-automation");
inline static const muse::rcommand::Command SELECT_AUTOMATION_TYPE_COMMAND("command://notation/select-automation-type"); // with params

// TAB commands
inline static const muse::rcommand::Command SET_DURATION_WHOLE_TAB_COMMAND("command://notation/set-duration-whole-tab");
inline static const muse::rcommand::Command SET_DURATION_HALF_TAB_COMMAND("command://notation/set-duration-half-tab");
inline static const muse::rcommand::Command SET_DURATION_QUARTER_TAB_COMMAND("command://notation/set-duration-quarter-tab");
inline static const muse::rcommand::Command SET_DURATION_EIGHTH_TAB_COMMAND("command://notation/set-duration-eighth-tab");
inline static const muse::rcommand::Command SET_DURATION_16TH_TAB_COMMAND("command://notation/set-duration-16th-tab");
inline static const muse::rcommand::Command SET_DURATION_32ND_TAB_COMMAND("command://notation/set-duration-32nd-tab");
inline static const muse::rcommand::Command SET_DURATION_64TH_TAB_COMMAND("command://notation/set-duration-64th-tab");
inline static const muse::rcommand::Command SET_DURATION_128TH_TAB_COMMAND("command://notation/set-duration-128th-tab");
inline static const muse::rcommand::Command SET_DURATION_256TH_TAB_COMMAND("command://notation/set-duration-256th-tab");
inline static const muse::rcommand::Command SET_DURATION_512TH_TAB_COMMAND("command://notation/set-duration-512th-tab");
inline static const muse::rcommand::Command SET_DURATION_1024TH_TAB_COMMAND("command://notation/set-duration-1024th-tab");

inline static const muse::rcommand::Command ENTER_FRET_0_COMMAND("command://notation/enter-fret-0");
inline static const muse::rcommand::Command ENTER_FRET_1_COMMAND("command://notation/enter-fret-1");
inline static const muse::rcommand::Command ENTER_FRET_2_COMMAND("command://notation/enter-fret-2");
inline static const muse::rcommand::Command ENTER_FRET_3_COMMAND("command://notation/enter-fret-3");
inline static const muse::rcommand::Command ENTER_FRET_4_COMMAND("command://notation/enter-fret-4");
inline static const muse::rcommand::Command ENTER_FRET_5_COMMAND("command://notation/enter-fret-5");
inline static const muse::rcommand::Command ENTER_FRET_6_COMMAND("command://notation/enter-fret-6");
inline static const muse::rcommand::Command ENTER_FRET_7_COMMAND("command://notation/enter-fret-7");
inline static const muse::rcommand::Command ENTER_FRET_8_COMMAND("command://notation/enter-fret-8");
inline static const muse::rcommand::Command ENTER_FRET_9_COMMAND("command://notation/enter-fret-9");
inline static const muse::rcommand::Command ENTER_FRET_10_COMMAND("command://notation/enter-fret-10");
inline static const muse::rcommand::Command ENTER_FRET_11_COMMAND("command://notation/enter-fret-11");
inline static const muse::rcommand::Command ENTER_FRET_12_COMMAND("command://notation/enter-fret-12");
inline static const muse::rcommand::Command ENTER_FRET_13_COMMAND("command://notation/enter-fret-13");
inline static const muse::rcommand::Command ENTER_FRET_14_COMMAND("command://notation/enter-fret-14");

inline static const muse::rcommand::Command ENTER_REST_TAB_COMMAND("command://notation/enter-rest-tab");

inline static const muse::rcommand::Command ADD_STANDARD_BEND_COMMAND("command://notation/add-standard-bend");
inline static const muse::rcommand::Command ADD_PRE_BEND_COMMAND("command://notation/add-pre-bend");
inline static const muse::rcommand::Command ADD_GRACE_NOTE_BEND_COMMAND("command://notation/add-grace-note-bend");
inline static const muse::rcommand::Command ADD_SLIGHT_BEND_COMMAND("command://notation/add-slight-bend");
inline static const muse::rcommand::Command ADD_DIVE_COMMAND("command://notation/add-dive");
inline static const muse::rcommand::Command ADD_PRE_DIVE_COMMAND("command://notation/add-pre-dive");
inline static const muse::rcommand::Command ADD_DIP_COMMAND("command://notation/add-dip");
inline static const muse::rcommand::Command ADD_SCOOP_COMMAND("command://notation/add-scoop");

inline static const muse::rcommand::Command ADD_HAMMER_ON_PULL_OFF_COMMAND("command://notation/add-hammer-on-pull-off");

inline static const muse::rcommand::Command GOTO_STRING_ABOVE_COMMAND("command://notation/goto-string-above");
inline static const muse::rcommand::Command GOTO_STRING_BELOW_COMMAND("command://notation/goto-string-below");

// view commands
inline static const muse::rcommand::Command ZOOM_IN_COMMAND("command://notation/view/zoom-in");
inline static const muse::rcommand::Command ZOOM_OUT_COMMAND("command://notation/view/zoom-out");
inline static const muse::rcommand::Command ZOOM_TO_PAGE_WIDTH_COMMAND("command://notation/view/zoom-to-page-width");
inline static const muse::rcommand::Command ZOOM_TO_WHOLE_PAGE_COMMAND("command://notation/view/zoom-to-whole-page");
inline static const muse::rcommand::Command ZOOM_TO_TWO_PAGES_COMMAND("command://notation/view/zoom-to-two-pages");
inline static const muse::rcommand::Command ZOOM_TO_100_COMMAND("command://notation/view/zoom-to-100");
inline static const muse::rcommand::Command ZOOM_TO_PERCENT_COMMAND("command://notation/view/zoom-to-percent"); // with params

inline static const muse::rcommand::Command VIEW_MODE_PAGE_COMMAND("command://notation/view/mode-page");
inline static const muse::rcommand::Command VIEW_MODE_FLOAT_COMMAND("command://notation/view/mode-float");
inline static const muse::rcommand::Command VIEW_MODE_CONTINUOUS_COMMAND("command://notation/view/mode-continuous");
inline static const muse::rcommand::Command VIEW_MODE_SINGLE_COMMAND("command://notation/view/mode-single");

inline static const muse::rcommand::Command NEXT_SCREEN_COMMAND("command://notation/view/next-screen");
inline static const muse::rcommand::Command PREV_SCREEN_COMMAND("command://notation/view/prev-screen");
inline static const muse::rcommand::Command NEXT_PAGE_COMMAND("command://notation/view/next-page");
inline static const muse::rcommand::Command PREV_PAGE_COMMAND("command://notation/view/prev-page");
inline static const muse::rcommand::Command TOP_OF_FIRST_PAGE_COMMAND("command://notation/view/top-of-first-page");
inline static const muse::rcommand::Command BOTTOM_OF_LAST_PAGE_COMMAND("command://notation/view/bottom-of-last-page");

inline static const muse::rcommand::Command CONTEXT_MENU_OF_SELECTION_COMMAND("command://notation/view/context-menu-of-selection");

inline static const muse::rcommand::Command SHOW_SEARCH_COMMAND("command://notation/view/show-search");

// piano keyboard commands
inline static const muse::rcommand::Command PIANO_KEYBOARD_SET_NUMBER_OF_KEYS_COMMAND("command://notation/piano-keyboard/set-number-of-keys");

// diagnostic commands
inline static const muse::rcommand::Command DIAGNOSTIC_VIEW_REDRAW_COMMAND("command://notation/diagnostic/view-redraw");

inline static const muse::rcommand::Command SHOW_ELEMENT_BOUNDING_RECTS_COMMAND("command://notation/diagnostic/show-element-bounding-rects");
inline static const muse::rcommand::Command COLOR_ELEMENT_SHAPES_COMMAND("command://notation/diagnostic/color-element-shapes");
inline static const muse::rcommand::Command SHOW_SEGMENT_SHAPES_COMMAND("command://notation/diagnostic/show-segment-shapes");
inline static const muse::rcommand::Command COLOR_SEGMENT_SHAPES_COMMAND("command://notation/diagnostic/color-segment-shapes");
inline static const muse::rcommand::Command SHOW_SKYLINES_COMMAND("command://notation/diagnostic/show-skylines");
inline static const muse::rcommand::Command SHOW_SYSTEM_BOUNDING_RECTS_COMMAND("command://notation/diagnostic/show-system-bounding-rects");
inline static const muse::rcommand::Command SHOW_ELEMENT_MASKS_COMMAND("command://notation/diagnostic/show-element-masks");
inline static const muse::rcommand::Command SHOW_LINE_ATTACH_POINTS_COMMAND("command://notation/diagnostic/show-line-attach-points");
inline static const muse::rcommand::Command MARK_EMPTY_STAFF_COMMAND("command://notation/diagnostic/mark-empty-staff");
inline static const muse::rcommand::Command MARK_CORRUPTED_MEASURES_COMMAND("command://notation/diagnostic/mark-corrupted-measures");
inline static const muse::rcommand::Command SHOW_GAP_RESTS_COMMAND("command://notation/diagnostic/show-gap-rests");
inline static const muse::rcommand::Command SHOW_ORIGIN_AND_COMBINED_COMMAND("command://notation/diagnostic/show-origin-and-combined");
inline static const muse::rcommand::Command CHECK_FOR_SCORE_CORRUPTIONS_COMMAND("command://notation/diagnostic/check-for-score-corruptions");
}
