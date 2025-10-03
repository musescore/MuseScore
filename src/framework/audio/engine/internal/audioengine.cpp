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

#include "audioengine.h"

#include "audio/common/audiosanitizer.h"
#include "audio/common/rpc/rpcpacker.h"

#include "audiobuffer.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::engine;
using namespace muse::audio::rpc;

static constexpr int MAX_SUPPORTED_AUDIO_CHANNELS = 2;

AudioEngine::AudioEngine()
{
    m_buffer = std::make_shared<AudioBuffer>();
    m_mixer = std::make_shared<Mixer>(nullptr);
}

AudioEngine::~AudioEngine()
{
    ONLY_AUDIO_MAIN_OR_ENGINE_THREAD;
}

Ret AudioEngine::init(const OutputSpec& outputSpec, const RenderConstraints& consts)
{
    ONLY_AUDIO_ENGINE_THREAD;

    if (m_inited) {
        return make_ret(Ret::Code::Ok);
    }

    IF_ASSERT_FAILED(consts.minSamplesToReserveWhenIdle != 0
                     && consts.minSamplesToReserveInRealtime != 0) {
        return make_ret(Ret::Code::InternalError);
    }

    IF_ASSERT_FAILED(outputSpec.audioChannelCount <= MAX_SUPPORTED_AUDIO_CHANNELS) {
        return make_ret(Ret::Code::InternalError);
    }

    LOGI() << "[OutputSpec] sampleRate: " << outputSpec.sampleRate
           << ", samplesPerChannel: " << outputSpec.samplesPerChannel
           << ", audioChannelCount: " << outputSpec.audioChannelCount;

    m_outputSpec = outputSpec;
    m_renderConsts = consts;

    m_buffer->init(outputSpec.audioChannelCount);
    updateBufferConstraints();

    m_mixer->init(consts.desiredAudioThreadNumber, consts.minTrackCountForMultithreading);
    m_mixer->setOutputSpec(outputSpec);

    rpcChannel()->onMethod(Method::SetOutputSpec, [this](const Msg& msg) {
        OutputSpec spec;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, spec)) {
            return;
        }
        this->setOutputSpec(spec);
    });

    setMode(RenderMode::IdleMode);

    m_inited = true;

    return make_ret(Ret::Code::Ok);
}

void AudioEngine::deinit()
{
    ONLY_AUDIO_ENGINE_THREAD;
    if (m_inited) {
        m_buffer->setSource(nullptr);
        m_buffer = nullptr;
        m_mixer = nullptr;
        m_inited = false;
    }
}

void AudioEngine::setOutputSpec(const OutputSpec& outputSpec)
{
    ONLY_AUDIO_ENGINE_THREAD;

    IF_ASSERT_FAILED(m_mixer) {
        return;
    }

    IF_ASSERT_FAILED(outputSpec.audioChannelCount <= MAX_SUPPORTED_AUDIO_CHANNELS) {
        return;
    }

    if (m_outputSpec == outputSpec) {
        return;
    }

    LOGI() << "[OutputSpec] sampleRate: " << outputSpec.sampleRate
           << ", samplesPerChannel: " << outputSpec.samplesPerChannel
           << ", audioChannelCount: " << outputSpec.audioChannelCount;

    bool isBufferChanged = m_outputSpec.samplesPerChannel != outputSpec.samplesPerChannel;

    m_outputSpec = outputSpec;

    m_mixer->setOutputSpec(outputSpec);

    if (isBufferChanged) {
        updateBufferConstraints();
    }

    m_outputSpecChanged.send(outputSpec);
}

OutputSpec AudioEngine::outputSpec() const
{
    return m_outputSpec;
}

async::Channel<OutputSpec> AudioEngine::outputSpecChanged() const
{
    return m_outputSpecChanged;
}

RenderMode AudioEngine::mode() const
{
    ONLY_AUDIO_ENGINE_THREAD;

    return m_mode;
}

void AudioEngine::setMode(const RenderMode newMode)
{
    ONLY_AUDIO_ENGINE_THREAD;

    if (newMode == m_mode) {
        return;
    }

    m_mode = newMode;

    switch (m_mode) {
    case RenderMode::RealTimeMode:
        m_buffer->setSource(m_mixer->mixedSource());
        m_mixer->setIsIdle(false);
        break;
    case RenderMode::IdleMode:
        m_buffer->setSource(m_mixer->mixedSource());
        m_mixer->setIsIdle(true);
        break;
    case RenderMode::OfflineMode:
        m_buffer->setSource(nullptr);
        m_mixer->setIsIdle(false);
        break;
    case RenderMode::Undefined:
        UNREACHABLE;
        break;
    }

    updateBufferConstraints();

    m_modeChanged.send(m_mode);
}

async::Channel<RenderMode> AudioEngine::modeChanged() const
{
    return m_modeChanged;
}

MixerPtr AudioEngine::mixer() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_mixer;
}

void AudioEngine::processAudioData()
{
    ONLY_AUDIO_ENGINE_THREAD;
    m_buffer->forward();
}

samples_t AudioEngine::process(float* buffer, samples_t samplesPerChannel)
{
    if (m_inited) {
        return m_mixer->process(buffer, samplesPerChannel);
    } else {
        std::memset(buffer, 0, samplesPerChannel * sizeof(float) * m_outputSpec.audioChannelCount);
        return 0;
    }
}

void AudioEngine::popAudioData(float* dest, size_t sampleCount)
{
    // driver thread
    m_buffer->pop(dest, sampleCount);
}

void AudioEngine::updateBufferConstraints()
{
    IF_ASSERT_FAILED(m_buffer) {
        return;
    }

    if (m_outputSpec.samplesPerChannel == 0) {
        return;
    }

    samples_t minSamplesToReserve = 0;

    if (m_mode == RenderMode::IdleMode) {
        minSamplesToReserve = std::max(m_outputSpec.samplesPerChannel, m_renderConsts.minSamplesToReserveWhenIdle);
    } else {
        minSamplesToReserve = std::max(m_outputSpec.samplesPerChannel, m_renderConsts.minSamplesToReserveInRealtime);
    }

    m_buffer->setMinSamplesPerChannelToReserve(minSamplesToReserve);
    m_buffer->setRenderStep(minSamplesToReserve);
}
