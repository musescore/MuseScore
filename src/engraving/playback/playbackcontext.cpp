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

#include "playbackcontext.h"

#include <cmath>

#include "dom/lyrics.h"
#include "dom/measure.h"
#include "dom/measurerepeat.h"
#include "dom/part.h"
#include "dom/playtechannotation.h"
#include "dom/repeatlist.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/soundflag.h"
#include "dom/staff.h"
#include "dom/stafftext.h"

#include "engraving/automation/automationdata.h"
#include "engraving/automation/automationutils.h"

#include "utils/arrangementutils.h"
#include "utils/expressionutils.h"
#include "types/constants.h"

#include "global/containers.h"
#include "global/realfn.h"

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

static dynamic_level_t toDynamicLevel(double normalizedValue)
{
    return static_cast<dynamic_level_t>(std::lround(normalizedValue * MAX_DYNAMIC_LEVEL));
}

template<typename ByTrackMap, typename ResultMap, typename MergeFn>
static void collectByTrackRange(const ByTrackMap& byTrackMap, const track_idx_t trackFrom, const track_idx_t trackTo,
                                const Score* score, const std::set<track_idx_t>& usedTracks, ResultMap& result, MergeFn merge)
{
    for (auto it = byTrackMap.lower_bound(trackFrom); it != byTrackMap.end() && it->first < trackTo; ++it) {
        if (!muse::contains(usedTracks, it->first)) {
            continue;
        }

        for (const auto& pair : it->second) {
            const timestamp_t timestamp = timestampFromTicks(score, pair.first);
            merge(result[timestamp], pair.second);
        }
    }
}

PlaybackContext::PlaybackContext(const Score* score)
    : m_score(score)
{
}

dynamic_level_t PlaybackContext::appliableDynamicLevel(const track_idx_t trackIdx, const int nominalPositionTick) const
{
    TRACEFUNC;

    const AutomationCurve* curve = dynamicsCurve(trackIdx);
    if (!curve) {
        return NATURAL_DYNAMIC_LEVEL;
    }

    const auto it = muse::findLessOrEqual(*curve, nominalPositionTick);
    if (it == curve->cend()) {
        return NATURAL_DYNAMIC_LEVEL;
    }

    const auto nextIt = std::next(it);
    if (nextIt == curve->cend()) {
        return toDynamicLevel(it->second.outValue);
    }

    const double factor = static_cast<double>(nominalPositionTick - it->first) / (nextIt->first - it->first);
    return toDynamicLevel(evaluateCurveAt(*curve, nextIt, factor));
}

std::pair<mpe::timestamp_t, PlayingTechniqueType> PlaybackContext::playingTechnique(const track_idx_t trackIdx,
                                                                                    const int nominalPositionTick) const
{
    auto mapIt = m_playTechniquesByTrack.find(trackIdx);
    if (mapIt == m_playTechniquesByTrack.cend()) {
        return std::make_pair(0, PlayingTechniqueType::Natural);
    }

    const PlayTechniquesMap& playTechniquesMap = mapIt->second;

    auto it = findLessOrEqual(playTechniquesMap, nominalPositionTick);
    if (it == playTechniquesMap.cend()) {
        return std::make_pair(0, PlayingTechniqueType::Natural);
    }

    return std::make_pair(timestampFromTicks(m_score, it->first), it->second);
}

muse::mpe::timestamp_t PlaybackContext::findPlayingTechniqueTimestamp(const track_idx_t trackIdx, PlayingTechniqueType type,
                                                                      const int startFromTick) const
{
    auto mapIt = m_playTechniquesByTrack.find(trackIdx);
    if (mapIt == m_playTechniquesByTrack.cend()) {
        return -1;
    }

    const PlayTechniquesMap& playTechniquesMap = mapIt->second;

    auto it = playTechniquesMap.upper_bound(startFromTick);

    for (; it != playTechniquesMap.end(); ++it) {
        if (it->second == type) {
            return timestampFromTicks(m_score, it->first);
        }
    }

    return -1;
}

