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

//! NOTE Serialization and deserialization of command parameter enums – a command contract
// In a separate file, because there’s a lot of repetitive, boilerplate code

#include "rcommand/enumcontract.h"
#include "notation/notationtypes.h"
#include "notation/types/noteinputtypes.h"
#include "engraving/types/types.h"

namespace mu::notation {
template<typename E, std::size_t N>
using EnumTable = muse::rcommand::EnumTable<E, N>;

struct NoteNameContract {
    static constexpr std::string_view key = "note";
    static constexpr NoteName def = NoteName::C;
    static constexpr EnumTable<NoteName, 7> table = { {
        { "c", NoteName::C, "note C" },
        { "d", NoteName::D, "note D" },
        { "e", NoteName::E, "note E" },
        { "f", NoteName::F, "note F" },
        { "g", NoteName::G, "note G" },
        { "a", NoteName::A, "note A" },
        { "b", NoteName::B, "note B" },
    } };
};

struct NoteInputMethodContract {
    static constexpr std::string_view key = "method";
    static constexpr NoteInputMethod def = NoteInputMethod::BY_NOTE_NAME;
    static constexpr EnumTable<NoteInputMethod, 8 > table { {
        { "default", NoteInputMethod::DEFAULT, "default note input method" },
        { "by-note-name", NoteInputMethod::BY_NOTE_NAME, "input by note name" },
        { "by-duration", NoteInputMethod::BY_DURATION, "input by duration" },
        { "repitch", NoteInputMethod::REPITCH, "repitch" },
        { "rhythm", NoteInputMethod::RHYTHM, "rhythm" },
        { "realtime-auto", NoteInputMethod::REALTIME_AUTO, "realtime auto" },
        { "realtime-manual", NoteInputMethod::REALTIME_MANUAL, "realtime manual" },
        { "timewise", NoteInputMethod::TIMEWISE, "timewise" },
    } };
};

struct DurationTypeContract {
    static constexpr std::string_view key = "duration";
    static constexpr DurationType def = DurationType::V_QUARTER;
    static constexpr EnumTable<DurationType, 13> table { {
        { "longa", DurationType::V_LONG, "longa" },
        { "breve", DurationType::V_BREVE, "breve" },
        { "whole", DurationType::V_WHOLE, "whole" },
        { "half", DurationType::V_HALF, "half" },
        { "quarter", DurationType::V_QUARTER, "quarter" },
        { "eighth", DurationType::V_EIGHTH, "eighth" },
        { "16th", DurationType::V_16TH, "16th" },
        { "32nd", DurationType::V_32ND, "32nd" },
        { "64th", DurationType::V_64TH, "64th" },
        { "128th", DurationType::V_128TH, "128th" },
        { "256th", DurationType::V_256TH, "256th" },
        { "512th", DurationType::V_512TH, "512th" },
        { "1024th", DurationType::V_1024TH, "1024th" },
    } };
};

struct ChangeDurationModeContract {
    static constexpr std::string_view key = "mode";
    static constexpr ChangeDurationMode def = ChangeDurationMode::Double;
    static constexpr EnumTable<ChangeDurationMode, 4> table { {
        { "double", ChangeDurationMode::Double, "double" },
        { "halve", ChangeDurationMode::Halve, "halve" },
        { "double-dotted", ChangeDurationMode::DoubleDotted, "double dotted" },
        { "halve-dotted", ChangeDurationMode::HalveDotted, "halve dotted" },
    } };
};

struct NoteAddingModeContract {
    static constexpr std::string_view key = "mode";
    static constexpr NoteAddingMode def = NoteAddingMode::CurrentChord;
    static constexpr EnumTable<NoteAddingMode, 3> table { {
        { "to-current-chord", NoteAddingMode::CurrentChord, "add to current chord" },
        { "to-next-chord", NoteAddingMode::NextChord, "add to next chord" },
        { "insert-to-chord", NoteAddingMode::InsertChord, "insert to chord" },
    } };
};

struct TupletNumberTypeContract {
    static constexpr std::string_view key = "number-type";
    static constexpr TupletNumberType def = TupletNumberType::SHOW_NUMBER;
    static constexpr EnumTable<TupletNumberType, 4> table { {
        { "number", TupletNumberType::SHOW_NUMBER, "show number" },
        { "relation", TupletNumberType::SHOW_RELATION, "show relation" },
        { "none", TupletNumberType::NO_TEXT, "no text" },
        { "default", TupletNumberType::DEFAULT, "default (from style)" },
    } };
};

struct TupletBracketTypeContract {
    static constexpr std::string_view key = "bracket-type";
    static constexpr TupletBracketType def = TupletBracketType::AUTO_BRACKET;
    static constexpr EnumTable<TupletBracketType, 4> table { {
        { "auto", TupletBracketType::AUTO_BRACKET, "auto bracket" },
        { "show", TupletBracketType::SHOW_BRACKET, "show bracket" },
        { "none", TupletBracketType::SHOW_NO_BRACKET, "no bracket" },
        { "default", TupletBracketType::DEFAULT, "default (from style)" },
    } };
};

struct TupletSelectedDurationContract {
    static constexpr std::string_view key = "selected-duration-as";
    static constexpr TupletSelectedDuration def = TupletSelectedDuration::TUPLET_LENGTH;
    static constexpr EnumTable<TupletSelectedDuration, 3> table { {
        { "subdivision", TupletSelectedDuration::SUBDIVISION, "subdivision" },
        { "tuplet-length", TupletSelectedDuration::TUPLET_LENGTH, "tuplet length" },
        { "default", TupletSelectedDuration::DEFAULT, "default (from configuration)" },
    } };
};

struct SelectionTargetContract {
    static constexpr std::string_view key = "target";
    static constexpr SelectionTarget def = SelectionTarget::FirstItem;
    static constexpr EnumTable<SelectionTarget, 34> table { {
        { "first-item", SelectionTarget::FirstItem, "select first item" },
        { "last-item", SelectionTarget::LastItem, "select last item" },
        { "next-item", SelectionTarget::NextItem, "select next item" },
        { "prev-item", SelectionTarget::PrevItem, "select previous item" },
        { "next-segment", SelectionTarget::NextSegment, "select next segment" },
        { "prev-segment", SelectionTarget::PrevSegment, "select previous segment" },
        { "next-chord", SelectionTarget::NextChord, "select next chord" },
        { "prev-chord", SelectionTarget::PrevChord, "select previous chord" },
        { "next-measure", SelectionTarget::NextMeasure, "select next measure" },
        { "prev-measure", SelectionTarget::PrevMeasure, "select previous measure" },
        { "next-track", SelectionTarget::NextTrack, "select next track" },
        { "prev-track", SelectionTarget::PrevTrack, "select previous track" },
        { "next-frame", SelectionTarget::NextFrame, "select next frame" },
        { "prev-frame", SelectionTarget::PrevFrame, "select previous frame" },
        { "next-system", SelectionTarget::NextSystem, "select next system" },
        { "prev-system", SelectionTarget::PrevSystem, "select previous system" },
        { "above-staff", SelectionTarget::AboveStaff, "select above staff" },
        { "below-staff", SelectionTarget::BelowStaff, "select below staff" },
        { "up-note-in-chord", SelectionTarget::UpNoteInChord, "select up note in chord" },
        { "down-note-in-chord", SelectionTarget::DownNoteInChord, "select down note in chord" },
        { "top-note-in-chord", SelectionTarget::TopNoteInChord, "select top note in chord" },
        { "bottom-note-in-chord", SelectionTarget::BottomNoteInChord, "select bottom note in chord" },
        { "top-staff", SelectionTarget::TopStaff, "select top staff" },
        { "notes-in-chord", SelectionTarget::NotesInChord, "select notes in chord" },
        { "empty-trailing-measure", SelectionTarget::EmptyTrailingMeasure, "select empty trailing measure" },
        { "similar", SelectionTarget::Similar, "select similar" },
        { "similar-in-staff", SelectionTarget::SimilarInStaff, "select similar in staff" },
        { "similar-in-range", SelectionTarget::SimilarInRange, "select similar in range" },
        { "all", SelectionTarget::All, "select all" },
        { "section", SelectionTarget::Section, "select section" },
        { "begin-system", SelectionTarget::BeginSystem, "select begin system" },
        { "end-system", SelectionTarget::EndSystem, "select end system" },
        { "begin-score", SelectionTarget::BeginScore, "select begin score" },
        { "end-score", SelectionTarget::EndScore, "select end score" },
    } };
};

struct PlayModeContract {
    static constexpr std::string_view key = "play-mode";
    static constexpr PlayMode def = PlayMode::NoPlay;
    static constexpr EnumTable<PlayMode, 3> table { {
        { "none", PlayMode::NoPlay, "no play" },
        { "note", PlayMode::PlayNote, "play note" },
        { "chord", PlayMode::PlayChord, "play chord" },
    } };
};

struct AccidentalTypeContract {
    static constexpr std::string_view key = "type";
    static constexpr AccidentalType def = AccidentalType::NATURAL;
    //! NOTE Only these types are used in the commands
    static constexpr EnumTable<AccidentalType, 5> table { {
        { "flat2", AccidentalType::FLAT2, "double flat" },
        { "flat", AccidentalType::FLAT, "flat" },
        { "natural", AccidentalType::NATURAL, "natural" },
        { "sharp", AccidentalType::SHARP, "sharp" },
        { "sharp2", AccidentalType::SHARP2, "double sharp" },
    } };
};

struct BoxTypeContract {
    static constexpr std::string_view key = "box-type";
    static constexpr BoxType def = BoxType::Unknown;
    static constexpr EnumTable<BoxType, 5> table { {
        { "vertical", BoxType::Vertical, "vertical" },
        { "horizontal", BoxType::Horizontal, "horizontal" },
        { "measure", BoxType::Measure, "measure" },
        { "text", BoxType::Text, "text" },
        { "fret", BoxType::Fret, "fret" },
    } };
};

struct AddBoxesTargetContract {
    static constexpr std::string_view key = "target";
    static constexpr AddBoxesTarget def = AddBoxesTarget::BeforeSelection;
    static constexpr EnumTable<AddBoxesTarget, 4> table { {
        { "after-selection", AddBoxesTarget::AfterSelection, "after selection" },
        { "before-selection", AddBoxesTarget::BeforeSelection, "before selection" },
        { "at-start-of-score", AddBoxesTarget::AtStartOfScore, "at start of score" },
        { "at-end-of-score", AddBoxesTarget::AtEndOfScore, "at end of score" },
    } };
};
}

