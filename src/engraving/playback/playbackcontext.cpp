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

#include "playbackcontext.h"

#include "dom/dynamic.h"
#include "dom/hairpin.h"
#include "dom/measure.h"
#include "dom/part.h"
#include "dom/playtechannotation.h"
#include "dom/stafftext.h"
#include "dom/soundflag.h"
#include "dom/repeatlist.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/spanner.h"
#include "dom/measurerepeat.h"

#include "utils/arrangementutils.h"
#include "utils/expressionutils.h"
#include "types/typesconv.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::mpe;

static bool soundFlagPlayable(const SoundFlag* flag)
{
    if (flag && flag->play()) {
        return !flag->soundPresets().empty() || !flag->playingTechnique().empty();
    }

    return false;
}

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

PlaybackParamMap PlaybackContext::playbackParamMap(const Score* score, const int nominalPositionTick, const staff_idx_t staffIdx) const
{
    auto it = muse::findLessOrEqual(m_playbackParamMap, nominalPositionTick);
    if (it == m_playbackParamMap.end()) {
        return {};
    }

    for (; it->first <= nominalPositionTick; it = std::prev(it)) {
        PlaybackParamList params;

        for (const PlaybackParam& param : it->second) {
            if (param.staffLayerIndex == staffIdx) {
                params.push_back(param);
            }
        }

        if (!params.empty()) {
            mpe::PlaybackParamMap result;
            result.insert_or_assign(timestampFromTicks(score, it->first), std::move(params));
            return result;
        }

        if (it == m_playbackParamMap.begin()) {
            return {};
        }
    }

    return {};
}

