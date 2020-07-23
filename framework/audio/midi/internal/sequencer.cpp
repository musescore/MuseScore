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

#include "log.h"
#include "realfn.h"

using namespace mu::audio::midi;

Sequencer::~Sequencer()
{
    if (m_status == Running) {
        stop();
    }
}

void Sequencer::init(float samplerate, float gain)
{
    reset();

    m_sampleRate = samplerate;

    synth()->init(samplerate, gain, [this, samplerate](bool success) {
        if (!success) {
            m_status = Error;
            LOGE() << "failed init sequencer (failed init synth)\n";
        } else {
            LOGD() << "success init sequencer, samplerate: " << samplerate << "\n";
        }
    });
}

void Sequencer::loadMIDI(const std::shared_ptr<MidiStream>& stream)
{
    m_midiStream = stream;
    m_streamState = StreamState();

    m_midiData = stream->initData;

    m_midiStream->stream.onReceive(this, [this](const MidiData& data) { onDataReceived(data); });
    m_midiStream->stream.onClose(this, [this]() { onStreamClosed(); });

    if (maxTicks(m_midiData.tracks) == 0) {
        //! NOTE If there is no data, then we will immediately request them from 0 tick,
        //! so that there is something to play.
        requestData(0);
    }

    buildTempoMap();
    synth()->loadSF(m_midiData.programs(), "", [this](uint16_t percent) {
        LOGI() << "sf loading: " << percent;
    });
}

void Sequencer::requestData(uint32_t tick)
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
    LOGI() << "onDataReceived: " << data.tracks.front().channels.front().events.front().tick;
    //! TODO implement merge
    m_midiData = data;
    m_streamState.requested = false;

    uint32_t curTick = ticks(m_curMsec);
    doSeekTracks(curTick, m_midiData.tracks);
}

void Sequencer::onStreamClosed()
{
    m_streamState.requested = false;
    m_streamState.closed = true;
}

void Sequencer::changeGain(float gain)
{
    synth()->setGain(gain);
}

void Sequencer::process(float sec)
{
    if (!m_internalRunning) {
        return;
    }

    uint64_t msec = static_cast<uint64_t>(sec * 1000);
    uint64_t delta = msec - m_lastTimeMsec;

    if (delta < 1) {
        return;
    }

    m_curMsec += (delta * m_playSpeed);

    uint32_t curTicks = ticks(m_curMsec);
    uint32_t maxTicks = this->maxTicks(m_midiData.tracks);
    if (curTicks >= maxTicks) {
        requestData(curTicks);
    }

    sendEvents(curTicks);
    m_lastTimeMsec = msec;
}

float Sequencer::getAudio(float sec, float* buf, unsigned int len)
{
    process(sec);

    synth()->writeBuf(buf, len);

    float cur_sec = static_cast<float>(m_curMsec) / 1000.f;
    return cur_sec;
}

bool Sequencer::hasEnded() const
{
    if (m_streamState.requested) {
        return false;
    }

    for (const Track& t : m_midiData.tracks) {
        for (const Channel& c : t.channels) {
            if (!channelEOT(c)) {
                return false;
            }
        }
    }
    return true;
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

    m_lastTimeMsec = static_cast<uint64_t>(init_sec * 1000);
    doRun();
    m_status = Running;

    return true;
}

void Sequencer::stop()
{
    LOGI() << "Sequencer::stop\n";
    doStop();
    reset();
    m_status = Stoped;
}

void Sequencer::reset()
{
    m_curMsec = 0;
    m_seekMsec = 0;
    for (auto it = m_chanStates.begin(); it != m_chanStates.end(); ++it) {
        it->second.eventIndex = 0;
    }
}

bool Sequencer::doRun()
{
    m_curMsec = m_seekMsec;
    m_internalRunning = true;
    return true;
}

void Sequencer::doStop()
{
    m_internalRunning = false;
    synth()->flushSound();
}

void Sequencer::doSeekTracks(uint32_t seekTicks, const std::vector<Track>& tracks)
{
    for (const Track& t : tracks) {
        for (const Channel& c : t.channels) {
            doSeekChan(seekTicks, c);
        }
    }
}

