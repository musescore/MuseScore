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
#ifndef MU_AUDIO_SEQUENCER_H
#define MU_AUDIO_SEQUENCER_H

#include <memory>
#include <variant>
#include <optional>
#include <map>
#include "clock.h"
#include "isequencer.h"
#include "imidiplayer.h"
#include "rpc/irpctarget.h"

namespace mu::audio {
class Sequencer : public ISequencer, public async::Asyncable, public rpc::IRPCTarget
{
public:
    Sequencer();

    Status status() const override;
    async::Channel<Status> statusChanged() const override;

    bool play() override;
    void pause() override;
    void stop() override;
    void seek(unsigned long miliseconds) override;
    void rewind() override;
    void setLoop(unsigned int from, unsigned int to) override;
    void unsetLoop() override;

    std::shared_ptr<Clock> clock() const;

    void initMIDITrack(track_id id) override;
    void setMIDITrack(track_id id, const std::shared_ptr<midi::MidiStream>& stream) override;
    void setAudioTrack(track_id id, const std::shared_ptr<IAudioStream>& stream) override;

    async::Channel<audio_track_t> audioTrackAdded() const override;
    void initAudioTrack(track_id id) override;

    async::Channel<mu::midi::tick_t> midiTickPlayed(track_id id) const override;
    async::Notification positionChanged() const override;
    float playbackPosition() const override;

    std::shared_ptr<IMIDIPlayer> instantlyPlayMidi(const midi::MidiData& data) override;

protected:
    void buildRpcReflection() override;

private:
    void setStatus(Status status);
    void timeUpdate();
    void beforeTimeUpdate(Clock::time_t time);

    std::optional<track_t> track(track_id id) const;
    midi_track_t midiTrack(track_id id) const;
    midi_track_t createMIDITrack(track_id id);

    audio_track_t audioTrack(track_id id) const;
    audio_track_t createAudioTrack(track_id id);

    Status m_status = STOPED;
    std::atomic<Status> m_nextStatus = STOPED;
    std::shared_ptr<Clock> m_clock = nullptr;
    std::map<track_id, track_t> m_tracks = {};
    std::list<std::pair<Clock::time_t, track_t> > m_backgroudPlayers = {};

    async::Channel<Status> m_statusChanged;
    async::Channel<audio_track_t> m_audioTrackAdded;
    async::Notification m_positionChanged;

    std::mutex m_syncMutex;
    std::optional<Clock::time_t> m_loopStart, m_loopEnd;
    std::optional<unsigned long> m_nextSeek;
};
}

#endif // MU_AUDIO_SEQUENCER_H
