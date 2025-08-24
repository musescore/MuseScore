/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#pragma once

#include "global/async/asyncable.h"
#include "modularity/ioc.h"
#include "../iaudioengine.h"

#include "common/audiotypes.h"

#include "../itracksequence.h"
#include "../iclock.h"

#include "track.h"
#include "igettracks.h"

namespace muse::audio::worker {
class Mixer;
class TrackSequence : public ITrackSequence, public IGetTracks, public muse::Injectable, public async::Asyncable
{
    Inject<IAudioEngine> audioEngine = { this };

public:
    TrackSequence(const TrackSequenceId id, const muse::modularity::ContextPtr& iocCtx);
    ~TrackSequence() override;

    // ITrackSequence
    TrackSequenceId id() const override;

    RetVal2<TrackId, AudioParams> addTrack(const std::string& trackName, const mpe::PlaybackData& playbackData,
                                           const AudioParams& requiredParams) override;
    RetVal2<TrackId, AudioParams> addTrack(const std::string& trackName, io::IODevice* device, const AudioParams& requiredParams) override;

    RetVal2<TrackId, AudioOutputParams> addAuxTrack(const std::string& trackName, const AudioOutputParams& requiredOutputParams) override;

    TrackName trackName(const TrackId id) const override;
    TrackIdList trackIdList() const override;

    Ret removeTrack(const TrackId id) override;
    void removeAllTracks() override;

    async::Channel<TrackId> trackAdded() const override;
    async::Channel<TrackId> trackRemoved() const override;

    ISequencePlayerPtr player() const override;
    ISequenceIOPtr audioIO() const override;

    // IGetTracks
    TrackPtr track(const TrackId id) const override;
    const TracksMap& allTracks() const override;

    async::Channel<TrackPtr> trackAboutToBeAdded() const override;
    async::Channel<TrackPtr> trackAboutToBeRemoved() const override;

private:
    TrackId newTrackId() const;

    std::shared_ptr<Mixer> mixer() const;

    TrackSequenceId m_id = -1;

    TracksMap m_tracks;

    ISequencePlayerPtr m_player = nullptr;
    ISequenceIOPtr m_audioIO = nullptr;

    IClockPtr m_clock = nullptr;

    async::Channel<TrackId> m_trackAdded;
    async::Channel<TrackId> m_trackRemoved;

    async::Channel<TrackPtr> m_trackAboutToBeAdded;
    async::Channel<TrackPtr> m_trackAboutToBeRemoved;

    TrackId m_prevActiveTrackId = INVALID_TRACK_ID;
};
}
