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

#ifndef MU_AUDIO_ITRACKSEQUENCE_H
#define MU_AUDIO_ITRACKSEQUENCE_H

#include "async/channel.h"
#include "retval.h"

#include "iaudiosource.h"
#include "isequenceplayer.h"
#include "isequenceio.h"
#include "audiotypes.h"

namespace mu::audio {
class ITrackSequence
{
public:
    virtual ~ITrackSequence() = default;

    virtual TrackSequenceId id() const = 0;

    virtual RetVal<TrackId> addTrack(const std::string& trackName, const midi::MidiData& midiData,
                                     const AudioOutputParams& outputParams) = 0;
    virtual RetVal<TrackId> addTrack(const std::string& trackName, const io::path& filePath, const AudioOutputParams& outputParams) = 0;

    virtual TrackIdList trackIdList() const = 0;

    virtual Ret removeTrack(const TrackId id) = 0;

    virtual async::Channel<TrackId> trackAdded() const = 0;
    virtual async::Channel<TrackId> trackRemoved() const = 0;

    virtual ISequencePlayerPtr player() const = 0;
    virtual ISequenceIOPtr audioIO() const = 0;
};

using ITrackSequencePtr = std::shared_ptr<ITrackSequence>;
}

#endif // MU_AUDIO_ITRACKSEQUENCE_H
