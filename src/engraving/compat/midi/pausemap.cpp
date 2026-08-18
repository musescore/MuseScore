/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

/**
 \file
 Implementation of class PauseMap.
*/

#include "pausemap.h"

#include "dom/repeatlist.h"
#include "dom/score.h"
#include "dom/sig.h"
#include "dom/tempotimeline.h"

#include "log.h"

using namespace mu;
using namespace muse;

namespace mu::engraving {
//---------------------------------------------------------
//   calculate
//    MIDI files cannot contain pauses so insert extra ticks and tempo changes instead.
//    The PauseMap and tempo events are fully unwound to account for pauses on repeats.
//---------------------------------------------------------

void PauseMap::calculate(const Score* s)
{
    IF_ASSERT_FAILED(s) {
        LOGE() << "failed to calculate pause map";
    }

    insert(std::pair<const int, int>(0, 0));    // can't start with a pause

    const TimeSigMap* sigmap = s->sigmap();
    const RepeatList& repeatList = s->repeatList();

    for (const TempoTimePoint& point : s->tempoTimeline().points()) {
        const int utick = point.utick;

        if (point.pause <= 0.0) {
            // We have a regular tempo change.
            m_tempoEvents[tickWithPauses(utick)] = point.bps;
            continue;
        }

        // We have a pause event.
        const int tick = repeatList.utick2tick(utick);
        Fraction timeSig(sigmap->timesig(tick).timesig());
        double quarterNotesPerMeasure = (4.0 * timeSig.numerator()) / timeSig.denominator();
        int ticksPerMeasure =  quarterNotesPerMeasure * Constants::DIVISION;           // store a full measure of ticks to keep barlines in same places
        m_tempoEvents[tickWithPauses(utick)] = quarterNotesPerMeasure / point.pause;           // new tempo for pause
        insert(std::pair<const int, int>(utick, ticksPerMeasure + offsetAtUTick(utick)));            // store running total of extra ticks
        m_tempoEvents[tickWithPauses(utick)] = point.bps;           // restore previous tempo
    }
}

//---------------------------------------------------------
//   offsetAtUTick
//    In total, how many extra ticks have been inserted prior to this utick.
//---------------------------------------------------------

int PauseMap::offsetAtUTick(int utick) const
{
    // make sure calculate was called
    IF_ASSERT_FAILED(!empty()) {
        LOGE() << "accessing empty container";
    }

    auto i = upper_bound(utick);
    if (i != begin()) {
        --i;
    }
    return i->second;
}
}
