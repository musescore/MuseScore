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

#include "modularity/ioc.h"
#include "itickerprovider.h"

namespace muse {
class Ticker
{
    GlobalInject<ITickerProvider> tickerProvider;

public:

    using Callback = std::function<void ()>;

    enum class Mode {
        Single,
        Repeat
    };

    Ticker() = default;
    ~Ticker();

    void start(uint32_t interval, const Callback& callback, Mode mode);
    void stop();

private:
    uint32_t m_taskId = 0;
};
}
