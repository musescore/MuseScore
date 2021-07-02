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

#include "audioerrors.h"

using namespace mu;
using namespace mu::audio;

Clock::Clock()
{
    m_status.set(PlaybackStatus::Stopped);
}

msecs_t Clock::currentTime() const
{
    return m_time;
}

void Clock::forward(const msecs_t nextMsecs)
{
    if (!isRunning()) {
        return;
    }

    m_time += nextMsecs;

    if (m_timeLoopStart < m_timeLoopEnd && m_time >= m_timeLoopEnd) {
        seek(m_timeLoopStart);
    }

    m_timeChanged.send(m_time);
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
    seek(m_time);
}

void Clock::seek(const msecs_t msecs)
{
    m_time = msecs;
    m_timeChanged.send(m_time);
    m_seekOccurred.notify();
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

async::Channel<msecs_t> Clock::timeChanged() const
{
    return m_timeChanged;
}

async::Notification Clock::seekOccurred() const
{
    return m_seekOccurred;
}

async::Channel<PlaybackStatus> Clock::statusChanged() const
{
    return m_status.ch;
}
