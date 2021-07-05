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

#ifndef MU_AUDIO_TRACKSEQUENCE_H
#define MU_AUDIO_TRACKSEQUENCE_H

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "imixerchannel.h"
#include "itracksequence.h"
#include "igettracks.h"
#include "iaudiosource.h"
#include "iclock.h"
#include "track.h"
#include "audiotypes.h"

namespace mu::audio {
class Mixer;
class TrackSequence : public ITrackSequence, public IGetTracks, public async::Asyncable
{
public:
    TrackSequence(const TrackSequenceId id);
    ~TrackSequence();

    // ITrackSequence
    TrackSequenceId id() const override;

    RetVal<TrackId> addTrack(const std::string& trackName, const midi::MidiData& midiData, const AudioOutputParams& outputParams) override;
    RetVal<TrackId> addTrack(const std::string& trackName, const io::path& filePath, const AudioOutputParams& outputParams) override;

    TrackIdList trackIdList() const override;

    Ret removeTrack(const TrackId id) override;

    async::Channel<TrackId> trackAdded() const override;
    async::Channel<TrackId> trackRemoved() const override;

    ISequencePlayerPtr player() const override;
    ISequenceIOPtr audioIO() const override;

    // IGetTracks
    TrackPtr track(const TrackId id) const override;
    TracksMap allTracks() const override;

private:
    std::shared_ptr<Mixer> mixer() const;

    TrackSequenceId m_id = -1;

    TracksMap m_tracks;

    ISequencePlayerPtr m_player = nullptr;
    ISequenceIOPtr m_audioIO = nullptr;

    IClockPtr m_clock = nullptr;

    async::Channel<TrackId> m_trackAdded;
    async::Channel<TrackId> m_trackRemoved;
};
}

#endif // MU_AUDIO_TRACKSEQUENCE_H
