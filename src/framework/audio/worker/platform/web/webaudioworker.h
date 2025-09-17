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

#include "audio/worker/iaudioworker.h"

#include "audio/common/audiotypes.h"

namespace muse::audio::rpc {
class IRpcChannel;
}

namespace muse::audio::worker {
//! NOTE This is a wrapper for Web Worker, communicates via RPC
class WebAudioWorker : public IAudioWorker
{
public:
    WebAudioWorker(std::shared_ptr<muse::audio::rpc::IRpcChannel> rpcChannel);
    ~WebAudioWorker();

    void registerExports() override;
    void run(const OutputSpec& outputSpec, const AudioWorkerConfig& conf) override;
    void stop() override;
    bool isRunning() const override;

    void popAudioData(float* dest, size_t sampleCount) override;

private:

    void init(const OutputSpec& outputSpec, const AudioWorkerConfig& conf);

    struct InitPending {
        OutputSpec outputSpec;
        AudioWorkerConfig conf;
        bool pending = false;
    };

    std::shared_ptr<rpc::IRpcChannel> m_rpcChannel;
    bool m_running = false;
    InitPending m_initPending;
};
}