std::map<timestamp_t, SoundPresetChangeEventList> PlaybackContext::soundPresets(const track_idx_t trackFrom,
                                                                                const track_idx_t trackTo) const
{
    std::map<timestamp_t, SoundPresetChangeEventList> result;

    collectByTrackRange(m_soundPresetsByTrack, trackFrom, trackTo, m_score, m_usedTracks, result,
                        [](SoundPresetChangeEventList& list, const SoundPresetChangeEventList& events) {
        list.insert(list.end(), events.begin(), events.end());
    });

    return result;
}

SoundPresetChangeEventList PlaybackContext::soundPresets(const track_idx_t trackIdx, const int nominalPositionTick) const
{
    auto presetsIt = m_soundPresetsByTrack.find(trackIdx);
    if (presetsIt == m_soundPresetsByTrack.end()) {
        return {};
    }

    const SoundPresetsMap& map = presetsIt->second;
    auto it = muse::findLessOrEqual(map, nominalPositionTick);
    if (it == map.end()) {
        return {};
    }

    return it->second;
}

std::map<timestamp_t, TextArticulationEventList> PlaybackContext::textArticulations(const track_idx_t trackFrom,
                                                                                    const track_idx_t trackTo) const
{
    std::map<timestamp_t, TextArticulationEventList> result;

    collectByTrackRange(m_textArticulationsByTrack, trackFrom, trackTo, m_score, m_usedTracks, result,
                        [](TextArticulationEventList& list, const TextArticulationEvent& event) {
        list.push_back(event);
    });

    return result;
}

TextArticulationEvent PlaybackContext::textArticulation(const track_idx_t trackIdx, const int nominalPositionTick) const
{
    auto articulationsIt = m_textArticulationsByTrack.find(trackIdx);
    if (articulationsIt == m_textArticulationsByTrack.end()) {
        return {};
    }

    const TextArticulationMap& map = articulationsIt->second;
    auto it = muse::findLessOrEqual(map, nominalPositionTick);
    if (it == map.end()) {
        return {};
    }

    TextArticulationEvent result = it->second;
    result.flags.setFlag(TextArticulationEvent::StartsAtPlaybackPosition, it->first == nominalPositionTick);

    return result;
}

std::map<timestamp_t, SyllableEventList> PlaybackContext::syllables(const track_idx_t trackFrom, const track_idx_t trackTo) const
{
    std::map<timestamp_t, SyllableEventList> result;

    collectByTrackRange(m_syllablesByTrack, trackFrom, trackTo, m_score, m_usedTracks, result,
                        [](SyllableEventList& list, const SyllableEvent& event) {
        list.push_back(event);
    });

    return result;
}

SyllableEvent PlaybackContext::syllable(const track_idx_t trackIdx, const int nominalPositionTick) const
{
    auto syllablesIt = m_syllablesByTrack.find(trackIdx);
    if (syllablesIt == m_syllablesByTrack.end()) {
        return {};
    }

    const SyllableMap& map = syllablesIt->second;
    auto it = muse::findLessOrEqual(map, nominalPositionTick);
    if (it == map.end()) {
        return {};
    }

    SyllableEvent result = it->second;
    result.flags.setFlag(SyllableEvent::StartsAtPlaybackPosition, it->first == nominalPositionTick);

    return result;
}

DynamicLevelLayers PlaybackContext::dynamicLevelLayers(const track_idx_t trackFrom, const track_idx_t trackTo) const
{
    TRACEFUNC;

    if (!m_score || !m_score->automationData()) {
        return {};
    }

    DynamicLevelLayers result;
    const AutomationCurve* lastCurve = nullptr;
    DynamicLevelMap lastLevelMap;

    for (track_idx_t trackIdx = trackFrom; trackIdx < trackTo; ++trackIdx) {
        const AutomationCurve* curve = dynamicsCurve(trackIdx);
        if (!curve || curve->empty()) {
            continue;
        }

        if (curve != lastCurve) {
            lastLevelMap = buildDynamicLevelMap(*curve);
            lastCurve = curve;
        }

        result[static_cast<layer_idx_t>(trackIdx)] = lastLevelMap;
    }

    return result;
}

