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

#include "annotationsmetaparser.h"

#include "dom/dynamic.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::mpe;

void AnnotationsMetaParser::doParse(const EngravingItem* item, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    IF_ASSERT_FAILED(item) {
        return;
    }

    mpe::ArticulationType type = mpe::ArticulationType::Undefined;

    if (item->isDynamic()) {
        const Dynamic* dynamic = toDynamic(item);
        const DynamicType dynamicType = dynamic->dynamicType();

        if (dynamicType == DynamicType::S
            || dynamicType == DynamicType::SF
            || dynamicType == DynamicType::SFF
            || dynamicType == DynamicType::SFFF
            || dynamicType == DynamicType::SFZ
            || dynamicType == DynamicType::SFFZ
            || dynamicType == DynamicType::SFFFZ
            || dynamicType == DynamicType::SFP
            || dynamicType == DynamicType::SFPP) {
            type = mpe::ArticulationType::Subito;
        }
    }

    if (type == mpe::ArticulationType::Undefined) {
        return;
    }

    const mpe::ArticulationPattern& pattern = ctx.profile->pattern(type);
    if (pattern.empty()) {
        return;
    }

    appendArticulationData(mpe::ArticulationMeta(type, pattern, ctx.nominalTimestamp, ctx.nominalDuration), result);
}
