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

namespace mu::notation {
template<typename E, std::size_t N>
using EnumTable = muse::rcommand::EnumTable<E, N>;

struct NoteNameContract {
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
    static constexpr EnumTable<ChangeDurationMode, 4> table { {
        { "double", ChangeDurationMode::Double, "double" },
        { "halve", ChangeDurationMode::Halve, "halve" },
        { "double-dotted", ChangeDurationMode::DoubleDotted, "double dotted" },
        { "halve-dotted", ChangeDurationMode::HalveDotted, "halve dotted" },
    } };
};

struct NoteAddingModeContract {
    static constexpr EnumTable<NoteAddingMode, 3> table { {
        { "to-current-chord", NoteAddingMode::CurrentChord, "add to current chord" },
        { "to-next-chord", NoteAddingMode::NextChord, "add to next chord" },
        { "insert-to-chord", NoteAddingMode::InsertChord, "insert to chord" },
    } };
};

struct TupletNumberTypeContract {
    static constexpr EnumTable<TupletNumberType, 4> table { {
        { "number", TupletNumberType::SHOW_NUMBER, "show number" },
        { "relation", TupletNumberType::SHOW_RELATION, "show relation" },
        { "none", TupletNumberType::NO_TEXT, "no text" },
        { "default", TupletNumberType::DEFAULT, "default (from style)" },
    } };
};

struct TupletBracketTypeContract {
    static constexpr EnumTable<TupletBracketType, 4> table { {
        { "auto", TupletBracketType::AUTO_BRACKET, "auto bracket" },
        { "show", TupletBracketType::SHOW_BRACKET, "show bracket" },
        { "none", TupletBracketType::SHOW_NO_BRACKET, "no bracket" },
        { "default", TupletBracketType::DEFAULT, "default (from style)" },
    } };
};

struct TupletSelectedDurationContract {
    static constexpr EnumTable<TupletSelectedDuration, 3> table { {
        { "subdivision", TupletSelectedDuration::SUBDIVISION, "subdivision" },
        { "tuplet-length", TupletSelectedDuration::TUPLET_LENGTH, "tuplet length" },
        { "default", TupletSelectedDuration::DEFAULT, "default (from configuration)" },
    } };
};

struct SelectionTargetContract {
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
    static constexpr EnumTable<PlayMode, 3> table { {
        { "none", PlayMode::NoPlay, "no play" },
        { "note", PlayMode::PlayNote, "play note" },
        { "chord", PlayMode::PlayChord, "play chord" },
    } };
};

struct AccidentalTypeContract {
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
    static constexpr EnumTable<BoxType, 5> table { {
        { "vertical", BoxType::Vertical, "vertical" },
        { "horizontal", BoxType::Horizontal, "horizontal" },
        { "measure", BoxType::Measure, "measure" },
        { "text", BoxType::Text, "text" },
        { "fret", BoxType::Fret, "fret" },
    } };
};

struct AddBoxesTargetContract {
    static constexpr EnumTable<AddBoxesTarget, 4> table { {
        { "after-selection", AddBoxesTarget::AfterSelection, "after selection" },
        { "before-selection", AddBoxesTarget::BeforeSelection, "before selection" },
        { "at-start-of-score", AddBoxesTarget::AtStartOfScore, "at start of score" },
        { "at-end-of-score", AddBoxesTarget::AtEndOfScore, "at end of score" },
    } };
};

struct GraceNoteTypeContract {
    static constexpr EnumTable<GraceNoteType, 9> table { {
        { "normal", GraceNoteType::NORMAL, "normal" },
        { "acciaccatura", GraceNoteType::ACCIACCATURA, "acciaccatura" },
        { "appoggiatura", GraceNoteType::APPOGGIATURA, "appoggiatura" },
        { "grace4", GraceNoteType::GRACE4, "grace4" },
        { "grace16", GraceNoteType::GRACE16, "grace16" },
        { "grace32", GraceNoteType::GRACE32, "grace32" },
        { "grace8-after", GraceNoteType::GRACE8_AFTER, "grace8 after" },
        { "grace16-after", GraceNoteType::GRACE16_AFTER, "grace16 after" },
        { "grace32-after", GraceNoteType::GRACE32_AFTER, "grace32 after" },
    } };
};

struct TextStyleTypeContract {
    static constexpr EnumTable<TextStyleType, 18> table { {
        { "title", TextStyleType::TITLE, "title" },
        { "subtitle", TextStyleType::SUBTITLE, "subtitle" },
        { "composer", TextStyleType::COMPOSER, "composer" },
        { "lyricist", TextStyleType::LYRICIST, "lyricist" },
        { "instrument-excerpt", TextStyleType::INSTRUMENT_EXCERPT, "instrument excerpt" },
        { "frame", TextStyleType::FRAME, "frame" },
        { "system", TextStyleType::SYSTEM, "system" },
        { "staff", TextStyleType::STAFF, "staff" },
        { "expression", TextStyleType::EXPRESSION, "expression" },
        { "rehearsalmark", TextStyleType::REHEARSAL_MARK, "rehearsalmark" },
        { "instrument-change", TextStyleType::INSTRUMENT_CHANGE, "instrument change" },
        { "fingering", TextStyleType::FINGERING, "fingering" },
        { "sticking", TextStyleType::STICKING, "sticking" },
        { "chord", TextStyleType::HARMONY_A, "chord" },
        { "roman-numeral", TextStyleType::HARMONY_ROMAN, "roman numeral" },
        { "nashville-number", TextStyleType::HARMONY_NASHVILLE, "nashville number" },
        { "lyrics", TextStyleType::LYRICS_ODD, "lyrics" },
        { "tempo", TextStyleType::TEMPO, "tempo" },
    } };
};

struct BeamModeContract {
    static constexpr EnumTable<BeamMode, 6> table { {
        { "auto", BeamMode::AUTO, "auto" },
        { "none", BeamMode::NONE, "none" },
        { "begin", BeamMode::BEGIN, "begin" },
        { "begin16", BeamMode::BEGIN16, "begin16" },
        { "begin32", BeamMode::BEGIN32, "begin32" },
        { "mid", BeamMode::MID, "mid" }
    } };
};

struct OrnamentTypeContract {
    static constexpr EnumTable<OrnamentType, 14> table { { { "turn", OrnamentType::TURN, "turn" },
        { "turn-inverted", OrnamentType::TURN_INVERTED, "turn inverted" },
        { "turn-slash", OrnamentType::TURN_SLASH, "turn slash" },
        { "turn-up", OrnamentType::TURN_UP, "turn up" },
        { "turn-inverted-up", OrnamentType::TURN_INVERTED_UP, "turn inverted up" },
        { "trill", OrnamentType::TRILL, "trill" },
        { "short-trill", OrnamentType::SHORT_TRILL, "short trill" },
        { "mordent", OrnamentType::MORDENT, "mordent" },
        { "haydn", OrnamentType::HAYDN, "haydn" },
        { "tremblement", OrnamentType::TREMBLEMENT, "tremblement" },
        { "prall-mordent", OrnamentType::PRALL_MORDENT, "prall mordent" },
        { "shake", OrnamentType::SHAKE, "shake" },
        { "shake-muffat", OrnamentType::SHAKE_MUFFAT, "shake muffat" },
        { "tremblement-couperin", OrnamentType::TREMBLEMENT_COUPERIN, "tremblement couperin" },
    } };
};

struct ArticulationTypeContract {
    static constexpr EnumTable<ArticulationType, 4> table { {
        { "marcato", ArticulationType::MARCATO, "marcato" },
        { "sforzato", ArticulationType::SFORZATO, "accent" },
        { "tenuto", ArticulationType::TENUTO, "tenuto" },
        { "staccato", ArticulationType::STACCATO, "staccato" },
    } };
};

struct OttavaTypeContract {
    static constexpr EnumTable<OttavaType, 6> table { {
        { "8va", OttavaType::OTTAVA_8VA, "8va alta" },
        { "8vb", OttavaType::OTTAVA_8VB, "8va bassa" },
        { "15ma", OttavaType::OTTAVA_15MA, "15ma alta" },
        { "15mb", OttavaType::OTTAVA_15MB, "15ma bassa" },
        { "22ma", OttavaType::OTTAVA_22MA, "22ma alta" },
        { "22mb", OttavaType::OTTAVA_22MB, "22ma bassa" },
    } };
};

struct HairpinTypeContract {
    static constexpr EnumTable<HairpinType, 4> table { {
        { "crescendo", HairpinType::CRESC_HAIRPIN, "crescendo hairpin" },
        { "diminuendo", HairpinType::DIM_HAIRPIN, "diminuendo hairpin" },
        { "crescendo-line", HairpinType::CRESC_LINE, "crescendo line" },
        { "diminuendo-line", HairpinType::DIM_LINE, "diminuendo line" },
    } };
};

struct TransposeDirectionContract {
    static constexpr EnumTable<TransposeDirection, 2> table { {
        { "up", TransposeDirection::UP, "transpose up" },
        { "down", TransposeDirection::DOWN, "transpose down" },
    } };
};

struct ScoreConfigTypeContract {
    static constexpr EnumTable<ScoreConfigType, 6> table { {
        { "invisible", ScoreConfigType::ShowInvisibleElements, "invisible elements" },
        { "unprintable", ScoreConfigType::ShowUnprintableElements, "formatting" },
        { "frames", ScoreConfigType::ShowFrames, "frames" },
        { "pageborders", ScoreConfigType::ShowPageMargins, "page margins" },
        { "soundflags", ScoreConfigType::ShowSoundFlags, "sound flags" },
        { "irregular", ScoreConfigType::MarkIrregularMeasures, "irregular measures" },
    } };
};

struct PastingTypeContract {
    static constexpr EnumTable<PastingType, 4> table { {
        { "default", PastingType::Default, "default" },
        { "half", PastingType::Half, "half duration" },
        { "double", PastingType::Double, "double duration" },
        { "special", PastingType::Special, "special" },
    } };
};

struct FontStyleContract {
    static constexpr EnumTable<FontStyle, 4> table { {
        { "bold", FontStyle::Bold, "bold" },
        { "italic", FontStyle::Italic, "italic" },
        { "underline", FontStyle::Underline, "underline" },
        { "strike", FontStyle::Strike, "strike" },
    } };
};

struct TextMoveDirectionContract {
    static constexpr EnumTable<TextMoveDirection, 2> table { {
        { "next", TextMoveDirection::Next, "next" },
        { "prev", TextMoveDirection::Prev, "prev" },
    } };
};

struct TextNavigationModeContract {
    static constexpr EnumTable<TextNavigationMode, 3> table { {
        { "element", TextNavigationMode::Element, "element" },
        { "word", TextNavigationMode::Word, "word" },
        { "beat", TextNavigationMode::Beat, "beat" },
    } };
};

struct LyricsNavigateActionContract {
    static constexpr EnumTable<LyricsNavigateAction, 3> table { {
        { "next-verse", LyricsNavigateAction::NextVerse, "next lyric verse" },
        { "prev-verse", LyricsNavigateAction::PrevVerse, "previous lyric verse" },
        { "next-syllable", LyricsNavigateAction::NextSyllable, "next syllable" },
    } };
};

struct VerticalAlignmentContract {
    static constexpr EnumTable<VerticalAlignment, 2> table { {
        { "subscript", VerticalAlignment::AlignSubScript, "subscript" },
        { "superscript", VerticalAlignment::AlignSuperScript, "superscript" },
    } };
};

struct ClefTypeContract {
    static constexpr EnumTable<ClefType, 37> table { {
        { "G", ClefType::G, "treble clef" },
        { "G15mb", ClefType::G15_MB, "treble clef 15ma bassa" },
        { "G8vb", ClefType::G8_VB, "treble clef 8va bassa" },
        { "G8va", ClefType::G8_VA, "treble clef 8va alta" },
        { "G15ma", ClefType::G15_MA, "treble clef 15ma alta" },
        { "G8vbo", ClefType::G8_VB_O, "double treble clef 8va bassa on 2nd line" },
        { "G8vbp", ClefType::G8_VB_P, "treble clef optional 8va bassa" },
        { "G1", ClefType::G_1, "french violin clef" },
        { "C1", ClefType::C1, "soprano clef" },
        { "C2", ClefType::C2, "mezzo-soprano clef" },
        { "C3", ClefType::C3, "alto clef" },
        { "C4", ClefType::C4, "tenor clef" },
        { "C5", ClefType::C5, "baritone clef (C clef)" },
        { "C_19C", ClefType::C_19C, "C clef, H shape (19th century)" },
        { "C1_F18C", ClefType::C1_F18C, "soprano clef (French, 18th century)" },
        { "C3_F18C", ClefType::C3_F18C, "alto clef (French, 18th century)" },
        { "C4_F18C", ClefType::C4_F18C, "tenor clef (French, 18th century)" },
        { "C1_F20C", ClefType::C1_F20C, "soprano clef (French, 20th century)" },
        { "C3_F20C", ClefType::C3_F20C, "alto clef (French, 20th century)" },
        { "C4_F20C", ClefType::C4_F20C, "tenor clef (French, 20th century)" },
        { "F", ClefType::F, "bass clef" },
        { "F15mb", ClefType::F15_MB, "bass clef 15ma bassa" },
        { "F8vb", ClefType::F8_VB, "bass clef 8va bassa" },
        { "F8va", ClefType::F_8VA, "bass clef 8va alta" },
        { "F15ma", ClefType::F_15MA, "bass clef 15ma alta" },
        { "F3", ClefType::F_B, "baritone clef (F clef)" },
        { "F5", ClefType::F_C, "subbass clef" },
        { "F_F18C", ClefType::F_F18C, "F clef (French, 18th century)" },
        { "F_19C", ClefType::F_19C, "F clef (19th century)" },
        { "PERC", ClefType::PERC, "percussion" },
        { "PERC2", ClefType::PERC2, "percussion 2" },
        { "TAB", ClefType::TAB, "tablature" },
        { "TAB4", ClefType::TAB4, "tablature 4 lines" },
        { "TAB2", ClefType::TAB_SERIF, "tablature serif" },
        { "TAB4_SERIF", ClefType::TAB4_SERIF, "tablature serif 4 lines" },
        { "C4_8VB", ClefType::C4_8VB, "tenor clef 8va bassa" },
        { "G8vbc", ClefType::G8_VB_C, "G clef ottava bassa with C clef" },
    } };
};

struct SnapDirectionContract {
    static constexpr EnumTable<SnapDirection, 2> table { {
        { "prev", SnapDirection::Prev, "snap to the previous item" },
        { "next", SnapDirection::Next, "snap to the next item" },
    } };
};
}

