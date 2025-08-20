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

#include "audio/common/audiotypes.h"

namespace muse::audio::rpc {
class IRpcChannel;
}

namespace muse::audio::worker {
class AudioWorkerConfiguration;
class AudioEngine;
class AudioBuffer;

class AudioThread
{
public:
    AudioThread(std::shared_ptr<rpc::IRpcChannel> channel);
    ~AudioThread();

    static std::thread::id ID;

    using Runnable = std::function<void ()>;

    void run();
    void setInterval(const msecs_t interval);
    void stop(const Runnable& onFinished = nullptr);
    bool isRunning() const;

private:
    void th_main();

    // services
    std::shared_ptr<AudioWorkerConfiguration> m_configuration;
    std::shared_ptr<AudioEngine> m_audioEngine;
    std::shared_ptr<AudioBuffer> m_audioBuffer;

    Runnable m_onFinished = nullptr;
    msecs_t m_intervalMsecs = 1;
    uint64_t m_intervalInWinTime = 0;

    std::shared_ptr<rpc::IRpcChannel> m_channel = nullptr;
    std::unique_ptr<std::thread> m_thread = nullptr;
    std::atomic<bool> m_running = false;
};
using AudioThreadPtr = std::shared_ptr<AudioThread>;
}

#endif // MUSE_AUDIO_AUDIOTHREAD_H
