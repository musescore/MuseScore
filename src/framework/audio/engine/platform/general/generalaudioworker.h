/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <memory>
#include <thread>
#include <atomic>

#include "audio/common/audiotypes.h"

namespace muse::audio::engine {
//! NOTE This is a thread for worker
class GeneralAudioWorker
{
public:
    GeneralAudioWorker();
    ~GeneralAudioWorker();

    using Callback = std::function<void ()>;

    void run(Callback callback);
    void setInterval(const msecs_t interval);
    void setInterval(const samples_t samples, const sample_rate_t sampleRate);
    void stop();
    bool isRunning() const;

public:
    void th_main(Callback callback);

    msecs_t m_intervalMsecs = 0;
    uint64_t m_intervalInWinTime = 0;

    std::unique_ptr<std::thread> m_thread = nullptr;
    std::atomic<bool> m_running = false;
};
}
