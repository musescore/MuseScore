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

#include <vector>
#include <map>

#include "../itickerprovider.h"

class QTimer;
namespace muse {
class TickerProvider : public ITickerProvider
{
public:
    TickerProvider() = default;

    void start() override;
    void stop() override;
    void forceSchedule() override;

    uint32_t /*id*/ addTask(const Task& task) override;
    void removeTask(const uint32_t& taskId) override;

private:

    void process();

    void addPending();
    void removePending();

    uint32_t m_ticks = 0;
    std::shared_ptr<QTimer> m_timer;
    std::map<uint32_t /*id*/, Task> m_tasks;
    std::vector<uint32_t /*ids*/> m_pendingToRemove;
    std::map<uint32_t /*id*/, Task> m_pendingToAdd;
};
}
