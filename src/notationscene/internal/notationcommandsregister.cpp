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

    // text editing
    CommandInfo{
        EDIT_NEXT_WORD_COMMAND,
        TranslatableString("action", "Edit next word"),
        TranslatableString("action", "Go to edit next notation word"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        EDIT_NEXT_TEXT_ELEMENT_COMMAND,
        TranslatableString("action", "Edit next text element"),
        TranslatableString("action", "Go to edit next notation text element"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        EDIT_PREV_TEXT_ELEMENT_COMMAND,
        TranslatableString("action", "Edit previous text element"),
        TranslatableString("action", "Go to edit previous notation text element"),
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
        NOTE_LONGA_COMMAND,
        TranslatableString("action", "Longa"),
        TranslatableString("action", "Set duration: longa"),
        InputSchema(),
        Decoration(IconCode::Code::LONGO)
    },
    CommandInfo{
        NOTE_BREVE_COMMAND,
        TranslatableString("action", "Breve"),
        TranslatableString("action", "Set duration: breve"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_WHOLE_DOUBLE)
    },
    CommandInfo{
        PAD_NOTE_1_COMMAND,
        TranslatableString("action", "Whole note"),
        TranslatableString("action", "Set duration: whole note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_WHOLE)
    },
    CommandInfo{
        PAD_NOTE_2_COMMAND,
        TranslatableString("action", "Half note"),
        TranslatableString("action", "Set duration: half note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_HALF)
    },
    CommandInfo{
        PAD_NOTE_4_COMMAND,
        TranslatableString("action", "Quarter note"),
        TranslatableString("action", "Set duration: quarter note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_QUARTER)
    },
    CommandInfo{
        PAD_NOTE_8_COMMAND,
        TranslatableString("action", "Eighth note"),
        TranslatableString("action", "Set duration: eighth note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_8TH)
    },
    CommandInfo{
        PAD_NOTE_16_COMMAND,
        TranslatableString("action", "16th note"),
        TranslatableString("action", "Set duration: 16th note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_16TH)
    },
    CommandInfo{
        PAD_NOTE_32_COMMAND,
        TranslatableString("action", "32nd note"),
        TranslatableString("action", "Set duration: 32nd note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_32ND)
    },
    CommandInfo{
        PAD_NOTE_64_COMMAND,
        TranslatableString("action", "64th note"),
        TranslatableString("action", "Set duration: 64th note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_64TH)
    },
    CommandInfo{
        PAD_NOTE_128_COMMAND,
        TranslatableString("action", "128th note"),
        TranslatableString("action", "Set duration: 128th note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_128TH)
    },
    CommandInfo{
        PAD_NOTE_256_COMMAND,
        TranslatableString("action", "256th note"),
        TranslatableString("action", "Set duration: 256th note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_256TH)
    },
    CommandInfo{
        PAD_NOTE_512_COMMAND,
        TranslatableString("action", "512th note"),
        TranslatableString("action", "Set duration: 512th note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_512TH)
    },
    CommandInfo{
        PAD_NOTE_1024_COMMAND,
        TranslatableString("action", "1024th note"),
        TranslatableString("action", "Set duration: 1024th note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_1024TH)
    },
    CommandInfo{
        PAD_DOT_COMMAND,
        TranslatableString("action", "Augmentation dot"),
        TranslatableString("action", "Toggle duration dot"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_DOTTED)
    },
    CommandInfo{
        PAD_DOT2_COMMAND,
        TranslatableString("action", "Double augmentation dot"),
        TranslatableString("action", "Toggle duration dot: double"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_DOTTED_2)
    },
    CommandInfo{
        PAD_DOT3_COMMAND,
        TranslatableString("action", "Triple augmentation dot"),
        TranslatableString("action", "Toggle duration dot: triple"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_DOTTED_3)
    },
    CommandInfo{
        PAD_DOT4_COMMAND,
        TranslatableString("action", "Quadruple augmentation dot"),
        TranslatableString("action", "Toggle duration dot: quadruple"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_DOTTED_4)
    },
    CommandInfo{
        PAD_REST_COMMAND,
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
        ADD_TIE_COMMAND,
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
        ADD_LV_COMMAND,
        TranslatableString("action", "Laissez vibrer"),
        TranslatableString("action", "Add laissez vibrer"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_LV)
    },
    CommandInfo{
        ADD_MARCATO_COMMAND,
        TranslatableString("action", "Marcato"),
        TranslatableString("action", "Add articulation: marcato"),
        InputSchema(),
        Decoration(IconCode::Code::MARCATO)
    },
    CommandInfo{
        ADD_SFORZATO_COMMAND,
        TranslatableString("action", "Accent"),
        TranslatableString("action", "Add articulation: accent"),
        InputSchema(),
        Decoration(IconCode::Code::ACCENT)
    },
    CommandInfo{
        ADD_TENUTO_COMMAND,
        TranslatableString("action", "Tenuto"),
        TranslatableString("action", "Add articulation: tenuto"),
        InputSchema(),
        Decoration(IconCode::Code::TENUTO)
    },
    CommandInfo{
        ADD_STACCATO_COMMAND,
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

    // tuplet
    CommandInfo{
        SHOW_TUPLET_CONFIGURE_COMMAND,
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
                  u"next-track, prev-track, next-frame, prev-frame, next-system, prev-system, "
                  u"up-note-in-chord, down-note-in-chord, top-note-in-chord, bottom-note-in-chord, notes-in-chord, "
                  u"similar, similar-in-staff, similar-in-range, all, section)") },
            { "play-mode", Arg(DataType::String, u"Play mode (none, note, chord)") },
        }),
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
