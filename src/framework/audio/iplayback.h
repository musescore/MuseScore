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
#ifndef MU_AUDIO_ISEQUENCER_H
#define MU_AUDIO_ISEQUENCER_H

#include "modularity/imoduleexport.h"
#include "async/channel.h"
#include "async/promise.h"

#include "iplayers.h"
#include "itracks.h"
#include "iaudioio.h"
#include "audiotypes.h"

namespace mu::audio {
class IPlayback : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IPlayback)

public:
    virtual ~IPlayback() = default;

    virtual async::Promise<TrackSequenceId> addSequence() = 0;
    virtual async::Promise<TrackSequenceIdList> sequenceIdList() const = 0;
    virtual void removeSequence(const TrackSequenceId id) = 0;

    virtual ITracksPtr tracks() const = 0;
    virtual IPlayersPtr players() const = 0;
    virtual IAudioIOPtr audioIO() const = 0;
};

using IPlaybackPtr = std::shared_ptr<IPlayback>;
}
#endif // MU_AUDIO_ISEQUENCER_H
