/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_NOTATION_NOTATIONTYPES_H
#define MU_NOTATION_NOTATIONTYPES_H

#include <QPixmap>
#include <QDate>
#include <unordered_set>

#include "io/path.h"
#include "translation.h"
#include "id.h"
#include "midi/midievent.h"

#include "libmscore/engravingitem.h"
#include "libmscore/page.h"
#include "libmscore/system.h"
#include "libmscore/durationtype.h"
#include "libmscore/mscore.h"
#include "libmscore/masterscore.h"
#include "libmscore/timesig.h"
#include "libmscore/key.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/stafftype.h"
#include "libmscore/chord.h"
#include "libmscore/articulation.h"
#include "libmscore/slur.h"
#include "libmscore/rest.h"
#include "libmscore/stem.h"
#include "libmscore/hook.h"
#include "libmscore/measure.h"
#include "libmscore/ottava.h"
#include "libmscore/hairpin.h"
#include "libmscore/harmony.h"
#include "libmscore/realizedharmony.h"
#include "libmscore/instrument.h"

#include "engraving/layout/layoutoptions.h"

namespace mu::notation {
using Page = mu::engraving::Page;
using System = mu::engraving::System;
using EngravingItem = mu::engraving::EngravingItem;
using ElementType = mu::engraving::ElementType;
using PropertyValue = engraving::PropertyValue;
using Note = mu::engraving::Note;
using Measure = mu::engraving::Measure;
using DurationType = mu::engraving::DurationType;
using Duration = mu::engraving::TDuration;
using SelectType = mu::engraving::SelectType;
using SelectionState = mu::engraving::SelState;
using Pad = mu::engraving::Pad;
using ViewMode = engraving::LayoutMode;
using PitchMode = mu::engraving::UpDownMode;
using StyleId = mu::engraving::Sid;
using SymbolId = mu::engraving::SymId;
using Key = mu::engraving::Key;
using KeyMode = mu::engraving::KeyMode;
using TimeSigType = mu::engraving::TimeSigType;
using TimeSignature = mu::engraving::TimeSig;
using Part = mu::engraving::Part;
using Staff = mu::engraving::Staff;
using NoteHead = mu::engraving::NoteHead;
using SharpFlat = mu::engraving::PreferSharpFlat;
using TransposeMode = mu::engraving::TransposeMode;
using TransposeDirection = mu::engraving::TransposeDirection;
using Fraction = mu::engraving::Fraction;
using ElementPattern = mu::engraving::ElementPattern;
using SelectionFilterType = mu::engraving::SelectionFilterType;
using Chord = mu::engraving::Chord;
using ChordRest = mu::engraving::ChordRest;
using Harmony = mu::engraving::Harmony;
using RealisedHarmony = mu::engraving::RealizedHarmony;
using Articulation = mu::engraving::Articulation;
using SlurSegment = mu::engraving::SlurSegment;
using Rest = mu::engraving::Rest;
using Stem = mu::engraving::Stem;
using Hook = mu::engraving::Hook;
using Fraction = mu::engraving::Fraction;
using NoteInputMethod = mu::engraving::NoteEntryMethod;
using AccidentalType = mu::engraving::AccidentalType;
using OttavaType = mu::engraving::OttavaType;
using HairpinType = mu::engraving::HairpinType;
using TextBase = mu::engraving::TextBase;
using TupletNumberType = mu::engraving::TupletNumberType;
using TupletBracketType = mu::engraving::TupletBracketType;
using GraceNoteType = mu::engraving::NoteType;
using BeamMode = mu::engraving::BeamMode;
using LayoutBreakType = mu::engraving::LayoutBreakType;
using Interval = mu::engraving::Interval;
using Drumset = mu::engraving::Drumset;
using StringData = mu::engraving::StringData;
using Clef = mu::engraving::Clef;
using ClefType = mu::engraving::ClefType;
using ClefTypeList = mu::engraving::ClefTypeList;
using BracketType = mu::engraving::BracketType;
using StaffGroup = mu::engraving::StaffGroup;
using StaffType = mu::engraving::StaffType;
using StaffTypeId = mu::engraving::StaffTypes;
using StaffName = mu::engraving::StaffName;
using StaffNameList = mu::engraving::StaffNameList;
using MidiArticulation = mu::engraving::MidiArticulation;
using TextStyleType = mu::engraving::TextStyleType;
using Trait = mu::engraving::Trait;
using TraitType = mu::engraving::TraitType;
using HarmonyDurationType = mu::engraving::HDuration;
using Voicing = mu::engraving::Voicing;
using InstrumentChannel = mu::engraving::InstrChannel;
using Instrument = mu::engraving::Instrument;
using InstrumentTemplate = mu::engraving::InstrumentTemplate;
using InstrumentTrait = mu::engraving::Trait;
using ScoreOrder = mu::engraving::ScoreOrder;
using ScoreOrderGroup = mu::engraving::ScoreGroup;
using InstrumentOverwrite = mu::engraving::InstrumentOverwrite;
using InstrumentGenre = mu::engraving::InstrumentGenre;
using InstrumentGroup = mu::engraving::InstrumentGroup;
using MidiArticulation = mu::engraving::MidiArticulation;
using PageList = std::vector<const Page*>;
using PartList = std::vector<const Part*>;
using InstrumentList = QList<Instrument>;
using InstrumentTemplateList = QList<const InstrumentTemplate*>;
using InstrumentGenreList = QList<const InstrumentGenre*>;
using ScoreOrderList = std::vector<mu::engraving::ScoreOrder>;
using InstrumentGroupList = QList<const InstrumentGroup*>;
using MidiArticulationList = QList<MidiArticulation>;
using InstrumentTrackId = mu::engraving::InstrumentTrackId;
using InstrumentTrackIdSet = mu::engraving::InstrumentTrackIdSet;

static const String COMMON_GENRE_ID("common");

enum class DragMode
{
    BothXY = 0,
    OnlyX,
    OnlyY,
    LassoList
};

enum class MoveDirection
{
    Undefined = 0,
    Left,
    Right,
    Up,
    Down
};

enum class MoveSelectionType
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

enum class ExpandSelectionMode
{
    BeginSystem,
    EndSystem,
    BeginScore,
    EndScore,
};

enum class BreaksSpawnIntervalType
{
    AfterEachSystem = -1,
    None = 0,
    MeasuresInterval
};

enum class BoxType
{
    Unknown,
    Vertical,
    Horizontal,
    Measure,
    Text
};

enum class AddBoxesTarget {
    AfterSelection,
    BeforeSelection,
    AtStartOfScore,
    AtEndOfScore
};

enum class NoteName
{
    C = 0,
    D,
    E,
    F,
    G,
    A,
    B
};

enum class NoteAddingMode
{
    CurrentChord,
    NextChord,
    InsertChord
};

enum class IntervalType
{
    Above,
    Below
};

enum class TupletType
{
    Duplet,
    Triplet,
    Quadruplet,
    Quintuplet,
    Sextuplet,
    Septuplet,
    Octuplet,
    Nonuplet
};

enum class PastingType {
    Default,
    Half,
    Double,
    Special
};

struct NoteInputState
{
    NoteInputMethod method = NoteInputMethod::UNKNOWN;
    Duration duration;
    AccidentalType accidentalType = AccidentalType::NONE;
    std::set<SymbolId> articulationIds;
    bool isRest = false;
    bool withSlur = false;
    engraving::voice_idx_t currentVoiceIndex = 0;
    engraving::track_idx_t currentTrack = 0;
    const Drumset* drumset = nullptr;
    StaffGroup staffGroup = StaffGroup::STANDARD;
};

enum class NoteFilter
{
    All,
    WithTie,
    WithSlur
};

enum class ZoomType {
    Percentage,
    PageWidth,
    WholePage,
    TwoPages
};

inline QString zoomTypeTitle(ZoomType type)
{
    switch (type) {
    case ZoomType::Percentage: return qtrc("notation", "Percentage");
    case ZoomType::PageWidth: return qtrc("notation", "Page width");
    case ZoomType::WholePage: return qtrc("notation", "Whole page");
    case ZoomType::TwoPages: return qtrc("notation", "Two pages");
    }

    return QString();
}

struct Tempo
{
    int valueBpm = 0;
    DurationType duration = DurationType::V_QUARTER;
    bool withDot = false;

