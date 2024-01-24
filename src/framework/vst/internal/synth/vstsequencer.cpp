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

#include "global/interpolation.h"

using namespace mu;
using namespace mu::vst;

static constexpr ControllIdx SUSTAIN_IDX = static_cast<ControllIdx>(Steinberg::Vst::kCtrlSustainOnOff);
static constexpr ControllIdx PITCH_BEND_IDX = static_cast<ControllIdx>(Steinberg::Vst::kPitchBend);

static const mpe::ArticulationTypeSet PEDAL_CC_SUPPORTED_TYPES {
    mpe::ArticulationType::Pedal,
};

static const mpe::ArticulationTypeSet BEND_SUPPORTED_TYPES {
    mpe::ArticulationType::Multibend,
};

static constexpr mpe::pitch_level_t MIN_SUPPORTED_PITCH_LEVEL = mpe::pitchLevel(mpe::PitchClass::C, 0);
static constexpr int MIN_SUPPORTED_NOTE = 12; // VST equivalent for C0
static constexpr mpe::pitch_level_t MAX_SUPPORTED_PITCH_LEVEL = mpe::pitchLevel(mpe::PitchClass::C, 8);
static constexpr int MAX_SUPPORTED_NOTE = 108; // VST equivalent for C8

void VstSequencer::init(ParamsMapping&& mapping)
{
    m_mapping = std::move(mapping);
    m_inited = true;

    updateMainStreamEvents(m_playbackEventsMap, m_dynamicLevelMap, m_playbackParamsMap);

    m_playbackEventsMap.clear();
    m_playbackParamsMap.clear();
}

void VstSequencer::updateOffStreamEvents(const mpe::PlaybackEventsMap& events, const mpe::PlaybackParamMap& params)
{
    m_offStreamEvents.clear();

    if (m_onOffStreamFlushed) {
        m_onOffStreamFlushed();
    }

    updatePlaybackEvents(m_offStreamEvents, events, params);
    updateOffSequenceIterator();
}

void VstSequencer::updateMainStreamEvents(const mpe::PlaybackEventsMap& events, const mpe::DynamicLevelMap& dynamics,
                                          const mpe::PlaybackParamMap& params)
{
    m_dynamicLevelMap = dynamics;

    if (!m_inited) {
        m_playbackEventsMap = events;
        m_playbackParamsMap = params;
        return;
    }

    m_mainStreamEvents.clear();
    m_dynamicEvents.clear();

    if (m_onMainStreamFlushed) {
        m_onMainStreamFlushed();
    }

    updatePlaybackEvents(m_mainStreamEvents, events, params);
    updateMainSequenceIterator();

    updateDynamicEvents(m_dynamicEvents, dynamics);
    updateDynamicChangesIterator();
}

audio::gain_t VstSequencer::currentGain() const
{
    mpe::dynamic_level_t currentDynamicLevel = dynamicLevel(m_playbackPosition);
    return expressionLevel(currentDynamicLevel);
}

void VstSequencer::updatePlaybackEvents(EventSequenceMap& destination, const mpe::PlaybackEventsMap& events,
                                        const mpe::PlaybackParamMap& params)
{
    appendKeySwitches(destination, params);

    for (const auto& pair : events) {
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
            appendPitchBend(destination, noteEvent, BEND_SUPPORTED_TYPES);
        }
    }
}

void VstSequencer::updateDynamicEvents(EventSequenceMap& destination, const mpe::DynamicLevelMap& dynamics)
{
    for (const auto& pair : dynamics) {
        destination[pair.first].emplace(expressionLevel(pair.second));
    }
}

