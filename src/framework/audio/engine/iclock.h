/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "global/types/ret.h"
#include "global/async/channel.h"
#include "global/async/notification.h"

#include "audio/common/audiotypes.h"

namespace muse::audio::engine {
class IClock
{
public:
    virtual ~IClock() = default;

    virtual secs_t currentTime() const = 0;

    virtual void forward(const secs_t secs) = 0;

    virtual void start() = 0;
    virtual void reset() = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;

    virtual void seek(const secs_t newPosition) = 0;
    virtual async::Notification seekOccurred() const = 0;

    virtual bool isRunning() const = 0;

    virtual PlaybackStatus status() const = 0;
    virtual async::Channel<PlaybackStatus> statusChanged() const = 0;

    virtual secs_t timeDuration() const = 0;
    virtual void setTimeDuration(const secs_t duration) = 0;
    virtual Ret setTimeLoop(const secs_t fromSec, const secs_t toSec) = 0;
    virtual void resetTimeLoop() = 0;

    virtual void setCountDown(const secs_t duration) = 0;
    virtual async::Notification countDownEnded() const = 0;

    virtual async::Channel<secs_t> timeChanged() const = 0;
};

using IClockPtr = std::shared_ptr<IClock>;
}
