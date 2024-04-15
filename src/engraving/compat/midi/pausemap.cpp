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

/**
 \file
 Implementation of class PauseMap.
*/

#include "pausemap.h"

#include "dom/repeatlist.h"
#include "dom/score.h"
#include "dom/sig.h"
#include "dom/tempo.h"

#include "log.h"

using namespace mu;
using namespace muse;

namespace mu::engraving {
//---------------------------------------------------------
//   calculate
//    MIDI files cannot contain pauses so insert extra ticks and tempo changes instead.
//    The PauseMap and new TempoMap are fully unwound to account for pauses on repeats.
//---------------------------------------------------------

void PauseMap::calculate(const Score* s)
{
    IF_ASSERT_FAILED(s) {
        LOGE() << "failed to calculate pause map";
    }

    TimeSigMap* sigmap = s->sigmap();
    TempoMap* tempomap = s->tempomap();

    insert(std::pair<const int, int>(0, 0));    // can't start with a pause

    m_tempomapWithPauses = std::make_shared<TempoMap>();
    m_tempomapWithPauses->setTempoMultiplier(tempomap->tempoMultiplier());

    for (const RepeatSegment* rs : s->repeatList()) {
        int startTick  = rs->tick;
        int endTick    = startTick + rs->len();
        int tickOffset = rs->utick - rs->tick;

        auto se = tempomap->lower_bound(startTick);
        auto ee = tempomap->lower_bound(endTick + 1);   // +1 to include first tick of next RepeatSegment

        for (auto it = se; it != ee; ++it) {
            int tick = it->first;
            int utick = tick + tickOffset;

            if (RealIsNull(it->second.pause)) {
                // We have a regular tempo change. Don't include tempo change from first tick of next RepeatSegment (it will be included later).
                if (tick != endTick) {
                    m_tempomapWithPauses->insert(std::pair<const int, TEvent>(tickWithPauses(utick), it->second));
                }
            } else {
                // We have a pause event. Don't include pauses from first tick of current RepeatSegment (it was included in the previous one).
                if (tick != startTick) {
                    Fraction timeSig(sigmap->timesig(tick).timesig());
                    double quarterNotesPerMeasure = (4.0 * timeSig.numerator()) / timeSig.denominator();
                    int ticksPerMeasure =  quarterNotesPerMeasure * Constants::DIVISION;           // store a full measure of ticks to keep barlines in same places
                    m_tempomapWithPauses->setTempo(tickWithPauses(utick), quarterNotesPerMeasure / it->second.pause);           // new tempo for pause
                    insert(std::pair<const int, int>(utick, ticksPerMeasure + offsetAtUTick(utick)));            // store running total of extra ticks
                    m_tempomapWithPauses->setTempo(tickWithPauses(utick), it->second.tempo);           // restore previous tempo
                }
            }
        }
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
