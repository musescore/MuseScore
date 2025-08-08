/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include <unordered_set>

#include "translation.h"

#include "types/id.h"
#include "types/translatablestring.h"

#include "engraving/dom/articulation.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/durationtype.h"
#include "engraving/dom/engravingitem.h"
#include "engraving/dom/guitarbend.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/hook.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/key.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/page.h"
#include "engraving/dom/part.h"
#include "engraving/dom/realizedharmony.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/score.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/stem.h"
#include "engraving/dom/system.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tuplet.h"

#include "engraving/rendering/layoutoptions.h"

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
using StyleIdSet = mu::engraving::StyleIdSet;
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
using SelectionFilterTypesVariant = mu::engraving::SelectionFilterTypesVariant;
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
using NoteInputParams = mu::engraving::NoteInputParams;
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
using Segment = mu::engraving::Segment;
using TextStyleType = mu::engraving::TextStyleType;
using TraitType = mu::engraving::TraitType;
using HarmonyDurationType = mu::engraving::HDuration;
using Voicing = mu::engraving::Voicing;
using Instrument = mu::engraving::Instrument;
using InstrumentTemplate = mu::engraving::InstrumentTemplate;
using InstrumentTrait = mu::engraving::Trait;
using ScoreOrder = mu::engraving::ScoreOrder;
using InstrumentGenre = mu::engraving::InstrumentGenre;
using InstrumentGroup = mu::engraving::InstrumentGroup;
using PageList = std::vector<const Page*>;
using PartList = std::vector<const Part*>;
using InstrumentTemplateList = std::vector<const InstrumentTemplate*>;
using InstrumentGenreList = std::vector<const InstrumentGenre*>;
using ScoreOrderList = std::vector<mu::engraving::ScoreOrder>;
using InstrumentGroupList = std::vector<const InstrumentGroup*>;
using InstrumentTrackId = mu::engraving::InstrumentTrackId;
using InstrumentTrackIdSet = mu::engraving::InstrumentTrackIdSet;
using voice_idx_t = mu::engraving::voice_idx_t;
using track_idx_t = mu::engraving::track_idx_t;
using staff_idx_t = mu::engraving::staff_idx_t;
using ScoreChanges = mu::engraving::ScoreChanges;
using GuitarBendType = mu::engraving::GuitarBendType;
using engraving::LoopBoundaryType;
using Pid = mu::engraving::Pid;
using VoiceAssignment = mu::engraving::VoiceAssignment;

static const muse::String COMMON_GENRE_ID("common");

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

using NoteVal = mu::engraving::NoteVal;
using NoteValList = mu::engraving::NoteValList;

enum class NoteAddingMode : unsigned char
{
    CurrentChord,
    NextChord,
    InsertChord
};

enum class PastingType : unsigned char {
    Default,
    Half,
    Double,
    Special
};

using NoteInputState = mu::engraving::InputState;

enum class NoteFilter : unsigned char
{
    All,
    WithTie,
    WithSlur
};

enum class ZoomType : unsigned char {
    Percentage,
    PageWidth,
    WholePage,
    TwoPages
};

