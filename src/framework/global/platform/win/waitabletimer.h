/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include <windows.h>

#include "log.h"

class WaitableTimer
{
public:
    WaitableTimer() = default;

    ~WaitableTimer()
    {
        ::CloseHandle(m_timer);
        m_timer = NULL;
    }

    bool init()
    {
#ifdef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
        m_timer = ::CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
                                           TIMER_ALL_ACCESS);
        if (!m_timer) {
            LOGE() << "Failed to create waitable time, error:" + std::to_string(::GetLastError());
            return false;
        }

        return true;
#else
        return false;
#endif
    }

    bool setAndWait(unsigned relativeTime100Ns)
    {
        LARGE_INTEGER dueTime = { 0 };
        dueTime.QuadPart = static_cast<LONGLONG>(relativeTime100Ns) * -1;

        BOOL res = ::SetWaitableTimerEx(m_timer, &dueTime, 0, NULL, NULL, NULL, FALSE);
        if (!res) {
            LOGD() << "SetWaitableTimerEx failed: " << std::to_string(::GetLastError());
            return false;
        }

        DWORD waitRes = ::WaitForSingleObject(m_timer, INFINITE);
        if (waitRes == WAIT_FAILED) {
            LOGD() << "WaitForSingleObject failed: " << std::to_string(::GetLastError());
            return false;
        }

        return true;
    }

private:
    HANDLE m_timer = NULL;
};