PlaybackParamMap PlaybackContext::playbackParamMap(const Score* score) const
{
    mpe::PlaybackParamMap result;

    for (const auto& pair : m_playbackParamMap) {
        result.insert_or_assign(timestampFromTicks(score, pair.first), pair.second);
    }

    return result;
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

void PlaybackContext::update(const ID partId, const Score* score)
{
    for (const RepeatSegment* repeatSegment : score->repeatList()) {
        std::vector<const MeasureRepeat*> measureRepeats;
        int tickPositionOffset = repeatSegment->utick - repeatSegment->tick;

        for (const Measure* measure : repeatSegment->measureList()) {
            for (Segment* segment = measure->first(); segment; segment = segment->next()) {
                int segmentStartTick = segment->tick().ticks() + tickPositionOffset;

                for (const EngravingItem* item : segment->elist()) {
                    if (item && item->isMeasureRepeat()) {
                        measureRepeats.push_back(toMeasureRepeat(item));
                    }
                }

                handleAnnotations(partId, score, segment, segmentStartTick);
            }
        }

        handleSpanners(partId, score, repeatSegment->tick,
                       repeatSegment->tick + repeatSegment->len(), tickPositionOffset);

        handleMeasureRepeats(measureRepeats, tickPositionOffset);
    }
}

void PlaybackContext::clear()
{
    m_dynamicsMap.clear();
    m_playTechniquesMap.clear();
    m_playbackParamMap.clear();
}

bool PlaybackContext::hasSoundFlags() const
{
    for (const auto& pair : m_playbackParamMap) {
        for (const mpe::PlaybackParam& param : pair.second) {
            if (param.code == mpe::SOUND_PRESET_PARAM_CODE
                || param.code == mpe::PLAY_TECHNIQUE_PARAM_CODE) {
                return true;
            }
        }
    }

    return false;
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
    if (!dynamic->playDynamic()) {
        return;
    }
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

void PlaybackContext::updatePlayTechMap(const ID partId, const Score* score, const PlayTechAnnotation* annotation,
                                        const int segmentPositionTick)
{
    const PlayingTechniqueType type = annotation->techniqueType();

    if (type == PlayingTechniqueType::Undefined) {
        return;
    }

    m_playTechniquesMap[segmentPositionTick] = articulationFromPlayTechType(type);

    bool cancelPlayTechniques = type == PlayingTechniqueType::Natural || type == PlayingTechniqueType::Open;

    if (cancelPlayTechniques && !m_playbackParamMap.empty()) {
        const Part* part = score->partById(partId);
        IF_ASSERT_FAILED(part && !part->staves().empty()) {
            return;
        }

        mpe::staff_layer_idx_t startIdx = static_cast <staff_layer_idx_t>(part->staves().front()->idx());
        mpe::staff_layer_idx_t endIdx = static_cast <staff_layer_idx_t>(startIdx + part->nstaves());

        for (mpe::staff_layer_idx_t idx = startIdx; idx < endIdx; ++idx) {
            PlaybackParam ordTechnique { mpe::PLAY_TECHNIQUE_PARAM_CODE, Val(mpe::ORDINARY_PLAYING_TECHNIQUE_CODE), idx };
            m_playbackParamMap[segmentPositionTick].push_back(ordTechnique);
        }
    }
}

void PlaybackContext::updatePlaybackParamMap(const ID partId, const Score* score, const SoundFlagMap& flagsOnSegment,
                                             const int segmentPositionTick)
{
    mpe::PlaybackParamList params;

    auto addParams = [&params](const SoundFlag* flag, staff_layer_idx_t idx) {
        for (const String& presetCode : flag->soundPresets()) {
            params.emplace_back(mpe::PlaybackParam { mpe::SOUND_PRESET_PARAM_CODE, Val(presetCode.toStdString()), idx });
        }

        if (!flag->playingTechnique().empty()) {
            params.emplace_back(mpe::PlaybackParam { mpe::PLAY_TECHNIQUE_PARAM_CODE, Val(flag->playingTechnique().toStdString()), idx });
        }
    };

    bool multipleFlagsOnSegment = flagsOnSegment.size() > 1;

    for (const auto& pair : flagsOnSegment) {
        const SoundFlag* flag = pair.second;
        staff_idx_t staffIdx = flag->staffIdx();

        if (flag->applyToAllStaves()) {
            const Part* part = score->partById(partId);
            IF_ASSERT_FAILED(part && !part->staves().empty()) {
                return;
            }

            staff_idx_t startIdx = part->staves().front()->idx();
            staff_idx_t endIdx = startIdx + part->nstaves();

            for (staff_idx_t idx = startIdx; idx < endIdx; ++idx) {
                if (multipleFlagsOnSegment) {
                    if (idx != staffIdx && muse::contains(flagsOnSegment, idx)) {
                        continue;
                    }
                }

                addParams(flag, static_cast<staff_layer_idx_t>(idx));
            }
        } else {
            addParams(flag, static_cast<staff_layer_idx_t>(staffIdx));
        }

        if (flag->playingTechnique().toStdString() == mpe::ORDINARY_PLAYING_TECHNIQUE_CODE) {
            m_playTechniquesMap[segmentPositionTick] = mpe::ArticulationType::Standard;
        }
    }

    IF_ASSERT_FAILED(!params.empty()) {
        return;
    }

    m_playbackParamMap.emplace(segmentPositionTick, std::move(params));
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

    auto intervals = spannerMap.findOverlapping(segmentStartTick, segmentEndTick - 1);
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
            Segment* startSegment = hairpin->startSegment();
            Dynamic* startDynamic = startSegment
                                    ? toDynamic(startSegment->findAnnotation(ElementType::DYNAMIC, hairpin->track(), hairpin->track()))
                                    : nullptr;
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

        // First, check if hairpin has its own start/end dynamics in the begin/end text
        const DynamicType dynamicTypeFrom = hairpin->dynamicTypeFrom();
        const DynamicType dynamicTypeTo = hairpin->dynamicTypeTo();

        // If it doesn't:
        // - for the start level, use the currently-applicable level at the start tick of the hairpin
        // - for the end level, check if there is a dynamic marking at the end of the hairpin
        const dynamic_level_t levelFrom = dynamicLevelFromType(dynamicTypeFrom, appliableDynamicLevel(spannerFrom + tickPositionOffset));
        const dynamic_level_t nominalLevelTo = dynamicLevelFromType(dynamicTypeTo, nominalDynamicLevel(spannerTo + tickPositionOffset));

        // If there is an end dynamic marking, check if it matches the 'direction' of the hairpin (cresc. vs decresc.)
        const bool isCrescendo = hairpin->isCrescendo();
        const bool hasNominalLevelTo = nominalLevelTo != mpe::dynamicLevelFromType(mpe::DynamicType::Natural);
        const bool useNominalLevelTo = hasNominalLevelTo && (isCrescendo
                                                             ? nominalLevelTo > levelFrom
                                                             : nominalLevelTo < levelFrom);

        const dynamic_level_t levelTo = useNominalLevelTo
                                        ? nominalLevelTo
                                        : levelFrom + (isCrescendo ? mpe::DYNAMIC_LEVEL_STEP : -mpe::DYNAMIC_LEVEL_STEP);

        std::map<int, int> dynamicsCurve = TConv::easingValueCurve(spannerDurationTicks,
                                                                   24 /*stepsCount*/,
                                                                   static_cast<int>(levelTo - levelFrom),
                                                                   hairpin->veloChangeMethod());

        for (const auto& pair : dynamicsCurve) {
            m_dynamicsMap.insert_or_assign(spannerFrom + pair.first + tickPositionOffset, levelFrom + pair.second);
        }

        if (hasNominalLevelTo && !useNominalLevelTo) {
            // If there is a dynamic at the end of the hairpin that we couldn't use because it didn't match the direction of the hairpin,
            // insert that dynamic directly after the hairpin
            m_dynamicsMap.insert_or_assign(spannerTo + tickPositionOffset, nominalLevelTo);
        }
    }
}

void PlaybackContext::handleAnnotations(const ID partId, const Score* score, const Segment* segment, const int segmentPositionTick)
{
    SoundFlagMap soundFlags;

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
            updatePlayTechMap(partId, score, toPlayTechAnnotation(annotation), segmentPositionTick);
            continue;
        }

        if (annotation->isStaffText()) {
            if (const SoundFlag* flag = toStaffText(annotation)->soundFlag()) {
                if (soundFlagPlayable(flag)) {
                    soundFlags.emplace(flag->staffIdx(), flag);
                }
            }
        }
    }

    if (!soundFlags.empty()) {
        updatePlaybackParamMap(partId, score, soundFlags, segmentPositionTick);
    }

    if (m_dynamicsMap.empty()) {
        m_dynamicsMap.emplace(0, mpe::dynamicLevelFromType(mpe::DynamicType::Natural));
    }
}

