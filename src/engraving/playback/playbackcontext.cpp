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

#include "playbackcontext.h"

#include "libmscore/score.h"
#include "libmscore/spanner.h"
#include "libmscore/measure.h"
#include "libmscore/hairpin.h"
#include "libmscore/spannermap.h"
#include "libmscore/repeatlist.h"
#include "libmscore/segment.h"
#include "libmscore/dynamic.h"
#include "libmscore/playtechannotation.h"

#include "utils/arrangementutils.h"
#include "utils/expressionutils.h"
#include "types/typesconv.h"

using namespace mu::engraving;
using namespace mu::mpe;

dynamic_level_t PlaybackContext::appliableDynamicLevel(const int nominalPositionTick) const
{
    for (auto it = m_dynamicsMap.rbegin(); it != m_dynamicsMap.rend(); ++it) {
        if (it->first <= nominalPositionTick) {
            return it->second;
        }
    }

    return mpe::dynamicLevelFromType(mpe::DynamicType::Natural);
}

ArticulationType PlaybackContext::persistentArticulationType(const int nominalPositionTick) const
{
    for (auto it = m_playTechniquesMap.rbegin(); it != m_playTechniquesMap.rend(); ++it) {
        if (it->first <= nominalPositionTick) {
            return it->second;
        }
    }

    return mpe::ArticulationType::Standard;
}

void PlaybackContext::update(const ID partId, const mu::engraving::Score* score)
{
    for (const mu::engraving::RepeatSegment* repeatSegment : score->repeatList()) {
        int tickPositionOffset = repeatSegment->utick - repeatSegment->tick;

        for (const mu::engraving::Measure* measure : repeatSegment->measureList()) {
            for (mu::engraving::Segment* segment = measure->first(); segment; segment = segment->next()) {
                int segmentStartTick = segment->tick().ticks() + tickPositionOffset;

                handleAnnotations(partId, segment, segmentStartTick);
            }
        }
    }

    handleSpanners(partId, score);
}

void PlaybackContext::clear()
{
    m_dynamicsMap.clear();
    m_playTechniquesMap.clear();
}

DynamicLevelMap PlaybackContext::dynamicLevelMap(const mu::engraving::Score* score) const
{
    DynamicLevelMap result;

    for (const auto& pair : m_dynamicsMap) {
        result.insert_or_assign(timestampFromTicks(score, pair.first), pair.second);
    }

    if (result.empty()) {
        result.emplace(0, mpe::dynamicLevelFromType(mpe::DynamicType::Natural));
    }

    return result;
}

dynamic_level_t PlaybackContext::nominalDynamicLevel(const int positionTick) const
{
    auto search = m_dynamicsMap.find(positionTick);

    if (search == m_dynamicsMap.cend()) {
        return mpe::dynamicLevelFromType(mpe::DynamicType::Natural);
    }

    return search->second;
}

void PlaybackContext::updateDynamicMap(const mu::engraving::Dynamic* dynamic, const mu::engraving::Segment* segment,
                                       const int segmentPositionTick)
{
    const mu::engraving::DynamicType type = dynamic->dynamicType();
    if (isOrdinaryDynamicType(type)) {
        m_dynamicsMap[segmentPositionTick] = dynamicLevelFromType(type);
        return;
    }

    if (isSingleNoteDynamicType(type)) {
        mpe::dynamic_level_t prevDynamicLevel = appliableDynamicLevel(segmentPositionTick);

        m_dynamicsMap[segmentPositionTick] = dynamicLevelFromType(type);

        if (!segment->next()) {
            return;
        }

        applyDynamicToNextSegment(segment, prevDynamicLevel);
        return;
    }

    const DynamicTransition& transition = dynamicTransitionFromType(type);
    m_dynamicsMap[segmentPositionTick] = dynamicLevelFromType(transition.from);
    applyDynamicToNextSegment(segment, dynamicLevelFromType(transition.to));
}

