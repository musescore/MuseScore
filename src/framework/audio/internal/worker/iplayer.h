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
#ifndef MU_AUDIO_IPLAYER_H
#define MU_AUDIO_IPLAYER_H

#include "async/channel.h"

namespace mu::audio {
class IPlayer
{
public:
    enum Status {
        Stoped = 0,
        Paused,
        Running,
        Error
    };

    virtual ~IPlayer() = default;

    virtual Status status() const = 0;
    virtual async::Channel<Status> statusChanged() const = 0;

    virtual bool isRunning() const = 0;

    virtual void run() = 0;
    virtual void seek(unsigned long milliseconds) = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;

    virtual unsigned long milliseconds() const = 0;
    virtual void forwardTime(unsigned long milliseconds) = 0;
};
}
#endif // MU_AUDIO_IPLAYER_H
