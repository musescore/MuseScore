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
#include "sequencerstub.h"

using namespace mu::audio;
using namespace mu;

ISequencer::Status SequencerStub::status() const
{
    return ISequencer::Status();
}

async::Channel<ISequencer::Status> SequencerStub::statusChanged() const
{
    return async::Channel<ISequencer::Status>();
}

void SequencerStub::initMIDITrack(ISequencer::TrackID)
{
}

void SequencerStub::initAudioTrack(ISequencer::TrackID)
{
}

void SequencerStub::setMIDITrack(ISequencer::TrackID, const std::shared_ptr<midi::MidiStream>&)
{
}

void SequencerStub::setAudioTrack(ISequencer::TrackID, const std::shared_ptr<IAudioStream>&)
{
}

void SequencerStub::play()
{
}

void SequencerStub::pause()
{
}

void SequencerStub::stop()
{
}

void SequencerStub::seek(uint64_t)
{
}

void SequencerStub::rewind()
{
}

void SequencerStub::setLoop(uint64_t, uint64_t)
{
}

void SequencerStub::unsetLoop()
{
}

async::Channel<midi::tick_t> SequencerStub::midiTickPlayed(ISequencer::TrackID) const
{
    return async::Channel<midi::tick_t>();
}

async::Notification SequencerStub::positionChanged() const
{
    return async::Notification();
}

float SequencerStub::playbackPosition() const
{
    return 0.f;
}

void SequencerStub::instantlyPlayMidi(const midi::MidiData&)
{
}
