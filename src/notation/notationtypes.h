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

#include "io/path.h"
#include "translation.h"
#include "id.h"
#include "midi/midievent.h"

#include "libmscore/element.h"
#include "libmscore/page.h"
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
using Page = Ms::Page;
using Element = Ms::Element;
using ElementType = Ms::ElementType;
using Note = Ms::Note;
using Measure = Ms::Measure;
using DurationType = Ms::TDuration::DurationType;
using Duration = Ms::TDuration;
using SelectType = Ms::SelectType;
using Pad = Ms::Pad;
using ViewMode = engraving::LayoutMode;
using PitchMode = Ms::UpDownMode;
using StyleId = Ms::Sid;
using SymbolId = Ms::SymId;
using Key = Ms::Key;
using KeyMode = Ms::KeyMode;
using TimeSigType = Ms::TimeSigType;
using TimeSignature = Ms::TimeSig;
using Part = Ms::Part;
using Staff = Ms::Staff;
using NoteHead = Ms::NoteHead;
using SharpFlat = Ms::PreferSharpFlat;
using TransposeMode = Ms::TransposeMode;
using TransposeDirection = Ms::TransposeDirection;
using Fraction = Ms::Fraction;
using ElementPattern = Ms::ElementPattern;
using SelectionFilterType = Ms::SelectionFilterType;
using Chord = Ms::Chord;
using ChordRest = Ms::ChordRest;
using Harmony = Ms::Harmony;
using RealisedHarmony = Ms::RealizedHarmony;
using Articulation = Ms::Articulation;
using SlurSegment = Ms::SlurSegment;
using Rest = Ms::Rest;
using Stem = Ms::Stem;
using Hook = Ms::Hook;
using Fraction = Ms::Fraction;
using NoteInputMethod = Ms::NoteEntryMethod;
using AccidentalType = Ms::AccidentalType;
using OttavaType = Ms::OttavaType;
using HairpinType = Ms::HairpinType;
using TextType = Ms::Tid;
using TextBase = Ms::TextBase;
using TupletNumberType = Ms::TupletNumberType;
using TupletBracketType = Ms::TupletBracketType;
using GraceNoteType = Ms::NoteType;
using BeamMode = Ms::Beam::Mode;
using LayoutBreakType = Ms::LayoutBreak::Type;
using Interval = Ms::Interval;
using Drumset = Ms::Drumset;
using StringData = Ms::StringData;
using Clef = Ms::Clef;
using ClefType = Ms::ClefType;
using ClefTypeList = Ms::ClefTypeList;
using BracketType = Ms::BracketType;
using StaffGroup = Ms::StaffGroup;
using StaffType = Ms::StaffTypes;
using StaffTypePreset = Ms::StaffType;
using StaffName = Ms::StaffName;
using StaffNameList = Ms::StaffNameList;
using MidiArticulation = Ms::MidiArticulation;
using Trait = Ms::Trait;
using TraitType = Ms::TraitType;
using InstrumentChannel = Ms::Channel;
using Instrument = Ms::Instrument;
using InstrumentTemplate = Ms::InstrumentTemplate;
using InstrumentTrait = Ms::Trait;
using ScoreOrder = Ms::ScoreOrder;
using ScoreOrderGroup = Ms::ScoreGroup;
using InstrumentOverwrite = Ms::InstrumentOverwrite;
using InstrumentGenre = Ms::InstrumentGenre;
using InstrumentGroup = Ms::InstrumentGroup;
using MidiArticulation = Ms::MidiArticulation;
using InstrumentChannelList = QList<InstrumentChannel>;
using PageList = std::vector<const Page*>;
using StaffList = QList<const Staff*>;
using PartList = QList<const Part*>;
using InstrumentList = QList<Instrument>;
using InstrumentTemplateList = QList<const InstrumentTemplate*>;
using InstrumentGenreList = QList<const InstrumentGenre*>;
using ScoreOrderList = QList<const ScoreOrder*>;
using InstrumentGroupList = QList<const InstrumentGroup*>;
using MidiArticulationList = QList<MidiArticulation>;

static const QString COMMON_GENRE_ID("common");

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
    Element,
    Chord,
    Measure,
    Track
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

enum class ResettableValueType
{
    Stretch,
    BeamMode,
    ShapesAndPosition,
    TextStyleOverriders
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
    int currentVoiceIndex = 0;
    int currentTrack = 0;
    const Drumset* drumset = nullptr;
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
    Fraction tick = Ms::Fraction(0, 1);
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

struct InstrumentsMeta
{
    InstrumentTemplateList instrumentTemplates;
    InstrumentGroupList groups;
    InstrumentGenreList genres;
    MidiArticulationList articulations;
    ScoreOrderList scoreOrders;

    void clear()
    {
        instrumentTemplates.clear();
        groups.clear();
        genres.clear();
        articulations.clear();
        scoreOrders.clear();
    }
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
    const Ms::System* system = nullptr;
    Fraction durationTicks;

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
    int string = Ms::INVALID_STRING_INDEX;
    int tpc = Ms::Tpc::TPC_INVALID;
    NoteHead::Group notehead = NoteHead::Group::HEAD_INVALID;
    Ms::TDuration durationType = Ms::TDuration();
    Ms::NoteType noteType = Ms::NoteType::INVALID;
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
    int linesCount = 0;
    double lineDistance = 0.0;
    mu::draw::Color linesColor;
    bool visibleLines = false;
    qreal userDistance = 0.0;
    double scale = 0.0;
    bool isSmall = false;
    bool cutaway = false;
    bool showIfEmpty = false;
    bool showClef = false;
    bool showTimeSignature = false;
    bool showKeySignature = false;
    bool showBarlines = false;
    bool showStemless = false;
    bool showLedgerLinesPitched = false;
    bool hideSystemBarline = false;
    bool mergeMatchingRests = false;
    Staff::HideMode hideMode = Staff::HideMode::AUTO;
    NoteHead::Scheme noteheadScheme = NoteHead::Scheme::HEAD_AUTO;
    ClefTypeList clefTypeList;
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

    QRect loopInRect = {};
    QRect loopOutRect = {};

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
        equals &= loopInRect == boundaries.loopInRect;
        equals &= loopOutRect == boundaries.loopOutRect;
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

inline QString staffTypeToString(StaffType type)
{
    const Ms::StaffType* preset = Ms::StaffType::preset(type);
    return preset ? preset->name() : QString();
}

inline QList<StaffType> allStaffTypes()
{
    QList<StaffType> result;

    for (const Ms::StaffType& preset: Ms::StaffType::presets()) {
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

inline ScoreOrder makeCustomOrder()
{
    ScoreOrder order;
    order.id = "custom";
    order.name = qtrc("OrderXML", "Custom");

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

constexpr bool isVoiceIndexValid(int voiceIndex)
{
    return 0 <= voiceIndex && voiceIndex < Ms::VOICES;
}

constexpr bool isFretIndexValid(int fretIndex)
{
    return 0 <= fretIndex && fretIndex < MAX_FRET;
}
}

Q_DECLARE_METATYPE(mu::notation::InstrumentTemplate)
Q_DECLARE_METATYPE(mu::notation::ScoreOrder)

#endif // MU_NOTATION_NOTATIONTYPES_H
