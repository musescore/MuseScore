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

MidiAudioSource::MidiAudioSource(const TrackId trackId, const MidiData& midiData)
    : m_trackId(trackId), m_stream(midiData.stream), m_mapping(midiData.mapping)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_stream.backgroundStream.onReceive(this, [this](Events events, tick_t endTick) {
        invalidateCaches(m_backgroundStreamEventsBuffer);

        m_backgroundStreamEventsBuffer.endTick = std::move(endTick);
        m_backgroundStreamEventsBuffer.push(std::move(events));
    });

    m_stream.mainStream.onReceive(this, [this](Events events, tick_t endTick) {
        m_mainStreamEventsBuffer.endTick = std::move(endTick);
        m_mainStreamEventsBuffer.push(std::move(events));

        m_hasActiveRequest = false;
    });

    buildTempoMap();

    requestNextEvents(MINIMAL_REQUIRED_LOOKAHEAD);
}

MidiAudioSource::~MidiAudioSource()
{
    m_stream.backgroundStream.resetOnReceive(this);
    m_stream.mainStream.resetOnReceive(this);
}

bool MidiAudioSource::isActive() const
{
    ONLY_AUDIO_WORKER_THREAD;

    if (!m_synth) {
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
        invalidateCaches(m_mainStreamEventsBuffer);
    }

    m_synth->setIsActive(active);
}

void MidiAudioSource::setupChannels()
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_synth) {
        return;
    }

    m_synth->setupSound(m_stream.controlEventsStream.val);
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

    if (m_mainStreamEventsBuffer.isEmpty()) {
        sendRequestFromTick(m_mainStreamEventsBuffer.currentTick);
        return;
    }

    tick_t newPositionTick = m_mainStreamEventsBuffer.currentTick + nextTicksNumber;

    if (newPositionTick > m_stream.lastTick) {
        return;
    }

    tick_t remainingTicks = m_mainStreamEventsBuffer.endTick - newPositionTick;

    if (remainingTicks < MINIMAL_REQUIRED_LOOKAHEAD) {
        if (m_mainStreamEventsBuffer.endTick == m_stream.lastTick) {
            return;
        }

        sendRequestFromTick(m_mainStreamEventsBuffer.endTick);
    }
}

void MidiAudioSource::sendRequestFromTick(const tick_t from)
{
    tick_t to = std::min(m_stream.lastTick, from + MINIMAL_REQUIRED_LOOKAHEAD);

    m_stream.eventsRequest.send(from, to);
    m_hasActiveRequest = true;
}

void MidiAudioSource::findAndSendNextEvents(MidiAudioSource::EventsBuffer& eventsBuffer, const tick_t nextTicks)
{
    if (eventsBuffer.isEmpty()) {
        return;
    }

    tick_t from = eventsBuffer.currentTick;
    tick_t to = eventsBuffer.currentTick + nextTicks;

    for (tick_t tick = from; tick <= to; ++tick) {
        eventsBuffer.currentTick = tick;

        if (!eventsBuffer.hasEventsForTick(tick)) {
            continue;
        }

        sendEvents(eventsBuffer.pop());
    }
}

void MidiAudioSource::handleBackgroundStream(const msecs_t nextMsecsNumber)
{
    tick_t nextTicksNumber = m_backgroundStreamEventsBuffer.currentTick + tickFromMsec(nextMsecsNumber);

    findAndSendNextEvents(m_backgroundStreamEventsBuffer, nextTicksNumber);
}

void MidiAudioSource::handleMainStream(const msecs_t nextMsecsNumber)
{
    if (m_mainStreamEventsBuffer.currentTick == m_stream.lastTick) {
        return;
    }

    tick_t nextTicksNumber = tickFromMsec(nextMsecsNumber);

    requestNextEvents(nextTicksNumber);
    findAndSendNextEvents(m_mainStreamEventsBuffer, nextTicksNumber);
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

    m_sampleRate = sampleRate;

    if (!m_synth) {
        return;
    }

    m_synth->setSampleRate(sampleRate);
}

unsigned int MidiAudioSource::audioChannelsCount() const
{
    ONLY_AUDIO_WORKER_THREAD;

    if (!m_synth) {
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

samples_t MidiAudioSource::process(float* buffer, samples_t samplesPerChannel)
{
    ONLY_AUDIO_WORKER_THREAD;

    if (!m_synth) {
        return 0;
    }

    samples_t processedSamplesCount = 0;
    msecs_t nextMsecsNumber = samplesPerChannel * 1000 / m_sampleRate;

    if (hasAnythingToPlayback(nextMsecsNumber)) {
        processedSamplesCount = m_synth->process(buffer, samplesPerChannel);
    }

    handleNextMsecs(nextMsecsNumber);

    return processedSamplesCount;
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

void MidiAudioSource::seek(const msecs_t newPositionMsecs)
{
    ONLY_AUDIO_WORKER_THREAD;

    invalidateCaches(m_mainStreamEventsBuffer);
    m_mainStreamEventsBuffer.currentTick = tickFromMsec(newPositionMsecs);

    requestNextEvents(MINIMAL_REQUIRED_LOOKAHEAD);
}

const AudioInputParams& MidiAudioSource::inputParams() const
{
    return m_params;
}

void MidiAudioSource::applyInputParams(const AudioInputParams& requiredParams)
{
    if (m_params.isValid() && m_params == requiredParams) {
        return;
    }

    m_synth = synthResolver()->resolveSynth(m_trackId, requiredParams);

    if (!m_synth) {
        m_synth = synthResolver()->resolveDefaultSynth(m_trackId);
    }

    m_synth->paramsChanged().onReceive(this, [this](const AudioInputParams& params) {
        m_paramsChanges.send(params);
    });

    m_synth->setSampleRate(m_sampleRate);
    setupChannels();

    m_params = m_synth->params();
    m_paramsChanges.send(m_params);
}

async::Channel<AudioInputParams> MidiAudioSource::inputParamsChanged() const
{
    return m_paramsChanges;
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

bool MidiAudioSource::hasAnythingToPlayback(const msecs_t nextMsecsNumber) const
{
    if (isActive()) {
        return true;
    }

    tick_t nextTicksNumber = tickFromMsec(nextMsecsNumber);

    return m_backgroundStreamEventsBuffer.hasEventsForNextTicks(nextTicksNumber);
}
