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

#include "global/interpolation.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::midi;
using namespace muse::mpe;

static constexpr mpe::pitch_level_t MIN_SUPPORTED_PITCH_LEVEL = mpe::pitchLevel(PitchClass::C, 0);
static constexpr note_idx_t MIN_SUPPORTED_NOTE = 12; // MIDI equivalent for C0
static constexpr mpe::pitch_level_t MAX_SUPPORTED_PITCH_LEVEL = mpe::pitchLevel(PitchClass::C, 8);
static constexpr note_idx_t MAX_SUPPORTED_NOTE = 108; // MIDI equivalent for C8

void FluidSequencer::init(const PlaybackSetupData& setupData, const std::optional<midi::Program>& programOverride,
                          bool useDynamicEvents)
{
    m_channels.init(setupData, programOverride);
    m_useDynamicEvents = useDynamicEvents;
}

int FluidSequencer::currentExpressionLevel() const
{
    if (m_useDynamicEvents) {
        return expressionLevel(dynamicLevel(m_playbackPosition));
    }

    return naturalExpressionLevel();
}

int FluidSequencer::naturalExpressionLevel() const
{
    static const int NATURAL_EXP_LVL = expressionLevel(dynamicLevelFromType(DynamicType::Natural));
    return NATURAL_EXP_LVL;
}

void FluidSequencer::updateOffStreamEvents(const mpe::PlaybackEventsMap& events, const PlaybackParamList&)
{
    m_offStreamEvents.clear();

    if (m_onOffStreamFlushed) {
        m_onOffStreamFlushed();
    }

    updatePlaybackEvents(m_offStreamEvents, events);
    updateOffSequenceIterator();
}

void FluidSequencer::updateMainStreamEvents(const mpe::PlaybackEventsMap& events, const mpe::DynamicLevelLayers& dynamics,
                                            const mpe::PlaybackParamLayers&)
{
    m_mainStreamEvents.clear();
    m_dynamicEvents.clear();

    if (m_onMainStreamFlushed) {
        m_onMainStreamFlushed();
    }

    updatePlaybackEvents(m_mainStreamEvents, events);
    updateMainSequenceIterator();

    if (m_useDynamicEvents) {
        updateDynamicEvents(m_dynamicEvents, dynamics);
        updateDynamicChangesIterator();
    }
}

muse::async::Channel<channel_t, Program> FluidSequencer::channelAdded() const
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

            appendControlSwitch(destination, noteEvent, PEDAL_CC_SUPPORTED_TYPES, midi::SUSTAIN_PEDAL_CONTROLLER);
            appendPitchBend(destination, noteEvent, BEND_SUPPORTED_TYPES, channelIdx);
        }
    }
}

