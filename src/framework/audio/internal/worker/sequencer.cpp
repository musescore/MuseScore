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

#include "internal/audiosanitizer.h"
#include "midiplayer.h"
#include "audioplayer.h"

using namespace mu::audio;

Sequencer::Sequencer()
{
    ONLY_AUDIO_WORKER_THREAD;

    m_clock = std::make_shared<Clock>();
    m_clock->addAfterCallback([this](Clock::time_t) {
        timeUpdate();
    });
    m_clock->addBeforeCallback([this](Clock::time_t milliseconds) {
        beforeTimeUpdate(milliseconds);
    });
}

Sequencer::~Sequencer()
{
    ONLY_AUDIO_WORKER_THREAD;
}

ISequencer::Status Sequencer::status() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_status;
}

mu::async::Channel<ISequencer::Status> Sequencer::statusChanged() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_statusChanged;
}

void Sequencer::play()
{
    ONLY_AUDIO_WORKER_THREAD;
    m_nextStatus = PLAYING;
}

void Sequencer::pause()
{
    ONLY_AUDIO_WORKER_THREAD;
    m_nextStatus = PAUSED;
}

void Sequencer::stop()
{
    ONLY_AUDIO_WORKER_THREAD;

    if (m_status == STOPED) {
        return;
    }

    m_nextStatus = STOPED;
    rewind();
}

void Sequencer::seek(uint64_t milliseconds)
{
    ONLY_AUDIO_WORKER_THREAD;
    m_nextSeek = milliseconds;
}

void Sequencer::rewind()
{
    ONLY_AUDIO_WORKER_THREAD;
    seek(0);
}

void Sequencer::initMIDITrack(ISequencer::TrackID id)
{
    ONLY_AUDIO_WORKER_THREAD;
    createMIDITrack(id);
}

void Sequencer::setMIDITrack(ISequencer::TrackID id, const std::shared_ptr<mu::midi::MidiStream>& stream)
{
    ONLY_AUDIO_WORKER_THREAD;
    MidiTrack track = midiTrack(id);
    if (!track) {
        track = createMIDITrack(id);
    }
    track->loadMIDI(stream);
}

void Sequencer::setAudioTrack(ISequencer::TrackID id, const std::shared_ptr<audio::IAudioStream>& stream)
{
    ONLY_AUDIO_WORKER_THREAD;
    AudioTrack track = audioTrack(id);
    if (!track) {
        track = createAudioTrack(id);
    }
    track->load(stream);
}

mu::async::Channel<Sequencer::AudioTrack> Sequencer::audioTrackAdded() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_audioTrackAdded;
}

void Sequencer::initAudioTrack(ISequencer::TrackID id)
{
    ONLY_AUDIO_WORKER_THREAD;
    createAudioTrack(id);
}

mu::async::Channel<mu::midi::tick_t> Sequencer::midiTickPlayed(ISequencer::TrackID id) const
{
    ONLY_AUDIO_WORKER_THREAD;
    if (MidiTrack track = midiTrack(id)) {
        return track->tickPlayed();
    }
    return {};
}

mu::async::Notification Sequencer::positionChanged() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_positionChanged;
}

float Sequencer::playbackPositionInSeconds() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return clock()->timeInSeconds();
}

void Sequencer::setLoop(uint64_t fromMilliseconds, uint64_t toMilliseconds)
{
    ONLY_AUDIO_WORKER_THREAD;
    m_loopStart = fromMilliseconds;
    m_loopEnd = toMilliseconds;
}

void Sequencer::unsetLoop()
{
    ONLY_AUDIO_WORKER_THREAD;
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
    for (auto& val : m_tracks) {
        Track& track = val.second;
        track->forwardTime(m_clock->timeInMiliSeconds());
        willcontinue |= track->isRunning();
    }
    m_positionChanged.notify();
    if (!willcontinue) {
        stop();
    }
}

void Sequencer::beforeTimeUpdate(Clock::time_t milliseconds)
{
    if (m_nextStatus != m_status) {
        setStatus(m_nextStatus);
    }

    if (m_nextSeek.has_value()) {
        auto milliseconds = m_nextSeek.value_or(0);
        m_nextSeek.reset();

        m_clock->seekMiliseconds(milliseconds);
        for (auto& track : m_tracks) {
            track.second->seek(milliseconds);
        }
        m_positionChanged.notify();
    }

    std::for_each(m_backgroudPlayers.begin(), m_backgroudPlayers.end(), [&milliseconds](auto& player) {
        player.first += milliseconds;
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

    std::function<void(const Track&)> applyStatus = [](const Track&) {};
    switch (m_status) {
    case PLAYING:
        applyStatus = [](const Track& track) { track->run(); };
        m_clock->start();
        break;
    case PAUSED:
        applyStatus = [](const Track& track) { track->pause(); };
        m_clock->pause();
        break;
    case STOPED:
        applyStatus = [](const Track& track) { track->stop(); };
        m_clock->stop();
        break;
    }

    for (auto& track : m_tracks) {
        applyStatus(track.second);
    }
}

std::optional<Sequencer::Track> Sequencer::track(Sequencer::TrackID id) const
{
    if (m_tracks.find(id) != m_tracks.end()) {
        return m_tracks.at(id);
    }
    return std::nullopt;
}

Sequencer::MidiTrack Sequencer::midiTrack(TrackID id) const
{
    if (auto track_var = track(id)) {
        if (auto track_value = track_var.value_or(nullptr)) {
            return std::dynamic_pointer_cast<IMIDIPlayer>(track_value);
        }
    }
    return nullptr;
}

Sequencer::MidiTrack Sequencer::createMIDITrack(TrackID id)
{
    auto player = std::make_shared<MIDIPlayer>();
    m_tracks[id] = player;
    return player;
}

Sequencer::AudioTrack Sequencer::createAudioTrack(TrackID id)
{
    auto player = std::make_shared<AudioPlayer>();
    m_tracks[id] = player;
    m_audioTrackAdded.send(player);
    return player;
}

Sequencer::AudioTrack Sequencer::audioTrack(TrackID id) const
{
    if (auto track_var = track(id)) {
        if (auto track_value = track_var.value_or(nullptr)) {
            return std::dynamic_pointer_cast<IAudioPlayer>(track_value);
        }
    }
    return nullptr;
}

void Sequencer::instantlyPlayMidi(const midi::MidiData& data)
{
    ONLY_AUDIO_WORKER_THREAD;
    if (!data.isValid()) {
        return;
    }
    auto player = std::make_shared<MIDIPlayer>();
    auto midiStream = std::make_shared<midi::MidiStream>();

    midiStream->initData = data;
    midiStream->isStreamingAllowed = false;
    midiStream->lastTick = data.lastChunksTick();
    player->loadMIDI(midiStream);
    player->run();

    m_backgroudPlayers.push_back({ 0 /*ms*/, player });
}
