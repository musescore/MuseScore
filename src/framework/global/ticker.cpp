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
#include "ticker.h"

using namespace muse;

Ticker::~Ticker()
{
    stop();
}

void Ticker::start(uint32_t interval, const Callback& callback, Mode mode)
{
    ITickerProvider::Task task;
    task.interval = interval;
    task.repeat = (mode == Mode::Repeat);
    task.call = callback;

    m_taskId = tickerProvider()->addTask(task);
}

void Ticker::stop()
{
    if (m_taskId != 0) {
        tickerProvider()->removeTask(m_taskId);
        m_taskId = 0;
    }
}
