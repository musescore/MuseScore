/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#include "spannerfilter.h"

#include "libmscore/spanner.h"
#include "libmscore/pedal.h"
#include "libmscore/segment.h"

using namespace mu;
using namespace mu::engraving;

bool SpannerFilter::isPlayable(const EngravingItem* item, const RenderingContext& ctx)
{
    IF_ASSERT_FAILED(item->isSpanner()) {
        return false;
    }

    const Spanner* spanner = toSpanner(item);

    int spannerFrom = spanner->tick().ticks();
    int spannerDurationTicks = spannerActualDurationTicks(spanner, spanner->ticks().ticks());
    int spannerTo = spannerFrom + spannerDurationTicks;

    if (spannerDurationTicks == 0
        || spannerTo < ctx.nominalPositionStartTick
        || spannerFrom >= ctx.nominalPositionEndTick) {
        return false;
    }

    return true;
}

int SpannerFilter::spannerActualDurationTicks(const Spanner* spanner, const int nominalDurationTicks)
{
    if (spanner->type() == ElementType::TRILL) {
        return spanner->endSegment()->tick().ticks() - spanner->tick().ticks() - 1;
    }

    if (spanner->type() == ElementType::PEDAL) {
        const Pedal* pedal = toPedal(spanner);
        if (pedal->endHookType() == HookType::HOOK_45) {
            return nominalDurationTicks - Constants::division / 4;
        }
    }

    if (spanner->type() == ElementType::SLUR) {
        EngravingItem* startItem = spanner->startElement();
        EngravingItem* endItem = spanner->endElement();

        if (!startItem || !endItem) {
            return nominalDurationTicks;
        }

        if (startItem->isChordRest() && endItem->isChordRest()) {
            ChordRest* startChord = toChordRest(startItem);
            ChordRest* endChord = toChordRest(endItem);
            return endChord->tick().ticks() + endChord->ticks().ticks() - startChord->tick().ticks();
        }
    }

    return nominalDurationTicks;
}

bool SpannerFilter::isMultiStaffSpanner(const Spanner* spanner)
{
    static const ElementTypeSet MULTI_STAFF_SPANNERS = {
        ElementType::PEDAL,
        ElementType::PEDAL_SEGMENT
    };

    return MULTI_STAFF_SPANNERS.find(spanner->type()) != MULTI_STAFF_SPANNERS.cend();
}
