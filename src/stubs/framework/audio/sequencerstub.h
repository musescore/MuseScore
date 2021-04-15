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
#ifndef MU_AUDIO_SEQUENCERSTUB_H
#define MU_AUDIO_SEQUENCERSTUB_H

#include "audio/isequencer.h"

namespace mu::audio {
class SequencerStub : public ISequencer
{
public:
    Status status() const override;
    async::Channel<Status> statusChanged() const override;

    void initMIDITrack(TrackID id) override;
    void initAudioTrack(TrackID id) override;
    void setMIDITrack(TrackID id, const std::shared_ptr<midi::MidiStream>& stream) override;
    void setAudioTrack(TrackID id, const std::shared_ptr<audio::IAudioStream>& stream) override;

    void play() override;
    void pause() override;
    void stop() override;
    void seek(uint64_t sec) override;
    void rewind() override;
    void setLoop(uint64_t fromMilliseconds, uint64_t toMilliseconds) override;
    void unsetLoop() override;

    async::Channel<midi::tick_t> midiTickPlayed(TrackID id) const override;
    async::Notification positionChanged() const override;

    float playbackPosition() const override;

    void instantlyPlayMidi(const midi::MidiData& data) override;
};
}

#endif // MU_AUDIO_SEQUENCERSTUB_H