DynamicLevelMap PlaybackContext::buildDynamicLevelMap(const AutomationCurve& curve) const
{
    DynamicLevelMap levelMap;

    if (curve.cbegin()->first > 0) {
        levelMap[timestampFromTicks(m_score, 0)] = NATURAL_DYNAMIC_LEVEL;
    }

    for (auto it = curve.cbegin(); it != curve.cend(); ++it) {
        levelMap[timestampFromTicks(m_score, it->first)] = toDynamicLevel(it->second.outValue);
        appendRampToLevelMap(curve, it, levelMap);
    }

    return levelMap;
}

void PlaybackContext::appendRampToLevelMap(const AutomationCurve& curve, AutomationCurve::const_iterator it,
                                           DynamicLevelMap& levelMap) const
{
    const auto nextIt = std::next(it);
    if (nextIt == curve.cend()) {
        return;
    }

    const dynamic_level_t baseLvl = toDynamicLevel(it->second.outValue);
    const dynamic_level_t nextLvl = toDynamicLevel(resolvedInValue(curve, nextIt));
    const int range = nextLvl - baseLvl;
    const bool isLinear = nextIt->second.bend.isNone();
    if (range == 0 && isLinear) {
        return;
    }

    // nextIt's own curve entry stores outValue, not inValue - if it's a discontinuity, end the ramp
    // one tick early so it still approaches the true target (inValue)
    const bool nextIsDiscontinuity = !muse::RealIsEqual(resolvedInValue(curve, nextIt), nextIt->second.outValue);
    const utick_t targetTick = nextIsDiscontinuity ? nextIt->first - 1 : nextIt->first;
    const int intervalDuration = targetTick - it->first;
    const int steps = std::max(intervalDuration / (Constants::DIVISION / 4), 24);

    for (int j = 1; j < steps; ++j) {
        const utick_t tick = it->first + static_cast<utick_t>(static_cast<float>(j) * intervalDuration / steps);

        dynamic_level_t level;
        if (isLinear) {
            const float factor = static_cast<float>(j) / static_cast<float>(steps);
            level = static_cast<dynamic_level_t>(std::lround(baseLvl + static_cast<double>(factor) * static_cast<double>(range)));
        } else {
            const double t = static_cast<double>(j) / static_cast<double>(steps);
            level = toDynamicLevel(evaluateCurveAt(curve, nextIt, t));
        }
        levelMap[timestampFromTicks(m_score, tick)] = level;
    }

    if (nextIsDiscontinuity) {
        levelMap[timestampFromTicks(m_score, targetTick)] = nextLvl;
    }
}

void PlaybackContext::update(const track_idx_t trackFrom, const track_idx_t trackTo, bool expandRepeats)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(m_score) {
        return;
    }

    IF_ASSERT_FAILED(trackFrom <= trackTo) {
        return;
    }

    m_dynamicsCurveByTrack.clear();

    for (const RepeatSegment* repeatSegment : m_score->repeatList(expandRepeats)) {
        std::vector<const MeasureRepeat*> measureRepeats;
        int tickPositionOffset = repeatSegment->utick - repeatSegment->tick;

        for (const Measure* measure : repeatSegment->measureList()) {
            for (const Segment* segment = measure->first(); segment; segment = segment->next()) {
                int segmentStartTick = segment->tick().ticks() + tickPositionOffset;

                handleSegmentElements(repeatSegment, segment, segmentStartTick, trackFrom, trackTo, measureRepeats);
                handleSegmentAnnotations(segment, segmentStartTick, trackFrom, trackTo);
            }
        }

        handleMeasureRepeats(measureRepeats, tickPositionOffset);
    }
}