void PlaybackContext::handleMeasureRepeats(const std::vector<const MeasureRepeat*>& measureRepeats, const int tickPositionOffset)
{
    for (const MeasureRepeat* mr : measureRepeats) {
        const Measure* currMeasure = mr->firstMeasureOfGroup();
        if (!currMeasure) {
            continue;
        }

        const Measure* referringMeasure = mr->referringMeasure(currMeasure);
        if (!referringMeasure) {
            continue;
        }

        int currentMeasureTick = currMeasure->tick().ticks();
        int referringMeasureTick = referringMeasure->tick().ticks();
        int newItemsOffsetTick = currentMeasureTick - referringMeasureTick;

        for (int num = 0; num < mr->numMeasures(); ++num) {
            int startTick = referringMeasure->tick().ticks() + tickPositionOffset;
            int endTick = referringMeasure->endTick().ticks() + tickPositionOffset;

            copyDynamicsInRange(startTick, endTick, newItemsOffsetTick);
            copyPlaybackParamsInRange(startTick, endTick, newItemsOffsetTick);
            copyPlayTechniquesInRange(startTick, endTick, newItemsOffsetTick);

            currMeasure = currMeasure->nextMeasure();
            if (!currMeasure) {
                break;
            }

            referringMeasure = mr->referringMeasure(currMeasure);
            if (!referringMeasure) {
                break;
            }
        }
    }
}

void PlaybackContext::copyDynamicsInRange(const int rangeStartTick, const int rangeEndTick, const int newDynamicsOffsetTick)
{
    auto startIt = m_dynamicsMap.lower_bound(rangeStartTick);
    if (startIt == m_dynamicsMap.end()) {
        return;
    }

    auto endIt = m_dynamicsMap.lower_bound(rangeEndTick);

    DynamicMap newDynamics;
    for (auto it = startIt; it != endIt; ++it) {
        int tick = it->first + newDynamicsOffsetTick;
        newDynamics.insert_or_assign(tick, it->second);
    }

    m_dynamicsMap.merge(std::move(newDynamics));
}

void PlaybackContext::copyPlaybackParamsInRange(const int rangeStartTick, const int rangeEndTick, const int newParamsOffsetTick)
{
    auto startIt = m_playbackParamMap.lower_bound(rangeStartTick);
    if (startIt == m_playbackParamMap.end()) {
        return;
    }

    auto endIt = m_playbackParamMap.lower_bound(rangeEndTick);

    ParamMap newParams;
    for (auto it = startIt; it != endIt; ++it) {
        int tick = it->first + newParamsOffsetTick;
        newParams.insert_or_assign(tick, it->second);
    }

    m_playbackParamMap.merge(std::move(newParams));
}

void PlaybackContext::copyPlayTechniquesInRange(const int rangeStartTick, const int rangeEndTick, const int newPlayTechOffsetTick)
{
    auto startIt = m_playTechniquesMap.lower_bound(rangeStartTick);
    if (startIt == m_playTechniquesMap.end()) {
        return;
    }

    auto endIt = m_playTechniquesMap.lower_bound(rangeEndTick);

    PlayTechniquesMap newPlayTechniques;
    for (auto it = startIt; it != endIt; ++it) {
        int tick = it->first + newPlayTechOffsetTick;
        newPlayTechniques.insert_or_assign(tick, it->second);
    }

    m_playTechniquesMap.merge(std::move(newPlayTechniques));
}
