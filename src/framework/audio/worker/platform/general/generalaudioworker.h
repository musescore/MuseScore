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

#pragma once

#include <memory>
#include <thread>
#include <atomic>

#include "global/async/asyncable.h"

#include "global/modularity/ioc.h"
#include "audio/common/rpc/irpcchannel.h"
#include "../../iaudioengine.h"

#include "../../iaudioworker.h"

namespace muse::audio::worker {
class StartWorkerController;

//! NOTE This is a thread for worker
class GeneralAudioWorker : public IAudioWorker, public async::Asyncable
{
    Inject<IAudioEngine> audioEngine;

public:
    GeneralAudioWorker(std::shared_ptr<rpc::IRpcChannel> rpcChannel);
    ~GeneralAudioWorker();

    void registerExports() override;

    void run(const OutputSpec& outputSpec, const AudioWorkerConfig& conf) override;
    void setInterval(const msecs_t interval);
    void stop() override;
    bool isRunning() const override;

    void popAudioData(float* dest, size_t sampleCount) override;

private:
    void th_main(const OutputSpec& outputSpec, const AudioWorkerConfig& conf);

    // service
    std::shared_ptr<rpc::IRpcChannel> m_rpcChannel;
    std::shared_ptr<StartWorkerController> m_startWorkerController;
    std::shared_ptr<IAudioEngine> m_engine;

    // thread
    msecs_t m_intervalMsecs = 0;
    uint64_t m_intervalInWinTime = 0;

    std::unique_ptr<std::thread> m_thread = nullptr;
    std::atomic<bool> m_running = false;
};
}
