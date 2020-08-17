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

#include "instruments/instrumentstypes.h"

namespace mu {
namespace notation {
using Element = Ms::Element;
using ElementType = Ms::ElementType;
using DurationType = Ms::TDuration::DurationType;
using SelectType = Ms::SelectType;
using Pad = Ms::Pad;
using PitchMode = Ms::UpDownMode;
using StyleId = Ms::Sid;
using Key = Ms::Key;
using KeyMode = Ms::KeyMode;
using TimeSigType = Ms::TimeSigType;

enum class DragMode {
    BothXY = 0,
    OnlyX,
    OnlyY
};

enum class MoveDirection {
    Undefined = 0,
    Left,
    Right,
    Up,
    Down
};

enum class MoveSelectionType {
    Undefined = 0,
    Element,
    Chord,
    Measure,
    Track
};

struct Meta {
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
    QList<instruments::InstrumentTemplate> instrumentTemplates;
};
}
}

#endif // MU_NOTATION_NOTATIONTYPES_H
