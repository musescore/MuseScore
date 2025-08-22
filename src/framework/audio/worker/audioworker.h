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
class GeneralRpcChannel;
}

namespace muse::audio::fx {
class FxResolver;
}

namespace muse::audio::synth  {
class SynthResolver;
class SoundFontRepository;
}

namespace muse::audio::worker {
class AudioWorkerConfiguration;
class AudioEngine;
class AudioBuffer;
class WorkerPlayback;
class WorkerChannelController;
class AudioWorker
{
public:
    AudioWorker(std::shared_ptr<rpc::GeneralRpcChannel> rpcChannel);
    ~AudioWorker();

    static std::thread::id ID;

    struct ActiveSpec {
        sample_rate_t sampleRate = 0;
        uint16_t bufferSize = 0;
    };

    void init();
    void run(const ActiveSpec& spec);
    void setInterval(const msecs_t interval);
    void stop();
    bool isRunning() const;

    const std::shared_ptr<AudioBuffer>& audioBuffer() const { return m_audioBuffer; }

private:
    void th_main(const ActiveSpec& spec);

    // services
    std::shared_ptr<rpc::GeneralRpcChannel> m_rpcChannel;
    std::shared_ptr<AudioWorkerConfiguration> m_configuration;
    std::shared_ptr<AudioEngine> m_audioEngine;
    std::shared_ptr<AudioBuffer> m_audioBuffer;
    std::shared_ptr<WorkerPlayback> m_workerPlayback;
    std::shared_ptr<WorkerChannelController> m_workerChannelController;
    std::shared_ptr<fx::FxResolver> m_fxResolver;
    std::shared_ptr<synth::SynthResolver> m_synthResolver;
    std::shared_ptr<synth::SoundFontRepository> m_soundFontRepository;

    // thread
    msecs_t m_intervalMsecs = 0;
    uint64_t m_intervalInWinTime = 0;

    std::unique_ptr<std::thread> m_thread = nullptr;
    std::atomic<bool> m_running = false;
};
using AudioThreadPtr = std::shared_ptr<AudioWorker>;
}

#endif // MUSE_AUDIO_AUDIOTHREAD_H
