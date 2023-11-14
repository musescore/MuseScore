/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#include "vstsequencer.h"

using namespace mu;
using namespace mu::vst;

static constexpr ControllIdx SUSTAIN_IDX = static_cast<ControllIdx>(Steinberg::Vst::kCtrlSustainOnOff);
static const mpe::ArticulationTypeSet PEDAL_CC_SUPPORTED_TYPES { mpe::ArticulationType::Pedal };

static constexpr mpe::pitch_level_t MIN_SUPPORTED_PITCH_LEVEL = mpe::pitchLevel(mpe::PitchClass::C, 0);
static constexpr int MIN_SUPPORTED_NOTE = 12; // VST equivalent for C0
static constexpr mpe::pitch_level_t MAX_SUPPORTED_PITCH_LEVEL = mpe::pitchLevel(mpe::PitchClass::C, 8);
static constexpr int MAX_SUPPORTED_NOTE = 108; // VST equivalent for C8

void VstSequencer::init(ParamsMapping&& mapping)
{
    m_mapping = std::move(mapping);

    updateMainStreamEvents(m_eventsMap, m_dynamicLevelLayers);
}

void VstSequencer::updateOffStreamEvents(const mpe::PlaybackEventsMap& events)
{
    m_offStreamEvents.clear();

    if (m_onOffStreamFlushed) {
        m_onOffStreamFlushed();
    }

    updatePlaybackEvents(m_offStreamEvents, events);
    updateOffSequenceIterator();
}

void VstSequencer::updateMainStreamEvents(const mpe::PlaybackEventsMap& events, const mpe::DynamicLevelLayers& dynamics)
{
    m_eventsMap = events;
    m_dynamicLevelLayers = dynamics;

    m_mainStreamEvents.clear();
    m_dynamicEvents.clear();

    if (m_onMainStreamFlushed) {
        m_onMainStreamFlushed();
    }

    updatePlaybackEvents(m_mainStreamEvents, events);
    updateMainSequenceIterator();

    updateDynamicEvents(m_dynamicEvents, dynamics);
    updateDynamicChangesIterator();
}

audio::gain_t VstSequencer::currentGain() const
{
    auto seqIt = findLessOrEqual(m_dynamicEvents, m_playbackPosition);
    if (seqIt == m_dynamicEvents.end()) {
        return expressionLevel(mpe::dynamicLevelFromType(mpe::DynamicType::Natural));
    }

    const EventSequence& seq = seqIt->second;
    if (seq.empty()) {
        return expressionLevel(mpe::dynamicLevelFromType(mpe::DynamicType::Natural));
    }

    return std::get<audio::gain_t>(*seq.begin());
}

void VstSequencer::updatePlaybackEvents(EventSequenceMap& destination, const mpe::PlaybackEventsMap& changes)
{
    for (const auto& pair : changes) {
        for (const mpe::PlaybackEvent& event : pair.second) {
            if (!std::holds_alternative<mpe::NoteEvent>(event)) {
                continue;
            }

            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);

            mpe::timestamp_t timestampFrom = noteEvent.arrangementCtx().actualTimestamp;
            mpe::timestamp_t timestampTo = timestampFrom + noteEvent.arrangementCtx().actualDuration;

            int32_t noteId = noteIndex(noteEvent.pitchCtx().nominalPitchLevel);
            float velocityFraction = noteVelocityFraction(noteEvent);
            float tuning = noteTuning(noteEvent, noteId);

            destination[timestampFrom].emplace(buildEvent(VstEvent::kNoteOnEvent, noteId, velocityFraction, tuning));
            destination[timestampTo].emplace(buildEvent(VstEvent::kNoteOffEvent, noteId, velocityFraction, tuning));

            appendControlSwitch(destination, noteEvent, PEDAL_CC_SUPPORTED_TYPES, SUSTAIN_IDX);
        }
    }
}

void VstSequencer::updateDynamicEvents(EventSequenceMap& destination, const mpe::DynamicLevelLayers& dynamics)
{
    mpe::DynamicLevelMap dynamicsMap;
    constexpr mpe::dynamic_level_t NATURAL_LEVEL = mpe::dynamicLevelFromType(mpe::DynamicType::Natural);

    for (const auto& layer : dynamics) {
        for (const auto& pair : layer.second) {
            auto dynamicIt = dynamicsMap.find(pair.first);

            if (dynamicIt == dynamicsMap.end()) {
                dynamicsMap.emplace(pair.first, pair.second);
            } else if (dynamicIt->second != pair.second && pair.second != NATURAL_LEVEL) {
                dynamicsMap.insert_or_assign(pair.first, pair.second);
            }
        }
    }

    for (const auto& pair : dynamicsMap) {
        destination[pair.first].emplace(expressionLevel(pair.second));
    }
}

