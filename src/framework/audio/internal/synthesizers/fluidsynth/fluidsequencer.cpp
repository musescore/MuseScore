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

#include "fluidsequencer.h"

using namespace mu;
using namespace mu::audio;
using namespace mu::midi;
using namespace mu::mpe;

void FluidSequencer::init(const ArticulationMapping& mapping, const std::unordered_map<midi::channel_t, midi::Program>& channels)
{
    m_articulationMapping = mapping;
    m_channels = channels;
}

void FluidSequencer::updateOffStreamEvents(const mpe::PlaybackEventsMap& changes)
{
    m_offStreamEvents.clear();
    updatePlaybackEvents(m_offStreamEvents, changes);
    updateOffSequenceIterator();
}

void FluidSequencer::updateMainStreamEvents(const mpe::PlaybackEventsMap& changes)
{
    m_mainStreamEvents.clear();
    updatePlaybackEvents(m_mainStreamEvents, changes);
    updateMainSequenceIterator();
}

void FluidSequencer::updateDynamicChanges(const mpe::DynamicLevelMap& changes)
{
    m_dynamicEvents.clear();

    for (const auto& pair : changes) {
        midi::Event event(midi::Event::Opcode::ControlChange, Event::MessageType::ChannelVoice10);
        event.setIndex(11);
        event.setData(expressionLevel(pair.second));

        m_dynamicEvents[pair.first].emplace(std::move(event));
    }

    updateDynamicChangesIterator();
}

void FluidSequencer::updatePlaybackEvents(EventSequenceMap& destination, const mpe::PlaybackEventsMap& changes)
{
    for (const auto& pair : changes) {
        for (const mpe::PlaybackEvent& event : pair.second) {
            if (!std::holds_alternative<mpe::NoteEvent>(event)) {
                continue;
            }

            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);

            timestamp_t timestampFrom = noteEvent.arrangementCtx().actualTimestamp;
            timestamp_t timestampTo = timestampFrom + noteEvent.arrangementCtx().actualDuration;

            channel_t channelIdx = channel(noteEvent);
            note_idx_t noteIdx = noteIndex(noteEvent.pitchCtx().nominalPitchLevel);
            velocity_t velocity = noteVelocity(noteEvent);

            midi::Event noteOn(Event::Opcode::NoteOn, Event::MessageType::ChannelVoice10);
            noteOn.setChannel(channelIdx);
            noteOn.setNote(noteIdx);
            noteOn.setVelocity(velocity);

            destination[timestampFrom].emplace(std::move(noteOn));

            midi::Event noteOff(Event::Opcode::NoteOff, Event::MessageType::ChannelVoice10);
            noteOff.setChannel(channelIdx);
            noteOff.setNote(noteIdx);

            destination[timestampTo].emplace(std::move(noteOff));

            appendControlSwitch(destination, noteEvent, PEDAL_CC_SUPPORTED_TYPES, 64);
            appendPitchBend(destination, noteEvent, BEND_SUPPORTED_TYPES, channelIdx);
        }
    }
}

void FluidSequencer::appendControlSwitch(EventSequenceMap& destination, const mpe::NoteEvent& noteEvent,
                                         const mpe::ArticulationTypeSet& appliableTypes, const int midiControlIdx)
{
    mpe::ArticulationType currentType = mpe::ArticulationType::Undefined;

    for (const mpe::ArticulationType type : appliableTypes) {
        if (noteEvent.expressionCtx().articulations.contains(type)) {
            currentType = type;
            break;
        }
    }

    if (currentType != mpe::ArticulationType::Undefined) {
        const ArticulationAppliedData& articulationData = noteEvent.expressionCtx().articulations.at(currentType);
        const ArticulationMeta& articulationMeta = articulationData.meta;

        midi::Event start(Event::Opcode::ControlChange, Event::MessageType::ChannelVoice10);
        start.setIndex(midiControlIdx);
        start.setData(127);

        destination[articulationMeta.timestamp].emplace(std::move(start));

        midi::Event end(Event::Opcode::ControlChange, Event::MessageType::ChannelVoice10);
        end.setIndex(midiControlIdx);
        end.setData(0);

        destination[articulationMeta.timestamp + articulationMeta.overallDuration].emplace(std::move(end));
    } else {
        midi::Event cc(Event::Opcode::ControlChange, Event::MessageType::ChannelVoice10);
        cc.setIndex(midiControlIdx);
        cc.setData(0);

        destination[noteEvent.arrangementCtx().actualTimestamp].emplace(std::move(cc));
    }
}

