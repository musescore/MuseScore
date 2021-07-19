/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "midiaudiosource.h"

#include <limits>
#include <cstring>

#include "log.h"
#include "realfn.h"
#include "internal/audiosanitizer.h"
#include "internal/synthesizers/fluidsynth/fluidsynth.h"

using namespace mu;
using namespace mu::audio;
using namespace mu::audio::synth;
using namespace mu::midi;

static tick_t MINIMAL_REQUIRED_LOOKAHEAD = 480 * 4 * 10; // about 10 measures of 4/4 time signature

MidiAudioSource::MidiAudioSource(const MidiData& midiData, async::Channel<AudioInputParams> inputParamsChanged)
    : m_stream(midiData.stream), m_mapping(midiData.mapping)
{
    ONLY_AUDIO_WORKER_THREAD;

    inputParamsChanged.onReceive(this, [this](const AudioInputParams& params) {
        MidiData newMidiData = std::get<MidiData>(params);

        m_stream = newMidiData.stream;
        m_mapping = newMidiData.mapping;

        resolveSynth(newMidiData.mapping.synthName);
        setupChannels();
    });

    resolveSynth(m_mapping.synthName);

    m_stream.backgroundStream.onReceive(this, [this](Events events, tick_t endTick) {
        invalidateCaches(m_backgroundStreamEvents);

        m_backgroundStreamEvents.endTick = std::move(endTick);
        m_backgroundStreamEvents.push(std::move(events));
    });

    m_stream.mainStream.onReceive(this, [this](Events events, tick_t endTick) {
        m_mainStreamEvents.endTick = std::move(endTick);
        m_mainStreamEvents.push(std::move(events));

        m_hasActiveRequest = false;
    });

    setupChannels();
    buildTempoMap();

    requestNextEvents(MINIMAL_REQUIRED_LOOKAHEAD);
}

bool MidiAudioSource::isActive() const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_synth) {
        return false;
    }

    return m_synth->isActive();
}

void MidiAudioSource::setIsActive(const bool active)
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_synth) {
        return;
    }

    // invalidate cached events when we stop playing
    if (!active) {
        invalidateCaches(m_mainStreamEvents);
    }

    m_synth->setIsActive(active);
}

void MidiAudioSource::setupChannels()
{
    ONLY_AUDIO_WORKER_THREAD;

    m_synth->setupMidiChannels(m_stream.controlEventsStream.val);
}

void MidiAudioSource::invalidateCaches(EventsBuffer& eventsBuffer)
{
    IF_ASSERT_FAILED(m_synth) {
        return;
    }

    m_synth->flushSound();
    eventsBuffer.reset();
}

void MidiAudioSource::requestNextEvents(const tick_t nextTicksNumber)
{
    if (m_hasActiveRequest) {
        return;
    }

    tick_t maxAvailablePositionTick = m_mainStreamEvents.endTick;
    tick_t newPositionTick = m_mainStreamEvents.currentTick + nextTicksNumber;
    tick_t remainingTicks = maxAvailablePositionTick == 0 ? 0 : maxAvailablePositionTick - newPositionTick;

    if (remainingTicks > MINIMAL_REQUIRED_LOOKAHEAD) {
        return;
    }

    tick_t requestFromTick = maxAvailablePositionTick;
    tick_t requestUpToTick = maxAvailablePositionTick + std::min(m_stream.lastTick - maxAvailablePositionTick, MINIMAL_REQUIRED_LOOKAHEAD);

    m_stream.eventsRequest.send(requestFromTick, requestUpToTick);

    m_hasActiveRequest = true;
}

void MidiAudioSource::findAndSendNextEvents(MidiAudioSource::EventsBuffer& eventsBuffer, const tick_t nextTicks)
{
    if (eventsBuffer.isEmpty()) {
        return;
    }

    for (tick_t tick = eventsBuffer.currentTick; tick <= nextTicks; ++tick) {
        eventsBuffer.currentTick = tick;

        if (!eventsBuffer.hasEventsForTick(tick)) {
            continue;
        }

        sendEvents(eventsBuffer.pop());
    }
}

