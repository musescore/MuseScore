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
        if (m_dynamicsMap.empty()) {
            return mpe::dynamicLevelFromType(mpe::DynamicType::Natural);
        }

        if (m_dynamicsMap.size() == 1) {
            return m_dynamicsMap.begin()->second;
        }

        auto firstNotLess = m_dynamicsMap.lower_bound(nominalPositionTick);
        if (firstNotLess != m_dynamicsMap.cend()) {
            return firstNotLess->second;
        }

        auto firstLess = std::prev(firstNotLess);
        if (firstNotLess->first > nominalPositionTick || firstNotLess == m_dynamicsMap.cend()) {
            return firstLess->second;
        }

        return mpe::dynamicLevelFromType(mpe::DynamicType::Natural);
    }

    mpe::ArticulationType persistentArticulationType(const int nominalPositionTick) const
    {
        if (m_playTechniquesMap.empty()) {
            return mpe::ArticulationType::Standard;
        }

        if (m_playTechniquesMap.size() == 1) {
            return m_playTechniquesMap.begin()->second;
        }

        auto firstNotLess = m_playTechniquesMap.lower_bound(nominalPositionTick);
        if (firstNotLess != m_playTechniquesMap.cend()) {
            return firstNotLess->second;
        }

        auto firstLess = std::prev(firstNotLess);
        if (firstNotLess->first > nominalPositionTick || firstNotLess == m_playTechniquesMap.cend()) {
            return firstLess->second;
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

    void remove(const int tickFrom, const int tickTo)
    {
        removeDynamicData(tickFrom, tickTo);
        removePlayTechniqueData(tickFrom, tickTo);
    }

private:
    void updateDynamicMap(const Ms::Dynamic* dynamic, const Ms::Segment* segment, const int segmentPositionTick)
    {
        const Ms::DynamicType type = dynamic->dynamicType();
        if (isOrdinaryDynamicType(type)) {
            m_dynamicsMap[segmentPositionTick] = dynamicLevelFromType(type);
            return;
        }

        if (isSingleNoteDynamicType(type)) {
            m_dynamicsMap[segmentPositionTick] = dynamicLevelFromType(type);

            if (!segment->next()) {
                return;
            }

            mpe::dynamic_level_t prevDynamicLevel = previousDynamicLevel(segmentPositionTick);
            applyDynamicToNextSegment(segment, prevDynamicLevel);
            return;
        }

        const DynamicTransition& transition = dynamicTransitionFromType(type);
        m_dynamicsMap[segmentPositionTick] = dynamicLevelFromType(transition.from);
        applyDynamicToNextSegment(segment, dynamicLevelFromType(transition.to));
    }

    void updatePlayTechMap(const Ms::PlayTechAnnotation* annotation, const int segmentPositionTick)
    {
        const Ms::PlayingTechniqueType type = annotation->techniqueType();

        if (type == Ms::PlayingTechniqueType::Undefined) {
            return;
        }

        m_playTechniquesMap[segmentPositionTick] = articulationFromPlayTechType(type);
    }

    void applyDynamicToNextSegment(const Ms::Segment* currentSegment, const mpe::dynamic_level_t dynamicLevel)
    {
        if (!currentSegment->next()) {
            return;
        }

        int nextSegmentPositionTick = currentSegment->next()->tick().ticks();
        m_dynamicsMap[nextSegmentPositionTick] = dynamicLevel;
    }

    mpe::dynamic_level_t previousDynamicLevel(const int segmentPositionTick) const
    {
        auto search = m_dynamicsMap.find(segmentPositionTick);

        if (search == m_dynamicsMap.cend()) {
            return mpe::dynamicLevelFromType(mpe::DynamicType::Natural);
        }

        if (search == m_dynamicsMap.begin()) {
            return search->second;
        }

        std::advance(search, -1);

        return search->second;
    }

    void removeDynamicData(const int from, const int to)
    {
        auto lowerBound = m_dynamicsMap.lower_bound(from);
        auto upperBound = m_dynamicsMap.upper_bound(to);

        for (auto it = lowerBound; it != upperBound;) {
            it = m_dynamicsMap.erase(it);
        }
    }

    void removePlayTechniqueData(const int from, const int to)
    {
        auto lowerBound = m_playTechniquesMap.lower_bound(from);
        auto upperBound = m_playTechniquesMap.upper_bound(to);

        for (auto it = lowerBound; it != upperBound;) {
            it = m_playTechniquesMap.erase(it);
        }
    }

    DynamicMap m_dynamicsMap;
    PlayTechniquesMap m_playTechniquesMap;
};
}

#endif // MU_ENGRAVING_PLAYBACKCONTEXT_H
