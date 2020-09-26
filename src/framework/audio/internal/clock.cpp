//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "clock.h"

using namespace mu::audio;

Clock::Clock()
{
}

Clock::time_t Clock::time() const
{
    return m_time;
}

float Clock::timeInSeconds() const
{
    return m_time / static_cast<float>(m_sampleRate);
}

Clock::time_t Clock::timeInMiliSeconds() const
{
    return m_time * 1000 / m_sampleRate;
}

void Clock::setSampleRate(unsigned int sampleRate)
{
    m_sampleRate = sampleRate;
}

void Clock::forward(Clock::time_t samples)
{
    auto deltaMiliseconds = samples * 1000 / m_sampleRate;
    runCallbacks(m_beforeCallbacks, deltaMiliseconds);

    if (m_status == Running) {
        m_time += samples;
        m_timeChanged.send(m_time);
        runCallbacks(m_afterCallbacks, deltaMiliseconds);
    }
}

void Clock::start()
{
    m_status = Running;
}

void Clock::reset()
{
    m_time = 0;
}

void Clock::stop()
{
    m_status = Stoped;
    reset();
}

void Clock::seek(time_t time)
{
    m_time = time;
    m_timeChanged.send(m_time);
}

void Clock::seekMiliseconds(Clock::time_t value)
{
    m_time = value * m_sampleRate / 1000;
    m_timeChanged.send(m_time);
}

void Clock::seekSeconds(float seconds)
{
    seek(seconds * m_sampleRate);
}

mu::async::Channel<Clock::time_t> Clock::timeChanged() const
{
    return m_timeChanged;
}

void Clock::addBeforeCallback(Clock::syncCallback callback)
{
    m_beforeCallbacks.push_back(callback);
}

void Clock::addAfterCallback(Clock::syncCallback callback)
{
    m_afterCallbacks.push_back(callback);
}

void Clock::runCallbacks(const std::list<Clock::syncCallback>& list, time_t miliseconds)
{
    for (auto& callback : list) {
        callback(miliseconds);
    }
}
