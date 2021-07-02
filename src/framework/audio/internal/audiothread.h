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

#ifndef MU_AUDIO_AUDIOTHREAD_H
#define MU_AUDIO_AUDIOTHREAD_H

#include <memory>
#include <thread>
#include <atomic>
#include <functional>

#include "iaudiobuffer.h"
#include "modularity/ioc.h"

namespace mu::audio {
class AudioThread
{
public:
    AudioThread();
    ~AudioThread();

    static std::thread::id ID;

    using OnStart = std::function<void ()>;
    using OnFinished = std::function<void ()>;

    void run(const OnStart& onStart = nullptr);
    void stop(const OnFinished& onFinished = nullptr);
    bool isRunning() const;

    void setAudioBuffer(std::shared_ptr<IAudioBuffer> buffer);

    //! use if you don't want to use internal thread
    void loopBody();

private:
    void main();

    OnStart m_onStart;
    OnFinished m_onFinished;
    std::shared_ptr<IAudioBuffer> m_buffer = nullptr;
    std::shared_ptr<std::thread> m_thread = nullptr;
    std::atomic<bool> m_running = false;
};
}

#endif // MU_AUDIO_AUDIOTHREAD_H
