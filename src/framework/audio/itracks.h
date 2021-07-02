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

#ifndef MU_AUDIO_ITRACKSHANDLER_H
#define MU_AUDIO_ITRACKSHANDLER_H

#include <memory>

#include "async/promise.h"
#include "async/channel.h"
#include "midi/miditypes.h"
#include "io/path.h"

#include "audiotypes.h"

namespace mu::audio {
class ITracks
{
public:
    virtual ~ITracks() = default;

    virtual async::Promise<TrackIdList> trackIdList(const TrackSequenceId sequenceId) const = 0;
    virtual async::Promise<TrackId> addTrack(const TrackSequenceId sequenceId, const std::string& trackName, midi::MidiData&& inParams,
                                             AudioOutputParams&& outParams) = 0;
    virtual async::Promise<TrackId> addTrack(const TrackSequenceId sequenceId, const std::string& trackName, io::path&& inParams,
                                             AudioOutputParams&& outParams) = 0;
    virtual void removeTrack(const TrackSequenceId sequenceId, const TrackId trackId) = 0;
    virtual void removeAllTracks(const TrackSequenceId sequenceId) = 0;

    virtual async::Channel<TrackSequenceId, TrackId> trackAdded() const = 0;
    virtual async::Channel<TrackSequenceId, TrackId> trackRemoved() const = 0;

    virtual async::Promise<AudioInputParams> inputParams(const TrackSequenceId sequenceId, const TrackId trackId) const = 0;
    virtual void setInputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioInputParams& params) = 0;
    virtual async::Channel<TrackSequenceId, TrackId, AudioInputParams> inputParamsChanged() const = 0;
};

using ITracksPtr = std::shared_ptr<ITracks>;
}

#endif // MU_AUDIO_ITRACKSHANDLER_H
