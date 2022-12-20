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

#include "arpeggiometaparser.h"

#include "libmscore/arpeggio.h"

using namespace mu::engraving;

void ArpeggioMetaParser::doParse(const EngravingItem* item, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    IF_ASSERT_FAILED(item->type() == ElementType::ARPEGGIO) {
        return;
    }

    const Arpeggio* arpeggio = toArpeggio(item);

    if (!arpeggio->playArpeggio()) {
        return;
    }

    mpe::ArticulationType type = mpe::ArticulationType::Undefined;

    switch (arpeggio->arpeggioType()) {
    case ArpeggioType::NORMAL:
        type = mpe::ArticulationType::Arpeggio;
        break;
    case ArpeggioType::UP:
        type = mpe::ArticulationType::ArpeggioUp;
        break;
    case ArpeggioType::DOWN:
        type = mpe::ArticulationType::ArpeggioDown;
        break;
    case ArpeggioType::DOWN_STRAIGHT:
        type = mpe::ArticulationType::ArpeggioStraightDown;
        break;
    case ArpeggioType::UP_STRAIGHT:
        type = mpe::ArticulationType::ArpeggioStraightUp;
        break;
    default:
        break;
    }

    if (type == mpe::ArticulationType::Undefined) {
        return;
    }

    mpe::ArticulationMeta articulationMeta;
    articulationMeta.type = type;
    articulationMeta.pattern = ctx.profile->pattern(type);
    articulationMeta.timestamp = ctx.nominalTimestamp;
    articulationMeta.overallDuration = ctx.nominalDuration * arpeggio->Stretch();

    appendArticulationData(std::move(articulationMeta), result);
}
