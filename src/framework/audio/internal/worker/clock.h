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
#ifndef MU_AUDIO_CLOCK_H
#define MU_AUDIO_CLOCK_H

#include "async/asyncable.h"

#include "iclock.h"

namespace mu::audio {
class Clock : public IClock, public async::Asyncable
{
public:
    Clock();

    msecs_t currentTime() const override;

    void forward(const msecs_t nextMsecs) override;

    void start() override;
    void reset() override;
    void stop() override;
    void pause() override;
    void resume() override;
    void seek(const msecs_t msecs) override;

    void setTimeDuration(const msecs_t duration) override;
    Ret setTimeLoop(const msecs_t fromMsec, const msecs_t toMsec) override;
    void resetTimeLoop() override;

    bool isRunning() const override;

    async::Channel<msecs_t> timeChanged() const override;
    async::Notification seekOccurred() const override;
    async::Channel<PlaybackStatus> statusChanged() const override;

private:

    ValCh<PlaybackStatus> m_status;
    msecs_t m_currentTime = 0;
    msecs_t m_timeDuration = 0;
    msecs_t m_timeLoopStart = 0;
    msecs_t m_timeLoopEnd = 0;

    async::Channel<msecs_t> m_timeChanged;
    async::Notification m_seekOccurred;
};
}

#endif // MU_AUDIO_CLOCK_H
