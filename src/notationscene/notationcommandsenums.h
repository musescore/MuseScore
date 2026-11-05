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
using Table = muse::rcommand::Table<E, N>;

struct NoteNameContract {
    static constexpr std::string_view key = "note";
    static constexpr NoteName def = NoteName::C;
    static constexpr Table<NoteName, 7> table = { {
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
    static constexpr Table<NoteInputMethod, 7 > table { {
        { "by-note-name", NoteInputMethod::BY_NOTE_NAME, "input by note name" },
        { "by-duration", NoteInputMethod::BY_DURATION, "input by duration" },
        { "repitch", NoteInputMethod::REPITCH, "repitch" },
        { "rhythm", NoteInputMethod::RHYTHM, "rhythm" },
        { "realtime-auto", NoteInputMethod::REALTIME_AUTO, "realtime auto" },
        { "realtime-manual", NoteInputMethod::REALTIME_MANUAL, "realtime manual" },
        { "timewise", NoteInputMethod::TIMEWISE, "timewise" },
    } };
};

struct NoteAddingModeContract {
    static constexpr std::string_view key = "mode";
    static constexpr NoteAddingMode def = NoteAddingMode::CurrentChord;
    static constexpr Table<NoteAddingMode, 3> table { {
        { "current", NoteAddingMode::CurrentChord, "add to current chord" },
        { "next", NoteAddingMode::NextChord, "add to next chord" },
        { "insert", NoteAddingMode::InsertChord, "insert to chord" },
    } };
};

struct TupletNumberTypeContract {
    static constexpr std::string_view key = "number-type";
    static constexpr engraving::TupletNumberType def = engraving::TupletNumberType::SHOW_NUMBER;
    static constexpr Table<engraving::TupletNumberType, 3> table { {
        { "number", engraving::TupletNumberType::SHOW_NUMBER, "show number" },
        { "relation", engraving::TupletNumberType::SHOW_RELATION, "show relation" },
        { "none", engraving::TupletNumberType::NO_TEXT, "no text" },
    } };
};

struct TupletBracketTypeContract {
    static constexpr std::string_view key = "bracket-type";
    static constexpr engraving::TupletBracketType def = engraving::TupletBracketType::AUTO_BRACKET;
    static constexpr Table<engraving::TupletBracketType, 3> table { {
        { "auto", engraving::TupletBracketType::AUTO_BRACKET, "auto bracket" },
        { "show", engraving::TupletBracketType::SHOW_BRACKET, "show bracket" },
        { "none", engraving::TupletBracketType::SHOW_NO_BRACKET, "no bracket" },
    } };
};

struct TupletSelectedDurationContract {
    static constexpr std::string_view key = "selected-duration-as";
    static constexpr engraving::TupletSelectedDuration def = engraving::TupletSelectedDuration::TUPLET_LENGTH;
    static constexpr Table<engraving::TupletSelectedDuration, 2> table { {
        { "subdivision", engraving::TupletSelectedDuration::SUBDIVISION, "subdivision" },
        { "tuplet-length", engraving::TupletSelectedDuration::TUPLET_LENGTH, "tuplet length" },
    } };
};

struct SelectionTargetContract {
    static constexpr std::string_view key = "target";
    static constexpr SelectionTarget def = SelectionTarget::FirstItem;
    static constexpr Table<SelectionTarget, 30> table { {
        { "first-item", SelectionTarget::FirstItem, "select first item" },
        { "last-item", SelectionTarget::LastItem, "select last item" },
        { "next-item", SelectionTarget::NextItem, "select next item" },
        { "prev-item", SelectionTarget::PrevItem, "select previous item" },
        { "next-segment-item", SelectionTarget::NextSegmentItem, "select next segment item" },
        { "prev-segment-item", SelectionTarget::PrevSegmentItem, "select previous segment item" },
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
    } };
};

struct PlayModeContract {
    static constexpr std::string_view key = "play-mode";
    static constexpr PlayMode def = PlayMode::NoPlay;
    static constexpr Table<PlayMode, 3> table { {
        { "none", PlayMode::NoPlay, "no play" },
        { "note", PlayMode::PlayNote, "play note" },
        { "chord", PlayMode::PlayChord, "play chord" },
    } };
};
}

namespace muse::rcommand {
DECLARE_CONTRACT(mu::notation::NoteName, mu::notation::NoteNameContract);
DECLARE_CONTRACT(mu::notation::NoteInputMethod, mu::notation::NoteInputMethodContract);
DECLARE_CONTRACT(mu::notation::NoteAddingMode, mu::notation::NoteAddingModeContract);
DECLARE_CONTRACT(mu::engraving::TupletNumberType, mu::notation::TupletNumberTypeContract);
DECLARE_CONTRACT(mu::engraving::TupletBracketType, mu::notation::TupletBracketTypeContract);
DECLARE_CONTRACT(mu::engraving::TupletSelectedDuration, mu::notation::TupletSelectedDurationContract);
DECLARE_CONTRACT(mu::notation::SelectionTarget, mu::notation::SelectionTargetContract);
DECLARE_CONTRACT(mu::notation::PlayMode, mu::notation::PlayModeContract);
}
