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

#include "io/path.h"

#include "libmscore/element.h"
#include "libmscore/durationtype.h"
#include "libmscore/mscore.h"
#include "libmscore/score.h"
#include "libmscore/timesig.h"
#include "libmscore/key.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/stafftype.h"
#include "libmscore/score.h"

#include "instruments/instrumentstypes.h"

namespace mu {
namespace notation {
using Element = Ms::Element;
using ElementType = Ms::ElementType;
using Measure = Ms::Measure;
using DurationType = Ms::TDuration::DurationType;
using Duration = Ms::TDuration;
using SelectType = Ms::SelectType;
using Pad = Ms::Pad;
using ViewMode = Ms::LayoutMode;  // Accomodate inconsistent convention from v3
using PitchMode = Ms::UpDownMode;
using StyleId = Ms::Sid;
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

struct Meta
{
    QString fileName;
    QString filePath;
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

struct ScoreCreateOptions {
    QString title;
    QString subtitle;
    QString composer;
    QString lyricist;
    QString copyright;

    bool withTempo = false;
    double tempo = 0.0;

    int timesigNumerator = 0;
    int timesigDenominator = 0;
    TimeSigType timesigType = TimeSigType::NORMAL;

    Key key = Key::C_B;
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
}
}

#endif // MU_NOTATION_NOTATIONTYPES_H
