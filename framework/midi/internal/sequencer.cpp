//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "sequencer.h"

#include <limits>
#include <cstring>

#include "log.h"
#include "realfn.h"

using namespace mu::midi;

Sequencer::~Sequencer()
{
    if (m_status == Running) {
        stop();
    }
}

void Sequencer::loadMIDI(const std::shared_ptr<MidiStream>& stream)
{
    m_midiStream = stream;
    m_streamState = StreamState();

    m_midiData = stream->initData;

    if (m_midiStream->isStreamingAllowed) {
        m_midiStream->stream.onReceive(this, [this](const MidiData& data) { onDataReceived(data); });
        m_midiStream->stream.onClose(this, [this]() { onStreamClosed(); });
    }

    if (m_midiStream->isStreamingAllowed && maxTick(m_midiData.events) == 0) {
        //! NOTE If there is no data, then we will immediately request them from 0 tick,
        //! so that there is something to play.
        requestData(0);
    }

    buildTempoMap();
    setupChannels();
}

void Sequencer::setupChannels()
{
    std::set<channel_t> chans = m_midiData.channels();
    for (channel_t ch : chans) {
        std::shared_ptr<ISynthesizer> synth = determineSynthesizer(ch, m_midiData.synthmap);
        synth->setIsActive(false);

        auto it = std::find_if(m_synthStates.begin(), m_synthStates.end(), [&synth](const SynthState& st) {
            return st.synth == synth;
        });

        if (it == m_synthStates.end()) {
            SynthState newst;
            newst.synth = synth;
            m_synthStates.push_back(std::move(newst));
            it = m_synthStates.end() - 1;
        }

        SynthState& st = *it;
        st.channels.insert(ch);
    }

    for (const SynthState& st : m_synthStates) {
        st.synth->setupChannels(m_midiData.initEventsForChannels(st.channels));
    }
}

void Sequencer::requestData(tick_t tick)
{
    LOGI() << "requestData: " << tick;
    if (m_streamState.closed) {
        LOGE() << "stream closed";
        m_streamState.requested = false;
        return;
    }

    m_streamState.requested = true;
    m_midiStream->request.send(tick);
}

void Sequencer::onDataReceived(const MidiData& data)
{
    //LOGI() << "onDataReceived: " << data.tracks.front().channels.front().events.front().tick;
    //! TODO implement merge
    m_midiData = data;
    m_streamState.requested = false;
}

void Sequencer::onStreamClosed()
{
    m_streamState.requested = false;
    m_streamState.closed = true;
}

void Sequencer::process(float sec)
{
    if (m_status != Running) {
        return;
    }

    uint64_t msec = static_cast<uint64_t>(sec * 1000);
    uint64_t delta = msec - m_prevMSec;

    if (delta < 1) {
        return;
    }

    m_curMSec += (delta * m_playSpeed);

    uint32_t curTicks = ticks(m_curMSec);
    uint32_t prevTicks = ticks(m_prevMSec);

    uint32_t maxTicks = this->maxTick(m_midiData.events);
    if (m_midiStream->isStreamingAllowed && curTicks >= maxTicks) {
        requestData(curTicks);
    }

    sendEvents(prevTicks, curTicks);

    m_prevMSec = m_curMSec;
}

std::shared_ptr<ISynthesizer> Sequencer::determineSynthesizer(channel_t ch, const std::map<channel_t, std::string>& synthmap) const
{
    auto it = synthmap.find(ch);
    if (it == synthmap.end()) {
        return synthesizersRegister()->defaultSynthesizer();
    }

    std::shared_ptr<ISynthesizer> synth = synthesizersRegister()->synthesizer(it->second);
    if (!synth) {
        return synthesizersRegister()->defaultSynthesizer();
    }

    return synth;
}

std::shared_ptr<ISynthesizer> Sequencer::synth(channel_t ch) const
{
    for (const SynthState& state : m_synthStates) {
        if (state.channels.find(ch) != state.channels.end()) {
            return state.synth;
        }
    }

    IF_ASSERT_FAILED_X(false, "not found synth state") {
        return m_synthStates.begin()->synth;
    }

    return nullptr;
}

bool Sequencer::sendEvents(tick_t fromTick, tick_t toTick)
{
    static const std::set<EventType> SKIP_EVENTS = { MIDI_EOT, META_TEMPO, ME_TICK1, ME_TICK2 };

    m_isPlayTickSet = false;

    auto pos = m_midiData.events.lower_bound(fromTick);
    if (pos == m_midiData.events.end()) {
        return false;
    }

    while (1) {
        if (pos == m_midiData.events.end()) {
            break;
        }

        if (pos->first >= toTick) {
            break;
        }

        const Event& event = pos->second;

        if (!m_isPlayTickSet) {
            m_playTick = pos->first;
            m_isPlayTickSet = true;
        }

        ChanState& chState = m_chanStates[event.channel];
        if (chState.muted || SKIP_EVENTS.find(event.type) != SKIP_EVENTS.end()) {
            // noop
        } else {
            auto s = synth(event.channel);
            s->handleEvent(event);
            s->setIsActive(true);
        }

        ++pos;
    }

    return true;
}

