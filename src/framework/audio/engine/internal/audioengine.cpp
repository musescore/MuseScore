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

#include "global/defer.h"

#include "audio/common/audiosanitizer.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::engine;

static constexpr int MAX_SUPPORTED_AUDIO_CHANNELS = 2;

AudioEngine::AudioEngine()
{
    m_mixer = std::make_shared<Mixer>();
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

    m_mixer->init(consts.desiredAudioThreadNumber, consts.minTrackCountForMultithreading);
    m_mixer->setOutputSpec(outputSpec);

    setMode(RenderMode::IdleMode);

    m_operationType = OperationType::NoOperation;

    m_inited = true;

    return make_ret(Ret::Code::Ok);
}

void AudioEngine::deinit()
{
    ONLY_AUDIO_ENGINE_THREAD;
    if (m_inited) {
        m_inited = false;
        m_mixer = nullptr;
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

    m_outputSpec = outputSpec;

    m_mixer->setOutputSpec(outputSpec);

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
        m_mixer->setIsIdle(false);
        break;
    case RenderMode::IdleMode:
        m_mixer->setIsIdle(true);
        break;
    case RenderMode::OfflineMode:
        m_mixer->setIsIdle(false);
        break;
    case RenderMode::Undefined:
        UNREACHABLE;
        break;
    }

    m_modeChanged.send(m_mode);
}

async::Channel<RenderMode> AudioEngine::modeChanged() const
{
    return m_modeChanged;
}

void AudioEngine::execOperation(OperationType type, const Operation& func)
{
    // wait end of processing
    while (m_processing) {
        LOGD() << "wait end of processing";
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1ms);
    }

    m_operationType = type;
    if (m_operationType == OperationType::QuickOperation) {
        m_quickOperationWaitMutex.lock();
    }

    func();

    if (m_operationType == OperationType::QuickOperation) {
        m_quickOperationWaitMutex.unlock();
    }
    m_operationType = OperationType::NoOperation;
}

OperationType AudioEngine::operation() const
{
    return m_operationType.load();
}

MixerPtr AudioEngine::mixer() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_mixer;
}

samples_t AudioEngine::fillSilent(float* buffer, samples_t samplesPerChannel)
{
    std::memset(buffer, 0, samplesPerChannel * sizeof(float) * m_outputSpec.audioChannelCount);
    return 0;
}

samples_t AudioEngine::process(float* buffer, samples_t samplesPerChannel)
{
    m_processing = true;
    DEFER {
        m_processing = false;
    };

    if (!m_inited) {
        return fillSilent(buffer, samplesPerChannel);
    }

    if (m_mode == RenderMode::RealTimeMode // playing
        || m_mode == RenderMode::IdleMode) { // individual events can be played
        // check current operation
        switch (m_operationType) {
        case OperationType::Undefined: {
            UNREACHABLE;
            return fillSilent(buffer, samplesPerChannel);
        }
        case OperationType::NoOperation: {
            // normal playing
            return m_mixer->process(buffer, samplesPerChannel);
        }
        case OperationType::QuickOperation: {
            // wait
            LOGD() << "wait end of quick operation";
            std::scoped_lock<std::mutex> lock(m_quickOperationWaitMutex);
            return m_mixer->process(buffer, samplesPerChannel);
        }
        case OperationType::LongOperation: {
            return fillSilent(buffer, samplesPerChannel);
        }
        }
    }

    return fillSilent(buffer, samplesPerChannel);
}
