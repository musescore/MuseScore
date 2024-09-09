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
#include "dom/lyrics.h"
#include "dom/sticking.h"

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

static mu::engraving::DynamicType findNominalStartDynamicType(const Hairpin* hairpin)
{
    return hairpin->dynamicTypeFrom();
}

static dynamic_level_t findNominalStartDynamicLevel(const Hairpin* hairpin)
{
    mu::engraving::DynamicType type = findNominalStartDynamicType(hairpin);

    if (isOrdinaryDynamicType(type)) {
        return dynamicLevelFromOrdinaryType(type);
    }

    if (isSingleNoteDynamicType(type)) {
        return dynamicLevelFromSingleNoteType(type);
    }

    if (isCompoundDynamicType(type)) {
        const CompoundDynamic& transition = compoundDynamicFromType(type);
        return dynamicLevelFromOrdinaryType(transition.to);
    }

    return NATURAL_DYNAMIC_LEVEL;
}

static mu::engraving::DynamicType findNominalEndDynamicType(const Hairpin* hairpin)
{
    const mu::engraving::DynamicType textDynamicType = hairpin->dynamicTypeTo();
    if (textDynamicType != mu::engraving::DynamicType::OTHER) {
        return textDynamicType;
    }

    const LineSegment* seg = hairpin->backSegment();
    if (!seg) {
        return mu::engraving::DynamicType::OTHER;
    }

    // Optimization: first check if there is a cached dynamic
    const EngravingItem* snappedItem = seg->ldata()->itemSnappedAfter();
    if (!snappedItem || !snappedItem->isDynamic()) {
        snappedItem = toHairpinSegment(seg)->findElementToSnapAfter(false /*ignoreInvisible*/);
        if (!snappedItem || !snappedItem->isDynamic()) {
            return mu::engraving::DynamicType::OTHER;
        }
    }

    return toDynamic(snappedItem)->dynamicType();
}

static dynamic_level_t findNominalEndDynamicLevel(const Hairpin* hairpin)
{
    mu::engraving::DynamicType type = findNominalEndDynamicType(hairpin);

    if (isOrdinaryDynamicType(type)) {
        return dynamicLevelFromOrdinaryType(type);
    }

    if (isSingleNoteDynamicType(type)) {
        return dynamicLevelFromSingleNoteType(type);
    }

    if (isCompoundDynamicType(type)) {
        const CompoundDynamic& transition = compoundDynamicFromType(type);
        return dynamicLevelFromOrdinaryType(transition.from);
    }

    return NATURAL_DYNAMIC_LEVEL;
}

dynamic_level_t PlaybackContext::appliableDynamicLevel(const track_idx_t trackIdx, const int nominalPositionTick) const
{
    auto dynamicsIt = m_dynamicsByTrack.find(trackIdx);
    if (dynamicsIt == m_dynamicsByTrack.end()) {
        return NATURAL_DYNAMIC_LEVEL;
    }

    const DynamicMap& dynamics = dynamicsIt->second;
    auto it = muse::findLessOrEqual(dynamics, nominalPositionTick);
    if (it == dynamics.end()) {
        return NATURAL_DYNAMIC_LEVEL;
    }

    return it->second.level;
}

ArticulationType PlaybackContext::persistentArticulationType(const int nominalPositionTick) const
{
    auto it = findLessOrEqual(m_playTechniquesMap, nominalPositionTick);
    if (it == m_playTechniquesMap.cend()) {
        return mpe::ArticulationType::Standard;
    }

    return it->second;
}

PlaybackParamList PlaybackContext::playbackParams(const track_idx_t trackIdx, const int nominalPositionTick) const
{
    PlaybackParamList result;

    auto addParams = [&result](const PlaybackParamList& params, bool startAtNominalTick) {
        if (params.empty()) {
            return;
        }

        result.insert(result.end(), params.begin(), params.end());

        bool persistent = !startAtNominalTick;
        for (size_t i = result.size() - params.size(); i < result.size(); ++i) {
            result.at(i).isPersistent = persistent;
        }
    };

    bool startAtNominalTick = false;
    const PlaybackParamList& sfParams = findParams(m_soundFlagParamsByTrack, trackIdx, nominalPositionTick, &startAtNominalTick);
    addParams(sfParams, startAtNominalTick);

    const PlaybackParamList& textParams = findParams(m_textParamsByTrack, trackIdx, nominalPositionTick, &startAtNominalTick);
    addParams(textParams, startAtNominalTick);

    return result;
}

