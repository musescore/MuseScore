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

static constexpr uint32_t CTRL_ON = 127;
static constexpr uint32_t CTRL_OFF = 0;

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

void FluidSequencer::updateOffStreamEvents(const mpe::PlaybackEventsMap& events, const mpe::DynamicLevelLayers& dynamics,
                                           const PlaybackParamList&)
{
    addPlaybackEvents(m_offStreamEvents, events);

    if (m_useDynamicEvents) {
        addDynamicEvents(m_offStreamEvents, dynamics);
    }

    updateOffSequenceIterator();
}

void FluidSequencer::updateMainStreamEvents(const mpe::PlaybackEventsMap& events, const mpe::DynamicLevelLayers& dynamics,
                                            const mpe::PlaybackParamLayers&)
{
    m_mainStreamEvents.clear();

    if (m_onMainStreamFlushed) {
        m_onMainStreamFlushed();
    }

    addPlaybackEvents(m_mainStreamEvents, events);

    if (m_useDynamicEvents) {
        addDynamicEvents(m_mainStreamEvents, dynamics);
    }

    updateMainSequenceIterator();
}

muse::async::Channel<channel_t, Program> FluidSequencer::channelAdded() const
{
    return m_channels.channelAdded;
}

const ChannelMap& FluidSequencer::channels() const
{
    return m_channels;
}

int FluidSequencer::lastStaff() const
{
    return m_lastStaff;
}

void FluidSequencer::addPlaybackEvents(EventSequenceMap& destination, const mpe::PlaybackEventsMap& events)
{
    SostenutoTimeAndDurations sostenutoTimeAndDurations;

    for (const auto& pair : events) {
        for (const mpe::PlaybackEvent& event : pair.second) {
            if (std::holds_alternative<mpe::NoteEvent>(event)) {
                addNoteEvent(destination, std::get<mpe::NoteEvent>(event), sostenutoTimeAndDurations);
            } else if (std::holds_alternative<mpe::ControllerChangeEvent>(event)) {
                addControlChangeEvent(destination, pair.first, std::get<mpe::ControllerChangeEvent>(event));
            }
        }
    }

    addSostenutoEvents(destination, sostenutoTimeAndDurations);
}

void FluidSequencer::addDynamicEvents(EventSequenceMap& destination, const mpe::DynamicLevelLayers& dynamics)
{
    for (const auto& layer : dynamics) {
        for (const auto& dynamic : layer.second) {
            midi::Event event(muse::midi::Event::Opcode::ControlChange, Event::MessageType::ChannelVoice10);
            event.setIndex(midi::EXPRESSION_CONTROLLER);
            event.setData(expressionLevel(dynamic.second));

            destination[dynamic.first].emplace(std::move(event));
        }
    }
}

void FluidSequencer::addNoteEvent(EventSequenceMap& destination, const mpe::NoteEvent& noteEvent,
                                  SostenutoTimeAndDurations& sostenutoTimeAndDurations)
{
    const ArrangementContext& arrangementCtx = noteEvent.arrangementCtx();
    const channel_t channelIdx = channel(noteEvent);
    const note_idx_t noteIdx = noteIndex(noteEvent.pitchCtx().nominalPitchLevel);
    const velocity_t velocity = noteVelocity(noteEvent);
    const tuning_t tuning = noteTuning(noteEvent, noteIdx);

    // Assumption: 1:1 mapping between staff and instrument and FluidSynth and FluidSequencer instances.
    // So staffLayerIndex is constant. It changes only when moving staffs.
    m_lastStaff = noteEvent.arrangementCtx().staffLayerIndex;

    if (arrangementCtx.hasStart()) {
        midi::Event noteOn(Event::Opcode::NoteOn, Event::MessageType::ChannelVoice20);
        noteOn.setChannel(channelIdx);
        noteOn.setNote(noteIdx);
        noteOn.setVelocity16(velocity);
        noteOn.setPitchNote(noteIdx, tuning);

        destination[arrangementCtx.actualTimestamp].emplace(std::move(noteOn));
    }

    if (arrangementCtx.hasEnd()) {
        midi::Event noteOff(Event::Opcode::NoteOff, Event::MessageType::ChannelVoice20);
        noteOff.setChannel(channelIdx);
        noteOff.setNote(noteIdx);
        noteOff.setPitchNote(noteIdx, tuning);

        const timestamp_t timestampTo = arrangementCtx.actualTimestamp + noteEvent.arrangementCtx().actualDuration;
        destination[timestampTo].emplace(std::move(noteOff));
    }

    for (const auto& artPair : noteEvent.expressionCtx().articulations) {
        const mpe::ArticulationMeta& meta = artPair.second.meta;

        if (muse::contains(BEND_SUPPORTED_TYPES, meta.type)) {
            addPitchCurve(destination, noteEvent, meta, channelIdx);
            continue;
        }

        if (muse::contains(SUSTAIN_PEDAL_CC_SUPPORTED_TYPES, meta.type)) {
            addControlChange(destination, meta.timestamp, midi::SUSTAIN_PEDAL_CONTROLLER, channelIdx, CTRL_ON);
            addControlChange(destination, meta.timestamp + meta.overallDuration,
                             midi::SUSTAIN_PEDAL_CONTROLLER, channelIdx, CTRL_OFF);
            continue;
        }

        if (muse::contains(SOSTENUTO_PEDAL_CC_SUPPORTED_TYPES, meta.type)) {
            const mpe::timestamp_t timestamp = arrangementCtx.actualTimestamp + arrangementCtx.actualDuration * 0.1; // add offset for Sostenuto to take effect
            sostenutoTimeAndDurations[channelIdx].push_back(TimestampAndDuration { timestamp, meta.overallDuration });
            continue;
        }
    }
}

