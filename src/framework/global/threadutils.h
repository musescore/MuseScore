/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

#include <thread>

#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <pthread.h>
#endif

namespace muse {
enum class ThreadPriority {
    Low,
    Medium,
    High
};

static bool setThreadPriority(std::thread& thread, ThreadPriority priority)
{
#ifdef Q_OS_WIN
    int winPriority = 0;
    switch (priority) {
    case ThreadPriority::Low:
        winPriority = THREAD_PRIORITY_LOWEST;
        break;
    case ThreadPriority::Medium:
        winPriority = THREAD_PRIORITY_NORMAL;
        break;
    case ThreadPriority::High:
        winPriority = THREAD_PRIORITY_HIGHEST;
        break;
    }

    if (!SetThreadPriority(thread.native_handle(), winPriority)) {
        return false;
    }
#else
    pthread_t pthread = thread.native_handle();
    struct sched_param param;
    int policy = 0;
    int error = pthread_getschedparam(pthread, &policy, &param);
    if (error != 0) {
        return false;
    }

    int minP = sched_get_priority_min(policy);
    int maxP = sched_get_priority_max(policy);
    switch (priority) {
    case ThreadPriority::Low:
        param.sched_priority = minP;
        break;
    case ThreadPriority::Medium:
        param.sched_priority = minP + (maxP - minP) / 2;
        break;
    case ThreadPriority::High:
        param.sched_priority = maxP;
        break;
    }

    error = pthread_setschedparam(pthread, policy, &param);
    if (error != 0) {
        return false;
    }
#endif

    return true;
}
}
