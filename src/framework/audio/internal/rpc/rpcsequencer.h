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
#ifndef MU_AUDIO_RPCSEQUENCER_H
#define MU_AUDIO_RPCSEQUENCER_H

#include "isequencer.h"
#include "irpcchannel.h"
#include "modularity/ioc.h"

namespace mu::audio::rpc {
class RpcSequencer : public ISequencer
{
    INJECT(audio, IRpcChannel, rpcChannel)

public:
    RpcSequencer();
    ~RpcSequencer();

    void setup();

    Status status() const override;
    async::Channel<Status> statusChanged() const override;

    void initMIDITrack(TrackID id) override;
    void initAudioTrack(TrackID id) override;
    void setMIDITrack(TrackID id, const std::shared_ptr<midi::MidiStream>& stream) override;
    void setAudioTrack(TrackID id, const std::shared_ptr<audio::IAudioStream>& stream) override;

    void play() override;
    void pause() override;
    void stop() override;
    void seek(uint64_t position) override;
    void rewind() override;
    void setLoop(uint64_t fromMilliseconds, uint64_t toMilliseconds) override;
    void unsetLoop() override;

    float playbackPositionInSeconds() const override;
    async::Notification positionChanged() const override;
    async::Channel<mu::midi::tick_t> midiTickPlayed(TrackID id) const override;

    void instantlyPlayMidi(const midi::MidiData& data) override;

private:
    Target m_target;
    IRpcChannel::ListenID m_listenID = -1;
    Status m_status = Status::STOPED;
    async::Channel<Status> m_statusChanged;
    async::Notification m_positionChanged;
    float m_playbackPositionInSeconds = 0.f;

    mutable std::map<TrackID, async::Channel<midi::tick_t> > m_midiTickPlayed;
};
}

#endif // MU_AUDIO_RPCSEQUENCER_H
