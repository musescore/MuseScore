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

#ifndef MU_ENGRAVING_RENDERBASE_H
#define MU_ENGRAVING_RENDERBASE_H

#include "log.h"

#include "playback/renderingcontext.h"

namespace mu::engraving {
template<class T>
class RenderBase
{
public:
    static bool isAbleToRender(const mpe::ArticulationType& type)
    {
        const mpe::ArticulationTypeSet& supportedTypes = T::supportedTypes();

        return supportedTypes.find(type) != supportedTypes.cend();
    }

    static void render(const EngravingItem* item, const mpe::ArticulationType preferredType, const RenderingContext& context,
                       mpe::PlaybackEventList& result)
    {
        IF_ASSERT_FAILED(item) {
            return;
        }

        T::doRender(item, preferredType, context, result);
    }

protected:
    static void updateArticulationBoundaries(const mpe::ArticulationType type, const mpe::timestamp_t nominalTimestamp,
                                             const mpe::duration_t nominalDuration,
                                             mpe::ArticulationMap& noteArticulationMap)
    {
        if (noteArticulationMap.empty()) {
            return;
        }

        const mpe::ArticulationAppliedData& articulationData = noteArticulationMap.at(type);

        mpe::timestamp_t articulationOccupiedFrom = nominalTimestamp - articulationData.meta.timestamp;
        mpe::timestamp_t articulationOccupiedTo = nominalTimestamp + nominalDuration - articulationData.meta.timestamp;

        noteArticulationMap.updateOccupiedRange(type,
                                                mpe::occupiedPercentage(articulationOccupiedFrom,
                                                                        articulationData.meta.overallDuration),
                                                mpe::occupiedPercentage(articulationOccupiedTo,
                                                                        articulationData.meta.overallDuration));
    }
};
}

#endif // MU_ENGRAVING_RENDERBASE_H