PlaybackParamLayers PlaybackContext::playbackParamLayers(const Score* score) const
{
    PlaybackParamLayers result;

    auto addParams = [score, &result](const ParamsByTrack& paramsByTrack) {
        for (const auto& params : paramsByTrack) {
            PlaybackParamMap& paramMap = result[static_cast<layer_idx_t>(params.first)];
            for (const auto& pair : params.second) {
                PlaybackParamList& list = paramMap[timestampFromTicks(score, pair.first)];
                list.insert(list.end(), pair.second.begin(), pair.second.end());
            }
        }
    };

    addParams(m_soundFlagParamsByTrack);
    addParams(m_textParamsByTrack);

    return result;
}

DynamicLevelLayers PlaybackContext::dynamicLevelLayers(const Score* score) const
{
    DynamicLevelLayers result;

    for (const auto& dynamics : m_dynamicsByTrack) {
        DynamicLevelMap dynamicLevelMap;
        for (const auto& dynamic : dynamics.second) {
            dynamicLevelMap.emplace(timestampFromTicks(score, dynamic.first), dynamic.second.level);
        }

        result.emplace(static_cast<layer_idx_t>(dynamics.first), std::move(dynamicLevelMap));
    }

    return result;
}

void PlaybackContext::update(const ID partId, const Score* score, bool expandRepeats)
{
    const Part* part = score->partById(partId);
    IF_ASSERT_FAILED(part) {
        return;
    }

    // cache them for optimization
    m_partStartTrack = part->startTrack();
    m_partEndTrack = part->endTrack();

    IF_ASSERT_FAILED(m_partStartTrack <= m_partEndTrack) {
        return;
    }

    const size_t ntracks = m_partEndTrack - m_partStartTrack;
    m_dynamicsByTrack.reserve(ntracks);
    m_soundFlagParamsByTrack.reserve(ntracks);
    m_textParamsByTrack.reserve(ntracks);

    for (const RepeatSegment* repeatSegment : score->repeatList(expandRepeats)) {
        std::vector<const MeasureRepeat*> measureRepeats;
        int tickPositionOffset = repeatSegment->utick - repeatSegment->tick;

        for (const Measure* measure : repeatSegment->measureList()) {
            for (const Segment* segment = measure->first(); segment; segment = segment->next()) {
                int segmentStartTick = segment->tick().ticks() + tickPositionOffset;

                handleSegmentElements(segment, segmentStartTick, measureRepeats);
                handleSegmentAnnotations(partId, segment, segmentStartTick);
            }
        }

        handleSpanners(partId, score, repeatSegment->tick,
                       repeatSegment->tick + repeatSegment->len(), tickPositionOffset);

        handleMeasureRepeats(measureRepeats, tickPositionOffset);
    }

    for (track_idx_t trackIdx = m_partStartTrack; trackIdx < m_partEndTrack; ++trackIdx) {
        DynamicMap& dynamics = m_dynamicsByTrack[trackIdx];
        if (!muse::contains(dynamics, 0)) {
            dynamics.emplace(0, DynamicInfo { dynamicLevelFromType(mpe::DynamicType::Natural), 0 });
        }
    }
}

void PlaybackContext::clear()
{
    m_partStartTrack = 0;
    m_partEndTrack = 0;
    m_dynamicsByTrack.clear();
    m_playTechniquesMap.clear();
    m_soundFlagParamsByTrack.clear();
    m_textParamsByTrack.clear();
}

bool PlaybackContext::hasSoundFlags() const
{
    return !m_soundFlagParamsByTrack.empty();
}

