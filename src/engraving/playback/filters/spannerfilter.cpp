/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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

#include "dom/spanner.h"
#include "dom/pedal.h"
#include "dom/segment.h"

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
        || spannerTo <= ctx.nominalPositionStartTick
        || spannerFrom >= ctx.nominalPositionEndTick) {
        return false;
    }

    return true;
}

int SpannerFilter::spannerActualDurationTicks(const Spanner* spanner, const int nominalDurationTicks)
{
    const ElementType type = spanner->type();

    if (type == ElementType::SLUR || type == ElementType::HAMMER_ON_PULL_OFF || type == ElementType::TRILL) {
        const EngravingItem* startItem = spanner->startElement();
        const EngravingItem* endItem = spanner->endElement();

        if (!startItem || !endItem) {
            return nominalDurationTicks;
        }

        if (startItem->isChordRest() && endItem->isChordRest()) {
            const ChordRest* startChord = toChordRest(startItem);
            const ChordRest* endChord = toChordRest(endItem);
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