namespace muse::rcommand {
DECLARE_ENUM_CONTRACT(mu::notation::NoteName, mu::notation::NoteNameContract);
DECLARE_ENUM_CONTRACT(mu::notation::NoteInputMethod, mu::notation::NoteInputMethodContract);
DECLARE_ENUM_CONTRACT(mu::engraving::DurationType, mu::notation::DurationTypeContract);
DECLARE_ENUM_CONTRACT(mu::notation::ChangeDurationMode, mu::notation::ChangeDurationModeContract);
DECLARE_ENUM_CONTRACT(mu::notation::NoteAddingMode, mu::notation::NoteAddingModeContract);
DECLARE_ENUM_CONTRACT(mu::notation::TupletNumberType, mu::notation::TupletNumberTypeContract);
DECLARE_ENUM_CONTRACT(mu::notation::TupletBracketType, mu::notation::TupletBracketTypeContract);
DECLARE_ENUM_CONTRACT(mu::notation::TupletSelectedDuration, mu::notation::TupletSelectedDurationContract);
DECLARE_ENUM_CONTRACT(mu::notation::SelectionTarget, mu::notation::SelectionTargetContract);
DECLARE_ENUM_CONTRACT(mu::notation::PlayMode, mu::notation::PlayModeContract);
DECLARE_ENUM_CONTRACT(mu::notation::AccidentalType, mu::notation::AccidentalTypeContract);
DECLARE_ENUM_CONTRACT(mu::notation::BoxType, mu::notation::BoxTypeContract);
DECLARE_ENUM_CONTRACT(mu::notation::AddBoxesTarget, mu::notation::AddBoxesTargetContract);
}