namespace muse::rcommand {
DECLARE_ENUM_CONTRACT(mu::notation::NoteNameContract);
DECLARE_ENUM_CONTRACT(mu::notation::NoteInputMethodContract);
DECLARE_ENUM_CONTRACT(mu::notation::DurationTypeContract);
DECLARE_ENUM_CONTRACT(mu::notation::ChangeDurationModeContract);
DECLARE_ENUM_CONTRACT(mu::notation::NoteAddingModeContract);
DECLARE_ENUM_CONTRACT(mu::notation::TupletNumberTypeContract);
DECLARE_ENUM_CONTRACT(mu::notation::TupletBracketTypeContract);
DECLARE_ENUM_CONTRACT(mu::notation::TupletSelectedDurationContract);
DECLARE_ENUM_CONTRACT(mu::notation::SelectionTargetContract);
DECLARE_ENUM_CONTRACT(mu::notation::PlayModeContract);
DECLARE_ENUM_CONTRACT(mu::notation::AccidentalTypeContract);
DECLARE_ENUM_CONTRACT(mu::notation::BoxTypeContract);
DECLARE_ENUM_CONTRACT(mu::notation::AddBoxesTargetContract);
DECLARE_ENUM_CONTRACT(mu::notation::GraceNoteTypeContract);
DECLARE_ENUM_CONTRACT(mu::notation::TextStyleTypeContract);
DECLARE_ENUM_CONTRACT(mu::notation::BeamModeContract);
DECLARE_ENUM_CONTRACT(mu::notation::OrnamentTypeContract);
DECLARE_ENUM_CONTRACT(mu::notation::ArticulationTypeContract);
DECLARE_ENUM_CONTRACT(mu::notation::OttavaTypeContract);
DECLARE_ENUM_CONTRACT(mu::notation::HairpinTypeContract);
DECLARE_ENUM_CONTRACT(mu::notation::TransposeDirectionContract);
DECLARE_ENUM_CONTRACT(mu::notation::ScoreConfigTypeContract);
DECLARE_ENUM_CONTRACT(mu::notation::PastingTypeContract);
DECLARE_ENUM_CONTRACT(mu::notation::FontStyleContract);
DECLARE_ENUM_CONTRACT(mu::notation::VerticalAlignmentContract);
DECLARE_ENUM_CONTRACT(mu::notation::ClefTypeContract);
DECLARE_ENUM_CONTRACT(mu::notation::SnapDirectionContract);
DECLARE_ENUM_CONTRACT(mu::notation::TextMoveDirectionContract);
DECLARE_ENUM_CONTRACT(mu::notation::TextNavigationModeContract);
DECLARE_ENUM_CONTRACT(mu::notation::LyricsNavigateActionContract);
}
