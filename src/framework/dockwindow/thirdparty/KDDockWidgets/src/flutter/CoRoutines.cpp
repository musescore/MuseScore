/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "CoRoutines.h"

#include <iostream>
#include <queue>

using namespace KDDockWidgets::flutter;

class SuspendTask
{
public:
    SuspendTask()
    {
    }

    constexpr bool await_ready() const noexcept
    {
        return false;
    }

    void await_suspend(std::coroutine_handle<> h) const noexcept
    {
        // assert(!s_task);
        // assert(!m_handle);
        m_handle = h.address();
        s_task = const_cast<SuspendTask *>(this);
    }

    void await_resume() const noexcept
    {
    }

    void resume()
    {
        auto h = std::coroutine_handle<>::from_address(m_handle);
        s_task = nullptr;
        m_handle = nullptr;

        h.resume();
    }

public:
    static SuspendTask *s_task;

private:
    mutable void *m_handle = nullptr;
};

SuspendTask *SuspendTask::s_task = nullptr;

QCoro::Task<> CoRoutines::suspend()
{
    co_await SuspendTask();
}

void CoRoutines::resume()
{
    if (SuspendTask::s_task)
        SuspendTask::s_task->resume();
}