void VstSequencer::appendKeySwitches(EventSequenceMap& destination, const mpe::PlaybackParamMap& params)
{
    for (const auto& pair : params) {
        for (const mpe::PlaybackParam& param : pair.second) {
            if (param.code != audio::KEYSWITCH_PARAM_CODE) {
                continue;
            }

            destination[pair.first].emplace(buildEvent(VstEvent::kNoteOnEvent, param.val.toInt(), 64.f, 0.f));
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

void VstSequencer::appendPitchBend(EventSequenceMap& destination, const mpe::NoteEvent& noteEvent,
                                   const mpe::ArticulationTypeSet& appliableTypes)
{
    auto pitchBendIt = m_mapping.find(PITCH_BEND_IDX);
    if (pitchBendIt == m_mapping.cend()) {
        return;
    }

    mpe::ArticulationType currentType = mpe::ArticulationType::Undefined;

    for (const mpe::ArticulationType type : appliableTypes) {
        if (noteEvent.expressionCtx().articulations.contains(type)) {
            currentType = type;
            break;
        }
    }

    mpe::timestamp_t timestampFrom = noteEvent.arrangementCtx().actualTimestamp;

    PluginParamInfo event;
    event.id = pitchBendIt->second;

    if (currentType == mpe::ArticulationType::Undefined || noteEvent.pitchCtx().pitchCurve.empty()) {
        event.defaultNormalizedValue = 0.5f;
        destination[timestampFrom].emplace(std::move(event));
        return;
    }

    mpe::duration_t duration = noteEvent.arrangementCtx().actualDuration;

    auto currIt = noteEvent.pitchCtx().pitchCurve.cbegin();
    auto nextIt = std::next(currIt);
    auto endIt = noteEvent.pitchCtx().pitchCurve.cend();

    if (nextIt == endIt) {
        mpe::timestamp_t time = timestampFrom + duration * mpe::percentageToFactor(currIt->first);
        event.defaultNormalizedValue = pitchBendLevel(currIt->second);
        destination[time].insert(std::move(event));
        return;
    }

    auto makePoint = [](mpe::timestamp_t time, float value) {
        return mu::Interpolation::Point { static_cast<double>(time), value };
    };

    for (; nextIt != endIt; currIt = nextIt, nextIt = std::next(currIt)) {
        float currBendValue = pitchBendLevel(currIt->second);
        float nextBendValue = pitchBendLevel(nextIt->second);

        mpe::timestamp_t currTime = timestampFrom + duration * mpe::percentageToFactor(currIt->first);
        mpe::timestamp_t nextTime = timestampFrom + duration * mpe::percentageToFactor(nextIt->first);

        mu::Interpolation::Point p0 = makePoint(currTime, currBendValue);
        mu::Interpolation::Point p1 = makePoint(nextTime, currBendValue);
        mu::Interpolation::Point p2 = makePoint(nextTime, nextBendValue);

        //! NOTE: Increasing this number results in fewer points being interpolated
        constexpr mpe::pitch_level_t POINT_WEIGHT = mpe::PITCH_LEVEL_STEP / 5;
        size_t pointCount = std::abs(nextIt->second - currIt->second) / POINT_WEIGHT;
        pointCount = std::max(pointCount, size_t(1));

        std::vector<mu::Interpolation::Point> points = mu::Interpolation::quadraticBezierCurve(p0, p1, p2, pointCount);

        for (const mu::Interpolation::Point& point : points) {
            mpe::timestamp_t time = static_cast<mpe::timestamp_t>(std::round(point.x));
            float bendValue = static_cast<float>(point.y);
            event.defaultNormalizedValue = bendValue;
            destination[time].insert(event);
        }
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

float VstSequencer::pitchBendLevel(const mpe::pitch_level_t pitchLevel) const
{
    static constexpr float SEMITONE_RANGE = 2.f;
    static constexpr float PITCH_BEND_SEMITONE_STEP = 0.5f / SEMITONE_RANGE;

    float pitchLevelSteps = pitchLevel / static_cast<float>(mpe::PITCH_LEVEL_STEP);
    float offset = pitchLevelSteps * PITCH_BEND_SEMITONE_STEP;

    return std::clamp(0.5f + offset, 0.f, 1.f);
}