    bool operator==(const Tempo& other) const
    {
        return valueBpm == other.valueBpm && duration == other.duration && withDot == other.withDot;
    }
};

static constexpr int MAX_STAVES  = 4;

struct ClefPair
{
    ClefType concertClef = ClefType::G;
    ClefType transposingClef = ClefType::G;
};

struct PitchRange
{
    int min = 0;
    int max = 0;

    PitchRange() = default;
    PitchRange(int min, int max)
        : min(min), max(max) {}

    bool operator ==(const PitchRange& other) const
    {
        return min == other.min && max == other.max;
    }

    bool operator !=(const PitchRange& other) const
    {
        return !operator ==(other);
    }
};

struct InstrumentKey
{
    QString instrumentId;
    ID partId;
    Fraction tick = mu::engraving::Fraction(0, 1);
};

inline QString formatInstrumentTitle(const QString& instrumentName, const InstrumentTrait& trait, int instrumentNumber = 0)
{
    QString result = instrumentName;

    switch (trait.type) {
    case TraitType::Tuning:
        result = mu::qtrc("notation", "%1 %2").arg(trait.name).arg(instrumentName);
        break;
    case TraitType::Course:
        result = mu::qtrc("notation", "%1 (%2)").arg(instrumentName).arg(trait.name);
        break;
    case TraitType::Transposition:
        result = mu::qtrc("notation", "%1 in %2").arg(instrumentName).arg(trait.name);
        break;
    case TraitType::Unknown:
        break;
    }

    if (!result.isEmpty() && instrumentNumber > 0) {
        result += " " + QString::number(instrumentNumber);
    }

    return result;
}

struct PartInstrument
{
    ID partId;
    InstrumentTemplate instrumentTemplate;

