/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

// Post-pass: resolve ottava (8va/8vb) line spanner endpoints.

#include "resolvers.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/ottava.h"

#include <algorithm>

using namespace mu::engraving;

namespace mu::iex::enc {
void resolveOttavas(BuildCtx& ctx)
{
    if (ctx.pendingOttavas.empty()) {
        return;
    }

    MasterScore* score = ctx.score;

    // Sort by staffIdx asc, then startTick asc so the "next ottava" lookup is a simple i+1 check.
    std::vector<PendingOttava> sorted = ctx.pendingOttavas;
    std::sort(sorted.begin(), sorted.end(), [](const PendingOttava& a, const PendingOttava& b) {
        if (a.staffIdx != b.staffIdx) {
            return a.staffIdx < b.staffIdx;
        }
        return a.startTick < b.startTick;
    });

    Measure* lastMeasure = score->lastMeasure();
    const Fraction scoreEnd = lastMeasure ? lastMeasure->endTick() : Fraction(0, 1);

    for (size_t i = 0; i < sorted.size(); ++i) {
        const PendingOttava& po = sorted[i];

        // po.track is derived from file data; skip a spanner whose track is out of range
        // rather than hand an out-of-bounds track to the engraving DOM.
        if (!validTrack(score, po.track)) {
            continue;
        }

        // Endpoint: start of the next ottava on the same staff, or end of score.
        Fraction endTick = scoreEnd;
        if (i + 1 < sorted.size() && sorted[i + 1].staffIdx == po.staffIdx) {
            endTick = sorted[i + 1].startTick;
        }

        if (endTick <= po.startTick) {
            continue;
        }

        Ottava* ottava = Factory::createOttava(score->dummy());
        ottava->setAnchor(Spanner::Anchor::SEGMENT);
        ottava->setTrack(po.track);
        ottava->setTrack2(po.track);
        ottava->setTick(po.startTick);
        ottava->setTick2(endTick);
        ottava->setOttavaType(po.ottavaType);
        score->addElement(ottava);
    }
}
} // namespace mu::iex::enc
