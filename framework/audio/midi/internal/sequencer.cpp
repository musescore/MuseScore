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

void Sequencer::loadMIDI(const std::shared_ptr<MidiData>& midi)
{
    m_midi = midi;
    buildTempoMap();
    synth()->loadSF(m_midi->programs(), "", [this](uint16_t percent) {
        LOGI() << "sf loading: " << percent;
    });
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
    uint64_t delta = msec - m_lastTimerMsec;

    if (delta < 1) {
        return;
    }

    player_callback(delta);
    m_lastTimerMsec = msec;
}

float Sequencer::getAudio(float sec, float* buf, unsigned int len)
{
    process(sec);

    synth()->writeBuf(buf, len);

    float cur_sec =  static_cast<float>(m_curMsec) / 1000.f;
    return cur_sec;
}

bool Sequencer::hasEnded() const
{
    for (const Track& t : m_midi->tracks) {
        for (const Channel& c : t.channels) {
            if (!channel_eot(c)) {
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

    IF_ASSERT_FAILED(m_midi) {
        return false;
    }

    m_lastTimerMsec = static_cast<uint64_t>(init_sec * 1000);
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

void Sequencer::doSeekChan(uint32_t seek_ticks, const Channel& c)
{
    ChanState& state = m_chanStates[c.num];
    state.eventIndex = 0;

    for (size_t i = 0; i < c.events.size(); ++i) {
        state.eventIndex = i;
        const Event& event = c.events.at(i);
        if (event.tick >= seek_ticks) {
            break;
        }
    }
}

void Sequencer::doSeek(uint64_t seek_msec)
{
    m_internalRunning = false;

    m_seekMsec = seek_msec;
    uint32_t seek_ticks = ticks(m_seekMsec);

    for (const Track& t : m_midi->tracks) {
        for (const Channel& c : t.channels) {
            doSeekChan(seek_ticks, c);
        }
    }

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

uint64_t Sequencer::max_ticks(const std::vector<Track>& tracks) const
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

bool Sequencer::channel_eot(const Channel& chan) const
{
    const ChanState& s = m_chanStates[chan.num];
    if (s.eventIndex >= chan.events.size()) {
        return true;
    }
    return false;
}

void Sequencer::buildTempoMap()
{
    IF_ASSERT_FAILED(m_midi) {
        return;
    }

    m_tempoMap.clear();

    std::vector<std::pair<uint32_t, uint32_t> > tempos;
    for (const auto& it : m_midi->tempomap) {
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
        t.onetickMsec = static_cast<double>(t.tempo)
                        / static_cast<double>(m_midi->division)
                        / 1000.;

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

bool Sequencer::player_callback(uint64_t timer_msec)
{
    //    static uint64_t last_msec{0};
    //    LOGI() << "msec: " << timer_msec  << ", delta: " << (timer_msec - last_msec) << "\n";
    //    last_msec = timer_msec;

    m_curMsec += (timer_msec * m_playSpeed);

    uint32_t cur_ticks = ticks(m_curMsec);
    //    LOGI() << "timer_msec: " << timer_msec
    //           << ", cur_msec: " << _cur_msec
    //           << ", cur_ticks: " << cur_ticks
    //           << "\n";

    auto sendEvents = [this, cur_ticks](const Channel& c) {
                          if (!channel_eot(c)) {
                              if (!send_chan_events(c, cur_ticks)) {
                                  LOGE() << "failed send events\n";
                              }
                          }
                      };

    for (const Track& t : m_midi->tracks) {
        for (const Channel& c : t.channels) {
            sendEvents(c);
        }
    }

    return true;
}

bool Sequencer::send_chan_events(const Channel& chan, uint32_t ticks)
{
    bool ret = true;

    ChanState& state = m_chanStates[chan.num];

    while (1)
    {
        if (channel_eot(chan)) {
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

bool Sequencer::isHasTrack(uint16_t ti) const
{
    if (!m_midi) {
        return false;
    }

    if (ti < m_midi->tracks.size()) {
        return true;
    }

    return false;
}

void Sequencer::setIsTrackMuted(int ti, bool mute)
{
    IF_ASSERT_FAILED(isHasTrack(ti)) {
        return;
    }

    auto setMuted = [this, mute](const Channel& c) {
                        ChanState& state = m_chanStates[c.num];
                        state.muted = mute;
                        synth()->channelSoundsOff(c.num);
                    };

    const Track& track = m_midi->tracks[ti];
    for (const Channel& c : track.channels) {
        setMuted(c);
    }
}

void Sequencer::setTrackVolume(int ti, float volume)
{
    IF_ASSERT_FAILED(isHasTrack(ti)) {
        return;
    }

    const Track& track = m_midi->tracks[ti];
    for (const Channel& c : track.channels) {
        synth()->channelVolume(c.num, volume);
    }
}

void Sequencer::setTrackBalance(int ti, float balance)
{
    IF_ASSERT_FAILED(isHasTrack(ti)) {
        return;
    }

    const Track& track = m_midi->tracks[ti];
    for (const Channel& c : track.channels) {
        synth()->channelBalance(c.num, balance);
    }
}
