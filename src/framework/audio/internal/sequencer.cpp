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

#include "log.h"

#include "internal/midiplayer.h"
#include "internal/audioplayer.h"

using namespace mu::audio;

Sequencer::Sequencer()
{
    m_clock = std::make_shared<Clock>();
    m_clock->addAfterCallback([this](Clock::time_t) {
        timeUpdate();
    });
    m_clock->addBeforeCallback([this](Clock::time_t miliseconds) {
        beforeTimeUpdate(miliseconds);
    });
}

ISequencer::Status Sequencer::status() const
{
    return m_status;
}

mu::async::Channel<ISequencer::Status> Sequencer::statusChanged() const
{
    return m_statusChanged;
}

void Sequencer::play()
{
    m_nextStatus = PLAYING;
}

void Sequencer::pause()
{
    m_nextStatus = STOPED;
}

void Sequencer::stop()
{
    m_nextStatus = STOPED;
    rewind();
}

void Sequencer::seek(uint64_t miliseconds)
{
    m_nextSeek = miliseconds;
}

void Sequencer::rewind()
{
    seek(0);
}

void Sequencer::initMIDITrack(ISequencer::track_id id)
{
    createMIDITrack(id);
}

void Sequencer::setMIDITrack(ISequencer::track_id id, const std::shared_ptr<mu::midi::MidiStream>& stream)
{
    midi_track_t track = midiTrack(id);
    if (!track) {
        track = createMIDITrack(id);
    }
    track->loadMIDI(stream);
}

void Sequencer::setAudioTrack(ISequencer::track_id id, const std::shared_ptr<audio::IAudioStream>& stream)
{
    audio_track_t track = audioTrack(id);
    if (!track) {
        track = createAudioTrack(id);
    }
    track->load(stream);
}

mu::async::Channel<ISequencer::audio_track_t> Sequencer::audioTrackAdded() const
{
    return m_audioTrackAdded;
}

void Sequencer::initAudioTrack(ISequencer::track_id id)
{
    createAudioTrack(id);
}

mu::async::Channel<mu::midi::tick_t> Sequencer::midiTickPlayed(ISequencer::track_id id) const
{
    if (midi_track_t track = midiTrack(id)) {
        return track->tickPlayed();
    }
    return {};
}

mu::async::Notification Sequencer::positionChanged() const
{
    return m_positionChanged;
}

float Sequencer::playbackPosition() const
{
    return clock()->timeInSeconds();
}

void Sequencer::setLoop(uint64_t fromMiliSeconds, uint64_t toMiliSeconds)
{
    m_loopStart = fromMiliSeconds;
    m_loopEnd = toMiliSeconds;
}

void Sequencer::unsetLoop()
{
    m_loopStart.reset();
    m_loopEnd.reset();
}

std::shared_ptr<Clock> Sequencer::clock() const
{
    return m_clock;
}

void Sequencer::timeUpdate()
{
    if (m_loopStart.has_value() && m_loopEnd.has_value()) {
        if (m_clock->timeInMiliSeconds() >= m_loopEnd) {
            seek(m_loopStart.value_or(0));
        }
    }

    bool willcontinue = false;
    for (auto& track : m_tracks) {
        track.second->forwardTime(m_clock->timeInMiliSeconds());
        willcontinue |= track.second->isRunning();
    }
    m_positionChanged.notify();
    if (!willcontinue) {
        stop();
    }
}

void Sequencer::beforeTimeUpdate(Clock::time_t miliseconds)
{
    if (m_nextStatus != m_status) {
        setStatus(m_nextStatus);
    }

    if (m_nextSeek.has_value()) {
        auto miliseconds = m_nextSeek.value_or(0);
        m_nextSeek.reset();

        m_clock->seekMiliseconds(miliseconds);
        for (auto& track : m_tracks) {
            track.second->seek(miliseconds);
        }
        m_positionChanged.notify();
    }

    std::for_each(m_backgroudPlayers.begin(), m_backgroudPlayers.end(), [&miliseconds](auto& player) {
        player.first += miliseconds;
        player.second->forwardTime(player.first);
    });

    m_backgroudPlayers.remove_if([](auto player) {
        return player.second->status() == IPlayer::Status::Stoped;
    });
}

void Sequencer::setStatus(Status status)
{
    m_status = status;
    m_statusChanged.send(m_status);

    std::function<void(const track_t&)> applyStaus = [](const track_t&) {};
    switch (m_status) {
    case PLAYING:
        applyStaus = [](const track_t& track) { track->play(); };
        m_clock->start();
        break;
    case STOPED:
        applyStaus = [](const track_t& track) { track->stop(); };
        m_clock->stop();
        break;
    }

    for (auto& track : m_tracks) {
        applyStaus(track.second);
    }
}

std::optional<Sequencer::track_t> Sequencer::track(Sequencer::track_id id) const
{
    if (m_tracks.find(id) != m_tracks.end()) {
        return m_tracks.at(id);
    }
    return std::nullopt;
}

Sequencer::midi_track_t Sequencer::midiTrack(Sequencer::track_id id) const
{
    if (auto track_var = track(id)) {
        if (auto track_value = track_var.value_or(nullptr)) {
            return std::dynamic_pointer_cast<IMIDIPlayer>(track_value);
        }
    }
    return nullptr;
}

Sequencer::midi_track_t Sequencer::createMIDITrack(ISequencer::track_id id)
{
    auto player = std::make_shared<MIDIPlayer>();
    m_tracks[id] = player;
    return player;
}

Sequencer::audio_track_t Sequencer::createAudioTrack(Sequencer::track_id id)
{
    auto player = std::make_shared<AudioPlayer>();
    m_tracks[id] = player;
    m_audioTrackAdded.send(player);
    return player;
}

Sequencer::audio_track_t Sequencer::audioTrack(Sequencer::track_id id) const
{
    if (auto track_var = track(id)) {
        if (auto track_value = track_var.value_or(nullptr)) {
            return std::dynamic_pointer_cast<IAudioPlayer>(track_value);
        }
    }
    return nullptr;
}

std::shared_ptr<IMIDIPlayer> Sequencer::instantlyPlayMidi(const midi::MidiData& data)
{
    if (!data.isValid()) {
        return nullptr;
    }
    auto player = std::make_shared<MIDIPlayer>();
    auto midiStream = std::make_shared<midi::MidiStream>();

    midiStream->initData = data;
    midiStream->isStreamingAllowed = false;
    midiStream->lastTick = data.lastChunksTick();
    player->loadMIDI(midiStream);
    player->play();

    m_backgroudPlayers.push_back({ 0 /*ms*/, player });
    return player;
}
