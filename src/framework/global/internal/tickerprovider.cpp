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

#include <QTimer>

#include "tickerprovider.h"

using namespace muse;

// heuristic value
static const int ONE_TICK_INTERVAL_MS = 16;

static uint32_t newTaskId()
{
    static uint32_t lastId = 0;
    ++lastId;
    return lastId;
}

void TickerProvider::start()
{
    m_ticks = 0;
    m_timer = std::make_shared<QTimer>();
    m_timer->setInterval(ONE_TICK_INTERVAL_MS);
    m_timer->setSingleShot(true);
    QObject::connect(m_timer.get(), &QTimer::timeout, [this]() {
        process();
    });
    m_timer->start();
}

void TickerProvider::stop()
{
    if (m_timer) {
        m_timer->stop();
        m_timer = nullptr;
    }
}

void TickerProvider::addPending()
{
    for (const auto& p : m_pendingToAdd) {
        m_tasks.insert({ p.first, p.second });
    }
    m_pendingToAdd.clear();
}

void TickerProvider::removePending()
{
    for (uint32_t taskId : m_pendingToRemove) {
        auto it = m_tasks.find(taskId);
        if (it != m_tasks.end()) {
            m_tasks.erase(it);
        }
    }
    m_pendingToRemove.clear();
}

void TickerProvider::process()
{
    addPending();

    removePending();

    for (auto& p : m_tasks) {
        Task& task = p.second;
        if (task.interval == 0 || m_ticks % task.interval == 0) {
            task.call();

            if (!task.repeat) {
                m_pendingToRemove.push_back(p.first);
            }
        }
    }

    removePending();

    ++m_ticks;

    // next iteration
    m_timer->start();
}

uint32_t TickerProvider::addTask(const Task& task)
{
    uint32_t taskId = newTaskId();
    m_pendingToAdd.insert({ taskId, task });
    return taskId;
}

void TickerProvider::removeTask(const uint32_t& taskId)
{
    m_pendingToRemove.push_back(taskId);
}
