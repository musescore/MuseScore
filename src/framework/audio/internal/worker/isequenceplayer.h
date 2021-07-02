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

#ifndef MU_AUDIO_ISEQUENCEPLAYER_H
#define MU_AUDIO_ISEQUENCEPLAYER_H

#include "ret.h"
#include "async/channel.h"

#include "audiotypes.h"

namespace mu::audio {
class ISequencePlayer
{
public:
    virtual ~ISequencePlayer() = default;

    virtual void play() = 0;
    virtual void seek(const msecs_t newPositionMsecs) = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;

    virtual Ret setLoop(const msecs_t fromMsec, const msecs_t toMsec) = 0;
    virtual void resetLoop() = 0;

    virtual async::Channel<msecs_t> playbackPositionMSecs() const = 0;
    virtual async::Channel<PlaybackStatus> playbackStatusChanged() const = 0;
};
using ISequencePlayerPtr = std::shared_ptr<ISequencePlayer>;
}

#endif // MU_AUDIO_ISEQUENCEPLAYER_H