void PlaybackContext::updatePlayTechMap(const mu::engraving::PlayTechAnnotation* annotation, const int segmentPositionTick)
{
    const mu::engraving::PlayingTechniqueType type = annotation->techniqueType();

    if (type == mu::engraving::PlayingTechniqueType::Undefined) {
        return;
    }

    m_playTechniquesMap[segmentPositionTick] = articulationFromPlayTechType(type);
}

void PlaybackContext::applyDynamicToNextSegment(const mu::engraving::Segment* currentSegment, const mpe::dynamic_level_t dynamicLevel)
{
    if (!currentSegment->next()) {
        return;
    }

    int nextSegmentPositionTick = currentSegment->next()->tick().ticks();
    m_dynamicsMap[nextSegmentPositionTick] = dynamicLevel;
}

void PlaybackContext::handleSpanners(const ID partId, const mu::engraving::Score* score)
{
    for (const auto& pair : score->spanner()) {
        const mu::engraving::Spanner* spanner = pair.second;

        if (!spanner->isHairpin()) {
            continue;
        }

        if (spanner->part()->id() != partId.toUint64()) {
            continue;
        }

        int spannerFrom = score->repeatList().tick2utick(spanner->tick().ticks());
        int spannerTo = spannerFrom + std::abs(spanner->ticks().ticks());

        int spannerDurationTicks = spannerTo - spannerFrom;

        if (spannerDurationTicks == 0) {
            continue;
        }

        const mu::engraving::Hairpin* hairpin = mu::engraving::toHairpin(spanner);

        mu::engraving::DynamicType dynamicTypeFrom = hairpin->dynamicTypeFrom();
        mu::engraving::DynamicType dynamicTypeTo = hairpin->dynamicTypeTo();

        dynamic_level_t nominalLevelFrom = dynamicLevelFromType(dynamicTypeFrom, appliableDynamicLevel(spannerFrom));
        dynamic_level_t nominalLevelTo = dynamicLevelFromType(dynamicTypeTo, nominalDynamicLevel(spannerTo));

        dynamic_level_t overallDynamicRange = dynamicLevelRangeByTypes(dynamicTypeFrom,
                                                                       dynamicTypeTo,
                                                                       nominalLevelFrom,
                                                                       nominalLevelTo,
                                                                       hairpin->isCrescendo());

        std::map<int, int> dynamicsCurve = TConv::easingValueCurve(spannerDurationTicks,
                                                                   24,
                                                                   static_cast<int>(overallDynamicRange),
                                                                   hairpin->veloChangeMethod());

        for (const auto& pair : dynamicsCurve) {
            m_dynamicsMap.insert_or_assign(spannerFrom + pair.first, nominalLevelFrom + pair.second);
        }
    }
}

void PlaybackContext::handleAnnotations(const ID partId, const mu::engraving::Segment* segment, const int segmentPositionTick)
{
    for (const mu::engraving::EngravingItem* annotation : segment->annotations()) {
        if (!annotation || !annotation->part()) {
            continue;
        }

        if (annotation->part()->id() != partId.toUint64()) {
            continue;
        }

        if (annotation->isDynamic()) {
            updateDynamicMap(mu::engraving::toDynamic(annotation), segment, segmentPositionTick);
            return;
        }

        if (annotation->isPlayTechAnnotation()) {
            updatePlayTechMap(mu::engraving::toPlayTechAnnotation(annotation), segmentPositionTick);
            return;
        }
    }
}

void PlaybackContext::removeDynamicData(const int from, const int to)
{
    auto lowerBound = m_dynamicsMap.lower_bound(from);
    auto upperBound = m_dynamicsMap.upper_bound(to);

    for (auto it = lowerBound; it != upperBound;) {
        it = m_dynamicsMap.erase(it);
    }
}

void PlaybackContext::removePlayTechniqueData(const int from, const int to)
{
    auto lowerBound = m_playTechniquesMap.lower_bound(from);
    auto upperBound = m_playTechniquesMap.upper_bound(to);

    for (auto it = lowerBound; it != upperBound;) {
        it = m_playTechniquesMap.erase(it);
    }
}