void PlaybackContext::clear(const track_idx_t trackFrom, const track_idx_t trackTo)
{
    const auto eraseTrackRange = [trackFrom, trackTo](auto& byTrackMap) {
        byTrackMap.erase(byTrackMap.lower_bound(trackFrom), byTrackMap.lower_bound(trackTo));
    };

    eraseTrackRange(m_soundPresetsByTrack);
    eraseTrackRange(m_textArticulationsByTrack);
    eraseTrackRange(m_syllablesByTrack);
    eraseTrackRange(m_playTechniquesByTrack);
    eraseTrackRange(m_multiVerseLyricsPositionMap);

    m_usedTracks.erase(m_usedTracks.lower_bound(trackFrom), m_usedTracks.lower_bound(trackTo));

    muse::remove_if(m_currentVerseNumByChordRest, [trackFrom, trackTo](const auto& pair) {
        const track_idx_t track = pair.first->track();
        return track >= trackFrom && track < trackTo;
    });
}

bool PlaybackContext::hasSoundFlags(const track_idx_t trackFrom, const track_idx_t trackTo) const
{
    auto hasEntryInRange = [trackFrom, trackTo](const auto& byTrackMap) {
        auto it = byTrackMap.lower_bound(trackFrom);
        return it != byTrackMap.end() && it->first < trackTo;
    };

    return hasEntryInRange(m_soundPresetsByTrack) || hasEntryInRange(m_textArticulationsByTrack);
}

void PlaybackContext::updatePlayTechMap(const Part* part, const PlayTechAnnotation* annotation, const int segmentPositionTick)
{
    if (!annotation->playPlayTechAnnotation()) {
        return;
    }
    const PlayingTechniqueType type = annotation->techniqueType();
    if (type == PlayingTechniqueType::Undefined) {
        return;
    }

    const bool cancelPlayTechniques = (type == PlayingTechniqueType::Natural || type == PlayingTechniqueType::Open)
                                      && !m_textArticulationsByTrack.empty();

    TextArticulationEvent textArticulation;
    textArticulation.text = mpe::ORDINARY_PLAYING_TECHNIQUE_CODE;

    const TrackRange trackRange = part->trackRange();
    for (track_idx_t idx = trackRange.startTrack; idx < trackRange.endTrack; ++idx) {
        m_playTechniquesByTrack[idx][segmentPositionTick] = type;

        if (cancelPlayTechniques) {
            textArticulation.layerIdx = static_cast<layer_idx_t>(idx);
            m_textArticulationsByTrack[idx][segmentPositionTick] = textArticulation;
        }
    }
}

void PlaybackContext::updateSoundPresetAndTextArticulationMap(const Part* part, const SoundFlagMap& flagsOnSegment,
                                                              const int segmentPositionTick)
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

    const TrackRange trackRange = part->trackRange();

    for (const auto& pair : flagsOnSegment) {
        const SoundFlag* flag = pair.second;

        for (track_idx_t trackIdx = trackRange.startTrack; trackIdx < trackRange.endTrack; ++trackIdx) {
            if (!trackAccepted(flag, trackIdx)) {
                continue;
            }

            SoundPresetChangeEventList presets;

            for (const String& soundPreset : flag->soundPresets()) {
                if (soundPreset.empty()) {
                    continue;
                }

                SoundPresetChangeEvent event;
                event.code = soundPreset;
                event.layerIdx = static_cast<layer_idx_t>(trackIdx);
                presets.emplace_back(std::move(event));
            }

            if (!presets.empty()) {
                SoundPresetChangeEventList& target = m_soundPresetsByTrack[trackIdx][segmentPositionTick];
                target.insert(target.end(), std::make_move_iterator(presets.begin()), std::make_move_iterator(presets.end()));
            }

            if (!flag->playingTechnique().empty()) {
                TextArticulationEvent event;
                event.text = flag->playingTechnique();
                event.layerIdx = static_cast<layer_idx_t>(trackIdx);
                m_textArticulationsByTrack[trackIdx][segmentPositionTick] = std::move(event);
            }
        }

        if (flag->playingTechnique() == mpe::ORDINARY_PLAYING_TECHNIQUE_CODE) {
            for (track_idx_t trackIdx = trackRange.startTrack; trackIdx < trackRange.endTrack; ++trackIdx) {
                m_playTechniquesByTrack[trackIdx][segmentPositionTick] = PlayingTechniqueType::Natural;
            }
        }
    }
}

