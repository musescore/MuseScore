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

#ifndef MU_ENGRAING_PARSERBASE_H
#define MU_ENGRAING_PARSERBASE_H

#include "log.h"
#include "mpe/mpetypes.h"

#include "playback/renderingcontext.h"

namespace Ms {
class EngravingItem;
}

namespace mu::engraving {
template<class T>
class MetaParserBase
{
public:
    static void parse(const Ms::EngravingItem* item, const RenderingContext& context, mpe::ArticulationMap& result)
    {
        IF_ASSERT_FAILED(item) {
            return;
        }

        T::doParse(item, context, result);
    }

protected:
    static void appendArticulationData(mpe::ArticulationMeta&& meta, mpe::ArticulationMap& result)
    {
        result.emplace(meta.type, mpe::ArticulationAppliedData(std::forward<mpe::ArticulationMeta>(meta), 0, mpe::HUNDRED_PERCENT));
    }
};
}

#endif // MU_ENGRAING_PARSERBASE_H
