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

#include "playback/playbackcontext.h"

namespace mu::engraving {

template<class T>
class RenderBase
{
public:
    static T* instance()
    {
        static T s;
        return &s;
    }

    bool isAbleToRender(const mpe::ArticulationType& type) const
    {
        const T* renderImpl = static_cast<const T*>(this);
        const mpe::ArticulationTypeSet& supportedTypes = renderImpl->supportedTypes();

        return supportedTypes.find(type) != supportedTypes.cend();
    }

    void render(const Ms::EngravingItem* item, const mpe::ArticulationType preferredType, PlaybackContext&& context,
                mpe::PlaybackEventList& result) const
    {
        IF_ASSERT_FAILED(item) {
            return;
        }

        const T* renderImpl = static_cast<const T*>(this);
        renderImpl->doRender(item, preferredType, std::move(context), result);
    }

protected:
    void updateArticulationBoundaries(const mpe::ArticulationType type, const mpe::timestamp_t nominalTimestamp,
                                      const mpe::duration_t nominalDuration,
                                      mpe::ArticulationMap& noteArticulationMap) const
    {
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
