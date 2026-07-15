/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <QPixmap>
#include <QDate>

#include "engraving/dom/articulation.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/durationtype.h"
#include "engraving/dom/engravingitem.h"
#include "engraving/dom/guitarbend.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/hook.h"
#include "engraving/dom/key.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/page.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/stem.h"
#include "engraving/dom/system.h"
#include "engraving/dom/timesig.h"
#include "engraving/types/types.h"

namespace mu::notation {
using Page = mu::engraving::Page;
using System = mu::engraving::System;
using EngravingItem = mu::engraving::EngravingItem;
using ElementType = mu::engraving::ElementType;
using Note = mu::engraving::Note;
using Measure = mu::engraving::Measure;
using DurationType = mu::engraving::DurationType;
using Duration = mu::engraving::TDuration;
using SelectType = mu::engraving::SelectType;
using SymbolId = mu::engraving::SymId;
using Key = mu::engraving::Key;
using KeyMode = mu::engraving::KeyMode;
using TimeSigType = mu::engraving::TimeSigType;
using TimeSignature = mu::engraving::TimeSig;
using NoteHead = mu::engraving::NoteHead;
using TransposeMode = mu::engraving::TransposeMode;
using TransposeDirection = mu::engraving::TransposeDirection;
using Fraction = mu::engraving::Fraction;
using Chord = mu::engraving::Chord;
using ChordRest = mu::engraving::ChordRest;
using Articulation = mu::engraving::Articulation;
using SlurSegment = mu::engraving::SlurSegment;
using Rest = mu::engraving::Rest;
using Stem = mu::engraving::Stem;
using Hook = mu::engraving::Hook;
using Fraction = mu::engraving::Fraction;
using AccidentalType = mu::engraving::AccidentalType;
using OttavaType = mu::engraving::OttavaType;
using HairpinType = mu::engraving::HairpinType;
using TextBase = mu::engraving::TextBase;
using GraceNoteType = mu::engraving::NoteType;
using BeamMode = mu::engraving::BeamMode;
using LayoutBreakType = mu::engraving::LayoutBreakType;
using Interval = mu::engraving::Interval;
using BracketType = mu::engraving::BracketType;
using Segment = mu::engraving::Segment;
using TextStyleType = mu::engraving::TextStyleType;
using PageList = std::vector<Page*>;
using voice_idx_t = mu::engraving::voice_idx_t;
using track_idx_t = mu::engraving::track_idx_t;
using staff_idx_t = mu::engraving::staff_idx_t;
using GuitarBendType = mu::engraving::GuitarBendType;
using Pid = mu::engraving::Pid;
using VoiceAssignment = mu::engraving::VoiceAssignment;

enum class DragMode : unsigned char
{
    BothXY = 0,
    OnlyX,
    OnlyY
};

enum class MoveDirection : unsigned char
{
    Undefined = 0,
    Left,
    Right,
    Up,
    Down
};

enum class MoveSelectionType : unsigned char
{
    Undefined = 0,
    EngravingItem,
    Chord,
    Measure,
    Track,
    Frame,
    System,
    String // TAB Staff
};

enum class ExpandSelectionMode : unsigned char
{
    BeginSystem,
    EndSystem,
    BeginScore,
    EndScore,
};

enum class AddRemoveSystemLockType : signed char
{
    AfterEachSystem = -1,
    None = 0,
    MeasuresInterval
};

enum class BoxType : unsigned char
{
    Unknown,
    Vertical,
    Horizontal,
    Measure,
    Text,
    Fret
};

enum class AddBoxesTarget : unsigned char {
    AfterSelection,
    BeforeSelection,
    AtStartOfScore,
    AtEndOfScore
};

enum class NoteName : unsigned char
{
    C = 0,
    D,
    E,
    F,
    G,
    A,
    B
};

inline NoteName str_conv(const std::string& name, NoteName def)
{
    if (name == "c") {
        return NoteName::C;
    } else if (name == "d") {
        return NoteName::D;
    } else if (name == "e") {
        return NoteName::E;
    } else if (name == "f") {
        return NoteName::F;
    } else if (name == "g") {
        return NoteName::G;
    } else if (name == "a") {
        return NoteName::A;
    } else if (name == "b") {
        return NoteName::B;
    }
    return def;
}

enum class PastingType : unsigned char {
    Default,
    Half,
    Double,
    Special
};

struct FilterElementsOptions
{
    ElementType elementType = ElementType::INVALID;
    int staffStart = -1;
    int staffEnd = -1;
    int voice = -1;
    const mu::engraving::System* system = nullptr;
    Fraction durationTicks{ -1, 1 };
    Fraction beat{ 0, 0 };
    const mu::engraving::Measure* measure = nullptr;

