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

    explicit Timer(time_t interval);
    ~Timer();

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    using Callback = std::function<void ()>;
    void onTimeout(const async::Asyncable* receiver, Callback callback);

    void start();
    void stop();

    bool isActive() const;
    float secondsSinceStart() const;

private:
    time_t m_interval;
    async::Notification m_notification;
    std::chrono::time_point<std::chrono::steady_clock> m_started;
    std::shared_ptr<void> m_handle;
};
}
