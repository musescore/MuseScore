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

#ifndef MU_ENGRAVING_PLAYBACKCONTEXT_H
#define MU_ENGRAVING_PLAYBACKCONTEXT_H

#include "mpe/mpetypes.h"

#include "libmscore/segment.h"
#include "libmscore/dynamic.h"
#include "libmscore/playtechannotation.h"

#include "utils/expressionutils.h"

namespace mu::engraving {
using DynamicMap = std::map<int /*nominalPositionTick*/, mpe::dynamic_level_t>;
using PlayTechniquesMap = std::map<int /*nominalPositionTick*/, mpe::ArticulationType>;

struct PlaybackContext {
    mpe::dynamic_level_t nominalDynamicLevel(const int nominalPositionTick) const
    {
        if (m_dynamicsMap.size() == 1) {
            return m_dynamicsMap.begin()->second;
        }

        auto it = m_dynamicsMap.lower_bound(nominalPositionTick);
        if (it != m_dynamicsMap.cend()) {
            return it->second;
        }

        return mpe::dynamicLevelFromType(mpe::DynamicType::Natural);
    }

    mpe::ArticulationType persistentArticulationType(const int nominalPositionTick) const
    {
        if (m_playTechniquesMap.size() == 1) {
            return m_playTechniquesMap.begin()->second;
        }

        auto it = m_playTechniquesMap.lower_bound(nominalPositionTick);
        if (it != m_playTechniquesMap.cend()) {
            return it->second;
        }

        return mpe::ArticulationType::Standard;
    }

    void update(const Ms::Segment* segment, const int segmentPositionTick)
    {
        for (const Ms::EngravingItem* annotation : segment->annotations()) {
            if (!annotation) {
                continue;
            }

            if (annotation->isDynamic()) {
                updateDynamicMap(Ms::toDynamic(annotation), segment, segmentPositionTick);
                return;
            }

            if (annotation->isPlayTechAnnotation()) {
                updatePlayTechMap(Ms::toPlayTechAnnotation(annotation), segmentPositionTick);
                return;
            }
        }
    }

private:
    void updateDynamicMap(const Ms::Dynamic* dynamic, const Ms::Segment* segment, const int segmentPositionTick)
    {
        const Ms::DynamicType type = dynamic->dynamicType();
        if (isOrdinaryDynamicType(type)) {
            m_dynamicsMap[segmentPositionTick] = dynamicLevelFromType(type);
            return;
        }

        const DynamicTransition& transition = dynamicTransitionFromType(type);
        m_dynamicsMap[segmentPositionTick] = dynamicLevelFromType(transition.from);

        if (!segment->next()) {
            return;
        }

        int nextSegmentPositionTick = segment->next()->tick().ticks();
        m_dynamicsMap[nextSegmentPositionTick] = dynamicLevelFromType(transition.to);
    }

    void updatePlayTechMap(const Ms::PlayTechAnnotation* annotation, const int segmentPositionTick)
    {
        const Ms::PlayingTechniqueType type = annotation->techniqueType();

        if (type == Ms::PlayingTechniqueType::Undefined) {
            return;
        }

        m_playTechniquesMap[segmentPositionTick] = articulationFromPlayTechType(type);
    }

    DynamicMap m_dynamicsMap;
    PlayTechniquesMap m_playTechniquesMap;
};
}

#endif // MU_ENGRAVING_PLAYBACKCONTEXT_H
