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

#include "dom/dynamic.h"
#include "dom/hairpin.h"
#include "dom/measure.h"
#include "dom/part.h"
#include "dom/playtechannotation.h"
#include "dom/repeatlist.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/spanner.h"

#include "utils/arrangementutils.h"
#include "utils/expressionutils.h"
#include "types/typesconv.h"

using namespace mu::engraving;
using namespace mu::mpe;

void PlaybackContext::update(const ID partId, const Score* score)
{
    TRACEFUNC;

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
    for (DynamicsMap& dynamics : m_dynamicsByVoice) {
        dynamics.clear();
    }

    m_playTechniquesMap.clear();
}

DynamicLevelLayers PlaybackContext::dynamicLevelLayers(const Score* score) const
{
    DynamicLevelLayers layers;

    for (voice_layer_idx_t voiceIdx = 0; voiceIdx < m_dynamicsByVoice.size(); ++voiceIdx) {
        DynamicLevelMap dynamicLevelMap;
        dynamicLevelMap.emplace(0, mpe::dynamicLevelFromType(mpe::DynamicType::Natural));

        const DynamicsMap& dynamics = m_dynamicsByVoice.at(voiceIdx);
        for (const auto& pair : dynamics) {
            dynamicLevelMap.insert_or_assign(timestampFromTicks(score, pair.first), pair.second);
        }

        dynamic_layer_idx_t layerIdx = makeDynamicLayerIndex(voiceIdx);
        layers.emplace(layerIdx, std::move(dynamicLevelMap));
    }

    return layers;
}

dynamic_level_t PlaybackContext::appliableDynamicLevel(const voice_idx_t voiceIdx, const int nominalPositionTick) const
{
    IF_ASSERT_FAILED(voiceIdx < m_dynamicsByVoice.size()) {
        return mpe::dynamicLevelFromType(mpe::DynamicType::Natural);
    }

    const DynamicsMap& dynamics = m_dynamicsByVoice.at(voiceIdx);

    auto it = findLessOrEqual(dynamics, nominalPositionTick);
    if (it == dynamics.cend()) {
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

dynamic_level_t PlaybackContext::nominalDynamicLevel(const voice_idx_t voiceIdx, const int positionTick) const
{
    IF_ASSERT_FAILED(voiceIdx < m_dynamicsByVoice.size()) {
        return mpe::dynamicLevelFromType(mpe::DynamicType::Natural);
    }

    const DynamicsMap& dynamics = m_dynamicsByVoice.at(voiceIdx);

    auto it = dynamics.find(positionTick);
    if (it == dynamics.cend()) {
        return mpe::dynamicLevelFromType(mpe::DynamicType::Natural);
    }

    return it->second;
}

void PlaybackContext::updateDynamicMap(const Dynamic* dynamic, const Segment* segment, const int segmentPositionTick)
{
    if (!dynamic->playDynamic()) {
        return;
    }

    const DynamicType type = dynamic->dynamicType();
    voice_idx_t voiceIdx = dynamic->voice();
    bool applyToAllVoices = dynamic->applyToAllVoices();

    if (isOrdinaryDynamicType(type)) {
        if (applyToAllVoices) {
            setDynamicLevelForAllVoices(segmentPositionTick, dynamicLevelFromType(type));
        } else {
            setDynamicLevel(voiceIdx, segmentPositionTick, dynamicLevelFromType(type));
        }

        return;
    }

    if (isSingleNoteDynamicType(type)) {
        mpe::dynamic_level_t prevDynamicLevel = appliableDynamicLevel(voiceIdx, segmentPositionTick);

        if (applyToAllVoices) {
            setDynamicLevelForAllVoices(segmentPositionTick, dynamicLevelFromType(type));
        } else {
            setDynamicLevel(voiceIdx, segmentPositionTick, dynamicLevelFromType(type));
        }

        applyDynamicToNextSegment(segment, voiceIdx, segmentPositionTick, prevDynamicLevel);
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

    if (applyToAllVoices) {
        for (const auto& pair : dynamicsCurve) {
            setDynamicLevelForAllVoices(segmentPositionTick + pair.first, levelFrom + pair.second);
        }
    } else {
        for (const auto& pair : dynamicsCurve) {
            setDynamicLevel(voiceIdx, segmentPositionTick + pair.first, levelFrom + pair.second);
        }
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

void PlaybackContext::applyDynamicToNextSegment(const Segment* currentSegment, const voice_idx_t voiceIdx, const int segmentPositionTick,
                                                const mpe::dynamic_level_t dynamicLevel)
{
    const Segment* nextSegment = currentSegment->next();
    if (!nextSegment) {
        return;
    }

    int tickPositionOffset = segmentPositionTick - currentSegment->tick().ticks();
    int nextSegmentPositionTick = nextSegment->tick().ticks() + tickPositionOffset;

    setDynamicLevel(voiceIdx, nextSegmentPositionTick, dynamicLevel);
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

        if (!spanner->isHairpin() || !toHairpin(spanner)->playHairpin()) {
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

        voice_idx_t voiceIdx = hairpin->voice();

        dynamic_level_t nominalLevelFrom
            = dynamicLevelFromType(dynamicTypeFrom, appliableDynamicLevel(voiceIdx, spannerFrom + tickPositionOffset));
        dynamic_level_t nominalLevelTo = dynamicLevelFromType(dynamicTypeTo, nominalDynamicLevel(voiceIdx, spannerTo + tickPositionOffset));

        dynamic_level_t overallDynamicRange = dynamicLevelRangeByTypes(dynamicTypeFrom,
                                                                       dynamicTypeTo,
                                                                       nominalLevelFrom,
                                                                       nominalLevelTo,
                                                                       hairpin->isCrescendo());

        std::map<int, int> dynamicsCurve = TConv::easingValueCurve(spannerDurationTicks,
                                                                   24 /*stepsCount*/,
                                                                   static_cast<int>(overallDynamicRange),
                                                                   hairpin->veloChangeMethod());

        if (hairpin->applyToAllVoices()) {
            for (const auto& pair : dynamicsCurve) {
                setDynamicLevelForAllVoices(spannerFrom + pair.first + tickPositionOffset, nominalLevelFrom + pair.second);
            }
        } else {
            for (const auto& pair : dynamicsCurve) {
                setDynamicLevel(voiceIdx, spannerFrom + pair.first + tickPositionOffset, nominalLevelFrom + pair.second);
            }
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

    if (m_dynamicsByVoice.empty()) {
        setDynamicLevelForAllVoices(0, mpe::dynamicLevelFromType(mpe::DynamicType::Natural));
    }
}

void PlaybackContext::setDynamicLevel(const voice_idx_t voiceIdx, const int positionTick, const dynamic_level_t lvl)
{
    m_dynamicsByVoice[voiceIdx].insert_or_assign(positionTick, lvl);
}

void PlaybackContext::setDynamicLevelForAllVoices(const int positionTick, const dynamic_level_t lvl)
{
    for (voice_idx_t idx = 0; idx < m_dynamicsByVoice.size(); ++idx) {
        setDynamicLevel(idx, positionTick, lvl);
    }
}