    bool isExistingPart = false;
    bool isSoloist = false;
};

using PartInstrumentList = QList<PartInstrument>;

struct PartInstrumentListScoreOrder
{
    PartInstrumentList instruments;
    ScoreOrder scoreOrder;
};

struct SearchCommand
{
    ElementType searchElementType = ElementType::INVALID;
    std::string code;
    std::string description;

    SearchCommand(const ElementType& searchElementType, const std::string& code, const std::string& description)
        : searchElementType(searchElementType), code(code), description(description) {}
};
using SearchCommands = QList<SearchCommand>;

struct FilterElementsOptions
{
    ElementType elementType = ElementType::INVALID;
    int staffStart = -1;
    int staffEnd = -1;
    int voice = -1;
    const mu::engraving::System* system = nullptr;
    Fraction durationTicks{ -1, 1 };

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
    engraving::NoteHeadGroup notehead = engraving::NoteHeadGroup::HEAD_INVALID;
    mu::engraving::TDuration durationType = mu::engraving::TDuration();
    mu::engraving::NoteType noteType = mu::engraving::NoteType::INVALID;
};

struct SelectionRange
{
    int startStaffIndex = 0;
    int endStaffIndex = 0;
    Fraction startTick;
    Fraction endTick;
};

struct StaffConfig
{
    bool visible = false;
    qreal userDistance = 0.0;
    bool cutaway = false;
    bool showIfEmpty = false;
    bool hideSystemBarline = false;
    bool mergeMatchingRests = false;
    Staff::HideMode hideMode = Staff::HideMode::AUTO;
    ClefTypeList clefTypeList;
    engraving::StaffType staffType;