void FluidSequencer::updateDynamicEvents(EventSequenceMap& destination, const mpe::DynamicLevelLayers& changes)
{
    for (const auto& layer : changes) {
        for (const auto& dynamic : layer.second) {
            midi::Event event(muse::midi::Event::Opcode::ControlChange, Event::MessageType::ChannelVoice10);
            event.setIndex(midi::EXPRESSION_CONTROLLER);
            event.setData(expressionLevel(dynamic.second));

            destination[dynamic.first].emplace(std::move(event));
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

    if (currentType == mpe::ArticulationType::Undefined) {
        return;
    }

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
}

void FluidSequencer::appendPitchBend(EventSequenceMap& destination, const mpe::NoteEvent& noteEvent,
                                     const mpe::ArticulationTypeSet& appliableTypes, const channel_t channelIdx)
{
    if (noteEvent.pitchCtx().pitchCurve.empty()) {
        return;
    }

    timestamp_t pitchBendTimestampFrom = 0;
    duration_t pitchBendDuration = 0;

    for (const auto& art : noteEvent.expressionCtx().articulations) {
        if (muse::contains(appliableTypes, art.first)) {
            const ArticulationMeta& articulationMeta = art.second.meta;
            pitchBendTimestampFrom = articulationMeta.timestamp;
            pitchBendDuration = articulationMeta.overallDuration;
            break;
        }
    }

    if (pitchBendDuration == 0) {
        return;
    }

    const timestamp_t noteTimestampTo = noteEvent.arrangementCtx().actualTimestamp + noteEvent.arrangementCtx().actualDuration;
    const timestamp_t pitchBendTimestampTo = std::min(pitchBendTimestampFrom + pitchBendDuration, noteTimestampTo);

    midi::Event event(Event::Opcode::PitchBend, Event::MessageType::ChannelVoice10);
    event.setChannel(channelIdx);
    event.setData(8192);
    destination[pitchBendTimestampTo].insert(event);

    auto currIt = noteEvent.pitchCtx().pitchCurve.cbegin();
    auto nextIt = std::next(currIt);
    auto endIt = noteEvent.pitchCtx().pitchCurve.cend();

    auto makePoint = [](mpe::timestamp_t time, int value) {
        return Interpolation::Point { static_cast<double>(time), static_cast<double>(value) };
    };

    for (; nextIt != endIt; currIt = nextIt, nextIt = std::next(currIt)) {
        int currBendValue = pitchBendLevel(currIt->second);
        int nextBendValue = pitchBendLevel(nextIt->second);

        timestamp_t currTime = pitchBendTimestampFrom + pitchBendDuration * percentageToFactor(currIt->first);
        timestamp_t nextTime = pitchBendTimestampFrom + pitchBendDuration * percentageToFactor(nextIt->first);

        Interpolation::Point p0 = makePoint(currTime, currBendValue);
        Interpolation::Point p1 = makePoint(nextTime, currBendValue);
        Interpolation::Point p2 = makePoint(nextTime, nextBendValue);

        //! NOTE: Increasing this number results in fewer points being interpolated
        constexpr mpe::pitch_level_t POINT_WEIGHT = mpe::PITCH_LEVEL_STEP / 5;
        size_t pointCount = std::abs(nextIt->second - currIt->second) / POINT_WEIGHT;
        pointCount = std::max(pointCount, size_t(1));

        std::vector<Interpolation::Point> points = Interpolation::quadraticBezierCurve(p0, p1, p2, pointCount);

        for (const Interpolation::Point& point : points) {
            timestamp_t time = static_cast<timestamp_t>(std::round(point.x));
            int bendValue = static_cast<int>(std::round(point.y));

            if (time < pitchBendTimestampTo) {
                event.setData(bendValue);
                destination[time].insert(event);
            }
        }
    }
}

channel_t FluidSequencer::channel(const mpe::NoteEvent& noteEvent) const
{
    return m_channels.resolveChannelForEvent(noteEvent);
}

note_idx_t FluidSequencer::noteIndex(const mpe::pitch_level_t pitchLevel) const
{
    if (pitchLevel <= MIN_SUPPORTED_PITCH_LEVEL) {
        return MIN_SUPPORTED_NOTE;
    }

    if (pitchLevel >= MAX_SUPPORTED_PITCH_LEVEL) {
        return MAX_SUPPORTED_NOTE;
    }

    float stepCount = MIN_SUPPORTED_NOTE
                      + ((pitchLevel - MIN_SUPPORTED_PITCH_LEVEL)
                         / static_cast<float>(mpe::PITCH_LEVEL_STEP));

    return stepCount;
}

tuning_t FluidSequencer::noteTuning(const mpe::NoteEvent& noteEvent, const int noteIdx) const
{
    int semitonesCount = noteIdx - MIN_SUPPORTED_NOTE;

    mpe::pitch_level_t tuningPitchLevel = noteEvent.pitchCtx().nominalPitchLevel
                                          - (semitonesCount * mpe::PITCH_LEVEL_STEP);

    return tuningPitchLevel / static_cast<float>(mpe::PITCH_LEVEL_STEP);
}

velocity_t FluidSequencer::noteVelocity(const mpe::NoteEvent& noteEvent) const
{
    static constexpr midi::velocity_t MAX_SUPPORTED_VELOCITY = 127;

    const mpe::ExpressionContext& expressionCtx = noteEvent.expressionCtx();

    if (expressionCtx.velocityOverride.has_value()) {
        velocity_t velocity = RealRound(expressionCtx.velocityOverride.value() * MAX_SUPPORTED_VELOCITY, 0);
        return std::clamp<velocity_t>(velocity, 0, MAX_SUPPORTED_VELOCITY);
    }

    if (m_useDynamicEvents) {
        velocity_t result = RealRound(expressionCtx.expressionCurve.velocityFraction() * MAX_SUPPORTED_VELOCITY, 0);
        return std::clamp<velocity_t>(result, 0, MAX_SUPPORTED_VELOCITY);
    }

    dynamic_level_t dynamicLevel = expressionCtx.expressionCurve.maxAmplitudeLevel();
    return expressionLevel(dynamicLevel);
}

int FluidSequencer::expressionLevel(const mpe::dynamic_level_t dynamicLevel) const
{
    static constexpr mpe::dynamic_level_t MIN_SUPPORTED_DYNAMICS_LEVEL = mpe::dynamicLevelFromType(DynamicType::ppp);
    static constexpr mpe::dynamic_level_t MAX_SUPPORTED_DYNAMICS_LEVEL = mpe::dynamicLevelFromType(DynamicType::fff);
    static constexpr int MIN_SUPPORTED_VOLUME = 16; // MIDI equivalent for PPP
    static constexpr int MAX_SUPPORTED_VOLUME = 127; // MIDI equivalent for FFF
    static constexpr int VOLUME_STEP = 16;

    if (dynamicLevel <= MIN_SUPPORTED_DYNAMICS_LEVEL) {
        return MIN_SUPPORTED_VOLUME;
    }

    if (dynamicLevel >= MAX_SUPPORTED_DYNAMICS_LEVEL) {
        return MAX_SUPPORTED_VOLUME;
    }

    float stepCount = ((dynamicLevel - MIN_SUPPORTED_DYNAMICS_LEVEL)
                       / static_cast<float>(mpe::DYNAMIC_LEVEL_STEP));

    if (dynamicLevel == mpe::dynamicLevelFromType(DynamicType::Natural)) {
        stepCount -= 0.5;
    }

    dynamic_level_t result = RealRound(MIN_SUPPORTED_VOLUME + (stepCount * VOLUME_STEP), 0);

    return std::min(result, MAX_SUPPORTED_DYNAMICS_LEVEL);
}

int FluidSequencer::pitchBendLevel(const mpe::pitch_level_t pitchLevel) const
{
    static constexpr int PITCH_BEND_SEMITONE_STEP = 4096 / 12;

    float pitchLevelSteps = pitchLevel / static_cast<float>(mpe::PITCH_LEVEL_STEP);

    int offset = pitchLevelSteps * PITCH_BEND_SEMITONE_STEP;

    return std::clamp(8192 + offset, 0, 16383);
}
