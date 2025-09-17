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

#include <memory>

#include "global/async/promise.h"
#include "global/async/channel.h"

#include "audio/common/audiotypes.h"

namespace muse::audio {
class IPlayer
{
public:
    virtual ~IPlayer() = default;

    virtual TrackSequenceId sequenceId() const = 0;

    virtual void play(const secs_t delay = 0) = 0;
    virtual void seek(const secs_t newPosition, const bool flushSound = true) = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume(const secs_t delay = 0) = 0;

    virtual PlaybackStatus playbackStatus() const = 0;
    virtual async::Channel<PlaybackStatus> playbackStatusChanged() const = 0;

    virtual void setDuration(const msecs_t durationMsec) = 0;
    virtual async::Promise<bool> setLoop(const msecs_t fromMsec, const msecs_t toMsec) = 0;
    virtual void resetLoop() = 0;

    virtual secs_t playbackPosition() const = 0;
    virtual async::Channel<secs_t> playbackPositionChanged() const = 0;
};

using IPlayerPtr = std::shared_ptr<IPlayer>;
}
