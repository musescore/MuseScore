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

#ifndef MU_ENGRAVING_PARSERBASE_H
#define MU_ENGRAVING_PARSERBASE_H

#include "log.h"
#include "mpe/mpetypes.h"

#include "playback/renderingcontext.h"

namespace mu::engraving {
class EngravingItem;

template<class T>
class MetaParserBase
{
public:
    static void parse(const EngravingItem* item, const RenderingContext& context, muse::mpe::ArticulationMap& result)
    {
        IF_ASSERT_FAILED(item) {
            return;
        }

        T::doParse(item, context, result);
    }

protected:
    static void appendArticulationData(muse::mpe::ArticulationMeta&& meta, muse::mpe::ArticulationMap& result)
    {
        result.emplace(meta.type,
                       muse::mpe::ArticulationAppliedData(std::forward<muse::mpe::ArticulationMeta>(meta), 0, muse::mpe::HUNDRED_PERCENT));
    }
};
}

#endif // MU_ENGRAVING_PARSERBASE_H