void MidiAudioSource::handleBackgroundStream(const msecs_t nextMsecsNumber)
{
    tick_t nextTicksNumber = m_backgroundStreamEvents.currentTick + tickFromMsec(nextMsecsNumber);

    findAndSendNextEvents(m_backgroundStreamEvents, nextTicksNumber);
}

void MidiAudioSource::handleMainStream(const msecs_t nextMsecsNumber)
{
    if (m_mainStreamEvents.endTick == m_stream.lastTick) {
        return;
    }

    tick_t nextTicksNumber = m_mainStreamEvents.currentTick + tickFromMsec(nextMsecsNumber);

    requestNextEvents(nextTicksNumber);

    findAndSendNextEvents(m_mainStreamEvents, nextTicksNumber);
}

void MidiAudioSource::handleNextMsecs(const msecs_t nextMsecsNumber)
{
    ONLY_AUDIO_WORKER_THREAD;

    handleBackgroundStream(nextMsecsNumber);

    if (isActive()) {
        handleMainStream(nextMsecsNumber);
    }
}

void MidiAudioSource::setSampleRate(unsigned int sampleRate)
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_synth) {
        return;
    }

    m_sampleRate = sampleRate;
    m_synth->setSampleRate(sampleRate);
}

unsigned int MidiAudioSource::audioChannelsCount() const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_synth) {
        return 0;
    }

    return m_synth->audioChannelsCount();
}

async::Channel<unsigned int> MidiAudioSource::audioChannelsCountChanged() const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_synth) {
        return {};
    }

    return m_synth->audioChannelsCountChanged();
}

void MidiAudioSource::process(float* buffer, unsigned int sampleCount)
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_synth) {
        return;
    }

    m_synth->process(buffer, sampleCount);

    handleNextMsecs(sampleCount * 1000 / m_sampleRate);
}

bool MidiAudioSource::sendEvents(const std::vector<Event>& events)
{
    IF_ASSERT_FAILED(m_synth) {
        return false;
    }

    for (const Event& event : events) {
        m_synth->handleEvent(event);
        midiOutPort()->sendEvent(event);
    }

    return true;
}

void MidiAudioSource::resolveSynth(const SynthName& synthName)
{
    if (m_synth && m_synth->name() == synthName) {
        return;
    }

    m_synth = synthFactory()->createDefault();
}

void MidiAudioSource::seek(const msecs_t newPositionMsecs)
{
    ONLY_AUDIO_WORKER_THREAD;

    invalidateCaches(m_mainStreamEvents);
    m_mainStreamEvents.currentTick = tickFromMsec(newPositionMsecs);

    requestNextEvents(MINIMAL_REQUIRED_LOOKAHEAD);
}

void MidiAudioSource::buildTempoMap()
{
    m_tempoMap.clear();

    std::vector<std::pair<uint32_t, uint32_t> > tempos;
    for (const auto& it : m_mapping.tempo) {
        tempos.push_back({ it.first, it.second });
    }

    if (tempos.empty()) {
        //! NOTE If temp is not set, then set the default temp to 120
        tempos.push_back({ 0, 500000 });
    }

    uint64_t msec { 0 };
    for (size_t i = 0; i < tempos.size(); ++i) {
        TempoItem t;

        t.tempo = tempos.at(i).second;
        t.startTicks = tempos.at(i).first;
        t.startMsec = msec;
        t.onetickMsec = static_cast<double>(t.tempo) / static_cast<double>(m_mapping.division) / 1000.;

        tick_t end_ticks = ((i + 1) < tempos.size()) ? tempos.at(i + 1).first : std::numeric_limits<uint32_t>::max();

        tick_t delta_ticks = end_ticks - t.startTicks;
        msec += static_cast<msecs_t>(delta_ticks * t.onetickMsec);

        m_tempoMap.insert({ msec, std::move(t) });
    }
}

tick_t MidiAudioSource::tickFromMsec(const msecs_t msec) const
{
    auto it = m_tempoMap.lower_bound(msec);

    const TempoItem& t = it->second;

    msecs_t delta = msec - t.startMsec;
    tick_t ticks = static_cast<tick_t>(delta / t.onetickMsec);
    return t.startTicks + ticks;
}