    bool bySubtype = false;
    int subtype = -1;

    bool isValid() const
    {
        return elementType != ElementType::INVALID;
    }

    virtual ~FilterElementsOptions() = default;
};

struct FilterNotesOptions : FilterElementsOptions
{
    int pitch = -1;
    int string = mu::engraving::INVALID_STRING_INDEX;
    int tpc = mu::engraving::Tpc::TPC_INVALID;
    mu::engraving::NoteHeadGroup notehead = mu::engraving::NoteHeadGroup::HEAD_INVALID;
    mu::engraving::TDuration durationType = mu::engraving::TDuration();
    mu::engraving::NoteType noteType = mu::engraving::NoteType::INVALID;
};

struct TransposeOptions
{
    TransposeMode mode = TransposeMode::UNKNOWN;
    TransposeDirection direction = TransposeDirection::UNKNOWN;
    Key key = Key::C;
    int interval = 0;
    bool needTransposeKeys = false;
    bool needTransposeChordNames = false;
    bool needTransposeDoubleSharpsFlats = false;
};

enum class ScoreConfigType : unsigned char
{
    ShowInvisibleElements,
    ShowUnprintableElements,
    ShowFrames,
    ShowPageMargins,
    ShowSoundFlags,
    MarkIrregularMeasures
};

struct ScoreConfig
{
    bool isShowInvisibleElements = false;
    bool isShowUnprintableElements = false;
    bool isShowFrames = false;
    bool isShowPageMargins = false;
    bool isShowSoundFlags = false;
    bool isMarkIrregularMeasures = false;

    bool operator==(const ScoreConfig& conf) const
    {
        bool equal = (isShowInvisibleElements == conf.isShowInvisibleElements);
        equal &= (isShowUnprintableElements == conf.isShowUnprintableElements);
        equal &= (isShowFrames == conf.isShowFrames);
        equal &= (isShowPageMargins == conf.isShowPageMargins);
        equal &= (isShowSoundFlags == conf.isShowSoundFlags);
        equal &= (isMarkIrregularMeasures == conf.isMarkIrregularMeasures);

        return equal;
    }
};

enum class BracketsType : unsigned char
{
    Brackets,
    Braces,
    Parentheses
};

static constexpr int MIN_NOTES_INTERVAL = -10;
static constexpr int MAX_NOTES_INTERVAL = 10;

static constexpr int MAX_FRET = 14;

constexpr bool isNotesIntervalValid(int interval)
{
    return interval >= MIN_NOTES_INTERVAL && interval <= MAX_NOTES_INTERVAL
           && interval != 0 && interval != -1;
}

constexpr bool isVoiceIndexValid(voice_idx_t voiceIndex)
{
    return voiceIndex < mu::engraving::VOICES;
}

inline bool isVerticalBoxTextStyle(TextStyleType type)
{
    static const std::set<TextStyleType> types {
        TextStyleType::TITLE,
        TextStyleType::SUBTITLE,
        TextStyleType::COMPOSER,
        TextStyleType::LYRICIST,
        TextStyleType::INSTRUMENT_EXCERPT,
    };

    return muse::contains(types, type);
}

static const mu::engraving::ElementTypeSet NOTE_REST_TYPES {
    mu::engraving::ElementType::NOTE,
    mu::engraving::ElementType::REST,
};
}
