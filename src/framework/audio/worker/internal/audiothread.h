/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef MUSE_AUDIO_AUDIOTHREAD_H
#define MUSE_AUDIO_AUDIOTHREAD_H

#include <memory>
#include <thread>
#include <atomic>
#include <functional>

#include "audiotypes.h"

namespace muse::audio::worker {
class AudioThread
{
public:
    AudioThread() = default;
    ~AudioThread();

    static std::thread::id ID;

    using Runnable = std::function<void ()>;

    void run(const Runnable& onStart, const Runnable& loopBody, const msecs_t interval = 1);
    void setInterval(const msecs_t interval);
    void stop(const Runnable& onFinished = nullptr);
    bool isRunning() const;

private:
    void main();

    Runnable m_onStart = nullptr;
    Runnable m_mainLoopBody = nullptr;
    Runnable m_onFinished = nullptr;
    msecs_t m_intervalMsecs = 0;
    uint64_t m_intervalInWinTime = 0;

    std::unique_ptr<std::thread> m_thread = nullptr;
    std::atomic<bool> m_running = false;
};
using AudioThreadPtr = std::shared_ptr<AudioThread>;
}

#endif // MUSE_AUDIO_AUDIOTHREAD_H
