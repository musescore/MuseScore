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

#include "chordlinemetaparser.h"

#include "dom/chordline.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::mpe;

void ChordLineMetaParser::doParse(const EngravingItem* item, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    IF_ASSERT_FAILED(item) {
        return;
    }

    if (!item->isChordLine()) {
        return;
    }

    const ChordLine* chordLine = toChordLine(item);

    ArticulationType type = chordLineArticulationType(chordLine->chordLineType());

    if (type == ArticulationType::Undefined) {
        return;
    }

    const mpe::ArticulationPattern& pattern = ctx.profile->pattern(type);
    if (pattern.empty()) {
        return;
    }

    appendArticulationData(mpe::ArticulationMeta(type, pattern, ctx.nominalTimestamp, ctx.nominalDuration), result);
}

ArticulationType ChordLineMetaParser::chordLineArticulationType(const ChordLineType chordLineType)
{
    static const std::unordered_map<ChordLineType, ArticulationType> ARTICULATION_TYPES = {
        { ChordLineType::DOIT, ArticulationType::Doit },
        { ChordLineType::FALL, ArticulationType::Fall },
        { ChordLineType::PLOP, ArticulationType::Plop },
        { ChordLineType::SCOOP, ArticulationType::Scoop }
    };

    auto search = ARTICULATION_TYPES.find(chordLineType);

    if (search != ARTICULATION_TYPES.cend()) {
        return search->second;
    }

    return ArticulationType::Undefined;
}