void PlaybackContext::updateSyllableMap(const TextBase* text, const int segmentPositionTick)
{
    IF_ASSERT_FAILED(text->isLyrics() || text->isSticking()) {
        return;
    }

    if (text->empty()) {
        return;
    }

    SyllableEvent syllable;
    syllable.text = text->plainText();

    if (text->isLyrics()) {
        const Lyrics* lyrics = toLyrics(text);

        switch (lyrics->syllabic()) {
        case LyricsSyllabic::BEGIN:
        case LyricsSyllabic::MIDDLE:
            syllable.flags.setFlag(SyllableEvent::HyphenedToNext);
            break;
        case LyricsSyllabic::SINGLE:
        case LyricsSyllabic::END:
            break;
        }
    }

    const staff_idx_t staffIdx = text->staffIdx();

    for (voice_idx_t voiceIdx = 0; voiceIdx < VOICES; ++voiceIdx) {
        track_idx_t trackIdx = staff2track(staffIdx, voiceIdx);
        syllable.layerIdx = static_cast<layer_idx_t>(trackIdx);
        m_syllablesByTrack[trackIdx][segmentPositionTick] = syllable;
    }
}

void PlaybackContext::handleSegmentAnnotations(const Segment* segment, const int segmentPositionTick,
                                               const track_idx_t trackFrom, const track_idx_t trackTo)
{
    std::unordered_map<const Part*, SoundFlagMap> soundFlagsByPart;

    for (const EngravingItem* annotation : segment->annotations()) {
        if (!annotation || !annotation->part()) {
            continue;
        }

        const Part* annotationPart = annotation->part();
        const TrackRange annotationPartTrackRange = annotationPart->trackRange();
        if (annotationPartTrackRange.startTrack >= trackTo || annotationPartTrackRange.endTrack <= trackFrom) {
            continue;
        }

        if (annotation->isPlayTechAnnotation()) {
            updatePlayTechMap(annotationPart, toPlayTechAnnotation(annotation), segmentPositionTick);
            continue;
        }

        if (annotation->isSticking()) {
            updateSyllableMap(toTextBase(annotation), segmentPositionTick);
            continue;
        }

        if (annotation->isStaffText()) {
            if (const SoundFlag* flag = toStaffText(annotation)->soundFlag()) {
                if (soundFlagPlayable(flag)) {
                    soundFlagsByPart[annotationPart].emplace(flag->staffIdx(), flag);
                }
            }
        }
    }

    for (const auto& pair : soundFlagsByPart) {
        const Part* part = pair.first;
        const SoundFlagMap& flagsOnSegment = pair.second;

        if (!flagsOnSegment.empty()) {
            updateSoundPresetAndTextArticulationMap(part, flagsOnSegment, segmentPositionTick);
        }
    }
}

void PlaybackContext::handleSegmentElements(const RepeatSegment* repeat, const Segment* segment,
                                            const int segmentPositionTick,
                                            const track_idx_t trackFrom, const track_idx_t trackTo,
                                            std::vector<const MeasureRepeat*>& foundMeasureRepeats)
{
    for (track_idx_t track = trackFrom; track < trackTo; ++track) {
        const EngravingItem* item = segment->element(track);
        if (!item) {
            continue;
        }

        if (item->isMeasureRepeat()) {
            foundMeasureRepeats.push_back(toMeasureRepeat(item));
            continue;
        }

        if (item->isChordRest()) {
            m_usedTracks.insert(track);

            const ChordRest* chordRest = toChordRest(item);
            if (chordRest->lyrics().empty()) {
                continue;
            }

            const Lyrics* lyrics = nullptr;

            auto verseNumIt = m_currentVerseNumByChordRest.find(chordRest);
            if (verseNumIt == m_currentVerseNumByChordRest.end()) {
                m_currentVerseNumByChordRest[chordRest] = 0;
                lyrics = chordRest->lyrics(0);
                if (chordRest->lyrics().size() > 1) {
                    m_multiVerseLyricsPositionMap[track].insert(chordRest->tick().ticks());
                }
            } else if (hasOnlyOneLyricsVerse(repeat, track)) {
                lyrics = chordRest->lyrics(0);
            } else {
                verseNumIt->second++;
                lyrics = chordRest->lyrics(verseNumIt->second);
            }

            if (lyrics) {
                updateSyllableMap(lyrics, segmentPositionTick);
            }
        }
    }
}

