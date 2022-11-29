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

#include "libmscore/dynamic.h"
#include "libmscore/hairpin.h"
#include "libmscore/measure.h"
#include "libmscore/part.h"
#include "libmscore/playtechannotation.h"
#include "libmscore/repeatlist.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/spanner.h"

#include "utils/arrangementutils.h"
#include "utils/expressionutils.h"
#include "types/typesconv.h"

using namespace mu::engraving;
using namespace mu::mpe;

dynamic_level_t PlaybackContext::appliableDynamicLevel(const int nominalPositionTick) const
{
    auto it = findLessOrEqual(m_dynamicsMap, nominalPositionTick);
    if (it == m_dynamicsMap.cend()) {
        return mpe::dynamicLevelFromType(mpe::DynamicType::Natural);
    }

    return it->second;
}

ArticulationType PlaybackContext::persistentArticulationType(const int nominalPositionTick) const
{
    auto it = findLessOrEqual(m_playTechniquesMap, nominalPositionTick);
    if (it == m_playTechniquesMap.cend()) {
        return mpe::ArticulationType::Standard;
    }

    return it->second;
}

void PlaybackContext::update(const ID partId, const Score* score)
{
    for (const RepeatSegment* repeatSegment : score->repeatList()) {
        int tickPositionOffset = repeatSegment->utick - repeatSegment->tick;

        for (const Measure* measure : repeatSegment->measureList()) {
            for (Segment* segment = measure->first(); segment; segment = segment->next()) {
                int segmentStartTick = segment->tick().ticks() + tickPositionOffset;

                handleAnnotations(partId, segment, segmentStartTick);
            }
        }

        handleSpanners(partId, score, repeatSegment->tick,
                       repeatSegment->tick + repeatSegment->len(), tickPositionOffset);
    }
}

void PlaybackContext::clear()
{
    m_dynamicsMap.clear();
    m_playTechniquesMap.clear();
}

DynamicLevelMap PlaybackContext::dynamicLevelMap(const Score* score) const
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

void PlaybackContext::updateDynamicMap(const Dynamic* dynamic, const Segment* segment, const int segmentPositionTick)
{
    const DynamicType type = dynamic->dynamicType();
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

        applyDynamicToNextSegment(segment, segmentPositionTick, prevDynamicLevel);
        return;
    }

    const DynamicTransition& transition = dynamicTransitionFromType(type);
    const int transitionDuration = dynamic->velocityChangeLength().ticks();

    dynamic_level_t levelFrom = dynamicLevelFromType(transition.from);
    dynamic_level_t levelTo = dynamicLevelFromType(transition.to);

    dynamic_level_t range = levelTo - levelFrom;

    std::map<int, int> dynamicsCurve = TConv::easingValueCurve(transitionDuration,
                                                               6 /*stepsCount*/,
                                                               static_cast<int>(range),
                                                               ChangeMethod::NORMAL);

    for (const auto& pair : dynamicsCurve) {
        m_dynamicsMap[segmentPositionTick + pair.first] = levelFrom + pair.second;
    }
}

void PlaybackContext::updatePlayTechMap(const PlayTechAnnotation* annotation, const int segmentPositionTick)
{
    const PlayingTechniqueType type = annotation->techniqueType();

    if (type == PlayingTechniqueType::Undefined) {
        return;
    }

    m_playTechniquesMap[segmentPositionTick] = articulationFromPlayTechType(type);
}

void PlaybackContext::applyDynamicToNextSegment(const Segment* currentSegment, const int segmentPositionTick,
                                                const mpe::dynamic_level_t dynamicLevel)
{
    if (!currentSegment->next()) {
        return;
    }

    const int tickPositionOffset = segmentPositionTick - currentSegment->tick().ticks();

    int nextSegmentPositionTick = currentSegment->next()->tick().ticks() + tickPositionOffset;
    m_dynamicsMap[nextSegmentPositionTick] = dynamicLevel;
}