void FluidSequencer::appendPitchBend(EventSequenceMap& destination, const mpe::NoteEvent& noteEvent,
                                     const mpe::ArticulationTypeSet& appliableTypes, const channel_t channelIdx)
{
    mpe::ArticulationType currentType = mpe::ArticulationType::Undefined;

    for (const mpe::ArticulationType type : appliableTypes) {
        if (noteEvent.expressionCtx().articulations.contains(type)) {
            currentType = type;
            break;
        }
    }

    timestamp_t timestampFrom = noteEvent.arrangementCtx().actualTimestamp;

    if (currentType != mpe::ArticulationType::Undefined) {
        for (const auto& pair : noteEvent.pitchCtx().pitchCurve) {
            midi::Event event(Event::Opcode::PitchBend, Event::MessageType::ChannelVoice10);
            event.setChannel(channelIdx);

            timestamp_t currentPoint = timestampFrom + noteEvent.arrangementCtx().actualDuration * percentageToFactor(pair.first);

            int pitchBendValue = 8192;

            if (pair.first == HUNDRED_PERCENT) {
                event.setData(pitchBendValue);
                destination[currentPoint].emplace(std::move(event));
                return;
            }

            float ratio = pair.second / static_cast<float>(ONE_PERCENT);
            ratio = std::clamp(ratio, -2.f, 2.f);

            pitchBendValue = 8192 + RealRound(ratio * 4096, 0);
            pitchBendValue = std::clamp(pitchBendValue, 0, 16383);

            event.setData(pitchBendValue);
            destination[currentPoint].emplace(std::move(event));
            return;
        }
    } else {
        midi::Event event(Event::Opcode::PitchBend, Event::MessageType::ChannelVoice10);
        event.setChannel(channelIdx);
        event.setData(8192);
        destination[timestampFrom].emplace(std::move(event));
    }
}

channel_t FluidSequencer::channel(const mpe::NoteEvent& noteEvent) const
{
    for (const auto& pair : m_articulationMapping) {
        if (noteEvent.expressionCtx().articulations.contains(pair.first)) {
            return findChannelByProgram(pair.second);
        }
    }

    return 0;
}

channel_t FluidSequencer::findChannelByProgram(const midi::Program& program) const
{
    for (const auto& pair : m_channels) {
        if (pair.second == program) {
            return pair.first;
        }
    }

    return 0;
}

note_idx_t FluidSequencer::noteIndex(const mpe::pitch_level_t pitchLevel) const
{
    static constexpr mpe::pitch_level_t MIN_SUPPORTED_LEVEL = mpe::pitchLevel(PitchClass::C, 0);
    static constexpr note_idx_t MIN_SUPPORTED_NOTE = 12; // MIDI equivalent for C0
    static constexpr mpe::pitch_level_t MAX_SUPPORTED_LEVEL = mpe::pitchLevel(PitchClass::C, 8);
    static constexpr note_idx_t MAX_SUPPORTED_NOTE = 108; // MIDI equivalent for C8

    if (pitchLevel <= MIN_SUPPORTED_LEVEL) {
        return MIN_SUPPORTED_NOTE;
    }

    if (pitchLevel >= MAX_SUPPORTED_LEVEL) {
        return MAX_SUPPORTED_NOTE;
    }

    float stepCount = MIN_SUPPORTED_NOTE + ((pitchLevel - MIN_SUPPORTED_LEVEL) / static_cast<float>(mpe::PITCH_LEVEL_STEP));

    return RealRound(stepCount, 0);
}

