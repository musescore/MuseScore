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

#include "global/types/retval.h"
#include "global/async/asyncable.h"

#include "../iclock.h"

namespace muse::audio::engine {
class Clock : public IClock, public async::Asyncable
{
public:
    Clock();

    void forward(const secs_t secs) override;

    void start() override;
    void reset() override;
    void stop() override;
    void pause() override;
    void resume() override;

    void seek(const secs_t newPosition) override;
    async::Notification seekOccurred() const override;

    bool isRunning() const override;

    PlaybackStatus status() const override;
    async::Channel<PlaybackStatus> statusChanged() const override;

    secs_t timeDuration() const override;
    void setTimeDuration(const secs_t duration) override;
    Ret setTimeLoop(const secs_t from, const secs_t to) override;
    void resetTimeLoop() override;

    void setCountDown(const secs_t duration) override;
    async::Notification countDownEnded() const override;

    secs_t currentTime() const override;
    async::Channel<secs_t> timeChanged() const override;

private:
    void setCurrentTime(secs_t time);

    ValCh<PlaybackStatus> m_status;
    secs_t m_currentTime = 0.;
    secs_t m_timeDuration = 0.;
    secs_t m_timeLoopStart = 0.;
    secs_t m_timeLoopEnd = 0.;
    secs_t m_countDown = 0.;

    async::Channel<secs_t> m_timeChanged;
    async::Notification m_seekOccurred;
    async::Notification m_countDownEnded;
};
}
