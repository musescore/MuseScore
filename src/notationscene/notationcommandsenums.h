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
        { "c", NoteName::C },
        { "d", NoteName::D },
        { "e", NoteName::E },
        { "f", NoteName::F },
        { "g", NoteName::G },
        { "a", NoteName::A },
        { "b", NoteName::B },
    } };
};

struct NoteInputMethodContract {
    static constexpr std::string_view key = "method";
    static constexpr NoteInputMethod def = NoteInputMethod::BY_NOTE_NAME;
    static constexpr Table<NoteInputMethod, 7 > table { {
        { "by-note-name", NoteInputMethod::BY_NOTE_NAME },
        { "by-duration", NoteInputMethod::BY_DURATION },
        { "repitch", NoteInputMethod::REPITCH },
        { "rhythm", NoteInputMethod::RHYTHM },
        { "realtime-auto", NoteInputMethod::REALTIME_AUTO },
        { "realtime-manual", NoteInputMethod::REALTIME_MANUAL },
        { "timewise", NoteInputMethod::TIMEWISE },
    } };
};

struct NoteAddingModeContract {
    static constexpr std::string_view key = "mode";
    static constexpr NoteAddingMode def = NoteAddingMode::CurrentChord;
    static constexpr Table<NoteAddingMode, 3> table { {
        { "current", NoteAddingMode::CurrentChord },
        { "next", NoteAddingMode::NextChord },
        { "insert", NoteAddingMode::InsertChord },
    } };
};

struct TupletNumberTypeContract {
    static constexpr std::string_view key = "number-type";
    static constexpr engraving::TupletNumberType def = engraving::TupletNumberType::SHOW_NUMBER;
    static constexpr Table<engraving::TupletNumberType, 3> table { {
        { "number", engraving::TupletNumberType::SHOW_NUMBER },
        { "relation", engraving::TupletNumberType::SHOW_RELATION },
        { "none", engraving::TupletNumberType::NO_TEXT },
    } };
};

struct TupletBracketTypeContract {
    static constexpr std::string_view key = "bracket-type";
    static constexpr engraving::TupletBracketType def = engraving::TupletBracketType::AUTO_BRACKET;
    static constexpr Table<engraving::TupletBracketType, 3> table { {
        { "auto", engraving::TupletBracketType::AUTO_BRACKET },
        { "show", engraving::TupletBracketType::SHOW_BRACKET },
        { "none", engraving::TupletBracketType::SHOW_NO_BRACKET },
    } };
};

struct SelectionTargetContract {
    static constexpr std::string_view key = "target";
    static constexpr SelectionTarget def = SelectionTarget::FirstItem;
    static constexpr Table<SelectionTarget, 30> table { {
        { "first-item", SelectionTarget::FirstItem },
        { "last-item", SelectionTarget::LastItem },
        { "next-item", SelectionTarget::NextItem },
        { "prev-item", SelectionTarget::PrevItem },
        { "next-segment-item", SelectionTarget::NextSegmentItem },
        { "prev-segment-item", SelectionTarget::PrevSegmentItem },
        { "next-chord", SelectionTarget::NextChord },
        { "prev-chord", SelectionTarget::PrevChord },
        { "next-measure", SelectionTarget::NextMeasure },
        { "prev-measure", SelectionTarget::PrevMeasure },
        { "next-track", SelectionTarget::NextTrack },
        { "prev-track", SelectionTarget::PrevTrack },
        { "next-frame", SelectionTarget::NextFrame },
        { "prev-frame", SelectionTarget::PrevFrame },
        { "next-system", SelectionTarget::NextSystem },
        { "prev-system", SelectionTarget::PrevSystem },
        { "above-staff", SelectionTarget::AboveStaff },
        { "below-staff", SelectionTarget::BelowStaff },
        { "up-note-in-chord", SelectionTarget::UpNoteInChord },
        { "down-note-in-chord", SelectionTarget::DownNoteInChord },
        { "top-note-in-chord", SelectionTarget::TopNoteInChord },
        { "bottom-note-in-chord", SelectionTarget::BottomNoteInChord },
        { "top-staff", SelectionTarget::TopStaff },
        { "notes-in-chord", SelectionTarget::NotesInChord },
        { "empty-trailing-measure", SelectionTarget::EmptyTrailingMeasure },
        { "similar", SelectionTarget::Similar },
        { "similar-in-staff", SelectionTarget::SimilarInStaff },
        { "similar-in-range", SelectionTarget::SimilarInRange },
        { "all", SelectionTarget::All },
        { "section", SelectionTarget::Section },
    } };
};

struct PlayModeContract {
    static constexpr std::string_view key = "play-mode";
    static constexpr PlayMode def = PlayMode::NoPlay;
    static constexpr Table<PlayMode, 3> table { {
        { "none", PlayMode::NoPlay },
        { "note", PlayMode::PlayNote },
        { "chord", PlayMode::PlayChord },
    } };
};
}

namespace muse::rcommand {
DECLARE_CONTRACT(mu::notation::NoteName, mu::notation::NoteNameContract);
DECLARE_CONTRACT(mu::notation::NoteInputMethod, mu::notation::NoteInputMethodContract);
DECLARE_CONTRACT(mu::notation::NoteAddingMode, mu::notation::NoteAddingModeContract);
DECLARE_CONTRACT(mu::engraving::TupletNumberType, mu::notation::TupletNumberTypeContract);
DECLARE_CONTRACT(mu::engraving::TupletBracketType, mu::notation::TupletBracketTypeContract);
DECLARE_CONTRACT(mu::notation::SelectionTarget, mu::notation::SelectionTargetContract);
DECLARE_CONTRACT(mu::notation::PlayMode, mu::notation::PlayModeContract);
}