template<typename ItemsMap>
static void copyItemsInRange(ItemsMap& source, const int rangeStartTick, const int rangeEndTick, const int newItemsOffsetTick)
{
    auto startIt = source.lower_bound(rangeStartTick);
    if (startIt == source.end()) {
        return;
    }

    auto endIt = source.lower_bound(rangeEndTick);

    ItemsMap newItems;
    for (auto it = startIt; it != endIt; ++it) {
        int tick = it->first + newItemsOffsetTick;
        newItems.insert_or_assign(tick, it->second);
    }

    source.merge(std::move(newItems));
}

template<typename ItemsMap>
static void copyItemsInRange(std::map<track_idx_t, ItemsMap>& source, const track_idx_t trackFrom, const track_idx_t trackTo,
                             const int rangeStartTick, const int rangeEndTick, const int newItemsOffsetTick)
{
    for (auto it = source.lower_bound(trackFrom); it != source.end() && it->first < trackTo; ++it) {
        copyItemsInRange(it->second, rangeStartTick, rangeEndTick, newItemsOffsetTick);
    }
}

void PlaybackContext::handleMeasureRepeats(const std::vector<const MeasureRepeat*>& measureRepeats, const int tickPositionOffset)
{
    for (const MeasureRepeat* mr : measureRepeats) {
        const Part* part = mr->part();
        if (!part) {
            continue;
        }

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

        const TrackRange trackRange = part->trackRange();

        for (int num = 0; num < mr->numMeasures(); ++num) {
            int startTick = referringMeasure->tick().ticks() + tickPositionOffset;
            int endTick = referringMeasure->endTick().ticks() + tickPositionOffset;

            copyItemsInRange(m_soundPresetsByTrack, trackRange.startTrack, trackRange.endTrack, startTick, endTick, newItemsOffsetTick);
            copyItemsInRange(m_textArticulationsByTrack, trackRange.startTrack, trackRange.endTrack, startTick, endTick,
                             newItemsOffsetTick);
            copyItemsInRange(m_syllablesByTrack, trackRange.startTrack, trackRange.endTrack, startTick, endTick, newItemsOffsetTick);
            copyItemsInRange(m_playTechniquesByTrack, trackRange.startTrack, trackRange.endTrack, startTick, endTick,
                             newItemsOffsetTick);

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

const AutomationCurve* PlaybackContext::dynamicsCurve(const track_idx_t trackIdx) const
{
    auto cacheIt = m_dynamicsCurveByTrack.find(trackIdx);
    if (cacheIt != m_dynamicsCurveByTrack.end()) {
        return cacheIt->second;
    }

    const AutomationDataConstPtr automation = m_score ? m_score->automationData() : nullptr;
    const Staff* staff = automation ? m_score->staff(track2staff(trackIdx)) : nullptr;

    const AutomationCurve* curve = nullptr;
    if (staff) {
        AutomationCurveKey key;
        key.type = AutomationType::Dynamics;
        key.staffId = staff->id();
        key.voiceIdx = track2voice(trackIdx);

        curve = &automation->curve(key);
        if (curve->empty()) {
            key.voiceIdx.reset();
            curve = &automation->curve(key);
        }
    }

    m_dynamicsCurveByTrack.emplace(trackIdx, curve);
    return curve;
}

bool PlaybackContext::hasOnlyOneLyricsVerse(const RepeatSegment* repeat, const track_idx_t track) const
{
    if (m_multiVerseLyricsPositionMap.empty()) {
        return true;
    }

    const auto trackIt = m_multiVerseLyricsPositionMap.find(track);
    if (trackIt == m_multiVerseLyricsPositionMap.cend()) {
        return true;
    }

    const int startTick = repeat->tick;
    const int endTick = repeat->endTick();
    const auto start = trackIt->second.lower_bound(startTick);
    const auto end = trackIt->second.lower_bound(endTick);

    return start == end;
}
