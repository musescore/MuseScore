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

#include "libmscore/mcursor.h"

#include "engraving/scoreaccess.h"

#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/durationtype.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/score.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/keysig.h"
#include "libmscore/timesig.h"

using namespace mu;

namespace Ms {
extern MScore* mscore;

//---------------------------------------------------------
//   MCursor
//---------------------------------------------------------

MCursor::MCursor(MasterScore* s)
{
    _score = s;
    move(0, Fraction(0, 1));
}

//---------------------------------------------------------
//   createMeasures
//---------------------------------------------------------

void MCursor::createMeasures()
{
    Measure* measure;
    for (;;) {
        Fraction tick = Fraction(0, 1);
        measure = _score->lastMeasure();
        if (measure) {
            tick = measure->tick() + measure->ticks();
            if (tick > _tick) {
                break;
            }
        }
        measure = new Measure(_score);
        measure->setTick(tick);
        measure->setTimesig(_sig);
        measure->setTicks(_sig);
        _score->measures()->add(measure);
    }
}

//---------------------------------------------------------
//   addChord
//---------------------------------------------------------

Chord* MCursor::addChord(int pitch, const TDuration& duration)
{
    createMeasures();
    Measure* measure = _score->tick2measure(_tick);
    Segment* segment = measure->getSegment(SegmentType::ChordRest, _tick);
    Chord* chord = toChord(segment->element(_track));
    if (chord == 0) {
        chord = new Chord(_score);
        chord->setTrack(_track);
        chord->setDurationType(duration);
        chord->setTicks(duration.fraction());
        segment->add(chord);
    }
    Note* note = new Note(_score);
    chord->add(note);
    note->setPitch(pitch);
    note->setTpcFromPitch();
    _tick += duration.ticks();
    return chord;
}

//---------------------------------------------------------
//   addKeySig
//---------------------------------------------------------

void MCursor::addKeySig(Key key)
{
    createMeasures();
    Measure* measure = _score->tick2measure(_tick);
    Segment* segment = measure->getSegment(SegmentType::KeySig, _tick);
    int n = _score->nstaves();
    for (int i = 0; i < n; ++i) {
        KeySig* ks = new KeySig(_score);
        ks->setKey(key);
        ks->setTrack(i * VOICES);
        segment->add(ks);
    }
}

//---------------------------------------------------------
//   addTimeSig
//---------------------------------------------------------

TimeSig* MCursor::addTimeSig(const Fraction& f)
{
    createMeasures();
    Measure* measure = _score->tick2measure(_tick);
    Segment* segment = measure->getSegment(SegmentType::TimeSig, _tick);
    TimeSig* ts = 0;
    for (int i = 0; i < _score->nstaves(); ++i) {
        ts = new TimeSig(_score);
        ts->setSig(f, TimeSigType::NORMAL);
        ts->setTrack(i * VOICES);
        segment->add(ts);
    }
    _score->sigmap()->add(_tick.ticks(), SigEvent(f));
    return ts;
}

//---------------------------------------------------------
//   createScore
//---------------------------------------------------------

void MCursor::createScore(const QString& name)
{
    delete _score;
    _score = mu::engraving::ScoreAccess::createMasterScore(mscore->baseStyle());
    _score->setName(name);
    move(0, Fraction(0, 1));
}

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void MCursor::move(int t, const Fraction& tick)
{
    _track = t;
    _tick = tick;
}

//---------------------------------------------------------
//   addPart
//---------------------------------------------------------

void MCursor::addPart(const QString& instrument)
{
    Part* part   = new Part(_score);
    Staff* staff = new Staff(_score);
    staff->setPart(part);
    InstrumentTemplate* it = searchTemplate(instrument);
    if (it == 0) {
        qFatal("Did not find instrument <%s>", qPrintable(instrument));
    }
    part->initFromInstrTemplate(it);
    staff->init(it, 0, 0);
    _score->appendPart(part);
    _score->insertStaff(staff, 0);
}
}