void FluidSequencer::addControlChangeEvent(EventSequenceMap& destination, const mpe::timestamp_t timestamp,
                                           const mpe::ControllerChangeEvent& event)
{
    const channel_t lastChannelIdx = m_channels.lastIndex();

    for (channel_t channelIdx = 0; channelIdx < lastChannelIdx; ++channelIdx) {
        switch (event.type) {
        case mpe::ControllerChangeEvent::Modulation:
            addControlChange(destination, timestamp, midi::MODWHEEL_CONTROLLER, channelIdx,
                             static_cast<uint32_t>(event.val * 127.f));
            break;
        case mpe::ControllerChangeEvent::SustainPedalOnOff:
            addControlChange(destination, timestamp, midi::SUSTAIN_PEDAL_CONTROLLER, channelIdx,
                             static_cast<uint32_t>(event.val * 127.f));
            break;
        case mpe::ControllerChangeEvent::PitchBend:
            addPitchBend(destination, timestamp, channelIdx,
                         static_cast<uint32_t>(event.val * 16383.f));
            break;
        case mpe::ControllerChangeEvent::Undefined:
            break;
        }
    }
}

void FluidSequencer::addControlChange(EventSequenceMap& destination, const mpe::timestamp_t timestamp,
                                      const int midiControlIdx, const channel_t channelIdx, const uint32_t value)
{
    midi::Event cc(Event::Opcode::ControlChange, Event::MessageType::ChannelVoice10);
    cc.setIndex(midiControlIdx);
    cc.setChannel(channelIdx);
    cc.setData(value);

    destination[timestamp].emplace(std::move(cc));
}

void FluidSequencer::addPitchCurve(EventSequenceMap& destination, const mpe::NoteEvent& noteEvent,
                                   const mpe::ArticulationMeta& artMeta, const channel_t channelIdx)
{
    if (noteEvent.pitchCtx().pitchCurve.empty()) {
        return;
    }

    const timestamp_t noteTimestampTo = noteEvent.arrangementCtx().actualTimestamp + noteEvent.arrangementCtx().actualDuration;
    const timestamp_t pitchBendTimestampTo = std::min(artMeta.timestamp + artMeta.overallDuration, noteTimestampTo);

    addPitchBend(destination, pitchBendTimestampTo, channelIdx, 8192);

    auto currIt = noteEvent.pitchCtx().pitchCurve.cbegin();
    auto nextIt = std::next(currIt);
    auto endIt = noteEvent.pitchCtx().pitchCurve.cend();

    auto makePoint = [](mpe::timestamp_t time, int value) {
        return Interpolation::Point { static_cast<double>(time), static_cast<double>(value) };
    };

    for (; nextIt != endIt; currIt = nextIt, nextIt = std::next(currIt)) {
        int currBendValue = pitchBendLevel(currIt->second);
        int nextBendValue = pitchBendLevel(nextIt->second);

        timestamp_t currTime = artMeta.timestamp + artMeta.overallDuration * percentageToFactor(currIt->first);
        timestamp_t nextTime = artMeta.timestamp + artMeta.overallDuration * percentageToFactor(nextIt->first);

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
                addPitchBend(destination, time, channelIdx, bendValue);
            }
        }
    }
}

void FluidSequencer::addPitchBend(EventSequenceMap& destination, const mpe::timestamp_t timestamp,
                                  const midi::channel_t channelIdx, const uint32_t value)
{
    midi::Event event(Event::Opcode::PitchBend, Event::MessageType::ChannelVoice10);
    event.setChannel(channelIdx);
    event.setData(value);
    destination[timestamp].insert(event);
}

void FluidSequencer::addSostenutoEvents(EventSequenceMap& destination, const SostenutoTimeAndDurations& sostenutoTimeAndDurations)
{
    for (const auto& channelPair : sostenutoTimeAndDurations) {
        for (size_t i = 0; i < channelPair.second.size(); ++i) {
            const TimestampAndDuration& currentTnD = channelPair.second.at(i);
            const timestamp_t timestampTo = currentTnD.timestamp + currentTnD.duration;

            addControlChange(destination, currentTnD.timestamp, midi::SOSTENUTO_PEDAL_CONTROLLER, channelPair.first, CTRL_ON);

            if (i == channelPair.second.size() - 1) {
                addControlChange(destination, timestampTo, midi::SOSTENUTO_PEDAL_CONTROLLER, channelPair.first, CTRL_OFF);
                continue;
            }

            const TimestampAndDuration& nextTnD = channelPair.second.at(i + 1);
            if (timestampTo <= nextTnD.timestamp) { // handle potential overlap
                addControlChange(destination, timestampTo, midi::SOSTENUTO_PEDAL_CONTROLLER, channelPair.first, CTRL_OFF);
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
    static constexpr midi::velocity_t MAX_SUPPORTED_VELOCITY = std::numeric_limits<midi::velocity_t>::max();

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
    return expressionLevel(dynamicLevel) << 9; // midi::Event::scaleUp(7,16)
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