float Sequencer::getAudio(float sec, float* buf, unsigned int samples)
{
    process(sec);

    unsigned int totalSamples = samples * AUDIO_CHANNELS;

    // write buffers
    for (SynthState& state : m_synthStates) {
        if (state.synth->isActive()) {
            if (state.buf.size() < totalSamples) {
                state.buf.resize(totalSamples);
            }
            std::memset(&state.buf[0], 0, totalSamples * sizeof(float));
            state.synth->writeBuf(&state.buf[0], samples);
        }
    }

    // mix
    std::memset(buf, 0, totalSamples * sizeof(float));
    for (SynthState& state : m_synthStates) {
        if (state.synth->isActive()) {
            for (unsigned int s = 0; s < totalSamples; ++s) {
                buf[s] += state.buf[s];
            }
        }
    }

    float cur_sec = static_cast<float>(m_curMSec) / 1000.f;
    return cur_sec;
}

bool Sequencer::hasEnded() const
{
    if (m_midiStream->isStreamingAllowed && m_streamState.requested) {
        return false;
    }

    uint32_t prev = ticks(m_prevMSec);
    uint32_t max = maxTick(m_midiData.events);
    if (prev >= max) {
        return true;
    }

    return false;
}

tick_t Sequencer::playTick() const
{
    return m_playTick;
}

Sequencer::Status Sequencer::status() const
{
    return m_status;
}

bool Sequencer::run(float init_sec)
{
    if (m_status == Running) {
        return true;
    }

    if (m_status == Error) {
        return false;
    }

    m_prevMSec = static_cast<uint64_t>(init_sec * 1000);
    m_curMSec = m_prevMSec;

    m_status = Running;

    return true;
}

void Sequencer::stop()
{
    LOGI() << "stop";

    for (SynthState& state : m_synthStates) {
        state.synth->flushSound();
    }

    reset();
}

void Sequencer::reset()
{
    m_curMSec = 0;
    m_prevMSec = 0;
}

void Sequencer::seek(float sec)
{
    IF_ASSERT_FAILED(!(sec < 0)) {
        sec = 0;
    }

    uint64_t seekMsec = static_cast<uint64_t>(sec * 1000.f);

    m_curMSec = seekMsec;
    m_prevMSec = seekMsec;

    for (SynthState& state : m_synthStates) {
        state.synth->flushSound();
    }
}

uint32_t Sequencer::maxTick(const Events& events) const
{
    if (events.empty()) {
        return 0;
    }

    auto last = events.rbegin();
    return last->first;
}

void Sequencer::buildTempoMap()
{
    m_tempoMap.clear();

    std::vector<std::pair<uint32_t, uint32_t> > tempos;
    for (const auto& it : m_midiData.tempomap) {
        tempos.push_back({ it.first, it.second });
    }

    if (tempos.empty()) {
        //! NOTE If temp is not set, then set the default temp to 120
        tempos.push_back({ 0, 500000 });
    }

    uint64_t msec{ 0 };
    for (size_t i = 0; i < tempos.size(); ++i) {
        TempoItem t;

        t.tempo = tempos.at(i).second;
        t.startTicks = tempos.at(i).first;
        t.startMsec = msec;
        t.onetickMsec = static_cast<double>(t.tempo) / static_cast<double>(m_midiData.division) / 1000.;

        uint32_t end_ticks = ((i + 1) < tempos.size()) ? tempos.at(i + 1).first : std::numeric_limits<uint32_t>::max();

        uint32_t delta_ticks = end_ticks - t.startTicks;
        msec += static_cast<uint64_t>(delta_ticks * t.onetickMsec);

        m_tempoMap.insert({ msec, std::move(t) });

        //        LOGI() << "TempoItem t.tick: " << t.start_ticks
        //               << ", t.tempo: " << t.tempo
        //               << ", t.onetick_msec: " << t.onetick_msec
        //               << ", end_msec: " << msec;
    }
}

tick_t Sequencer::ticks(uint64_t msec) const
{
    auto it = m_tempoMap.lower_bound(msec);

    const TempoItem& t = it->second;

    uint64_t delta = msec - t.startMsec;
    tick_t ticks = static_cast<tick_t>(delta / t.onetickMsec);
    return t.startTicks + ticks;
}

float Sequencer::playbackSpeed() const
{
    return m_playSpeed;
}

void Sequencer::setPlaybackSpeed(float speed)
{
    m_playSpeed = speed;
}

bool Sequencer::hasTrack(track_t ti) const
{
    if (!m_midiData.isValid()) {
        return false;
    }

    if (ti < m_midiData.tracks.size()) {
        return true;
    }

    return false;
}

void Sequencer::setIsTrackMuted(track_t trackIndex, bool mute)
{
    IF_ASSERT_FAILED(hasTrack(trackIndex)) {
        return;
    }

    auto setMuted = [this, mute](channel_t ch) {
                        ChanState& state = m_chanStates[ch];
                        state.muted = mute;
                        synth(ch)->channelSoundsOff(ch);
                    };

    const Track& track = m_midiData.tracks[trackIndex];
    for (channel_t ch : track.channels) {
        setMuted(ch);
    }
}

void Sequencer::setTrackVolume(track_t trackIndex, float volume)
{
    IF_ASSERT_FAILED(hasTrack(trackIndex)) {
        return;
    }

    const Track& track = m_midiData.tracks[trackIndex];
    for (channel_t ch : track.channels) {
        synth(ch)->channelVolume(ch, volume);
    }
}

void Sequencer::setTrackBalance(track_t trackIndex, float balance)
{
    IF_ASSERT_FAILED(hasTrack(trackIndex)) {
        return;
    }

    const Track& track = m_midiData.tracks[trackIndex];
    for (channel_t ch : track.channels) {
        synth(ch)->channelBalance(ch, balance);
    }
}
