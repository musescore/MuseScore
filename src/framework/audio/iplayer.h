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

#ifndef MUSE_AUDIO_IPLAYER_H
#define MUSE_AUDIO_IPLAYER_H

#include <memory>

#include "global/async/promise.h"
#include "global/async/channel.h"

#include "audiotypes.h"

namespace muse::audio {
class IPlayerHandler
{
public:
    virtual ~IPlayerHandler() = default;

    virtual void play(const TrackSequenceId sequenceId) = 0;
    virtual void seek(const TrackSequenceId sequenceId, const msecs_t newPositionMsecs) = 0;
    virtual void stop(const TrackSequenceId sequenceId) = 0;
    virtual void pause(const TrackSequenceId sequenceId) = 0;
    virtual void resume(const TrackSequenceId sequenceId) = 0;

    virtual void setDuration(const TrackSequenceId sequenceId, const msecs_t durationMsec) = 0;
    virtual async::Promise<bool> setLoop(const TrackSequenceId sequenceId, const msecs_t fromMsec, const msecs_t toMsec) = 0;
    virtual void resetLoop(const TrackSequenceId sequenceId) = 0;

    virtual async::Channel<TrackSequenceId, msecs_t> playbackPositionMsecs() const = 0;
    virtual async::Channel<TrackSequenceId, PlaybackStatus> playbackStatusChanged() const = 0;
};

using IPlayerHandlerPtr = std::shared_ptr<IPlayerHandler>;

class IPlayer
{
public:
    virtual ~IPlayer() = default;

    virtual TrackSequenceId sequenceId() const = 0;

    virtual void play() = 0;
    virtual void seek(const msecs_t newPositionMsecs) = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;

    virtual void setDuration(const msecs_t durationMsec) = 0;
    virtual async::Promise<bool> setLoop(const msecs_t fromMsec, const msecs_t toMsec) = 0;
    virtual void resetLoop() = 0;

    virtual async::Channel<msecs_t> playbackPositionMsecs() const = 0;
    virtual async::Channel<PlaybackStatus> playbackStatusChanged() const = 0;
};

using IPlayerPtr = std::shared_ptr<IPlayer>;
}

#endif // MUSE_AUDIO_IPLAYER_H
