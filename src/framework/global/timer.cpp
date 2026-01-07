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

#include "timer.h"

#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>

namespace muse {
class TimerScheduler
{
public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = Clock::duration;
    using Callback = std::function<void ()>;

    static TimerScheduler& instance()
    {
        static TimerScheduler scheduler;
        return scheduler;
    }

    struct TimerHandle {
        std::atomic_bool active { true };
    };

    using TimerHandlePtr = std::shared_ptr<TimerHandle>;

    TimerHandlePtr schedule(Duration delay, Callback callback, bool repeat = false)
    {
        auto handle = std::make_shared<TimerHandle>();

        {
            std::lock_guard lock(m_mutex);
            m_queue.push({
                    Clock::now() + delay,
                    delay,
                    repeat,
                    std::move(callback),
                    handle
                });
        }

        m_cv.notify_one();
        return handle;
    }

    void cancel(const TimerHandlePtr& handle)
    {
        if (handle) {
            handle->active = false;
        }
        m_cv.notify_one();
    }

private:
    TimerScheduler()
        : m_running(true),
        m_thread([this] { run(); })
    {}

    ~TimerScheduler()
    {
        {
            std::lock_guard lock(m_mutex);
            m_running = false;
        }
        m_cv.notify_one();
        m_thread.join();
    }

    struct Entry {
        TimePoint time;
        Duration interval;
        bool repeat = false;
        Callback callback;
        TimerHandlePtr handle;

        bool operator>(const Entry& other) const
        {
            return time > other.time;
        }
    };

    void run()
    {
        std::unique_lock lock(m_mutex);

        while (m_running) {
            if (m_queue.empty()) {
                m_cv.wait(lock);
                continue;
            }

            auto& next = m_queue.top();
            if (m_cv.wait_until(lock, next.time) == std::cv_status::timeout) {
                auto entry = m_queue.top();
                m_queue.pop();

                if (entry.handle->active) {
                    lock.unlock();
                    entry.callback();
                    lock.lock();

                    if (entry.repeat && entry.handle->active) {
                        entry.time = Clock::now() + entry.interval;
                        m_queue.push(entry);
                    }
                }
            }
        }
    }

    std::priority_queue<Entry,
                        std::vector<Entry>,
                        std::greater<> > m_queue;

    bool m_running = false;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::thread m_thread;
};
}

using namespace muse;

Timer::Timer(time_t interval)
    : m_interval(interval) {}

Timer::~Timer()
{
    stop();
}

void Timer::onTimeout(const async::Asyncable* receiver, Callback callback)
{
    m_notification.onNotify(receiver, std::move(callback), async::Asyncable::Mode::SetReplace);
}

void Timer::start()
{
    if (m_handle) {
        return;
    }

    m_started = std::chrono::steady_clock::now();

    auto realHandle = TimerScheduler::instance().schedule(
        m_interval,
        [this]() { m_notification.notify(); },
        true
        );

    m_handle = std::static_pointer_cast<void>(realHandle);
}

void Timer::stop()
{
    if (!m_handle) {
        return;
    }

    auto realHandle = std::static_pointer_cast<TimerScheduler::TimerHandle>(m_handle);
    TimerScheduler::instance().cancel(realHandle);
    m_handle = nullptr;
}

bool Timer::isActive() const
{
    return m_handle != nullptr;
}

float Timer::secondsSinceStart() const
{
    std::chrono::duration<float, std::ratio<1> > diff(std::chrono::steady_clock::now() - m_started);
    return diff.count();
}
