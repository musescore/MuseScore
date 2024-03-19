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

#include "engravingcompat.h"

#include "engraving/dom/beam.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/masterscore.h"

using namespace mu::engraving;

namespace mu::engraving::compat {
void EngravingCompat::doPostLayoutCompatIfNeeded(MasterScore* score)
{
    if (score->mscVersion() >= 440) {
        return;
    }

    bool needRelayout = false;

    if (relayoutUserModifiedCrossStaffBeams(score)) {
        needRelayout = true;
    }
    // As we progress, likely that more things will be done here

    if (needRelayout) {
        score->update();
    }
}

bool EngravingCompat::relayoutUserModifiedCrossStaffBeams(MasterScore* score)
{
    bool found = false;

    auto findBeam = [&found, score](ChordRest* cr) {
        Beam* beam = cr->beam();
        if (beam && beam->userModified() && beam->cross() && beam->elements().front() == cr) {
            found = true;
            beam->triggerLayout();
        }
    };

    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment& seg : toMeasure(mb)->segments()) {
            if (!seg.isChordRestType()) {
                continue;
            }
            for (EngravingItem* item : seg.elist()) {
                if (!item) {
                    continue;
                }
                findBeam(toChordRest(item));
                if (item->isChord()) {
                    for (Chord* grace : toChord(item)->graceNotes()) {
                        findBeam(grace);
                    }
                }
            }
        }
    }

    return true;
}
} // namespace mu::engraving::compat
