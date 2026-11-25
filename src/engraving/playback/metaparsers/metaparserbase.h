/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#pragma once
#ifndef MU_ENGRAVING_PARSERBASE_H
#define MU_ENGRAVING_PARSERBASE_H

#include <set>

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

        if (!context.sourceRecorder) {
            T::doParse(item, context, result);
            return;
        }

        std::set<muse::mpe::ArticulationType> typesBefore;

        for (const auto& pair : result) {
            typesBefore.insert(pair.first);
        }

        T::doParse(item, context, result);

        // Nested parses (e.g. a tie inside a slur) finish first, so they claim
        // their types before the enclosing item can
        for (const auto& pair : result) {
            if (!typesBefore.contains(pair.first)) {
                context.sourceRecorder->sources.try_emplace(pair.first, item);
            }
        }
    }

protected:
    static void appendArticulationData(muse::mpe::ArticulationMeta&& meta, muse::mpe::ArticulationMap& result)
    {
        result.emplace(meta.type,
                       muse::mpe::ArticulationAppliedData(std::forward<muse::mpe::ArticulationMeta>(meta), 0, muse::mpe::HUNDRED_PERCENT));
    }
};
} // namespace mu::engraving

#endif // MU_ENGRAVING_PARSERBASE_H
