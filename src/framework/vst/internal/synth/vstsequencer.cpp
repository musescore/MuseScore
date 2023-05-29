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

    updateDynamicChanges(m_dynamicLevelMap);
    updateMainStreamEvents(m_playbackEventsMap);
}

void VstSequencer::updateOffStreamEvents(const mpe::PlaybackEventsMap& changes)
{
    m_offStreamEvents.clear();
    m_offStreamFlushed.notify();
    updatePlaybackEvents(m_offStreamEvents, changes);
    updateOffSequenceIterator();
}

void VstSequencer::updateMainStreamEvents(const mpe::PlaybackEventsMap& changes)
{
    m_mainStreamEvents.clear();
    m_mainStreamFlushed.notify();
    updatePlaybackEvents(m_mainStreamEvents, changes);
    updateMainSequenceIterator();
}

void VstSequencer::updateDynamicChanges(const mpe::DynamicLevelMap& changes)
{
    m_dynamicEvents.clear();

    for (const auto& pair : changes) {
        m_dynamicEvents[pair.first].emplace(expressionLevel(pair.second));
    }

    updateDynamicChangesIterator();
}

audio::gain_t VstSequencer::currentGain() const
{
    mpe::dynamic_level_t currentDynamicLevel = dynamicLevel(m_playbackPosition);
    return expressionLevel(currentDynamicLevel);
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

            if (noteEvent.arrangementCtx().actualDuration > 0) {
                destination[timestampFrom].emplace(buildEvent(VstEvent::kNoteOnEvent, noteId, velocityFraction, tuning));
            }

            destination[timestampTo].emplace(buildEvent(VstEvent::kNoteOffEvent, noteId, velocityFraction, tuning));

            appendControlSwitch(destination, noteEvent, PEDAL_CC_SUPPORTED_TYPES, SUSTAIN_IDX);
        }
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
