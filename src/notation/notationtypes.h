//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_NOTATION_NOTATIONTYPES_H
#define MU_NOTATION_NOTATIONTYPES_H

#include <QPixmap>
#include <QDate>

#include "io/path.h"

#include "libmscore/element.h"
#include "libmscore/page.h"
#include "libmscore/durationtype.h"
#include "libmscore/mscore.h"
#include "libmscore/score.h"
#include "libmscore/timesig.h"
#include "libmscore/key.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/stafftype.h"
#include "libmscore/score.h"
#include "libmscore/chord.h"
#include "libmscore/articulation.h"
#include "libmscore/slur.h"
#include "libmscore/rest.h"
#include "libmscore/stem.h"
#include "libmscore/hook.h"
#include "libmscore/measure.h"
#include "libmscore/ottava.h"
#include "libmscore/hairpin.h"

#include "instruments/instrumentstypes.h"

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
using ViewMode = Ms::LayoutMode;  // Accomodate inconsistent convention from v3
using PitchMode = Ms::UpDownMode;
using StyleId = Ms::Sid;
using SymbolId = Ms::SymId;
using Key = Ms::Key;
using KeyMode = Ms::KeyMode;
using TimeSigType = Ms::TimeSigType;
using Part = Ms::Part;
using Staff = Ms::Staff;
using StaffType = Ms::StaffTypes;
using NoteHead = Ms::NoteHead;
using SharpFlat = Ms::PreferSharpFlat;
using TransposeMode = Ms::TransposeMode;
using TransposeDirection = Ms::TransposeDirection;
using Fraction = Ms::Fraction;
using ElementPattern = Ms::ElementPattern;
using Chord = Ms::Chord;
using ChordRest = Ms::ChordRest;
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
using TupletNumberType = Ms::TupletNumberType;
using TupletBracketType = Ms::TupletBracketType;
using GraceNoteType = Ms::NoteType;
using BeamMode = Ms::Beam::Mode;

using PageList = std::vector<const Page*>;
using StaffList = QList<const Staff*>;
using PartList = QList<const Part*>;

enum class DragMode
{
    BothXY = 0,
    OnlyX,
    OnlyY
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

enum class SaveMode
{
    Save,
    SaveAs,
    SaveCopy,
    SaveSelection,
    SaveOnline
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

struct Meta
{
    io::path fileName;
    io::path filePath;
    QString title;
    QString subtitle;
    QString composer;
    QString lyricist;
    QString copyright;
    QString translator;
    QString arranger;
    size_t partsCount = 0;
    QPixmap thumbnail;
    QDate creationDate;

    QString source;
    QString platform;
    QString musescoreVersion;
    int musescoreRevision = 0;
    int mscVersion = 0;

    QVariantMap additionalTags;
};

using MetaList = QList<Meta>;

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

struct ScoreCreateOptions
{
    QString title;
    QString subtitle;
    QString composer;
    QString lyricist;
    QString copyright;

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

    io::path templatePath;
    instruments::InstrumentList instruments;
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
    int string = Ms::STRING_NONE;
    int tpc = Ms::Tpc::TPC_INVALID;
    NoteHead::Group notehead = NoteHead::Group::HEAD_INVALID;
    Ms::TDuration durationType = Ms::TDuration();
    Ms::NoteType noteType = Ms::NoteType::INVALID;
};

struct SelectionRange {
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
    QColor linesColor;
    bool visibleLines = false;
    qreal userDistance = 0.0;
    double scale = 0.0;
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
    Ms::ClefTypeList clefType;
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

    QRect loopInRect;
    QRect loopOutRect;

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
    return Ms::StaffType::preset(type)->name();
}

inline QList<StaffType> allStaffTypes()
{
    QList<StaffType> result;

    for (const Ms::StaffType& staffType: Ms::StaffType::presets()) {
        result << staffType.type();
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

static constexpr int MIN_NOTES_INTERVAL = -9;
static constexpr int MAX_NOTES_INTERVAL = 9;

inline bool isNotesIntervalValid(int interval)
{
    return interval >= MIN_NOTES_INTERVAL && interval <= MAX_NOTES_INTERVAL
           && interval != 0 && interval != -1;
}

inline bool isVoiceIndexValid(int voiceIndex)
{
    return 0 <= voiceIndex && voiceIndex < VOICES;
}
}

#endif // MU_NOTATION_NOTATIONTYPES_H
