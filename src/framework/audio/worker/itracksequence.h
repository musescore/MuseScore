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

#include "global/async/channel.h"
#include "global/types/retval.h"
#include "mpe/events.h"

#include "audio/common/audiotypes.h"

#include "isequenceplayer.h"
#include "isequenceio.h"

namespace muse::audio::worker {
class ITrackSequence
{
public:
    virtual ~ITrackSequence() = default;

    virtual TrackSequenceId id() const = 0;

    virtual RetVal2<TrackId, AudioParams> addTrack(const std::string& trackName, const mpe::PlaybackData& playbackData,
                                                   const AudioParams& requiredParams) = 0;
    virtual RetVal2<TrackId, AudioParams> addTrack(const std::string& trackName, io::IODevice* device,
                                                   const AudioParams& requiredParams) = 0;

    virtual RetVal2<TrackId, AudioOutputParams> addAuxTrack(const std::string& trackName,
                                                            const AudioOutputParams& requiredOutputParams) = 0;

    virtual TrackName trackName(const TrackId id) const = 0;
    virtual TrackIdList trackIdList() const = 0;

    virtual Ret removeTrack(const TrackId id) = 0;
    virtual void removeAllTracks() = 0;

    virtual async::Channel<TrackId> trackAdded() const = 0;
    virtual async::Channel<TrackId> trackRemoved() const = 0;

    virtual ISequencePlayerPtr player() const = 0;
    virtual ISequenceIOPtr audioIO() const = 0;
};

using ITrackSequencePtr = std::shared_ptr<ITrackSequence>;
}