void Sequencer::doSeekChan(uint32_t seekTicks, const Channel& c)
{
    ChanState& state = m_chanStates[c.num];
    state.eventIndex = 0;

    for (size_t i = 0; i < c.events.size(); ++i) {
        state.eventIndex = i;
        const Event& event = c.events.at(i);
        if (event.tick >= seekTicks) {
            break;
        }
    }
}

void Sequencer::doSeek(uint64_t seekMsec)
{
    m_internalRunning = false;

    m_seekMsec = seekMsec;
    uint32_t seekTicks = ticks(m_seekMsec);

    doSeekTracks(seekTicks, m_midiData.tracks);

    m_curMsec = m_seekMsec;
    m_internalRunning = true;
}

float Sequencer::playbackPosition() const
{
    return m_curMsec / 1000.f;
}

void Sequencer::seek(float sec)
{
    doSeek(static_cast<uint64_t>(sec * 1000.f));
    synth()->flushSound();
}

uint64_t Sequencer::maxTicks(const std::vector<Track>& tracks) const
{
    uint64_t maxTicks = 0;

    auto findMaxTicks = [&maxTicks](const Channel& c) {
                            for (const Event& e : c.events) {
                                if (e.tick > maxTicks) {
                                    maxTicks = e.tick;
                                }
                            }
                        };

    for (const Track& t : tracks) {
        for (const Channel& c : t.channels) {
            findMaxTicks(c);
        }
    }

    return maxTicks;
}

bool Sequencer::channelEOT(const Channel& chan) const
{
    const ChanState& s = m_chanStates[chan.num];
    if (s.eventIndex >= chan.events.size()) {
        return true;
    }
    return false;
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

uint32_t Sequencer::ticks(uint64_t msec) const
{
    auto it = m_tempoMap.lower_bound(msec);

    const TempoItem& t = it->second;

    uint64_t delta = msec - t.startMsec;
    uint32_t ticks = static_cast<uint32_t>(delta / t.onetickMsec);
    return t.startTicks + ticks;
}

bool Sequencer::sendEvents(uint32_t curTicks)
{
    for (const Track& t : m_midiData.tracks) {
        for (const Channel& c : t.channels) {
            if (!sendChanEvents(c, curTicks)) {
                LOGE() << "failed send events\n";
            }
        }
    }

    return true;
}

bool Sequencer::sendChanEvents(const Channel& chan, uint32_t ticks)
{
    bool ret = true;

    ChanState& state = m_chanStates[chan.num];

    while (1)
    {
        if (channelEOT(chan)) {
            return ret;
        }

        const Event& event = chan.events.at(state.eventIndex);
        if (event.tick > ticks) {
            return ret;
        }

        if (state.muted || event.type == MIDI_EOT || event.type == META_TEMPO) {
            // noop
        } else {
            synth()->handleEvent(chan.num, event);
        }

        ++state.eventIndex;
    }

    return ret;
}

float Sequencer::playbackSpeed() const
{
    return m_playSpeed;
}

void Sequencer::setPlaybackSpeed(float speed)
{
    m_playSpeed = speed;
}

bool Sequencer::hasTrack(uint16_t ti) const
{
    if (!m_midiData.isValid()) {
        return false;
    }

    if (ti < m_midiData.tracks.size()) {
        return true;
    }

    return false;
}

void Sequencer::setIsTrackMuted(uint16_t trackIndex, bool mute)
{
    IF_ASSERT_FAILED(hasTrack(trackIndex)) {
        return;
    }

    auto setMuted = [this, mute](const Channel& c) {
                        ChanState& state = m_chanStates[c.num];
                        state.muted = mute;
                        synth()->channelSoundsOff(c.num);
                    };

    const Track& track = m_midiData.tracks[trackIndex];
    for (const Channel& c : track.channels) {
        setMuted(c);
    }
}

void Sequencer::setTrackVolume(uint16_t trackIndex, float volume)
{
    IF_ASSERT_FAILED(hasTrack(trackIndex)) {
        return;
    }

    const Track& track = m_midiData.tracks[trackIndex];
    for (const Channel& c : track.channels) {
        synth()->channelVolume(c.num, volume);
    }
}

void Sequencer::setTrackBalance(uint16_t trackIndex, float balance)
{
    IF_ASSERT_FAILED(hasTrack(trackIndex)) {
        return;
    }

    const Track& track = m_midiData.tracks[trackIndex];
    for (const Channel& c : track.channels) {
        synth()->channelBalance(c.num, balance);
    }
}
