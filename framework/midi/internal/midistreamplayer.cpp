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
#include "midistreamplayer.h"
#include "log.h"
#include "sequencer.h"

using namespace mu;
using namespace mu::midi;

MidiStreamPlayer::MidiStreamPlayer()
{
    m_status.val = PlayStatus::UNDEFINED;
}

IMidiStreamPlayer::PlayStatus MidiStreamPlayer::status() const
{
    return m_status.val;
}

async::Channel<IMidiStreamPlayer::PlayStatus> MidiStreamPlayer::statusChanged() const
{
    return m_status.ch;
}

async::Channel<uint32_t> MidiStreamPlayer::midiTickPlayed() const
{
    return m_midiTickPlayed;
}

void MidiStreamPlayer::setMidiStream(const std::shared_ptr<midi::MidiStream>& stream)
{
    if (stream) {
        m_stream = stream;
        m_status.set(PlayStatus::STOPED);
    }
}

bool MidiStreamPlayer::play()
{
    LOGD() << "try play \n";
    if (m_status.val == PlayStatus::PLAYING) {
        LOGW() << "already playing \n";
        return true;
    }

    if (!doPlay()) {
        LOGE() << "failed do play \n";
        return false;
    }

    m_status.set(PlayStatus::PLAYING);

    return true;
}

void MidiStreamPlayer::pause()
{
    doPause();
    m_status.set(PlayStatus::PAUSED);
}

void MidiStreamPlayer::stop()
{
    doStop();
}

void MidiStreamPlayer::rewind()
{
    doStop();
}

bool MidiStreamPlayer::init()
{
    if (m_inited) {
        return true;
    }

    sequencer()->stopped().onNotify(this, [this]() { onStop(); });
    sequencer()->tickPlayed().onReceive(this, [this](midi::tick_t tick) {
        m_lastMidiPlayTick = tick;
        m_midiTickPlayed.send(tick);
    });
    m_inited = true;
    return m_inited;
}

bool MidiStreamPlayer::isInited() const
{
    return m_inited;
}

bool MidiStreamPlayer::doPlay()
{
    if (!init()) {
        return false;
    }

    if (!hasTracks()) {
        return false;
    }

    sequencer()->loadMIDI(m_stream);
    sequencer()->run(m_beginPlayPosition);
    return true;
}

void MidiStreamPlayer::doPause()
{
    m_beginPlayPosition = currentPlayPosition();
    sequencer()->stop();
}

void MidiStreamPlayer::doStop()
{
    sequencer()->stop();
}

void MidiStreamPlayer::onStop()
{
    m_beginPlayPosition = 0;
    m_status.set(PlayStatus::STOPED);
}

float MidiStreamPlayer::currentPlayPosition() const
{
    return sequencer()->position();
}

float MidiStreamPlayer::playbackPosition() const
{
    if (m_status.val == PlayStatus::PLAYING) {
        return currentPlayPosition();
    }
    return m_beginPlayPosition;
}

void MidiStreamPlayer::setPlaybackPosition(float sec)
{
    sec = std::max(sec, 0.f);

    m_beginPlayPosition = sec;
    if (status() == PlayStatus::PLAYING) {
        sequencer()->seek(m_beginPlayPosition);
    }
}

bool MidiStreamPlayer::hasTracks() const
{
    return m_stream->initData.tracks.size() > 0;
}

void MidiStreamPlayer::playMidi(const midi::MidiData& data)
{
    auto singleSequencer = std::make_shared<Sequencer>();
    auto task = [data, singleSequencer]() {
                    auto midiStream = std::make_shared<midi::MidiStream>();
                    midiStream->initData = data;
                    midiStream->isStreamingAllowed = false;
                    midiStream->lastTick = data.lastChunksTick();
                    singleSequencer->loadMIDI(midiStream);
                    singleSequencer->run(0);

                    while (!singleSequencer->hasEnded()) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }
                };
    auto t = std::thread(task);
    t.detach();
}
