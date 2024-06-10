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
#include "clock.h"

#include "../../audioerrors.h"

using namespace muse;
using namespace muse::audio;

Clock::Clock()
{
    m_status.set(PlaybackStatus::Stopped);
}

msecs_t Clock::currentTime() const
{
    return m_currentTime;
}

void Clock::forward(const msecs_t nextMsecs)
{
    if (!isRunning()) {
        return;
    }

    msecs_t newTime = m_currentTime + nextMsecs;

    if (m_timeLoopStart < m_timeLoopEnd && newTime >= m_timeLoopEnd) {
        seek(m_timeLoopStart);

        //!Note No matter of the time loop boundaries, the current frame still should be handled
        setCurrentTime(m_timeLoopStart + nextMsecs);
        return;
    }

    if (newTime >= m_timeDuration) {
        setCurrentTime(m_timeDuration);
        pause();
        return;
    }

    setCurrentTime(newTime);
}

void Clock::setCurrentTime(msecs_t time)
{
    if (m_currentTime == time) {
        return;
    }

    m_currentTime = time;
    m_timeChangedInSecs.send(microsecsToSecs(m_currentTime));
}

void Clock::start()
{
    m_status.set(PlaybackStatus::Running);
}

void Clock::reset()
{
    seek(0);
    resetTimeLoop();
}

void Clock::stop()
{
    m_status.set(PlaybackStatus::Stopped);
    seek(0);
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

void Clock::seek(const msecs_t msecs)
{
    if (m_currentTime == msecs) {
        return;
    }

    setCurrentTime(msecs);
    m_seekOccurred.notify();
}

msecs_t Clock::timeDuration() const
{
    return m_timeDuration;
}

void Clock::setTimeDuration(const msecs_t duration)
{
    m_timeDuration = duration;
}

Ret Clock::setTimeLoop(const msecs_t fromMsec, const msecs_t toMsec)
{
    if (fromMsec >= toMsec) {
        return make_ret(Err::InvalidTimeLoop);
    }

    m_timeLoopStart = fromMsec;
    m_timeLoopEnd = toMsec;

    return Ret(Ret::Code::Ok);
}

void Clock::resetTimeLoop()
{
    m_timeLoopStart = 0;
    m_timeLoopEnd = 0;
}

bool Clock::isRunning() const
{
    return m_status.val == PlaybackStatus::Running;
}

async::Channel<secs_t> Clock::timeChanged() const
{
    return m_timeChangedInSecs;
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
