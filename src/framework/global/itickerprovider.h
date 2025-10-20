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

#include <functional>
#include "modularity/imoduleinterface.h"

namespace muse {
class ITickerProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ITickerProvider)
public:

    virtual ~ITickerProvider() = default;

    struct Task {
        uint32_t interval = 0;
        bool repeat = true;
        std::function<void()> call;
    };

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual uint32_t /*id*/ addTask(const Task& task) = 0;
    virtual void removeTask(const uint32_t& taskId) = 0;
};
}
