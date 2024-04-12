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

#ifndef WRITER_H
#define WRITER_H

/**
 \file
 Definition of class Writer
 */

#include <QtCore/QString>

#include "symbols.h"

class QIODevice;

namespace Bww {
static const int maxBeamLevel = 3;

enum class BeamType
{
    BM_NONE,
    BM_BEGIN,
    BM_CONTINUE,
    BM_END,
    BM_FORWARD_HOOK,
    BM_BACKWARD_HOOK
};

/**
   The flags that need to be handled at the beginning of a measure.
   */

struct MeasureBeginFlags {
    bool repeatBegin;
    bool endingFirst;
    bool endingSecond;
    bool firstOfSystem;
    bool irregular;
    MeasureBeginFlags()
        : repeatBegin(false),
        endingFirst(false),
        endingSecond(false),
        firstOfSystem(false),
        irregular(false)
    {}
};

/**
   The flags that need to be handled at the end of a measure.
   */

struct MeasureEndFlags {
    bool repeatEnd;
    bool endingEnd;
    bool lastOfSystem;
    bool lastOfPart;
    bool doubleBarLine;
    MeasureEndFlags()
        : repeatEnd(false),
        endingEnd(false),
        lastOfSystem(false),
        lastOfPart(false),
        doubleBarLine(false)
    {}
};

/**
   The writer that generates the output.
   */

class Writer
{
public:
    virtual void header(const QString title, const QString type, const QString composer, const QString footer, const unsigned int temp) = 0;
    virtual void tsig(const int beats, const int beat) = 0;
    virtual void trailer() = 0;
    virtual void beginMeasure(const Bww::MeasureBeginFlags mbf) = 0;
    virtual void endMeasure(const Bww::MeasureEndFlags mef) = 0;
    virtual void note(const QString pitch, const QVector<BeamType> beamList, const QString type, const int dots, bool tieStart = false,
                      bool tieStop = false, StartStop triplet = StartStop::ST_NONE, bool grace = false) = 0;
    QString instrumentName() const { return "Bagpipe"; }
    int midiProgram() const { return 110; }
};
} // namespace Bww

#endif // WRITER_H
