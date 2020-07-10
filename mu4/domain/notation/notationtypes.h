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
#ifndef MU_DOMAIN_NOTATIONTYPES_H
#define MU_DOMAIN_NOTATIONTYPES_H

#include <QPixmap>

#include "libmscore/element.h"
#include "libmscore/durationtype.h"
#include "libmscore/mscore.h"
#include "libmscore/score.h"
#include "libmscore/timesig.h"
#include "libmscore/key.h"

namespace mu {
namespace domain {
namespace notation {
using Element = Ms::Element;
using ElementType = Ms::ElementType;
using DurationType = Ms::TDuration::DurationType;
using SelectType = Ms::SelectType;
using Pad = Ms::Pad;
using PitchMode = Ms::UpDownMode;

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
    QString title;
    QString composer;
    QString arranger;
    size_t partsCount = 0;
    QPixmap thumbnail;
};

struct ScoreCreateOptions {
    QString title;
    QString subtitle;
    QString composer;
    QString poet;
    QString copyright;

    double tempo = 0.0;
    int timesigNumerator = 0;
    int timesigDenominator = 0;
    Ms::TimeSigType timesigType = Ms::TimeSigType::NORMAL;

    Ms::Key key = Ms::Key::C_B;

    int measures = 0;
    int measureTimesigNumerator = 0;
    int measureTimesigDenominator = 0;

    QString templatePath;
};

}
}
}

#endif // MU_DOMAIN_NOTATIONTYPES_H
