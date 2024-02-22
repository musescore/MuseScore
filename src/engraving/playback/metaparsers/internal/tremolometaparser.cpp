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

#include "tremolometaparser.h"

#include "dom/tremolosinglechord.h"
#include "dom/tremolotwochord.h"
#include "dom/chord.h"

#include "mpe/mpetypes.h"
#include "../../renderingcontext.h"

using namespace mu::engraving;

static mu::mpe::ArticulationType toArticulationType(TremoloType type)
{
    switch (type) {
    case TremoloType::R8:
    case TremoloType::C8:
        return mu::mpe::ArticulationType::Tremolo8th;
        break;

    case TremoloType::R16:
    case TremoloType::C16:
        return mu::mpe::ArticulationType::Tremolo16th;
        break;

    case TremoloType::R32:
    case TremoloType::C32:
        return mu::mpe::ArticulationType::Tremolo32nd;
        break;

    case TremoloType::R64:
    case TremoloType::C64:
        return mu::mpe::ArticulationType::Tremolo64th;
        break;

    default:
        break;
    }

    return mu::mpe::ArticulationType::Undefined;
}

void TremoloSingleMetaParser::doParse(const EngravingItem* item, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    IF_ASSERT_FAILED(item->type() == ElementType::TREMOLO_SINGLECHORD) {
        return;
    }

    const TremoloSingleChord* tremolo = item_cast<const TremoloSingleChord*>(item);

    mpe::ArticulationType type = toArticulationType(tremolo->tremoloType());
    if (type == mpe::ArticulationType::Undefined) {
        return;
    }

    int overallDurationTicks = ctx.nominalDurationTicks;

    const mpe::ArticulationPattern& pattern = ctx.profile->pattern(type);
    if (pattern.empty()) {
        return;
    }

    mpe::ArticulationMeta articulationMeta;
    articulationMeta.type = type;
    articulationMeta.pattern = pattern;
    articulationMeta.timestamp = ctx.nominalTimestamp;
    articulationMeta.overallDuration = timestampFromTicks(tremolo->score(), ctx.nominalPositionStartTick + overallDurationTicks)
                                       - ctx.nominalTimestamp;

    appendArticulationData(std::move(articulationMeta), result);
}

void TremoloTwoMetaParser::doParse(const EngravingItem* item, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    IF_ASSERT_FAILED(item->type() == ElementType::TREMOLO_TWOCHORD) {
        return;
    }

    const TremoloTwoChord* tremolo = item_cast<const TremoloTwoChord*>(item);

    const Chord* chord2 = tremolo->chord2();
    IF_ASSERT_FAILED(chord2) {
        return;
    }

    if (chord2->tick().ticks() == ctx.nominalPositionStartTick) {
        return;
    }

    mpe::ArticulationType type = toArticulationType(tremolo->tremoloType());
    if (type == mpe::ArticulationType::Undefined) {
        return;
    }

    int overallDurationTicks = ctx.nominalDurationTicks;
    if (tremolo->chord1() && tremolo->chord2()) {
        overallDurationTicks = tremolo->chord1()->actualTicks().ticks() + tremolo->chord2()->actualTicks().ticks();
    }

    const mpe::ArticulationPattern& pattern = ctx.profile->pattern(type);
    if (pattern.empty()) {
        return;
    }

    mpe::ArticulationMeta articulationMeta;
    articulationMeta.type = type;
    articulationMeta.pattern = pattern;
    articulationMeta.timestamp = ctx.nominalTimestamp;
    articulationMeta.overallDuration = timestampFromTicks(tremolo->score(), ctx.nominalPositionStartTick + overallDurationTicks)
                                       - ctx.nominalTimestamp;

    appendArticulationData(std::move(articulationMeta), result);
}
