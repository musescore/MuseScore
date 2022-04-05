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

#include "annotationsmetaparser.h"

#include "libmscore/dynamic.h"

using namespace mu::engraving;
using namespace mu::mpe;

void AnnotationsMetaParser::doParse(const Ms::EngravingItem* item, const RenderingContext& ctx,
                                    mpe::ArticulationMap& result)
{
    IF_ASSERT_FAILED(ctx.isValid()) {
        return;
    }

    mpe::ArticulationType type = mpe::ArticulationType::Undefined;

    if (item->isDynamic()) {
        const Ms::Dynamic* dynamic = Ms::toDynamic(item);
        const Ms::DynamicType dynamicType = dynamic->dynamicType();

        if (dynamicType == Ms::DynamicType::S
            || dynamicType == Ms::DynamicType::SF
            || dynamicType == Ms::DynamicType::SFF
            || dynamicType == Ms::DynamicType::SFFZ
            || dynamicType == Ms::DynamicType::SFP
            || dynamicType == Ms::DynamicType::SFPP
            || dynamicType == Ms::DynamicType::SFZ) {
            type = mpe::ArticulationType::Subito;
        }
    }

    if (type == mpe::ArticulationType::Undefined) {
        return;
    }

    appendArticulationData(mpe::ArticulationMeta(type, ctx.profile->pattern(type), ctx.nominalTimestamp, ctx.nominalDuration), result);
}