dynamic_level_t PlaybackContext::nominalDynamicLevel(const track_idx_t trackIdx, const int positionTick) const
{
    auto dynamicsIt = m_dynamicsByTrack.find(trackIdx);
    if (dynamicsIt == m_dynamicsByTrack.end()) {
        return mpe::dynamicLevelFromType(mpe::DynamicType::Natural);
    }

    const DynamicMap& dynamics = dynamicsIt->second;
    auto it = dynamics.find(positionTick);
    if (it == dynamics.end()) {
        return mpe::dynamicLevelFromType(mpe::DynamicType::Natural);
    }

    return it->second.level;
}

void PlaybackContext::updateDynamicMap(const Dynamic* dynamic, const Segment* segment, const int segmentPositionTick)
{
    if (!dynamic->playDynamic()) {
        return;
    }

    const DynamicType type = dynamic->dynamicType();

    if (isOrdinaryDynamicType(type)) {
        applyDynamic(dynamic, dynamicLevelFromOrdinaryType(type), segmentPositionTick);
        return;
    }

    if (isSingleNoteDynamicType(type)) {
        mpe::dynamic_level_t prevDynamicLevel = appliableDynamicLevel(dynamic->track(), segmentPositionTick);
        applyDynamic(dynamic, dynamicLevelFromSingleNoteType(type), segmentPositionTick);

        if (segment->next()) {
            int tickPositionOffset = segmentPositionTick - segment->tick().ticks();
            int nextSegmentPositionTick = segment->next()->tick().ticks() + tickPositionOffset;
            applyDynamic(dynamic, prevDynamicLevel, nextSegmentPositionTick);
        }

        return;
    }

    if (isCompoundDynamicType(type)) {
        const CompoundDynamic& transition = compoundDynamicFromType(type);
        const int transitionDuration = dynamic->velocityChangeLength().ticks();

        dynamic_level_t levelFrom = dynamicLevelFromOrdinaryType(transition.from);
        dynamic_level_t levelTo = dynamicLevelFromOrdinaryType(transition.to);

        dynamic_level_t range = levelTo - levelFrom;

        std::map<int, int> dynamicsCurve = TConv::easingValueCurve(transitionDuration,
                                                                   6 /*stepsCount*/,
                                                                   static_cast<int>(range),
                                                                   ChangeMethod::NORMAL);

        for (const auto& pair : dynamicsCurve) {
            applyDynamic(dynamic, levelFrom + pair.second, segmentPositionTick + pair.first);
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

    bool cancelPlayTechniques = type == PlayingTechniqueType::Natural || type == PlayingTechniqueType::Open;

    if (cancelPlayTechniques && !m_soundFlagParamsByTrack.empty()) {
        PlaybackParam ordTechnique(PlaybackParam::PlayingTechnique, mpe::ORDINARY_PLAYING_TECHNIQUE_CODE);

        for (track_idx_t idx = m_partStartTrack; idx < m_partEndTrack; ++idx) {
            m_soundFlagParamsByTrack[idx][segmentPositionTick].push_back(ordTechnique);
        }
    }
}

void PlaybackContext::updatePlaybackParamsForSoundFlags(const SoundFlagMap& flagsOnSegment, const int segmentPositionTick)
{
    auto trackAccepted = [&flagsOnSegment](const SoundFlag* flag, track_idx_t trackIdx) {
        staff_idx_t staffIdx = track2staff(trackIdx);

        if (flag->staffIdx() == staffIdx) {
            return true;
        }

        if (flag->applyToAllStaves()) {
            return !muse::contains(flagsOnSegment, staffIdx);
        }

        return false;
    };

    for (const auto& pair : flagsOnSegment) {
        const SoundFlag* flag = pair.second;

        for (track_idx_t trackIdx = m_partStartTrack; trackIdx < m_partEndTrack; ++trackIdx) {
            if (!trackAccepted(flag, trackIdx)) {
                continue;
            }

            mpe::PlaybackParamList& params = m_soundFlagParamsByTrack[trackIdx][segmentPositionTick];

            for (const String& soundPreset : flag->soundPresets()) {
                if (!soundPreset.empty()) {
                    params.emplace_back(PlaybackParam::SoundPreset, soundPreset);
                }
            }

            if (!flag->playingTechnique().empty()) {
                params.emplace_back(PlaybackParam::PlayingTechnique, flag->playingTechnique());
            }
        }

        if (flag->playingTechnique() == mpe::ORDINARY_PLAYING_TECHNIQUE_CODE) {
            m_playTechniquesMap[segmentPositionTick] = mpe::ArticulationType::Standard;
        }
    }
}

void PlaybackContext::updatePlaybackParamsForText(const TextBase* text, const int segmentPositionTick)
{
    IF_ASSERT_FAILED(text->isLyrics() || text->isSticking()) {
        return;
    }

    PlaybackParam param(PlaybackParam::Syllable, text->plainText());

    staff_idx_t staffIdx = text->staffIdx();

    for (voice_idx_t voiceIdx = 0; voiceIdx < VOICES; ++voiceIdx) {
        track_idx_t trackIdx = staff2track(staffIdx, voiceIdx);
        m_textParamsByTrack[trackIdx][segmentPositionTick].push_back(param);
    }
}

void PlaybackContext::handleSpanners(const ID partId, const Score* score, const int segmentStartTick, const int segmentEndTick,
                                     const int tickPositionOffset)
{
    const SpannerMap& spannerMap = score->spannerMap();
    if (spannerMap.empty()) {
        return;
    }

    auto intervals = spannerMap.findOverlapping(segmentStartTick + 1, segmentEndTick - 1);
    for (const auto& interval : intervals) {
        const Spanner* spanner = interval.value;

        if (!spanner->isHairpin() || !spanner->playSpanner() || spanner->segmentsEmpty()) {
            continue;
        }

        if (spanner->part()->id() != partId.toUint64()) {
            continue;
        }

        const Staff* staff = spanner->staff();
        if (staff && !staff->isPrimaryStaff()) {
            continue; // ignore linked staves
        }

        handleHairpin(toHairpin(spanner), tickPositionOffset);
    }
}

void PlaybackContext::handleHairpin(const Hairpin* hairpin, const int tickPositionOffset)
{
    int spannerFrom = hairpin->tick().ticks();
    int spannerTo = spannerFrom + std::abs(hairpin->ticks().ticks());

    const track_idx_t trackIdx = hairpin->track();

    // --- Check start tick
    {
        const Segment* startSegment = hairpin->startSegment();
        const Dynamic* startDynamic = startSegment
                                      ? toDynamic(startSegment->findAnnotation(ElementType::DYNAMIC, trackIdx, trackIdx))
                                      : nullptr;
        if (startDynamic) {
            const DynamicType dynamicType = startDynamic->dynamicType();

            if (dynamicType != DynamicType::OTHER
                && !isOrdinaryDynamicType(dynamicType)
                && !isSingleNoteDynamicType(dynamicType)) {
                // The hairpin starts with a compound dynamic; we should start the hairpin after the transition is complete
                // This solution should be replaced once we have better infrastructure to see relations between Dynamics and Hairpins.
                spannerFrom += startDynamic->velocityChangeLength().ticks();
            }
        }
    }

    // --- Determine levelFrom
    const dynamic_level_t nominalLevelFrom = findNominalStartDynamicLevel(hairpin);
    const bool hasNominalLevelFrom = nominalLevelFrom != NATURAL_DYNAMIC_LEVEL;

    // If the hairpin has no specific start level, use the currently-applicable level at the start tick of the hairpin
    const dynamic_level_t levelFrom
        = hasNominalLevelFrom ? nominalLevelFrom : appliableDynamicLevel(trackIdx, spannerFrom + tickPositionOffset);

    // --- Determine levelTo
    const dynamic_level_t nominalLevelTo = findNominalEndDynamicLevel(hairpin);
    const bool hasNominalLevelTo = nominalLevelTo != NATURAL_DYNAMIC_LEVEL;

    // If there is an end dynamic marking, check if it matches the 'direction' of the hairpin (cresc. vs decresc.)
    const bool isCrescendo = hairpin->isCrescendo();
    const bool useNominalLevelTo = hasNominalLevelTo && (isCrescendo
                                                         ? nominalLevelTo > levelFrom
                                                         : nominalLevelTo < levelFrom);

    const dynamic_level_t levelTo = useNominalLevelTo
                                    ? nominalLevelTo
                                    : levelFrom + (isCrescendo ? mpe::DYNAMIC_LEVEL_STEP : -mpe::DYNAMIC_LEVEL_STEP);

    // --- Check end tick
    const dynamic_level_t dynamicLevelAtEndTick = nominalDynamicLevel(trackIdx, spannerTo + tickPositionOffset);
    const bool hasDynamicAtEndTick = dynamicLevelAtEndTick != NATURAL_DYNAMIC_LEVEL;

    if (hasDynamicAtEndTick && dynamicLevelAtEndTick != levelTo) {
        // Fix overlap with the following dynamic by subtracting a small fraction
        spannerTo -= Fraction::eps().ticks();
    }

    const int spannerDurationTicks = spannerTo - spannerFrom;
    if (spannerDurationTicks <= 0) {
        return;
    }

    // --- Render
    std::map<int, int> dynamicsCurve = TConv::easingValueCurve(spannerDurationTicks,
                                                               24 /*stepsCount*/,
                                                               static_cast<int>(levelTo - levelFrom),
                                                               hairpin->veloChangeMethod());

    for (const auto& pair : dynamicsCurve) {
        applyDynamic(hairpin, levelFrom + pair.second, spannerFrom + pair.first + tickPositionOffset);
    }

    if (hasNominalLevelTo && !useNominalLevelTo && !hasDynamicAtEndTick) {
        // If there is a dynamic at the end of the hairpin that we couldn't use because it didn't match the direction of the hairpin,
        // insert that dynamic directly after the hairpin
        applyDynamic(hairpin, nominalLevelTo, spannerTo + tickPositionOffset);
    }
}

void PlaybackContext::handleSegmentAnnotations(const ID partId, const Segment* segment, const int segmentPositionTick)
{
    SoundFlagMap soundFlagsOnSegment;

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

        if (annotation->isSticking()) {
            updatePlaybackParamsForText(toTextBase(annotation), segmentPositionTick);
            continue;
        }

        if (annotation->isStaffText()) {
            if (const SoundFlag* flag = toStaffText(annotation)->soundFlag()) {
                if (soundFlagPlayable(flag)) {
                    soundFlagsOnSegment.emplace(flag->staffIdx(), flag);
                }
            }
        }
    }

    if (!soundFlagsOnSegment.empty()) {
        updatePlaybackParamsForSoundFlags(soundFlagsOnSegment, segmentPositionTick);
    }
}

void PlaybackContext::handleSegmentElements(const Segment* segment, const int segmentPositionTick,
                                            std::vector<const MeasureRepeat*>& foundMeasureRepeats)
{
    for (track_idx_t track = m_partStartTrack; track < m_partEndTrack; ++track) {
        const EngravingItem* item = segment->elementAt(track);
        if (!item) {
            continue;
        }

        if (item->isMeasureRepeat()) {
            foundMeasureRepeats.push_back(toMeasureRepeat(item));
            continue;
        }

        if (item->isChordRest()) {
            const ChordRest* chordRest = toChordRest(item);
            for (const Lyrics* lyrics : chordRest->lyrics()) {
                updatePlaybackParamsForText(lyrics, segmentPositionTick);
            }
        }
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

            copyDynamicsInRange(m_dynamicsByTrack, startTick, endTick, newItemsOffsetTick);
            copyPlaybackParamsInRange(m_soundFlagParamsByTrack, startTick, endTick, newItemsOffsetTick);
            copyPlaybackParamsInRange(m_textParamsByTrack, startTick, endTick, newItemsOffsetTick);
            copyPlayTechniquesInRange(m_playTechniquesMap, startTick, endTick, newItemsOffsetTick);

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

void PlaybackContext::applyDynamic(const EngravingItem* dynamicItem, const dynamic_level_t dynamicLevel, const int positionTick)
{
    const VoiceAssignment voiceAssignment = dynamicItem->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>();
    const track_idx_t dynamicTrackIdx = dynamicItem->track();
    const staff_idx_t dynamicStaffIdx = dynamicItem->staffIdx();
    const int dynamicPriority = static_cast<int>(voiceAssignment);

    //! See: https://github.com/musescore/MuseScore/issues/23355
    auto trackAccepted = [voiceAssignment, dynamicTrackIdx, dynamicStaffIdx](const track_idx_t trackIdx) {
        switch (voiceAssignment) {
        case VoiceAssignment::CURRENT_VOICE_ONLY:
            return dynamicTrackIdx == trackIdx;
        case VoiceAssignment::ALL_VOICE_IN_STAFF:
            return dynamicStaffIdx == track2staff(trackIdx);
        case VoiceAssignment::ALL_VOICE_IN_INSTRUMENT:
            return true;
        }

        return false;
    };

    for (track_idx_t trackIdx = m_partStartTrack; trackIdx < m_partEndTrack; ++trackIdx) {
        if (!trackAccepted(trackIdx)) {
            continue;
        }

        DynamicInfo& dynamic = m_dynamicsByTrack[trackIdx][positionTick];
        if (dynamic.priority <= dynamicPriority) {
            dynamic.level = dynamicLevel;
            dynamic.priority = dynamicPriority;
        }
    }
}

void PlaybackContext::copyDynamicsInRange(DynamicsByTrack& source, const int rangeStartTick, const int rangeEndTick,
                                          const int newDynamicsOffsetTick)
{
    for (auto& pair : source) {
        DynamicMap& dynamics = pair.second;
        auto startIt = dynamics.lower_bound(rangeStartTick);
        if (startIt == dynamics.end()) {
            return;
        }

        auto endIt = dynamics.lower_bound(rangeEndTick);

        DynamicMap newDynamics;
        for (auto it = startIt; it != endIt; ++it) {
            int tick = it->first + newDynamicsOffsetTick;
            newDynamics.insert_or_assign(tick, it->second);
        }

        dynamics.merge(std::move(newDynamics));
    }
}

void PlaybackContext::copyPlaybackParamsInRange(ParamsByTrack& source, const int rangeStartTick, const int rangeEndTick,
                                                const int newParamsOffsetTick)
{
    for (auto& pair : source) {
        ParamMap& params = pair.second;
        auto startIt = params.lower_bound(rangeStartTick);
        if (startIt == params.end()) {
            return;
        }

        auto endIt = params.lower_bound(rangeEndTick);

        ParamMap newParams;
        for (auto it = startIt; it != endIt; ++it) {
            int tick = it->first + newParamsOffsetTick;
            newParams.insert_or_assign(tick, it->second);
        }

        params.merge(std::move(newParams));
    }
}

void PlaybackContext::copyPlayTechniquesInRange(PlayTechniquesMap& source, const int rangeStartTick, const int rangeEndTick,
                                                const int newPlayTechOffsetTick)
{
    auto startIt = source.lower_bound(rangeStartTick);
    if (startIt == source.end()) {
        return;
    }

    auto endIt = source.lower_bound(rangeEndTick);

    PlayTechniquesMap newPlayTechniques;
    for (auto it = startIt; it != endIt; ++it) {
        int tick = it->first + newPlayTechOffsetTick;
        newPlayTechniques.insert_or_assign(tick, it->second);
    }

    source.merge(std::move(newPlayTechniques));
}

const PlaybackParamList& PlaybackContext::findParams(const ParamsByTrack& allParams, const track_idx_t trackIdx,
                                                     const int nominalPositionTick, bool* startAtNominalTick)
{
    static const PlaybackParamList dummyList;

    if (allParams.empty()) {
        return dummyList;
    }

    auto paramsIt = allParams.find(trackIdx);
    if (paramsIt == allParams.end()) {
        return dummyList;
    }

    const ParamMap& params = paramsIt->second;
    auto it = muse::findLessOrEqual(params, nominalPositionTick);
    if (it == params.end()) {
        return dummyList;
    }

    if (startAtNominalTick) {
        *startAtNominalTick = it->first == nominalPositionTick;
    }

    return it->second;
}
