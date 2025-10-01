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

#include "../ienginecontroller.h"

namespace muse::audio::rpc {
class IRpcChannel;
}

namespace muse::audio::fx {
class FxResolver;
}

namespace muse::audio::synth  {
class SynthResolver;
class SoundFontRepository;
}

namespace muse::audio::engine {
class AudioEngineConfiguration;
class AudioEngine;
class WebAudioChannel;
class EnginePlayback;
class EngineRpcChannelController;

class EngineController : public IEngineController
{
public:
    EngineController(std::shared_ptr<rpc::IRpcChannel> rpcChannel);

    void registerExports() override;
    void onStartRunning() override;
    void init(const OutputSpec& outputSpec, const AudioEngineConfig& conf) override;
    void deinit() override;

    OutputSpec outputSpec() const;
    async::Channel<OutputSpec> outputSpecChanged() const;

    void process(float* stream, unsigned samplesPerChannel);

    void process();
    void popAudioData(float* stream, unsigned samplesPerChannel);

private:
    std::shared_ptr<rpc::IRpcChannel> m_rpcChannel;
    std::shared_ptr<AudioEngineConfiguration> m_configuration;
    std::shared_ptr<AudioEngine> m_audioEngine;
    std::shared_ptr<EnginePlayback> m_playback;
    std::shared_ptr<EngineRpcChannelController> m_rpcChannelController;
    std::shared_ptr<fx::FxResolver> m_fxResolver;
    std::shared_ptr<synth::SynthResolver> m_synthResolver;
    std::shared_ptr<synth::SoundFontRepository> m_soundFontRepository;
    std::shared_ptr<WebAudioChannel> m_webAudioChannel;
};
}