    bool operator==(const StaffConfig& conf) const
    {
        bool equal = visible == conf.visible;
        equal &= RealIsEqual(userDistance, conf.userDistance);
        equal &= cutaway == conf.cutaway;
        equal &= showIfEmpty == conf.showIfEmpty;
        equal &= hideSystemBarline == conf.hideSystemBarline;
        equal &= mergeMatchingRests == conf.mergeMatchingRests;
        equal &= hideMode == conf.hideMode;
        equal &= clefTypeList == conf.clefTypeList;
        equal &= staffType == conf.staffType;

        return equal;
    }
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

struct TupletOptions
{
    Fraction ratio = { -1, -1 };
    TupletNumberType numberType = TupletNumberType::SHOW_NUMBER;
    TupletBracketType bracketType = TupletBracketType::AUTO_BRACKET;
};

enum class LoopBoundaryType
{
    Unknown,
    LoopIn,
    LoopOut
};

struct LoopBoundaries
{
    int loopInTick = 0;
    int loopOutTick = 0;
    bool visible = false;

    bool isNull() const
    {
        return loopInTick == 0 && loopOutTick == 0;
    }

    bool operator==(const LoopBoundaries& boundaries) const
    {
        bool equals = true;

        equals &= loopInTick == boundaries.loopInTick;
        equals &= loopOutTick == boundaries.loopOutTick;
        equals &= visible == boundaries.visible;

        return equals;
    }

    bool operator!=(const LoopBoundaries& boundaries) const
    {
        return !(*this == boundaries);
    }
};

enum class ScoreConfigType
{
    ShowInvisibleElements,
    ShowUnprintableElements,
    ShowFrames,
    ShowPageMargins,
    MarkIrregularMeasures
};

struct ScoreConfig
{
    bool isShowInvisibleElements = false;
    bool isShowUnprintableElements = false;
    bool isShowFrames = false;
    bool isShowPageMargins = false;
    bool isMarkIrregularMeasures = false;
};

inline QString staffTypeToString(StaffTypeId type)
{
    const StaffType* preset = StaffType::preset(type);
    return preset ? preset->name().toQString() : QString();
}

inline QList<StaffTypeId> allStaffTypes()
{
    QList<StaffTypeId> result;

    for (const StaffType& preset: StaffType::presets()) {
        result << preset.type();
    }

    return result;
}

struct MeasureBeat
{
    int measureIndex = 0;
    int maxMeasureIndex = 0;
    int beatIndex = 0;
    int maxBeatIndex = 0;
};

enum class BracketsType
{
    Brackets,
    Braces,
    Parentheses
};

struct ScoreCreateOptions
{
    bool withTempo = false;
    Tempo tempo;

    int timesigNumerator = 0;
    int timesigDenominator = 1;
    TimeSigType timesigType = TimeSigType::NORMAL;

    Key key = Key::C;
    KeyMode keyMode = KeyMode::UNKNOWN;

    bool withPickupMeasure = false;
    int measures = 0;
    int measureTimesigNumerator = 0;
    int measureTimesigDenominator = 0;

    PartInstrumentList parts;
    ScoreOrder order;
};

inline const ScoreOrder& customOrder()
{
    static ScoreOrder order;
    order.id = "custom";
    order.name = mtrc("OrderXML", "Custom");

    return order;
}

static constexpr int MIN_NOTES_INTERVAL = -9;
static constexpr int MAX_NOTES_INTERVAL = 9;

static constexpr int MAX_FRET = 14;

constexpr bool isNotesIntervalValid(int interval)
{
    return interval >= MIN_NOTES_INTERVAL && interval <= MAX_NOTES_INTERVAL
           && interval != 0 && interval != -1;
}

constexpr bool isVoiceIndexValid(size_t voiceIndex)
{
    return voiceIndex < mu::engraving::VOICES;
}

constexpr bool isFretIndexValid(int fretIndex)
{
    return 0 <= fretIndex && fretIndex < MAX_FRET;
}
}

#endif // MU_NOTATION_NOTATIONTYPES_H
