/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "async/notification.h"
#include "async/asyncable.h"

#include <chrono>
#include <thread>

namespace muse {
/*!
 * muse::Timer
 * usage:
 *      auto timer = new Timer(std::chrono::microseconds(20));
 *      timer.onTimeout(this, []() { LOGI() << "Timer call";});
 *      timer.start();
 */
class Timer
{
public:
    using time_t = std::chrono::microseconds;

    explicit Timer(time_t interval)
        : m_interval(interval) {}

    ~Timer()
    {
        stop();

        if (m_thread.joinable()) {
            m_thread.join();
        }
    }

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    template<typename Func>
    void onTimeout(const async::Asyncable* receiver, Func function)
    {
        m_notification.onNotify(receiver, function, async::Asyncable::Mode::SetReplace);
    }

    void start()
    {
        if (m_active) {
            return;
        }

        m_active = true;
        m_started = std::chrono::steady_clock::now();
        m_thread = std::thread([this]() { timerLoop(); });
    }

    void stop()
    {
        m_active = false;
    }

    bool isActive() const
    {
        return m_active;
    }

    float secondsSinceStart() const
    {
        std::chrono::duration<float, std::ratio<1> > diff(std::chrono::steady_clock::now() - m_started);
        return diff.count();
    }

private:
    void timerLoop()
    {
        std::this_thread::sleep_for(m_interval);
        while (m_active) {
            auto start = std::chrono::steady_clock::now();
            m_notification.notify();
            auto end = std::chrono::steady_clock::now();
            if (m_interval > (end - start)) {
                auto diff = m_interval - (end - start);
                std::this_thread::sleep_for(diff);
            }
        }
    }

    time_t m_interval;
    std::chrono::time_point<std::chrono::steady_clock> m_started;
    std::atomic_bool m_active { false };
    async::Notification m_notification;
    std::thread m_thread;
};
}
