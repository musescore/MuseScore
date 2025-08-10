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
#ifndef MUSE_AUDIO_CLOCK_H
#define MUSE_AUDIO_CLOCK_H

#include "global/types/retval.h"
#include "global/async/asyncable.h"

#include "../iclock.h"

namespace muse::audio::worker {
class Clock : public IClock, public async::Asyncable
{
public:
    Clock();

    void forward(const msecs_t nextMsecs) override;

    void start() override;
    void reset() override;
    void stop() override;
    void pause() override;
    void resume() override;
    void seek(const msecs_t msecs) override;
    async::Notification seekOccurred() const override;
    bool isRunning() const override;

    PlaybackStatus status() const override;
    async::Channel<PlaybackStatus> statusChanged() const override;

    msecs_t timeDuration() const override;
    void setTimeDuration(const msecs_t duration) override;
    Ret setTimeLoop(const msecs_t fromMsec, const msecs_t toMsec) override;
    void resetTimeLoop() override;

    void setCountDown(const msecs_t duration) override;
    async::Notification countDownEnded() const override;

    msecs_t currentTime() const override;
    async::Channel<secs_t> timeChanged() const override;

private:
    void setCurrentTime(msecs_t time);

    ValCh<PlaybackStatus> m_status;
    msecs_t m_currentTime = 0;
    msecs_t m_timeDuration = 0;
    msecs_t m_timeLoopStart = 0;
    msecs_t m_timeLoopEnd = 0;
    msecs_t m_countDown = 0;

    async::Channel<secs_t> m_timeChangedInSecs;
    async::Notification m_seekOccurred;
    async::Notification m_countDownEnded;
};
}

#endif // MUSE_AUDIO_CLOCK_H