velocity_t FluidSequencer::noteVelocity(const mpe::NoteEvent& noteEvent) const
{
    static constexpr mpe::dynamic_level_t MIN_SUPPORTED_LEVEL = mpe::dynamicLevelFromType(DynamicType::ppp);
    static constexpr mpe::dynamic_level_t MAX_SUPPORTED_LEVEL = mpe::dynamicLevelFromType(DynamicType::fff);
    static constexpr midi::velocity_t NORMAL_SUPPORTED_VELOCITY = 63;
    static constexpr midi::velocity_t MAX_SUPPORTED_VELOCITY = 127;
    static constexpr midi::velocity_t VELOCITY_STEP = 16;

    static constexpr float MAX_VELOCITY_FACTOR = (MAX_SUPPORTED_LEVEL - MIN_SUPPORTED_LEVEL) / static_cast<float>(mpe::TEN_PERCENT);
    static constexpr float MIN_VELOCITY_FACTOR = 0.f;

    mpe::duration_percentage_t attackDuration = noteEvent.expressionCtx().expressionCurve.attackPhaseDuration();
    mpe::dynamic_level_t amplitude = noteEvent.expressionCtx().expressionCurve.maxAmplitudeLevel();

    if (attackDuration == 0) {
        return MAX_SUPPORTED_VELOCITY;
    }

    float velocityFactor = amplitude / static_cast<float>(attackDuration);

    if (RealIsEqualOrMore(velocityFactor, MAX_VELOCITY_FACTOR)) {
        return MAX_SUPPORTED_VELOCITY;
    }

    if (RealIsEqualOrLess(velocityFactor, MIN_VELOCITY_FACTOR)) {
        return NORMAL_SUPPORTED_VELOCITY;
    }

    velocity_t result = NORMAL_SUPPORTED_VELOCITY + (velocityFactor * VELOCITY_STEP);

    return std::clamp(result, NORMAL_SUPPORTED_VELOCITY, MAX_SUPPORTED_VELOCITY);
}

int FluidSequencer::expressionLevel(const mpe::dynamic_level_t dynamicLevel) const
{
    static constexpr mpe::dynamic_level_t MIN_SUPPORTED_LEVEL = mpe::dynamicLevelFromType(DynamicType::ppp);
    static constexpr mpe::dynamic_level_t MAX_SUPPORTED_LEVEL = mpe::dynamicLevelFromType(DynamicType::fff);
    static constexpr int MIN_SUPPORTED_VOLUME = 16; // MIDI equivalent for PPP
    static constexpr int MAX_SUPPORTED_VOLUME = 127; // MIDI equivalent for FFF
    static constexpr int VOLUME_STEP = 16;

    if (dynamicLevel <= MIN_SUPPORTED_LEVEL) {
        return MIN_SUPPORTED_VOLUME;
    }

    if (dynamicLevel >= MAX_SUPPORTED_LEVEL) {
        return MAX_SUPPORTED_VOLUME;
    }

    float stepCount = ((dynamicLevel - MIN_SUPPORTED_LEVEL) / static_cast<float>(mpe::DYNAMIC_LEVEL_STEP));

    if (dynamicLevel == mpe::dynamicLevelFromType(DynamicType::Natural)) {
        stepCount -= 0.5;
    }

    if (dynamicLevel > mpe::dynamicLevelFromType(DynamicType::Natural)) {
        stepCount -= 1;
    }

    dynamic_level_t result = RealRound(MIN_SUPPORTED_VOLUME + (stepCount * VOLUME_STEP), 0);

    return std::min(result, MAX_SUPPORTED_LEVEL);
}
