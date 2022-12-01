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

static constexpr mpe::pitch_level_t MIN_SUPPORTED_LEVEL = mpe::pitchLevel(PitchClass::C, 0);
static constexpr note_idx_t MIN_SUPPORTED_NOTE = 12; // MIDI equivalent for C0
static constexpr mpe::pitch_level_t MAX_SUPPORTED_LEVEL = mpe::pitchLevel(PitchClass::C, 8);
static constexpr note_idx_t MAX_SUPPORTED_NOTE = 108; // MIDI equivalent for C8

void FluidSequencer::init(const PlaybackSetupData& setupData)
{
    m_channels.init(setupData);
}

int FluidSequencer::currentExpressionLevel() const
{
    return expressionLevel(dynamicLevel(m_playbackPosition));
}

void FluidSequencer::updateOffStreamEvents(const mpe::PlaybackEventsMap& changes)
{
    m_offStreamEvents.clear();
    m_offStreamFlushed.notify();
    updatePlaybackEvents(m_offStreamEvents, changes);
    updateOffSequenceIterator();
}

void FluidSequencer::updateMainStreamEvents(const mpe::PlaybackEventsMap& changes)
{
    m_mainStreamEvents.clear();
    m_mainStreamFlushed.notify();
    updatePlaybackEvents(m_mainStreamEvents, changes);
    updateMainSequenceIterator();
}

void FluidSequencer::updateDynamicChanges(const mpe::DynamicLevelMap& changes)
{
    m_dynamicEvents.clear();

    for (const auto& pair : changes) {
        midi::Event event(midi::Event::Opcode::ControlChange, Event::MessageType::ChannelVoice10);
        event.setIndex(midi::EXPRESSION_CONTROLLER);
        event.setData(expressionLevel(pair.second));

        m_dynamicEvents[pair.first].emplace(std::move(event));
    }

    updateDynamicChangesIterator();
}

async::Channel<channel_t, Program> FluidSequencer::channelAdded() const
{
    return m_channels.channelAdded;
}

const ChannelMap& FluidSequencer::channels() const
{
    return m_channels;
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
            tuning_t tuning = noteTuning(noteEvent, noteIdx);

            midi::Event noteOn(Event::Opcode::NoteOn, Event::MessageType::ChannelVoice20);
            noteOn.setChannel(channelIdx);
            noteOn.setNote(noteIdx);
            noteOn.setVelocity(velocity);
            noteOn.setPitchNote(noteIdx, tuning);

            destination[timestampFrom].emplace(std::move(noteOn));

            midi::Event noteOff(Event::Opcode::NoteOff, Event::MessageType::ChannelVoice20);
            noteOff.setChannel(channelIdx);
            noteOff.setNote(noteIdx);
            noteOff.setPitchNote(noteIdx, tuning);

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

        destination[noteEvent.arrangementCtx().actualTimestamp].emplace(std::move(start));

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
    midi::Event event(Event::Opcode::PitchBend, Event::MessageType::ChannelVoice10);
    event.setChannel(channelIdx);

    if (currentType != mpe::ArticulationType::Undefined) {
        auto it = noteEvent.pitchCtx().pitchCurve.cbegin();
        auto last = noteEvent.pitchCtx().pitchCurve.cend();

        while (it != last) {
            auto nextToCurrent = std::next(it);
            if (nextToCurrent == last) {
                timestamp_t currentPoint = timestampFrom + noteEvent.arrangementCtx().actualDuration * percentageToFactor(it->first);

                event.setData(pitchBendLevel(it->second));
                destination[currentPoint].emplace(event);
                return;
            }

            percentage_t positionDistance = nextToCurrent->first - it->first;
            int stepsCount = positionDistance / (mpe::ONE_PERCENT * 5);
            float posStep = positionDistance / static_cast<float>(stepsCount);
            float pitchStep = (nextToCurrent->second - it->second) / static_cast<float>(stepsCount);

            for (int i = 0; i < stepsCount; ++i) {
                timestamp_t currentPoint = timestampFrom + noteEvent.arrangementCtx().actualDuration
                                           * percentageToFactor(it->first + (i * posStep));

                int pitchBendVal = pitchBendLevel(it->second + (i * pitchStep));
                event.setData(pitchBendVal);
                destination[currentPoint].emplace(event);
            }

            it++;
        }

        return;
    }

    event.setData(8192);
    destination[timestampFrom].emplace(std::move(event));
}

channel_t FluidSequencer::channel(const mpe::NoteEvent& noteEvent) const
{
    return m_channels.resolveChannelForEvent(noteEvent);
}

note_idx_t FluidSequencer::noteIndex(const mpe::pitch_level_t pitchLevel) const
{
    if (pitchLevel <= MIN_SUPPORTED_LEVEL) {
        return MIN_SUPPORTED_NOTE;
    }

    if (pitchLevel >= MAX_SUPPORTED_LEVEL) {
        return MAX_SUPPORTED_NOTE;
    }

    float stepCount = MIN_SUPPORTED_NOTE + ((pitchLevel - MIN_SUPPORTED_LEVEL) / static_cast<float>(mpe::PITCH_LEVEL_STEP));

    return stepCount;
}

tuning_t FluidSequencer::noteTuning(const mpe::NoteEvent& noteEvent, const int noteIdx) const
{
    int semitonesCount = noteIdx - MIN_SUPPORTED_NOTE;

    mpe::pitch_level_t tuningPitchLevel = noteEvent.pitchCtx().nominalPitchLevel - (semitonesCount * mpe::PITCH_LEVEL_STEP);

    return tuningPitchLevel / static_cast<float>(mpe::PITCH_LEVEL_STEP);
}

velocity_t FluidSequencer::noteVelocity(const mpe::NoteEvent& noteEvent) const
{
    static constexpr midi::velocity_t MAX_SUPPORTED_VELOCITY = 127;

    velocity_t result = RealRound(noteEvent.expressionCtx().expressionCurve.velocityFraction() * MAX_SUPPORTED_VELOCITY, 0);

    return std::clamp<velocity_t>(result, 0, MAX_SUPPORTED_VELOCITY);
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

int FluidSequencer::pitchBendLevel(const mpe::pitch_level_t pitchLevel) const
{
    static constexpr int PITCH_BEND_SEMITONE_STEP = 4096 / 12;

    float pitchLevelSteps = pitchLevel / static_cast<float>(mpe::PITCH_LEVEL_STEP);

    int offset = pitchLevelSteps * PITCH_BEND_SEMITONE_STEP;

    return std::clamp(8192 + offset, 0, 16383);
}
