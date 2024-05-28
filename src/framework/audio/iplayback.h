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
#ifndef MUSE_AUDIO_ISEQUENCER_H
#define MUSE_AUDIO_ISEQUENCER_H

#include "modularity/imoduleinterface.h"
#include "global/async/channel.h"
#include "global/async/promise.h"

#include "audiotypes.h"

namespace muse::audio {
class ITracks;
class IPlayer;
class IAudioOutput;

class IPlayback : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IPlayback)

public:
    virtual ~IPlayback() = default;

    virtual void init() = 0;
    virtual void deinit() = 0;
    virtual bool isInited() const = 0;

    // A quick guide how to playback something:

    // 1. Add Sequence
    virtual async::Promise<TrackSequenceId> addSequence() = 0;
    virtual async::Promise<TrackSequenceIdList> sequenceIdList() const = 0;
    virtual void removeSequence(const TrackSequenceId id) = 0;

    virtual async::Channel<TrackSequenceId> sequenceAdded() const = 0;
    virtual async::Channel<TrackSequenceId> sequenceRemoved() const = 0;

    // 2. Setup tracks for Sequence
    virtual std::shared_ptr<ITracks> tracks() const = 0;

    // 3. Play Sequence
    virtual std::shared_ptr<IPlayer> player(const TrackSequenceId id) const = 0;

    // 4. Adjust a Sequence output
    virtual std::shared_ptr<IAudioOutput> audioOutput() const = 0;
};

using IPlaybackPtr = std::shared_ptr<IPlayback>;
}
#endif // MUSE_AUDIO_ISEQUENCER_H
