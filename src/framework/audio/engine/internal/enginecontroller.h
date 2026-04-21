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

#include <memory>

#include "global/modularity/ioc.h"
#include "../iaudioengineconfiguration.h"
#include "../ienginecontroller.h"
#include "../isynthresolver.h"
#include "../iaudioengine.h"
#include "../iengineplayback.h"
#include "../itransporteventsdispatcher.h"

namespace muse::audio::rpc {
class IRpcChannel;
}

namespace muse::audio::engine {
class WebAudioChannel;
class EngineRpcController;

class EngineController : public IEngineController
{
    muse::GlobalInject<IAudioEngineConfiguration> configuration;
    muse::GlobalInject<synth::ISynthResolver> synthResolver;
    muse::GlobalInject<IAudioEngine> audioEngine;
    muse::GlobalInject<IEnginePlayback> playback;
    muse::GlobalInject<ITransportEventsDispatcher> transportEventsDispatcher;

public:
    EngineController(std::shared_ptr<rpc::IRpcChannel> rpcChannel);

    void onStartRunning() override;
    void init(const OutputSpec& outputSpec, const AudioEngineConfig& conf) override;
    void deinit() override;

    OutputSpec outputSpec() const;
    async::Channel<OutputSpec> outputSpecChanged() const;

    void process(float* stream, unsigned samplesPerChannel);

private:
    std::shared_ptr<rpc::IRpcChannel> m_rpcChannel;
    std::shared_ptr<EngineRpcController> m_rpcController;
    std::shared_ptr<WebAudioChannel> m_webAudioChannel;
};
}