inline muse::TranslatableString zoomTypeTitle(ZoomType type)
{
    switch (type) {
    case ZoomType::Percentage: return muse::TranslatableString("notation", "Percentage");
    case ZoomType::PageWidth: return muse::TranslatableString("notation", "Page width");
    case ZoomType::WholePage: return muse::TranslatableString("notation", "Whole page");
    case ZoomType::TwoPages: return muse::TranslatableString("notation", "Two pages");
    }

    return {};
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

struct InstrumentKey
{
    muse::String instrumentId;
    muse::ID partId;
    Fraction tick = mu::engraving::Fraction(0, 1);
};

inline bool isMainInstrumentForPart(const InstrumentKey& instrumentKey, const Part* part)
{
    return instrumentKey.instrumentId == part->instrumentId() && instrumentKey.tick == Part::MAIN_INSTRUMENT_TICK;
}

inline QString formatInstrumentTitle(const QString& instrumentName, const InstrumentTrait& trait)
{
    // Comments for translators start with //:
    switch (trait.type) {
    case TraitType::Tuning:
        //: %1=tuning ("D"), %2=name ("Tin Whistle"). Example: "D Tin Whistle"
        return muse::qtrc("notation", "%1 %2", "Tuned instrument displayed in the UI")
               .arg(trait.name, instrumentName);
    case TraitType::Transposition:
        //: %1=name ("Horn"), %2=transposition ("C alto"). Example: "Horn in C alto"
        return muse::qtrc("notation", "%1 in %2", "Transposing instrument displayed in the UI")
               .arg(instrumentName, trait.name);
    case TraitType::Course:
        //: %1=name ("Tenor Lute"), %2=course/strings ("7-course"). Example: "Tenor Lute (7-course)"
        return muse::qtrc("notation", "%1 (%2)", "String instrument displayed in the UI")
               .arg(instrumentName, trait.name);
    case TraitType::Unknown:
        return instrumentName; // Example: "Flute"
    }
    Q_UNREACHABLE();
}

inline QString formatInstrumentTitle(const QString& instrumentName, const InstrumentTrait& trait, int instrumentNumber)
{
    if (instrumentNumber == 0) {
        // Only one instance of this instrument in the score
        return formatInstrumentTitle(instrumentName, trait);
    }

    QString number = QString::number(instrumentNumber);

    // Comments for translators start with //:
    switch (trait.type) {
    case TraitType::Tuning:
        //: %1=tuning ("D"), %2=name ("Tin Whistle"), %3=number ("2"). Example: "D Tin Whistle 2"
        return muse::qtrc("notation", "%1 %2 %3", "One of several tuned instruments displayed in the UI")
               .arg(trait.name, instrumentName, number);
    case TraitType::Transposition:
        //: %1=name ("Horn"), %2=transposition ("C alto"), %3=number ("2"). Example: "Horn in C alto 2"
        return muse::qtrc("notation", "%1 in %2 %3", "One of several transposing instruments displayed in the UI")
               .arg(instrumentName, trait.name, number);
    case TraitType::Course:
        //: %1=name ("Tenor Lute"), %2=course/strings ("7-course"), %3=number ("2"). Example: "Tenor Lute (7-course) 2"
        return muse::qtrc("notation", "%1 (%2) %3", "One of several string instruments displayed in the UI")
               .arg(instrumentName, trait.name, number);
    case TraitType::Unknown:
        //: %1=name ("Flute"), %2=number ("2"). Example: "Flute 2"
        return muse::qtrc("notation", "%1 %2", "One of several instruments displayed in the UI")
               .arg(instrumentName, number);
    }
    Q_UNREACHABLE();
}

struct PartInstrument
{
    muse::ID partId;
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

struct StaffConfig
{
    bool visible = false;
    engraving::Spatium userDistance = engraving::Spatium(0.0);
    bool cutaway = false;
    bool showIfEmpty = false;
    bool hideSystemBarline = false;
    engraving::AutoOnOff mergeMatchingRests = engraving::AutoOnOff::AUTO;
    bool reflectTranspositionInLinkedTab = false;
    ClefTypeList clefTypeList;
    engraving::StaffType staffType;

    bool operator==(const StaffConfig& conf) const
    {
        bool equal = visible == conf.visible;
        equal &= userDistance == conf.userDistance;
        equal &= cutaway == conf.cutaway;
        equal &= showIfEmpty == conf.showIfEmpty;
        equal &= hideSystemBarline == conf.hideSystemBarline;
        equal &= mergeMatchingRests == conf.mergeMatchingRests;
        equal &= clefTypeList == conf.clefTypeList;
        equal &= staffType == conf.staffType;
        equal &= reflectTranspositionInLinkedTab == conf.reflectTranspositionInLinkedTab;

        return equal;
    }

    bool operator!=(const StaffConfig& conf) const
    {
        return !(*this == conf);
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
    bool autoBaseLen = false;
};

struct LoopBoundaries
{
    int loopInTick = 0;
    int loopOutTick = 0;
    bool enabled = false;

    bool isNull() const
    {
        return loopInTick == 0 && loopOutTick == 0;
    }

    bool operator==(const LoopBoundaries& boundaries) const
    {
        bool equals = true;

        equals &= loopInTick == boundaries.loopInTick;
        equals &= loopOutTick == boundaries.loopOutTick;
        equals &= enabled == boundaries.enabled;

        return equals;
    }

    bool operator!=(const LoopBoundaries& boundaries) const
    {
        return !(*this == boundaries);
    }
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

inline QString staffTypeToString(StaffTypeId type)
{
    const StaffType* preset = StaffType::preset(type);
    return preset ? preset->name().toQString() : QString();
}

struct MeasureBeat
{
    int measureIndex = 0;
    int maxMeasureIndex = 0;
    float beat = 0.f;
    int maxBeatIndex = 0;
};

enum class BracketsType : unsigned char
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
    order.name = muse::TranslatableString("engraving/scoreorder", "Custom");

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

struct StringTuningPreset
{
    std::string name;
    std::vector<int> value;
    bool useFlats = false;
};

struct StringTuningsInfo
{
    size_t number = 0;
    std::vector<StringTuningPreset> presets;
};

using InstrumentStringTuningsMap = std::map<std::string, std::vector<StringTuningsInfo> >;

enum class PercussionPanelAutoShowMode {
    UNPITCHED_STAFF,
    UNPITCHED_STAFF_NOTE_INPUT,
    NEVER,
};

static const mu::engraving::ElementTypeSet NOTE_REST_TYPES {
    mu::engraving::ElementType::NOTE,
    mu::engraving::ElementType::REST,
};
}
