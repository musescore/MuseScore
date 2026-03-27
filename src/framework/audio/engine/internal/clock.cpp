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
#include "clock.h"

#include "common/audioerrors.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::engine;

Clock::Clock()
    : m_timeChanged(async::makeOpt()
                    .name("audio::clock::timeChanged")
                    .disableWaitPendingsOnSend())
{
    m_status.set(PlaybackStatus::Stopped);
}

secs_t Clock::currentTime() const
{
    return m_currentTime;
}

void Clock::forward(const secs_t secs)
{
    if (!isRunning()) {
        return;
    }

    if (!m_countDown.is_zero()) {
        m_countDown -= secs;

        if (m_countDown > 0.) {
            return;
        }

        m_countDown = 0.;
        m_countDownEnded.notify();
    }

    secs_t newTime = m_currentTime + secs;

    if (m_timeLoopStart < m_timeLoopEnd && newTime >= m_timeLoopEnd) {
        seek(m_timeLoopStart);

        //!Note No matter of the time loop boundaries, the current frame still should be handled
        setCurrentTime(m_timeLoopStart + secs);
        return;
    }

    if (newTime >= m_timeDuration) {
        setCurrentTime(m_timeDuration);
        pause();
        return;
    }

    setCurrentTime(newTime);
}

void Clock::setCurrentTime(secs_t time)
{
    if (m_currentTime == time) {
        return;
    }

    m_currentTime = time;
    m_timeChanged.send(m_currentTime);
}

void Clock::start()
{
    m_status.set(PlaybackStatus::Running);
}

void Clock::reset()
{
    seek(0.);
    resetTimeLoop();
    m_countDown = 0.;
}

void Clock::stop()
{
    m_status.set(PlaybackStatus::Stopped);
    seek(0.);
    m_countDown = 0.;
}

void Clock::pause()
{
    m_status.set(PlaybackStatus::Paused);
}

void Clock::resume()
{
    m_status.set(PlaybackStatus::Running);
    seek(m_currentTime);
}

void Clock::seek(const secs_t newPosition)
{
    if (m_currentTime == newPosition) {
        return;
    }

    setCurrentTime(newPosition);
    m_seekOccurred.notify();
}

secs_t Clock::timeDuration() const
{
    return m_timeDuration;
}

void Clock::setTimeDuration(const secs_t duration)
{
    m_timeDuration = duration;
}

Ret Clock::setTimeLoop(const secs_t from, const secs_t to)
{
    if (from >= to) {
        return make_ret(Err::InvalidTimeLoop);
    }

    m_timeLoopStart = from;
    m_timeLoopEnd = to;

    return Ret(Ret::Code::Ok);
}

void Clock::resetTimeLoop()
{
    m_timeLoopStart = 0;
    m_timeLoopEnd = 0;
}

void Clock::setCountDown(const secs_t duration)
{
    m_countDown = duration;
}

async::Notification Clock::countDownEnded() const
{
    return m_countDownEnded;
}

bool Clock::isRunning() const
{
    return m_status.val == PlaybackStatus::Running;
}

async::Channel<secs_t> Clock::timeChanged() const
{
    return m_timeChanged;
}

async::Notification Clock::seekOccurred() const
{
    return m_seekOccurred;
}

PlaybackStatus Clock::status() const
{
    return m_status.val;
}

async::Channel<PlaybackStatus> Clock::statusChanged() const
{
    return m_status.ch;
}