void VstSequencer::appendControlSwitch(EventSequenceMap& destination, const mpe::NoteEvent& noteEvent,
                                       const mpe::ArticulationTypeSet& appliableTypes, const ControllIdx controlIdx)
{
    auto controlIt = m_mapping.find(controlIdx);
    if (controlIt == m_mapping.cend()) {
        return;
    }

    mpe::ArticulationType currentType = mpe::ArticulationType::Undefined;

    for (const mpe::ArticulationType type : appliableTypes) {
        if (noteEvent.expressionCtx().articulations.contains(type)) {
            currentType = type;
            break;
        }
    }

    if (currentType != mpe::ArticulationType::Undefined) {
        const mpe::ArticulationAppliedData& articulationData = noteEvent.expressionCtx().articulations.at(currentType);
        const mpe::ArticulationMeta& articulationMeta = articulationData.meta;

        destination[noteEvent.arrangementCtx().actualTimestamp].emplace(buildParamInfo(controlIt->second, 1 /*on*/));
        destination[articulationMeta.timestamp + articulationMeta.overallDuration].emplace(buildParamInfo(controlIt->second, 0 /*off*/));
    } else {
        destination[noteEvent.arrangementCtx().actualTimestamp].emplace(buildParamInfo(controlIt->second, 0 /*off*/));
    }
}

VstEvent VstSequencer::buildEvent(const VstEvent::EventTypes type, const int32_t noteIdx, const float velocityFraction,
                                  const float tuning) const
{
    VstEvent result;

    result.busIndex = 0;
    result.sampleOffset = 0;
    result.ppqPosition = 0;
    result.flags = VstEvent::kIsLive;
    result.type = type;

    if (type == VstEvent::kNoteOnEvent) {
        result.noteOn.noteId = -1;
        result.noteOn.channel = 0;
        result.noteOn.pitch = noteIdx;
        result.noteOn.tuning = tuning;
        result.noteOn.velocity = velocityFraction;
    } else {
        result.noteOff.noteId = -1;
        result.noteOff.channel = 0;
        result.noteOff.pitch = noteIdx;
        result.noteOff.tuning = tuning;
        result.noteOff.velocity = velocityFraction;
    }

    return result;
}

PluginParamInfo VstSequencer::buildParamInfo(const PluginParamId id, const PluginParamValue value) const
{
    PluginParamInfo info;
    info.id = id;
    info.defaultNormalizedValue = value;

    return info;
}

int32_t VstSequencer::noteIndex(const mpe::pitch_level_t pitchLevel) const
{
    if (pitchLevel <= MIN_SUPPORTED_PITCH_LEVEL) {
        return MIN_SUPPORTED_NOTE;
    }

    if (pitchLevel >= MAX_SUPPORTED_PITCH_LEVEL) {
        return MAX_SUPPORTED_NOTE;
    }

    float stepCount = MIN_SUPPORTED_NOTE + ((pitchLevel - MIN_SUPPORTED_PITCH_LEVEL) / static_cast<float>(mpe::PITCH_LEVEL_STEP));

    return stepCount;
}

float VstSequencer::noteTuning(const mpe::NoteEvent& noteEvent, const int noteIdx) const
{
    int semitonesCount = noteIdx - MIN_SUPPORTED_NOTE;

    mpe::pitch_level_t tuningPitchLevel = noteEvent.pitchCtx().nominalPitchLevel - (semitonesCount * mpe::PITCH_LEVEL_STEP);

    return (tuningPitchLevel / static_cast<float>(mpe::PITCH_LEVEL_STEP)) * 100.f;
}

float VstSequencer::noteVelocityFraction(const mpe::NoteEvent& noteEvent) const
{
    return std::clamp(noteEvent.expressionCtx().expressionCurve.velocityFraction(), 0.f, 1.f);
}

float VstSequencer::expressionLevel(const mpe::dynamic_level_t dynamicLevel) const
{
    static constexpr mpe::dynamic_level_t MIN_SUPPORTED_DYNAMIC_LEVEL = mpe::dynamicLevelFromType(mpe::DynamicType::ppp);
    static constexpr mpe::dynamic_level_t MAX_SUPPORTED_DYNAMIC_LEVEL = mpe::dynamicLevelFromType(mpe::DynamicType::fff);
    static constexpr mpe::dynamic_level_t AVAILABLE_RANGE = MAX_SUPPORTED_DYNAMIC_LEVEL - MIN_SUPPORTED_DYNAMIC_LEVEL;

    if (dynamicLevel <= MIN_SUPPORTED_DYNAMIC_LEVEL) {
        return (0.5f * mpe::ONE_PERCENT) / AVAILABLE_RANGE;
    }

    if (dynamicLevel >= MAX_SUPPORTED_DYNAMIC_LEVEL) {
        return 1.f;
    }

    return RealRound((dynamicLevel - MIN_SUPPORTED_DYNAMIC_LEVEL) / static_cast<float>(AVAILABLE_RANGE), 2);
}
