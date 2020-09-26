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

#include "midiplayer.h"

#include <limits>
#include <cstring>

#include "log.h"
#include "realfn.h"

using namespace mu::audio;
using namespace mu::midi;

static tick_t REQUEST_BUFFER_SIZE = 480 * 4 * 10; // about 10 measures of 4/4 time signature

MIDIPlayer::MIDIPlayer()
{
}

MIDIPlayer::~MIDIPlayer()
{
    if (isRunning()) {
        stop();
    }
}

void MIDIPlayer::loadMIDI(const std::shared_ptr<MidiStream>& stream)
{
    m_midiStream = stream;
    m_streamState.reset();

    m_midiData = stream->initData;

    if (m_midiStream->isStreamingAllowed) {
        m_midiStream->stream.onReceive(this, [this](const Chunk& chunk) { onChunkReceived(chunk); });
    }

    if (m_midiStream->isStreamingAllowed && validChunkTick(0, m_midiData.chunks, REQUEST_BUFFER_SIZE) == 0) {
        //! NOTE If there is no data, then we will immediately request them from 0 tick,
        //! so that there is something to play.
        requestData(0);
    }

    buildTempoMap();
    setupChannels();
    midiPortDataSender()->setMidiStream(stream);
}

void MIDIPlayer::setupChannels()
{
    std::set<channel_t> chans = m_midiData.channels();
    m_synthStates.clear();
    for (channel_t ch : chans) {
        std::shared_ptr<ISynthesizer> synth = determineSynthesizer(ch, m_midiData.synthMap);
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

void MIDIPlayer::requestData(tick_t tick)
{
    if (m_streamState.requested) {
        return;
    }

    if (tick >= m_midiStream->lastTick) {
        return;
    }

    m_streamState.requested = true;
    m_midiStream->request.send(tick);
}

void MIDIPlayer::onChunkReceived(const Chunk& chunk)
{
    std::lock_guard<std::mutex> lock(m_dataMutex);
    m_midiData.chunks.insert({ chunk.beginTick, chunk });
    m_streamState.requested = false;
}

void MIDIPlayer::forwardTime(unsigned long miliseconds)
{
    if (!isRunning()) {
        return;
    }

    msec_t msec = static_cast<msec_t>(miliseconds);
    msec_t delta = msec - m_prevMSec;

    if (delta < 1) {
        return;
    }

    msec_t curMSec = m_curMSec + (delta * m_playSpeed);
    tick_t curTick = tick(curMSec);
    tick_t prevTicks = tick(m_prevMSec);
    tick_t maxValidTick = validChunkTick(curTick, m_midiData.chunks, REQUEST_BUFFER_SIZE);

    if (m_midiStream->isStreamingAllowed) {
        tick_t bufSize = maxValidTick - curTick;
        if (bufSize < REQUEST_BUFFER_SIZE) {
            requestData(maxValidTick);
        }
    }

    tick_t toTick = curTick;
    if (toTick > maxValidTick) {
        toTick = maxValidTick;
    }

    while (m_streamState.requested) {
        //wait NotationPlayback send data
    }

    m_curMSec = curMSec;

    sendEvents(prevTicks, toTick);

    m_onTickPlayed.send(m_playTick);

    m_prevMSec = m_curMSec;
    checkPosition();
}

void MIDIPlayer::checkPosition()
{
    if (status() == Error) {
        return;
    }

    if (m_midiStream->isStreamingAllowed && m_streamState.requested) {
        stop();
        return;
    }

    tick_t prev = tick(m_prevMSec);
    if (prev >= m_midiStream->lastTick) {
        stop();
        return;
    }
}

std::shared_ptr<ISynthesizer> MIDIPlayer::determineSynthesizer(channel_t ch, const std::map<channel_t, std::string>& synthmap) const
{
    auto it = synthmap.find(ch);
    if (it == synthmap.end()) {
        LOGI() << "use default synth for ch " << ch;
        return synthesizersRegister()->defaultSynthesizer();
    }

    std::shared_ptr<ISynthesizer> synth = synthesizersRegister()->synthesizer(it->second);
    if (!synth) {
        LOGW() << "Synth " << it->second << " for ch " << ch << " not found. Use default.";
        return synthesizersRegister()->defaultSynthesizer();
    }

    if (!synth->isValid()) {
        LOGW() << "Synth " << it->second << " for ch " << ch << " is not valid. Use default.";
        return synthesizersRegister()->defaultSynthesizer();
    }

    return synth;
}

std::shared_ptr<ISynthesizer> MIDIPlayer::synth(channel_t ch) const
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

bool MIDIPlayer::sendEvents(tick_t fromTick, tick_t toTick)
{
    std::lock_guard<std::mutex> lock(m_dataMutex);

    m_isPlayTickSet = false;

    if (m_midiData.chunks.empty()) {
        return false;
    }

    auto chunkIt = m_midiData.chunks.upper_bound(fromTick);
    --chunkIt;

    const Chunk& chunk = chunkIt->second;
    auto pos = chunk.events.lower_bound(fromTick);

    while (1) {
        const Chunk& curChunk = chunkIt->second;
        if (pos == curChunk.events.end()) {
            ++chunkIt;
            if (chunkIt == m_midiData.chunks.end()) {
                break;
            }

            const Chunk& nextChunk = chunkIt->second;
            if (nextChunk.events.empty()) {
                break;
            }

            pos = nextChunk.events.begin();
        }

        if (pos->first >= toTick) {
            break;
        }

        const Event& event = pos->second;

        if (!m_isPlayTickSet) {
            m_playTick = pos->first;
            m_isPlayTickSet = true;
        }

        ChanState& chState = m_chanStates[event.channel()];
        if (event && !chState.muted) {
            auto s = synth(event.channel());
            s->handleEvent(event);
            s->setIsActive(true);

            if (event.isChannelVoice() && event.opcode() == midi::Event::Opcode::NoteOn) {
                auto noteOff = event;
                noteOff.setOpcode(midi::Event::Opcode::NoteOff);
                m_noteCache[event.note()] = noteOff;
            } else if (event.isChannelVoice() && event.opcode() == midi::Event::Opcode::NoteOff) {
                m_noteCache[event.note()] = Event::NOOP();
            }
        }

        ++pos;
    }

    midiPortDataSender()->sendEvents(fromTick, toTick);
    return true;
}

void MIDIPlayer::sendClear()
{
    for (auto& cache: m_noteCache) {
        auto event = cache.second;
        if (event) {
            auto s = synth(event.channel());
            s->handleEvent(event);
            midiPortDataSender()->sendSingleEvent(event);
        }
    }
    m_noteCache.clear();
}

void MIDIPlayer::play()
{
    if (m_midiStream && status() != Status::Error) {
        setStatus(Status::Running);
    }
}

void MIDIPlayer::stop()
{
    if (status() != Status::Error) {
        setStatus(Status::Stoped);
    }
    sendClear();
}

unsigned long MIDIPlayer::miliseconds() const
{
    return m_curMSec;
}

mu::async::Channel<tick_t> MIDIPlayer::tickPlayed() const
{
    return m_onTickPlayed;
}

void MIDIPlayer::seek(unsigned long miliseconds)
{
    m_curMSec = miliseconds;
    m_prevMSec = miliseconds;

    if (m_midiStream && m_midiStream->isStreamingAllowed) {
        tick_t curTick = tick(m_curMSec);
        tick_t maxValidTick = validChunkTick(curTick, m_midiData.chunks, REQUEST_BUFFER_SIZE);
        tick_t bufSize = maxValidTick - curTick;
        if (bufSize < REQUEST_BUFFER_SIZE) {
            requestData(maxValidTick);
        }
    }
}

tick_t MIDIPlayer::validChunkTick(tick_t fromTick, const Chunks& chunks, tick_t maxDistanceTick) const
{
    if (chunks.empty()) {
        return 0;
    }

    auto it = chunks.upper_bound(fromTick);
    --it;
    for (; it != chunks.end(); ++it) {
        const Chunk& chunk = it->second;

        if ((chunk.endTick - fromTick) > maxDistanceTick) {
            return chunk.endTick;
        }

        auto nextIt = it;
        ++nextIt;
        if (nextIt == chunks.end()) {
            return chunk.endTick;
        }

        const Chunk& nextChunk = nextIt->second;
        if (chunk.endTick != nextChunk.beginTick) {
            return chunk.endTick;
        }
    }

    return chunks.rbegin()->second.endTick;
}

void MIDIPlayer::buildTempoMap()
{
    m_tempoMap.clear();

    std::vector<std::pair<uint32_t, uint32_t> > tempos;
    for (const auto& it : m_midiData.tempoMap) {
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
    }
}

tick_t MIDIPlayer::tick(uint64_t msec) const
{
    auto it = m_tempoMap.lower_bound(msec);

    const TempoItem& t = it->second;

    uint64_t delta = msec - t.startMsec;
    tick_t ticks = static_cast<tick_t>(delta / t.onetickMsec);
    return t.startTicks + ticks;
}

float MIDIPlayer::playbackSpeed() const
{
    return m_playSpeed;
}

void MIDIPlayer::setPlaybackSpeed(float speed)
{
    m_playSpeed = speed;
}

bool MIDIPlayer::hasTrack(track_t ti) const
{
    if (!m_midiData.isValid()) {
        return false;
    }

    if (ti < m_midiData.tracks.size()) {
        return true;
    }

    return false;
}

void MIDIPlayer::setIsTrackMuted(track_t trackIndex, bool mute)
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

void MIDIPlayer::setTrackVolume(track_t trackIndex, float volume)
{
    IF_ASSERT_FAILED(hasTrack(trackIndex)) {
        return;
    }

    const Track& track = m_midiData.tracks[trackIndex];
    for (channel_t ch : track.channels) {
        synth(ch)->channelVolume(ch, volume);
    }
}

void MIDIPlayer::setTrackBalance(track_t trackIndex, float balance)
{
    IF_ASSERT_FAILED(hasTrack(trackIndex)) {
        return;
    }

    const Track& track = m_midiData.tracks[trackIndex];
    for (channel_t ch : track.channels) {
        synth(ch)->channelBalance(ch, balance);
    }
}
