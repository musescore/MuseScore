/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#ifndef MU_AUDIO_SEQUENCER_H
#define MU_AUDIO_SEQUENCER_H

#include <memory>
#include <variant>
#include <optional>
#include <map>
#include "clock.h"
#include "isequencer.h"
#include "imidiplayer.h"
#include "iaudioplayer.h"

namespace mu::audio {
class Sequencer : public ISequencer, public async::Asyncable
{
public:
    Sequencer();
    ~Sequencer();

    using MidiTrack = std::shared_ptr<IMIDIPlayer>;
    using AudioTrack = std::shared_ptr<IAudioPlayer>;
    using Track = std::shared_ptr<IPlayer>;

    Status status() const override;
    async::Channel<Status> statusChanged() const override;

    void play() override;
    void pause() override;
    void stop() override;
    void seek(uint64_t milliseconds) override;
    void rewind() override;
    void setLoop(uint64_t fromMilliseconds, uint64_t toMilliseconds) override;
    void unsetLoop() override;

    std::shared_ptr<Clock> clock() const;

    void initMIDITrack(TrackID id) override;
    void setMIDITrack(TrackID id, const std::shared_ptr<midi::MidiStream>& stream) override;
    void setAudioTrack(TrackID id, const std::shared_ptr<IAudioStream>& stream) override;

    async::Channel<AudioTrack> audioTrackAdded() const;
    void initAudioTrack(TrackID id) override;

    async::Channel<mu::midi::tick_t> midiTickPlayed(TrackID id) const override;
    async::Notification positionChanged() const override;
    float playbackPositionInSeconds() const override;

    void instantlyPlayMidi(const midi::MidiData& data) override;

private:
    void setStatus(Status status);
    void timeUpdate();
    void beforeTimeUpdate(Clock::time_t time);

    std::optional<Track> track(TrackID id) const;
    MidiTrack midiTrack(TrackID id) const;
    MidiTrack createMIDITrack(TrackID id);

    AudioTrack audioTrack(TrackID id) const;
    AudioTrack createAudioTrack(TrackID id);

    Status m_status = STOPED;
    std::atomic<Status> m_nextStatus = STOPED;
    std::shared_ptr<Clock> m_clock = nullptr;
    std::map<TrackID, Track> m_tracks = {};
    std::list<std::pair<Clock::time_t, Track> > m_backgroudPlayers = {};

    async::Channel<Status> m_statusChanged;
    async::Channel<AudioTrack> m_audioTrackAdded;
    async::Notification m_positionChanged;

    std::optional<Clock::time_t> m_loopStart, m_loopEnd;
    std::optional<uint64_t> m_nextSeek;
};
}

#endif // MU_AUDIO_SEQUENCER_H
