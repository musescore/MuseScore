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

#ifndef MU_AUDIO_ICLOCK_H
#define MU_AUDIO_ICLOCK_H

#include <memory>

#include "ret.h"
#include "async/channel.h"
#include "async/notification.h"

#include "audiotypes.h"

namespace mu::audio {
class IClock
{
public:
    virtual ~IClock() = default;

    virtual msecs_t currentTime() const = 0;

    virtual void forward(const msecs_t nextMsecs) = 0;

    virtual void start() = 0;
    virtual void reset() = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void seek(const msecs_t msecs) = 0;

    virtual Ret setTimeLoop(const msecs_t fromMsec, const msecs_t toMsec) = 0;
    virtual void resetTimeLoop() = 0;

    virtual bool isRunning() const = 0;

    virtual async::Channel<msecs_t> timeChanged() const = 0;
    virtual async::Notification seekOccurred() const = 0;
    virtual async::Channel<PlaybackStatus> statusChanged() const = 0;
};

using IClockPtr = std::shared_ptr<IClock>;
}

#endif // MU_AUDIO_ICLOCK_H
