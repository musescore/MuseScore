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

#include "notationcommandsregister.h"

#include "../notationcommands.h"
#include "rcommand/commandtypes.h"

using namespace muse;
using namespace muse::rcommand;
using namespace muse::ui;
using namespace mu::notation;

// avoid translation duplication

// //: This is comment for translator

//: Note
static const TranslatableString noteC = TranslatableString("action", "C");
//: Note
static const TranslatableString noteD = TranslatableString("action", "D");
//: Note
static const TranslatableString noteE = TranslatableString("action", "E");
//: Note
static const TranslatableString noteF = TranslatableString("action", "F");
//: Note
static const TranslatableString noteG = TranslatableString("action", "G");
//: Note
static const TranslatableString noteA = TranslatableString("action", "A");
//: Note
static const TranslatableString noteB = TranslatableString("action", "B");

static const TranslatableString Enter_note_X = TranslatableString("action", "Enter note %1");
static const TranslatableString Add_X_to_chord = TranslatableString("action", "Add %1 to chord");
static const TranslatableString Add_note_X_to_chord = TranslatableString("action", "Add note %1 to chord");
static const TranslatableString Insert_X = TranslatableString("action", "Insert %1");

static const std::vector<CommandInfo> s_commandInfos = {
    // copy, cut, paste, delete, cancel
    CommandInfo{
        COPY_COMMAND,
        TranslatableString("action", "&Copy"),
        TranslatableString("action", "Copy the current notation element"),
        InputSchema(),
        Decoration(IconCode::Code::COPY)
    },
    CommandInfo{
        COPY_PASTE_SWAP_COMMAND,
        TranslatableString("action", "&Swap with clipboard"),
        TranslatableString("action", "Copy/paste: swap with clipboard"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        CUT_COMMAND,
        TranslatableString("action", "Cu&t"),
        TranslatableString("action", "Cut the current notation element"),
        InputSchema(),
        Decoration(IconCode::Code::CUT)
    },
    CommandInfo{
        PASTE_COMMAND,
        TranslatableString("action", "Past&e"),
        TranslatableString("action", "Paste the current notation element"),
        InputSchema(),
        Decoration(IconCode::Code::PASTE)
    },
    CommandInfo{
        PASTE_HALF_COMMAND,
        TranslatableString("action", "Paste &half duration"),
        TranslatableString("action", "Paste half duration"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PASTE_DOUBLE_COMMAND,
        TranslatableString("action", "Paste &double duration"),
        TranslatableString("action", "Paste double duration"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PASTE_SPECIAL_COMMAND,
        TranslatableString("action", "Paste special"),
        TranslatableString("action", "Paste special"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        DELETE_COMMAND,
        TranslatableString("action", "De&lete"),
        TranslatableString("action", "Delete the current notation element"),
        InputSchema(),
        Decoration(IconCode::Code::DELETE_TANK)
    },
    CommandInfo{
        CANCEL_COMMAND,
        TranslatableString("action", "Cancel"),
        TranslatableString("action", "Cancel the current notation operation"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        UNDO_COMMAND,
        TranslatableString("action", "Undo"),
        TranslatableString("action", "Undo the last notation operation"),
        InputSchema(),
        Decoration(IconCode::Code::UNDO)
    },
    CommandInfo{
        REDO_COMMAND,
        TranslatableString("action", "Redo"),
        TranslatableString("action", "Redo the last notation operation"),
        InputSchema(),
        Decoration(IconCode::Code::REDO)
    },

    // navigation
    CommandInfo{
        MOVE_RIGHT_COMMAND,
        TranslatableString("action", "Move right"),
        TranslatableString("action", "Go to next notation element"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        MOVE_RIGHT_QUICKLY_COMMAND,
        TranslatableString("action", "Move right quickly"),
        TranslatableString("action", "Go to next notation element quickly"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        MOVE_LEFT_COMMAND,
        TranslatableString("action", "Move left"),
        TranslatableString("action", "Go to previous notation element"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        MOVE_LEFT_QUICKLY_COMMAND,
        TranslatableString("action", "Move left quickly"),
        TranslatableString("action", "Go to previous notation element quickly"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PITCH_UP_COMMAND,
        TranslatableString("action", "Pitch up"),
        TranslatableString("action", "Pitch up the current note"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PITCH_DOWN_COMMAND,
        TranslatableString("action", "Pitch down"),
        TranslatableString("action", "Pitch down the current note"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PITCH_UP_OCTAVE_COMMAND,
        TranslatableString("action", "Pitch up octave"),
        TranslatableString("action", "Pitch up the current note by an octave"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PITCH_DOWN_OCTAVE_COMMAND,
        TranslatableString("action", "Pitch down octave"),
        TranslatableString("action", "Pitch down the current note by an octave"),
        InputSchema(),
        Decoration()
    },

    // note input
    CommandInfo{
        TOGGLE_NOTE_INPUT_COMMAND,
        TranslatableString("action", "Note input"),
        TranslatableString("action", "Toggle note input mode"),
        InputSchema(),
        Decoration(IconCode::Code::EDIT, rcommand::Checkable::Yes)
    },
    CommandInfo{
        TOGGLE_NOTE_INPUT_BY_NOTE_NAME_COMMAND,
        TranslatableString("action", "Note input by note name"),
        TranslatableString("action", "Toggle note input mode by note name"),
        InputSchema(),
        Decoration(IconCode::Code::EDIT, rcommand::Checkable::Yes)
    },
    CommandInfo{
        TOGGLE_NOTE_INPUT_BY_DURATION_COMMAND,
        TranslatableString("action", "Note input by duration"),
        TranslatableString("action", "Toggle note input mode by duration"),
        InputSchema(),
        Decoration(IconCode::Code::DURATION_CURSOR, rcommand::Checkable::Yes)
    },
    CommandInfo{
        TOGGLE_NOTE_INPUT_RHYTHM_COMMAND,
        TranslatableString("action", "Rhythm only (not pitch)"),
        TranslatableString("action", "Toggle note input mode: rhythm only (not pitch)"),
        InputSchema(),
        Decoration(IconCode::Code::RHYTHM_ONLY, rcommand::Checkable::Yes)
    },
    CommandInfo{
        TOGGLE_NOTE_INPUT_REPITCH_COMMAND,
        TranslatableString("action", "Re-pitch existing notes"),
        TranslatableString("action", "Toggle note input mode: re-pitch existing notes"),
        InputSchema(),
        Decoration(IconCode::Code::RE_PITCH, rcommand::Checkable::Yes)
    },
    CommandInfo{
        TOGGLE_NOTE_INPUT_REALTIME_AUTO_COMMAND,
        TranslatableString("action", "Real-time (metronome)"),
        TranslatableString("action", "Toggle note input mode: real-time (metronome)"),
        InputSchema(),
        Decoration(IconCode::Code::METRONOME, rcommand::Checkable::Yes)
    },
    CommandInfo{
        TOGGLE_NOTE_INPUT_REALTIME_MANUAL_COMMAND,
        TranslatableString("action", "Real-time (foot pedal)"),
        TranslatableString("action", "Toggle note input mode: real-time (foot pedal)"),
        InputSchema(),
        Decoration(IconCode::Code::FOOT_PEDAL, rcommand::Checkable::Yes)
    },
    CommandInfo{
        TOGGLE_NOTE_INPUT_TIMEWISE_COMMAND,
        TranslatableString("action", "Insert"),
        TranslatableString("action", "Toggle note input mode: insert (increases measure duration)"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_PLUS, rcommand::Checkable::Yes)
    },
    CommandInfo{
        REALTIME_ADVANCE_COMMAND,
        TranslatableString("action", "Real-time advance"),
        TranslatableString("action", "Real-time advance"),
        InputSchema(),
        Decoration(IconCode::Code::METRONOME)
    },
    CommandInfo{
        SET_DURATION_LONGA_COMMAND,
        TranslatableString("action", "Longa"),
        TranslatableString("action", "Set duration: longa"),
        InputSchema(),
        Decoration(IconCode::Code::LONGO)
    },
    CommandInfo{
        SET_DURATION_BREVE_COMMAND,
        TranslatableString("action", "Breve"),
        TranslatableString("action", "Set duration: breve"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_WHOLE_DOUBLE)
    },
    CommandInfo{
        SET_DURATION_WHOLE_COMMAND,
        TranslatableString("action", "Whole note"),
        TranslatableString("action", "Set duration: whole note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_WHOLE)
    },
    CommandInfo{
        SET_DURATION_HALF_COMMAND,
        TranslatableString("action", "Half note"),
        TranslatableString("action", "Set duration: half note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_HALF)
    },
    CommandInfo{
        SET_DURATION_QUARTER_COMMAND,
        TranslatableString("action", "Quarter note"),
        TranslatableString("action", "Set duration: quarter note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_QUARTER)
    },
    CommandInfo{
        SET_DURATION_EIGHTH_COMMAND,
        TranslatableString("action", "Eighth note"),
        TranslatableString("action", "Set duration: eighth note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_8TH)
    },
    CommandInfo{
        SET_DURATION_16TH_COMMAND,
        TranslatableString("action", "16th note"),
        TranslatableString("action", "Set duration: 16th note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_16TH)
    },
    CommandInfo{
        SET_DURATION_32ND_COMMAND,
        TranslatableString("action", "32nd note"),
        TranslatableString("action", "Set duration: 32nd note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_32ND)
    },
    CommandInfo{
        SET_DURATION_64TH_COMMAND,
        TranslatableString("action", "64th note"),
        TranslatableString("action", "Set duration: 64th note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_64TH)
    },
    CommandInfo{
        SET_DURATION_128TH_COMMAND,
        TranslatableString("action", "128th note"),
        TranslatableString("action", "Set duration: 128th note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_128TH)
    },
    CommandInfo{
        SET_DURATION_256TH_COMMAND,
        TranslatableString("action", "256th note"),
        TranslatableString("action", "Set duration: 256th note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_256TH)
    },
    CommandInfo{
        SET_DURATION_512TH_COMMAND,
        TranslatableString("action", "512th note"),
        TranslatableString("action", "Set duration: 512th note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_512TH)
    },
    CommandInfo{
        SET_DURATION_1024TH_COMMAND,
        TranslatableString("action", "1024th note"),
        TranslatableString("action", "Set duration: 1024th note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_1024TH)
    },
    CommandInfo{
        SET_DOUBLE_DURATION_COMMAND,
        TranslatableString("action", "Double duration"),
        TranslatableString("action", "Double selected duration"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        SET_HALVE_DURATION_COMMAND,
        TranslatableString("action", "Halve duration"),
        TranslatableString("action", "Halve selected duration"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        SET_DOUBLE_DURATION_DOTTED_COMMAND,
        TranslatableString("action", "Double selected duration (dotted)"),
        TranslatableString("action", "Double selected duration (includes dotted values)"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        SET_HALVE_DURATION_DOTTED_COMMAND,
        TranslatableString("action", "Halve selected duration (dotted)"),
        TranslatableString("action", "Halve selected duration (includes dotted values)"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        TOGGLE_DOT_COMMAND,
        TranslatableString("action", "Augmentation dot"),
        TranslatableString("action", "Toggle duration dot"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_DOTTED)
    },
    CommandInfo{
        TOGGLE_DOT2_COMMAND,
        TranslatableString("action", "Double augmentation dot"),
        TranslatableString("action", "Toggle duration dot: double"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_DOTTED_2)
    },
    CommandInfo{
        TOGGLE_DOT3_COMMAND,
        TranslatableString("action", "Triple augmentation dot"),
        TranslatableString("action", "Toggle duration dot: triple"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_DOTTED_3)
    },
    CommandInfo{
        TOGGLE_DOT4_COMMAND,
        TranslatableString("action", "Quadruple augmentation dot"),
        TranslatableString("action", "Toggle duration dot: quadruple"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_DOTTED_4)
    },
    CommandInfo{
        TOGGLE_REST_COMMAND,
        TranslatableString("action", "Rest"),
        TranslatableString("action", "Toggle rest"),
        InputSchema(),
        Decoration(IconCode::Code::REST)
    },
    CommandInfo{
        TOGGLE_FLAT2_COMMAND,
        TranslatableString("action", "Toggle double-flat"),
        TranslatableString("action", "Toggle accidental: double-flat"),
        InputSchema(),
        Decoration(IconCode::Code::FLAT_DOUBLE)
    },
    CommandInfo{
        TOGGLE_FLAT_COMMAND,
        TranslatableString("action", "Toggle flat"),
        TranslatableString("action", "Toggle accidental: flat"),
        InputSchema(),
        Decoration(IconCode::Code::FLAT)
    },
    CommandInfo{
        TOGGLE_NAT_COMMAND,
        TranslatableString("action", "Toggle natural"),
        TranslatableString("action", "Toggle accidental: natural"),
        InputSchema(),
        Decoration(IconCode::Code::NATURAL)
    },
    CommandInfo{
        TOGGLE_SHARP_COMMAND,
        TranslatableString("action", "Toggle sharp"),
        TranslatableString("action", "Toggle accidental: sharp"),
        InputSchema(),
        Decoration(IconCode::Code::SHARP)
    },
    CommandInfo{
        TOGGLE_SHARP2_COMMAND,
        TranslatableString("action", "Toggle double-sharp"),
        TranslatableString("action", "Toggle accidental: double-sharp"),
        InputSchema(),
        Decoration(IconCode::Code::SHARP_DOUBLE)
    },
    CommandInfo{
        TOGGLE_TIE_COMMAND,
        TranslatableString("action", "Tie"),
        TranslatableString("action", "Add tied note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_TIE)
    },
    CommandInfo{
        ADD_SLUR_COMMAND,
        TranslatableString("action", "Slur"),
        TranslatableString("action", "Add slur"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_SLUR)
    },
    CommandInfo{
        TOGGLE_LV_COMMAND,
        TranslatableString("action", "Laissez vibrer"),
        TranslatableString("action", "Add laissez vibrer"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_LV)
    },
    CommandInfo{
        TOGGLE_MARCATO_COMMAND,
        TranslatableString("action", "Marcato"),
        TranslatableString("action", "Add articulation: marcato"),
        InputSchema(),
        Decoration(IconCode::Code::MARCATO)
    },
    CommandInfo{
        TOGGLE_SFORZATO_COMMAND,
        TranslatableString("action", "Accent"),
        TranslatableString("action", "Add articulation: accent"),
        InputSchema(),
        Decoration(IconCode::Code::ACCENT)
    },
    CommandInfo{
        TOGGLE_TENUTO_COMMAND,
        TranslatableString("action", "Tenuto"),
        TranslatableString("action", "Add articulation: tenuto"),
        InputSchema(),
        Decoration(IconCode::Code::TENUTO)
    },
    CommandInfo{
        TOGGLE_STACCATO_COMMAND,
        TranslatableString("action", "Staccato"),
        TranslatableString("action", "Add articulation: staccato"),
        InputSchema(),
        Decoration(IconCode::Code::STACCATO)
    },
    CommandInfo{
        USE_VOICE_1_COMMAND,
        TranslatableString("action", "Voice 1"),
        TranslatableString("action", "Use voice 1"),
        InputSchema(),
        Decoration(IconCode::Code::VOICE_1)
    },
    CommandInfo{
        USE_VOICE_2_COMMAND,
        TranslatableString("action", "Voice 2"),
        TranslatableString("action", "Use voice 2"),
        InputSchema(),
        Decoration(IconCode::Code::VOICE_2)
    },
    CommandInfo{
        USE_VOICE_3_COMMAND,
        TranslatableString("action", "Voice 3"),
        TranslatableString("action", "Use voice 3"),
        InputSchema(),
        Decoration(IconCode::Code::VOICE_3)
    },
    CommandInfo{
        USE_VOICE_4_COMMAND,
        TranslatableString("action", "Voice 4"),
        TranslatableString("action", "Use voice 4"),
        InputSchema(),
        Decoration(IconCode::Code::VOICE_4)
    },
    CommandInfo{
        SWAP_VOICE_X12_COMMAND,
        TranslatableString("action", "Exchange voice &1-2"),
        TranslatableString("action", "Exchange voice 1-2"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        SWAP_VOICE_X13_COMMAND,
        TranslatableString("action", "Exchange voice 1-3"),
        TranslatableString("action", "Exchange voice 1-3"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        SWAP_VOICE_X14_COMMAND,
        TranslatableString("action", "Exchange voice 1-&4"),
        TranslatableString("action", "Exchange voice 1-4"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        SWAP_VOICE_X23_COMMAND,
        TranslatableString("action", "Exchange voice &2-3"),
        TranslatableString("action", "Exchange voice 2-3"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        SWAP_VOICE_X24_COMMAND,
        TranslatableString("action", "Exchange voice 2-4"),
        TranslatableString("action", "Exchange voice 2-4"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        SWAP_VOICE_X34_COMMAND,
        TranslatableString("action", "Exchange voice &3-4"),
        TranslatableString("action", "Exchange voice 3-4"),
        InputSchema(),
        Decoration()
    },

    CommandInfo{
        FLIP_COMMAND,
        TranslatableString("action", "Flip direction"),
        TranslatableString("action", "Flip direction"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_FLIP)
    },
    CommandInfo{
        FLIP_HORIZONTALLY_COMMAND,
        TranslatableString("action", "Flip horizontally"),
        TranslatableString("action", "Flip horizontally"),
        InputSchema(),
        Decoration()
    },

    // note operation
    CommandInfo{
        ADD_NOTE_COMMAND,
        TranslatableString("action", "Add note"),
        TranslatableString("action", "Add note"),
        InputSchema({
            { "note", Arg(DataType::String, u"Note name (c, d, e, f, g, a, b)") },
            { "mode", Arg(DataType::String, u"Adding mode (current, next, insert)") },
        }),
        Decoration()
    },
    CommandInfo{
        ENTER_NOTE_C_COMMAND,
        noteC,
        Enter_note_X.arg(noteC),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ENTER_NOTE_D_COMMAND,
        noteD,
        Enter_note_X.arg(noteD),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ENTER_NOTE_E_COMMAND,
        noteE,
        Enter_note_X.arg(noteE),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ENTER_NOTE_F_COMMAND,
        noteF,
        Enter_note_X.arg(noteF),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ENTER_NOTE_G_COMMAND,
        noteG,
        Enter_note_X.arg(noteG),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ENTER_NOTE_A_COMMAND,
        noteA,
        Enter_note_X.arg(noteA),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ENTER_NOTE_B_COMMAND,
        noteB,
        Enter_note_X.arg(noteB),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_NOTE_C_COMMAND,
        Add_X_to_chord.arg(noteC),
        Add_note_X_to_chord.arg(noteC),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_NOTE_D_COMMAND,
        Add_X_to_chord.arg(noteD),
        Add_note_X_to_chord.arg(noteD),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_NOTE_E_COMMAND,
        Add_X_to_chord.arg(noteE),
        Add_note_X_to_chord.arg(noteE),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_NOTE_F_COMMAND,
        Add_X_to_chord.arg(noteF),
        Add_note_X_to_chord.arg(noteF),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_NOTE_G_COMMAND,
        Add_X_to_chord.arg(noteG),
        Add_note_X_to_chord.arg(noteG),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_NOTE_A_COMMAND,
        Add_X_to_chord.arg(noteA),
        Add_note_X_to_chord.arg(noteA),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_NOTE_B_COMMAND,
        Add_X_to_chord.arg(noteB),
        Add_note_X_to_chord.arg(noteB),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        INSERT_NOTE_C_COMMAND,
        Insert_X.arg(noteC),
        Insert_X.arg(noteC),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        INSERT_NOTE_D_COMMAND,
        Insert_X.arg(noteD),
        Insert_X.arg(noteD),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        INSERT_NOTE_E_COMMAND,
        Insert_X.arg(noteE),
        Insert_X.arg(noteE),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        INSERT_NOTE_F_COMMAND,
        Insert_X.arg(noteF),
        Insert_X.arg(noteF),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        INSERT_NOTE_G_COMMAND,
        Insert_X.arg(noteG),
        Insert_X.arg(noteG),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        INSERT_NOTE_A_COMMAND,
        Insert_X.arg(noteA),
        Insert_X.arg(noteA),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        INSERT_NOTE_B_COMMAND,
        Insert_X.arg(noteB),
        Insert_X.arg(noteB),
        InputSchema(),
        Decoration()
    },

    CommandInfo{
        ENTER_REST_COMMAND,
        TranslatableString("action", "Rest"),
        TranslatableString("action", "Enter rest"),
        InputSchema(),
        Decoration(IconCode::Code::REST)
    },

    // tuplet
    CommandInfo{
        OPEN_TUPLET_CONFIGURE_COMMAND,
        TranslatableString("action", "Othe&r…"),
        TranslatableString("action", "Show tuplet configure"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_TUPLET_COMMAND,
        TranslatableString("action", "Tuplet"),
        TranslatableString("action", "Enter tuplet"),
        InputSchema({
            { "ratio", Arg(DataType::String, u"Tuplet ratio (e.g. 3/2, 4/3, 5/4, 6/5, 7/6, 8/7, 9/8)") },
            { "number-type", Arg(DataType::String, u"Tuplet number type (number, relation, none)") },
            { "bracket-type", Arg(DataType::String, u"Tuplet bracket type (auto, show, none)") },
            { "auto-baselen", Arg(DataType::Boolean, u"Auto base length") },
        }),
        Decoration()
    },
    CommandInfo{
        ADD_DUPLET_COMMAND,
        TranslatableString("action", "Duplet"),
        TranslatableString("action", "Enter tuplet: duplet"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_TRIPLET_COMMAND,
        TranslatableString("action", "Triplet"),
        TranslatableString("action", "Enter tuplet: triplet"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_QUADRUPLET_COMMAND,
        TranslatableString("action", "Quadruplet"),
        TranslatableString("action", "Enter tuplet: quadruplet"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_QUINTUPLET_COMMAND,
        TranslatableString("action", "Quintuplet"),
        TranslatableString("action", "Enter tuplet: quintuplet"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_SEXTUPLET_COMMAND,
        TranslatableString("action", "Sextuplet"),
        TranslatableString("action", "Enter tuplet: sextuplet"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_SEPTUPLET_COMMAND,
        TranslatableString("action", "Septuplet"),
        TranslatableString("action", "Enter tuplet: septuplet"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_OCTUPLET_COMMAND,
        TranslatableString("action", "Octuplet"),
        TranslatableString("action", "Enter tuplet: octuplet"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_NONUPLET_COMMAND,
        TranslatableString("action", "Nonuplet"),
        TranslatableString("action", "Enter tuplet: nonuplet"),
        InputSchema(),
        Decoration()
    },

    // navigation and selection commands

    CommandInfo{
        SELECT_COMMAND,
        TranslatableString("action", "Select"),
        TranslatableString("action", "Select items by target"),
        InputSchema({
            { "target",
              Arg(DataType::String,
                  u"Selection target (first-item, last-item, next-item, prev-item, "
                  u"next-chord, prev-chord, next-measure, prev-measure, "
                  u"next-track, prev-track, next-frame, prev-frame, next-system, prev-system, "
                  u"up-note-in-chord, down-note-in-chord, top-note-in-chord, bottom-note-in-chord, notes-in-chord, "
                  u"similar, similar-in-staff, similar-in-range, all, section)") },
            { "play-mode", Arg(DataType::String, u"Play mode (none, note, chord)") },
        }),
        Decoration()
    },
    CommandInfo{
        OPEN_SELECTION_OPTIONS_COMMAND,
        TranslatableString("action", "Selection options"),
        TranslatableString("action", "Open selection options"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        GOTO_FIRST_ELEMENT_COMMAND,
        TranslatableString("action", "First element"),
        TranslatableString("action", "Go to first element in score"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        GOTO_LAST_ELEMENT_COMMAND,
        TranslatableString("action", "Last element"),
        TranslatableString("action", "Go to last element in score"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        GOTO_NEXT_ELEMENT_COMMAND,
        TranslatableString("action", "Next element"),
        TranslatableString("action", "Select next element in score"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        GOTO_PREV_ELEMENT_COMMAND,
        TranslatableString("action", "Previous element"),
        TranslatableString("action", "Select previous element in score"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        GOTO_NEXT_SEGMENT_ELEMENT_COMMAND,
        TranslatableString("action", "Next segment element"),
        TranslatableString("action", "Select next segment element"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        GOTO_PREV_SEGMENT_ELEMENT_COMMAND,
        TranslatableString("action", "Previous segment element"),
        TranslatableString("action", "Select previous segment element"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        GOTO_NEXT_TRACK_COMMAND,
        TranslatableString("action", "Next staff or voice"),
        TranslatableString("action", "Go to next staff or voice"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        GOTO_PREV_TRACK_COMMAND,
        TranslatableString("action", "Previous staff or voice"),
        TranslatableString("action", "Go to previous staff or voice"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        GOTO_NEXT_FRAME_COMMAND,
        TranslatableString("action", "Next frame"),
        TranslatableString("action", "Go to next frame"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        GOTO_PREV_FRAME_COMMAND,
        TranslatableString("action", "Previous frame"),
        TranslatableString("action", "Go to previous frame"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        GOTO_NEXT_SYSTEM_COMMAND,
        TranslatableString("action", "Next system"),
        TranslatableString("action", "Go to next system"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        GOTO_PREV_SYSTEM_COMMAND,
        TranslatableString("action", "Previous system"),
        TranslatableString("action", "Go to previous system"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        GOTO_UPNOTE_IN_CHORD_COMMAND,
        TranslatableString("action", "Up note in chord"),
        TranslatableString("action", "Select note/rest above"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        GOTO_DOWNNOTE_IN_CHORD_COMMAND,
        TranslatableString("action", "Down note in chord"),
        TranslatableString("action", "Select note/rest below"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        GOTO_TOPNOTE_IN_CHORD_COMMAND,
        TranslatableString("action", "Top note in chord"),
        TranslatableString("action", "Select top note in chord"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        GOTO_BOTTOMNOTE_IN_CHORD_COMMAND,
        TranslatableString("action", "Bottom note in chord"),
        TranslatableString("action", "Select bottom note in chord"),
        InputSchema(),
        Decoration()
    },

    // selection commands
    CommandInfo{
        SELECT_SIMILAR_COMMAND,
        TranslatableString("action", "Similar"),
        TranslatableString("action", "Select similar elements"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        SELECT_SIMILAR_IN_STAFF_COMMAND,
        TranslatableString("action", "Similar on this staff"),
        TranslatableString("action", "Select similar elements on the same staff"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        SELECT_SIMILAR_IN_RANGE_COMMAND,
        TranslatableString("action", "Similar in this range"),
        TranslatableString("action", "Select similar elements in the selected range"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        SELECT_NOTES_IN_CHORD_COMMAND,
        TranslatableString("action", "Notes in chord"),
        TranslatableString("action", "Select notes in chord"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        SELECT_ALL_COMMAND,
        TranslatableString("action", "Select &all"),
        TranslatableString("action", "Select all"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        SELECT_SECTION_COMMAND,
        TranslatableString("action", "Select sectio&n"),
        TranslatableString("action", "Select section"),
        InputSchema(),
        Decoration()
    },

    // text navigation commands
    CommandInfo{
        EDITTEXT_NEXT_WORD_COMMAND,
        TranslatableString("action", "Edit next word"),
        TranslatableString("action", "Go to edit next notation word"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        EDITTEXT_NEXT_ELEMENT_COMMAND,
        TranslatableString("action", "Edit next text element"),
        TranslatableString("action", "Go to edit next notation text element"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        EDITTEXT_PREV_ELEMENT_COMMAND,
        TranslatableString("action", "Edit previous text element"),
        TranslatableString("action", "Go to edit previous notation text element"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        EDITTEXT_NEXT_BEAT_COMMAND,
        TranslatableString("action", "Advance cursor: next beat (chord symbols)"),
        TranslatableString("action", "Advance cursor: next beat (chord symbols)"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        EDITTEXT_PREV_BEAT_COMMAND,
        TranslatableString("action", "Advance cursor: previous beat (chord symbols)"),
        TranslatableString("action", "Advance cursor: previous beat (chord symbols)"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        EDITTEXT_ADVANCE_LONGA_COMMAND,
        TranslatableString("action", "Advance cursor: longa (chord symbols/figured bass)"),
        TranslatableString("action", "Advance cursor: longa (chord symbols/figured bass)"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        EDITTEXT_ADVANCE_BREVE_COMMAND,
        TranslatableString("action", "Advance cursor: breve (chord symbols/figured bass)"),
        TranslatableString("action", "Advance cursor: breve (chord symbols/figured bass)"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        EDITTEXT_ADVANCE_1_COMMAND,
        TranslatableString("action", "Advance cursor: whole note (chord symbols/figured bass)"),
        TranslatableString("action", "Advance cursor: whole note (chord symbols/figured bass)"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        EDITTEXT_ADVANCE_2_COMMAND,
        TranslatableString("action", "Advance cursor: half note (chord symbols/figured bass)"),
        TranslatableString("action", "Advance cursor: half note (chord symbols/figured bass)"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        EDITTEXT_ADVANCE_4_COMMAND,
        TranslatableString("action", "Advance cursor: quarter note (chord symbols/figured bass)"),
        TranslatableString("action", "Advance cursor: quarter note (chord symbols/figured bass)"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        EDITTEXT_ADVANCE_8_COMMAND,
        TranslatableString("action", "Advance cursor: eighth note (chord symbols/figured bass)"),
        TranslatableString("action", "Advance cursor: eighth note (chord symbols/figured bass)"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        EDITTEXT_ADVANCE_16_COMMAND,
        TranslatableString("action", "Advance cursor: 16th note (chord symbols/figured bass)"),
        TranslatableString("action", "Advance cursor: 16th note (chord symbols/figured bass)"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        EDITTEXT_ADVANCE_32_COMMAND,
        TranslatableString("action", "Advance cursor: 32nd note (chord symbols/figured bass)"),
        TranslatableString("action", "Advance cursor: 32nd note (chord symbols/figured bass)"),
        InputSchema(),
        Decoration()
    },

    CommandInfo{
        EDITTEXT_ADVANCE_64_COMMAND,
        TranslatableString("action", "Advance cursor: 64th note (chord symbols/figured bass)"),
        TranslatableString("action", "Advance cursor: 64th note (chord symbols/figured bass)"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        EDITLYRIC_NEXT_VERSE_COMMAND,
        TranslatableString("action", "Next lyric verse"),
        TranslatableString("action", "Move text/go to next lyric verse"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        EDITLYRIC_PREV_VERSE_COMMAND,
        TranslatableString("action", "Previous lyric verse"),
        TranslatableString("action", "Move text/go to previous lyric verse"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        EDITLYRIC_NEXT_SYLLABLE_COMMAND,
        TranslatableString("action", "Next syllable"),
        TranslatableString("action", "Lyrics: enter hyphen"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        EDITLYRIC_ADD_MELISMA_COMMAND,
        TranslatableString("action", "Add extension line"),
        TranslatableString("action", "Lyrics: enter extension line"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        EDITLYRIC_ADD_VERSE_COMMAND,
        TranslatableString("action", "Add lyrics verse"),
        TranslatableString("action", "Add lyrics verse"),
        InputSchema(),
        Decoration()
    },

    // properties commands
    CommandInfo{
        TOGGLE_VISIBLE_COMMAND,
        TranslatableString("action", "Toggle visibility of elements"),
        TranslatableString("action", "Toggle visibility of elements"),
        InputSchema(),
        Decoration()
    },

    // snap commands
    CommandInfo{
        TOGGLE_SNAP_TO_PREV_COMMAND,
        TranslatableString("action", "Snap to &previous"),
        TranslatableString("action", "Snap to previous"),
        InputSchema(),
        Decoration(rcommand::Checkable::Yes)
    },
    CommandInfo{
        TOGGLE_SNAP_TO_NEXT_COMMAND,
        TranslatableString("action", "Snap to &next"),
        TranslatableString("action", "Snap to next"),
        InputSchema(),
        Decoration(rcommand::Checkable::Yes)
    },

    // layout commands
    CommandInfo{
        TOGGLE_SYSTEM_BREAK_COMMAND,
        TranslatableString("action", "Add/remove system break"),
        TranslatableString("action", "Add/remove system break"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        TOGGLE_PAGE_BREAK_COMMAND,
        TranslatableString("action", "Add/remove page break"),
        TranslatableString("action", "Add/remove page break"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        TOGGLE_SECTION_BREAK_COMMAND,
        TranslatableString("action", "Add/remove section break"),
        TranslatableString("action", "Add/remove section break"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        APPLY_SYSTEM_LOCK_COMMAND,
        TranslatableString("action", "Add/remove system lock"),
        TranslatableString("action", "Add/remove system lock"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        TOGGLE_SYSTEM_LOCK_COMMAND,
        TranslatableString("action", "Lock/unlock selected system(s)"),
        TranslatableString("action", "Lock/unlock selected system(s)"),
        InputSchema(),
        Decoration(IconCode::Code::SYSTEM_LOCK)
    },
    CommandInfo{
        APPLY_PAGE_LOCK_COMMAND,
        TranslatableString("action", "Add/remove page lock"),
        TranslatableString("action", "Add/remove page lock"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        TOGGLE_PAGE_LOCK_COMMAND,
        TranslatableString("action", "Lock/unlock selected page(s)"),
        TranslatableString("action", "Lock/unlock selected page(s)"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        TOGGLE_SCORE_LOCK_COMMAND,
        TranslatableString("action", "Lock/unlock all systems"),
        TranslatableString("action", "Lock/unlock all systems"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        MAKE_INTO_SYSTEM_COMMAND,
        TranslatableString("action", "Create system from selection"),
        TranslatableString("action", "Create system from selection"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        MAKE_INTO_PAGE_COMMAND,
        TranslatableString("action", "Create page from selection"),
        TranslatableString("action", "Create page from selection"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        MOVE_MEASURE_TO_PREV_SYSTEM_COMMAND,
        TranslatableString("action", "Move measure to previous system"),
        TranslatableString("action", "Move measure to previous system"),
        InputSchema(),
        Decoration(IconCode::Code::ARROW_UP)
    },
    CommandInfo{
        MOVE_MEASURE_TO_NEXT_SYSTEM_COMMAND,
        TranslatableString("action", "Move measure to next system"),
        TranslatableString("action", "Move measure to next system"),
        InputSchema(),
        Decoration(IconCode::Code::ARROW_DOWN)
    },
    CommandInfo{
        MOVE_SYSTEM_TO_PREV_PAGE_COMMAND,
        TranslatableString("action", "Move system to previous page"),
        TranslatableString("action", "Move system to previous page"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        MOVE_SYSTEM_TO_NEXT_PAGE_COMMAND,
        TranslatableString("action", "Move system to next page"),
        TranslatableString("action", "Move system to next page"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        SPLIT_MEASURE_COMMAND,
        TranslatableString("action", "&Split measure before selected note/rest"),
        TranslatableString("action", "Split measure before selected note/rest"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        JOIN_MEASURES_COMMAND,
        TranslatableString("action", "&Join selected measures"),
        TranslatableString("action", "Join selected measures"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        INSERT_MEASURE_COMMAND,
        TranslatableString("action", "&Insert one measure before selection"),
        TranslatableString("action", "Insert one measure before selection"),
        InputSchema(),
        Decoration(IconCode::Code::INSERT_ONE_MEASURE)
    },
    CommandInfo{
        INSERT_MEASURES_COMMAND,
        TranslatableString("action", "Insert &before selection…"),
        TranslatableString("action", "Insert measures before selection"),
        InputSchema({
            { "count", Arg(DataType::Integer, u"Number of measures to insert") },
        }),
        Decoration()
    },
    CommandInfo{
        INSERT_MEASURES_AFTER_SELECTION_COMMAND,
        TranslatableString("action", "Insert &after selection…"),
        TranslatableString("action", "Insert measures after selection"),
        InputSchema({
            { "count", Arg(DataType::Integer, u"Number of measures to insert") },
        }),
        Decoration()
    },
    CommandInfo{
        INSERT_MEASURES_AT_START_OF_SCORE_COMMAND,
        TranslatableString("action", "Insert at &start of score…"),
        TranslatableString("action", "Insert measures at start of score"),
        InputSchema({
            { "count", Arg(DataType::Integer, u"Number of measures to insert") },
        }),
        Decoration()
    },
    CommandInfo{
        APPEND_MEASURE_COMMAND,
        TranslatableString("action", "Insert &one measure at end of score"),
        TranslatableString("action", "Insert one measure at end of score"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        APPEND_MEASURES_COMMAND,
        TranslatableString("action", "Insert at &end of score…"),
        TranslatableString("action", "Insert measures at end of score"),
        InputSchema({
            { "count", Arg(DataType::Integer, u"Number of measures to insert") },
        }),
        Decoration()
    },
    CommandInfo{
        INSERT_HBOX_COMMAND,
        TranslatableString("action", "Insert &horizontal frame"),
        TranslatableString("action", "Insert horizontal frame"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        INSERT_VBOX_COMMAND,
        TranslatableString("action", "Insert &vertical frame"),
        TranslatableString("action", "Insert vertical frame"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        INSERT_TEXTFRAME_COMMAND,
        TranslatableString("action", "Insert &text frame"),
        TranslatableString("action", "Insert text frame"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        INSERT_FRETFRAME_COMMAND,
        TranslatableString("action", "Insert &fretboard diagram legend"),
        TranslatableString("action", "Insert fretboard diagram legend"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        APPEND_HBOX_COMMAND,
        TranslatableString("action", "&Horizontal frame"),
        TranslatableString("action", "Insert horizontal frame at end of score"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        APPEND_VBOX_COMMAND,
        TranslatableString("action", "&Vertical frame"),
        TranslatableString("action", "Insert vertical frame at end of score"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        APPEND_TEXTFRAME_COMMAND,
        TranslatableString("action", "&Text frame"),
        TranslatableString("action", "Insert text frame at end of score"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        APPEND_FRETFRAME_COMMAND,
        TranslatableString("action", "&Fretboard diagram legend"),
        TranslatableString("action", "Insert fretboard diagram legend at end of score"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        OPEN_EDIT_STYLE_COMMAND,
        TranslatableString("action", "&Style…"),
        TranslatableString("action", "Format style"),
        InputSchema({
            { "page_code", Arg(DataType::String, u"Page code") },
            { "sub_page_code", Arg(DataType::String, u"Sub page code") },
        }),
        Decoration()
    },
    CommandInfo{
        OPEN_PAGE_SETTINGS_COMMAND,
        TranslatableString("action", "&Page settings…"),
        TranslatableString("action", "Page settings"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        OPEN_STAFF_PROPERTIES_COMMAND,
        TranslatableString("action", "Instrument / Staff properties…"),
        TranslatableString("action", "Instrument / Staff properties"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        OPEN_EDIT_STRINGS_COMMAND,
        TranslatableString("action", "Edit strings…"),
        TranslatableString("action", "Edit strings"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        OPEN_BREAKS_COMMAND,
        TranslatableString("action", "Measures per s&ystem…"),
        TranslatableString("action", "Measures per system"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        OPEN_STAFF_TEXT_PROPERTIES_COMMAND,
        TranslatableString("action", "Staff text properties…"),
        TranslatableString("action", "Staff text properties"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        OPEN_SYSTEM_TEXT_PROPERTIES_COMMAND,
        TranslatableString("action", "System text properties…"),
        TranslatableString("action", "System text properties"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        OPEN_MEASURE_PROPERTIES_COMMAND,
        TranslatableString("action", "Measure properties…"),
        TranslatableString("action", "Measure properties"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        OPEN_TRANSPOSE_COMMAND,
        TranslatableString("action", "&Transpose…"),
        TranslatableString("action", "Transpose"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        OPEN_PARTS_COMMAND,
        TranslatableString("action", "&Parts…"),
        TranslatableString("action", "Parts"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        OPEN_EDITGRIDSIZE_COMMAND,
        TranslatableString("action", "&Grid size…"),
        TranslatableString("action", "Grid size"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        OPEN_REALIZECHORDSYMBOLS_COMMAND,
        TranslatableString("action", "Realize &chord symbols"),
        TranslatableString("action", "Realize chord symbols"),
        InputSchema(),
        Decoration()
    },

    // style commands
    CommandInfo{
        LOAD_STYLE_COMMAND,
        TranslatableString("action", "&Load style…"),
        TranslatableString("action", "Load style"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        SAVE_STYLE_COMMAND,
        TranslatableString("action", "S&ave style…"),
        TranslatableString("action", "Save style"),
        InputSchema(),
        Decoration()
    },

    // fretboard diagram commands
    CommandInfo{
        ADD_FRETBOARD_DIAGRAM_COMMAND,
        TranslatableString("action", "Add &fretboard diagram"),
        TranslatableString("action", "Add fretboard diagram"),
        InputSchema(),
        Decoration()
    },

    CommandInfo{
        ADD_OTTAVA_8VA_COMMAND,
        TranslatableString("action", "Ottava 8va &alta"),
        TranslatableString("action", "Add ottava 8va alta"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_OTTAVA_8VB_COMMAND,
        TranslatableString("action", "Ottava 8va &bassa"),
        TranslatableString("action", "Add ottava 8va bassa"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_DYNAMIC_COMMAND,
        TranslatableString("action", "&Dynamic"),
        TranslatableString("action", "Add dynamic"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_HAIRPIN_COMMAND,
        TranslatableString("action", "&Crescendo"),
        TranslatableString("action", "Add hairpin: crescendo"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_HAIRPIN_REVERSE_COMMAND,
        TranslatableString("action", "&Diminuendo"),
        TranslatableString("action", "Add hairpin: diminuendo"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_NOTELINE_COMMAND,
        TranslatableString("action", "&Note-anchored line"),
        TranslatableString("action", "Add note-anchored line"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_IMAGE_COMMAND,
        TranslatableString("action", "Image"),
        TranslatableString("action", "Add image"),
        InputSchema(),
        Decoration()
    },

    // add text commands
    CommandInfo{
        ADD_TITLE_TEXT_COMMAND,
        TranslatableString("action", "&Title"),
        TranslatableString("action", "Add text: title"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_SUBTITLE_TEXT_COMMAND,
        TranslatableString("action", "&Subtitle"),
        TranslatableString("action", "Add text: subtitle"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_COMPOSER_TEXT_COMMAND,
        TranslatableString("action", "&Composer"),
        TranslatableString("action", "Add text: composer"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_LYRICIST_TEXT_COMMAND,
        TranslatableString("action", "&Lyricist"),
        TranslatableString("action", "Add text: lyricist"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_PART_TEXT_COMMAND,
        TranslatableString("action", "&Part name"),
        TranslatableString("action", "Add text: part name"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_FRAME_TEXT_COMMAND,
        TranslatableString("action", "Text"),
        TranslatableString("action", "Add frame text"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_SYSTEM_TEXT_COMMAND,
        TranslatableString("action", "Syst&em text"),
        TranslatableString("action", "Add text: system text"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_STAFF_TEXT_COMMAND,
        TranslatableString("action", "St&aff text"),
        TranslatableString("action", "Add text: staff text"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_EXPRESSION_TEXT_COMMAND,
        TranslatableString("action", "E&xpression text"),
        TranslatableString("action", "Add text: expression text"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_REHEARSALMARK_TEXT_COMMAND,
        TranslatableString("action", "&Rehearsal mark"),
        TranslatableString("action", "Add text: rehearsal mark"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_INSTRUMENT_CHANGE_TEXT_COMMAND,
        TranslatableString("action", "&Instrument change"),
        TranslatableString("action", "Add text: instrument change"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_FINGERING_TEXT_COMMAND,
        TranslatableString("action", "&Fingering"),
        TranslatableString("action", "Add text: fingering"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_STICKING_TEXT_COMMAND,
        TranslatableString("action", "Stic&king"),
        TranslatableString("action", "Add text: sticking"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_CHORD_TEXT_COMMAND,
        TranslatableString("action", "C&hord symbol"),
        TranslatableString("action", "Add text: chord symbol"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_ROMAN_NUMERAL_TEXT_COMMAND,
        TranslatableString("action", "R&oman numeral analysis"),
        TranslatableString("action", "Add text: Roman numeral analysis"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_NASHVILLE_NUMBER_TEXT_COMMAND,
        TranslatableString("action", "&Nashville number"),
        TranslatableString("action", "Add text: Nashville number"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_LYRICS_COMMAND,
        TranslatableString("action", "L&yrics"),
        TranslatableString("action", "Add text: lyrics"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_FIGURED_BASS_COMMAND,
        TranslatableString("action", "Figured &bass"),
        TranslatableString("action", "Add text: figured bass"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_TEMPO_COMMAND,
        TranslatableString("action", "Tempo &marking"),
        TranslatableString("action", "Add text: tempo marking"),
        InputSchema(),
        Decoration()
    },

    // layout commands: stretch
    CommandInfo{
        STRETCH_DECREASE_COMMAND,
        TranslatableString("action", "&Decrease layout stretch"),
        TranslatableString("action", "Decrease layout stretch"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        STRETCH_INCREASE_COMMAND,
        TranslatableString("action", "&Increase layout stretch"),
        TranslatableString("action", "Increase layout stretch"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        STRETCH_RESET_COMMAND,
        TranslatableString("action", "&Reset layout stretch"),
        TranslatableString("action", "Reset layout stretch"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        RESET_TEXT_STYLE_OVERRIDES_COMMAND,
        TranslatableString("action", "Reset &text style overrides"),
        TranslatableString("action", "Reset all text style overrides to default"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        RESET_BEAMS_COMMAND,
        TranslatableString("action", "Reset &beams"),
        TranslatableString("action", "Reset beams to default grouping"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        RESET_SHAPES_AND_POSITIONS_COMMAND,
        TranslatableString("action", "Reset s&hapes and positions"),
        TranslatableString("action", "Reset shapes and positions"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        RESET_TO_DEFAULT_LAYOUT_COMMAND,
        TranslatableString("action", "Reset entire score to &default layout"),
        TranslatableString("action", "Reset entire score to default layout"),
        InputSchema(),
        Decoration()
    },

    // show commands
    CommandInfo{
        SHOW_INVISIBLE_COMMAND,
        TranslatableString("action", "Show &invisible"),
        TranslatableString("action", "Show/hide invisible elements"),
        InputSchema(),
        Decoration(rcommand::Checkable::Yes)
    },
    CommandInfo{
        SHOW_UNPRINTABLE_COMMAND,
        TranslatableString("action", "Show f&ormatting"),
        TranslatableString("action", "Show/hide formatting"),
        InputSchema(),
        Decoration(rcommand::Checkable::Yes)
    },
    CommandInfo{
        SHOW_FRAMES_COMMAND,
        TranslatableString("action", "Show &frames"),
        TranslatableString("action", "Show/hide frames"),
        InputSchema(),
        Decoration(rcommand::Checkable::Yes)
    },
    CommandInfo{
        SHOW_PAGEBORDERS_COMMAND,
        TranslatableString("action", "Show page &margins"),
        TranslatableString("action", "Show/hide page margins"),
        InputSchema(),
        Decoration(rcommand::Checkable::Yes)
    },
    CommandInfo{
        SHOW_SOUNDFLAGS_COMMAND,
        TranslatableString("action", "Show sound flags"), // todo &
        TranslatableString("action", "Show/hide sound flags"),
        InputSchema(),
        Decoration(rcommand::Checkable::Yes)
    },
    CommandInfo{
        SHOW_IRREGULAR_COMMAND,
        TranslatableString("action", "Mark i&rregular measures"),
        TranslatableString("action", "Mark irregular measures"),
        InputSchema(),
        Decoration(rcommand::Checkable::Yes)
    }
};

std::string NotationCommandsRegister::moduleName() const
{
    return "notation";
}

const std::vector<Command>& NotationCommandsRegister::commandList() const
{
    static std::vector<Command> commands;
    if (commands.empty()) {
        commands.reserve(s_commandInfos.size());
        for (const auto& info : s_commandInfos) {
            commands.push_back(info.command);
        }
    }
    return commands;
}

const std::vector<CommandInfo>& NotationCommandsRegister::commandInfoList() const
{
    return s_commandInfos;
}