void PlaybackContext::handleSpanners(const ID partId, const Score* score, const int segmentStartTick, const int segmentEndTick,
                                     const int tickPositionOffset)
{
    const SpannerMap& spannerMap = score->spannerMap();
    if (spannerMap.empty()) {
        return;
    }

    auto intervals = spannerMap.findOverlapping(segmentStartTick, segmentEndTick);
    for (const auto& interval : intervals) {
        const Spanner* spanner = interval.value;

        if (!spanner->isHairpin()) {
            continue;
        }

        if (spanner->part()->id() != partId.toUint64()) {
            continue;
        }

        int spannerFrom = spanner->tick().ticks();
        int spannerTo = spannerFrom + std::abs(spanner->ticks().ticks());

        int spannerDurationTicks = spannerTo - spannerFrom;

        if (spannerDurationTicks <= 0) {
            continue;
        }

        const Hairpin* hairpin = toHairpin(spanner);

        {
            Dynamic* startDynamic
                = toDynamic(hairpin->startSegment()->findAnnotation(ElementType::DYNAMIC, hairpin->track(), hairpin->track()));
            if (startDynamic) {
                if (startDynamic->dynamicType() != DynamicType::OTHER
                    && !isOrdinaryDynamicType(startDynamic->dynamicType())
                    && !isSingleNoteDynamicType(startDynamic->dynamicType())) {
                    // The hairpin starts with a transition dynamic; we should start the hairpin after the transition is complete
                    // This solution should be replaced once we have better infrastructure to see relations between Dynamics and Hairpins.
                    spannerFrom += startDynamic->velocityChangeLength().ticks();

                    spannerDurationTicks = spannerTo - spannerFrom;

                    if (spannerDurationTicks <= 0) {
                        continue;
                    }
                }
            }
        }

        DynamicType dynamicTypeFrom = hairpin->dynamicTypeFrom();
        DynamicType dynamicTypeTo = hairpin->dynamicTypeTo();

        dynamic_level_t nominalLevelFrom = dynamicLevelFromType(dynamicTypeFrom, appliableDynamicLevel(spannerFrom + tickPositionOffset));
        dynamic_level_t nominalLevelTo = dynamicLevelFromType(dynamicTypeTo, nominalDynamicLevel(spannerTo + tickPositionOffset));

        dynamic_level_t overallDynamicRange = dynamicLevelRangeByTypes(dynamicTypeFrom,
                                                                       dynamicTypeTo,
                                                                       nominalLevelFrom,
                                                                       nominalLevelTo,
                                                                       hairpin->isCrescendo());

        std::map<int, int> dynamicsCurve = TConv::easingValueCurve(spannerDurationTicks,
                                                                   24 /*stepsCount*/,
                                                                   static_cast<int>(overallDynamicRange),
                                                                   hairpin->veloChangeMethod());

        for (const auto& pair : dynamicsCurve) {
            m_dynamicsMap.insert_or_assign(spannerFrom + pair.first + tickPositionOffset, nominalLevelFrom + pair.second);
        }
    }
}

void PlaybackContext::handleAnnotations(const ID partId, const Segment* segment, const int segmentPositionTick)
{
    for (const EngravingItem* annotation : segment->annotations()) {
        if (!annotation || !annotation->part()) {
            continue;
        }

        if (annotation->part()->id() != partId.toUint64()) {
            continue;
        }

        if (annotation->isDynamic()) {
            updateDynamicMap(toDynamic(annotation), segment, segmentPositionTick);
            continue;
        }

        if (annotation->isPlayTechAnnotation()) {
            updatePlayTechMap(toPlayTechAnnotation(annotation), segmentPositionTick);
            continue;
        }
    }

    if (m_dynamicsMap.empty()) {
        m_dynamicsMap.emplace(0, mpe::dynamicLevelFromType(mpe::DynamicType::Natural));
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
