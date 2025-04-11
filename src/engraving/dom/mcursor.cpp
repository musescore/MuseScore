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

#include "mcursor.h"

#include "compat/scoreaccess.h"

#include "chord.h"
#include "durationtype.h"
#include "factory.h"
#include "instrtemplate.h"
#include "keysig.h"
#include "masterscore.h"
#include "measure.h"
#include "note.h"
#include "part.h"
#include "segment.h"
#include "staff.h"
#include "timesig.h"
#include "sig.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   MCursor
//---------------------------------------------------------

MCursor::MCursor(MasterScore* s)
{
    m_score = s;
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
        measure = m_score->lastMeasure();
        if (measure) {
            tick = measure->tick() + measure->ticks();
            if (tick > m_tick) {
                break;
            }
        }
        measure = Factory::createMeasure(m_score->dummy()->system());
        measure->setTick(tick);
        measure->setTimesig(m_sig);
        measure->setTicks(m_sig);
        m_score->measures()->append(measure);
    }
}

//---------------------------------------------------------
//   addChord
//---------------------------------------------------------

Chord* MCursor::addChord(int pitch, const TDuration& duration)
{
    createMeasures();
    Measure* measure = m_score->tick2measure(m_tick);
    Segment* segment = measure->getSegment(SegmentType::ChordRest, m_tick);
    Chord* chord = toChord(segment->element(m_track));
    if (chord == 0) {
        chord = Factory::createChord(segment);
        chord->setTrack(m_track);
        chord->setDurationType(duration);
        chord->setTicks(duration.fraction());
        segment->add(chord);
    }
    Note* note = Factory::createNote(chord);
    chord->add(note);
    note->setPitch(pitch);
    note->setTpcFromPitch();
    m_tick += duration.ticks();
    return chord;
}

//---------------------------------------------------------
//   addKeySig
//---------------------------------------------------------

void MCursor::addKeySig(Key key)
{
    createMeasures();
    Measure* measure = m_score->tick2measure(m_tick);
    Segment* segment = measure->getSegment(SegmentType::KeySig, m_tick);
    size_t n = m_score->nstaves();
    for (staff_idx_t i = 0; i < n; ++i) {
        KeySig* ks = Factory::createKeySig(segment);
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
    Measure* measure = m_score->tick2measure(m_tick);
    Segment* segment = measure->getSegment(SegmentType::TimeSig, m_tick);
    TimeSig* ts = 0;
    for (size_t i = 0; i < m_score->nstaves(); ++i) {
        ts = Factory::createTimeSig(segment);
        ts->setSig(f, TimeSigType::NORMAL);
        ts->setTrack(i * VOICES);
        segment->add(ts);
    }
    m_score->sigmap()->add(m_tick.ticks(), SigEvent(f));
    return ts;
}

//---------------------------------------------------------
//   createScore
//---------------------------------------------------------

void MCursor::createScore(const muse::modularity::ContextPtr& iocCtx, const String& /*name*/)
{
    delete m_score;
    m_score = compat::ScoreAccess::createMasterScoreWithBaseStyle(iocCtx);
    // TODO: set path/filename
    NOT_IMPLEMENTED;
    move(0, Fraction(0, 1));
}

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void MCursor::move(int t, const Fraction& tick)
{
    m_track = t;
    m_tick = tick;
}

//---------------------------------------------------------
//   addPart
//---------------------------------------------------------

void MCursor::addPart(const String& instrument)
{
    Part* part   = new Part(m_score);
    Staff* staff = Factory::createStaff(part);
    const InstrumentTemplate* it = searchTemplate(instrument);
    IF_ASSERT_FAILED(it) {
        return;
    }

    part->initFromInstrTemplate(it);
    staff->init(it, 0, 0);
    m_score->appendPart(part);
    m_score->insertStaff(staff, 0);
}
}
