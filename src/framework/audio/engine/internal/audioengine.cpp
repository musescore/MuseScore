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
#include "audio/common/audioerrors.h"

#include "audiocontext.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::engine;

static constexpr int MAX_SUPPORTED_AUDIO_CHANNELS = 2;

AudioEngine::AudioEngine()
{
}

AudioEngine::~AudioEngine()
{
    ONLY_AUDIO_MAIN_OR_ENGINE_THREAD;
}

Ret AudioEngine::init(const OutputSpec& outputSpec)
{
    ONLY_AUDIO_ENGINE_THREAD;

    if (m_inited) {
        return make_ret(Ret::Code::Ok);
    }

    IF_ASSERT_FAILED(outputSpec.audioChannelCount <= MAX_SUPPORTED_AUDIO_CHANNELS) {
        return make_ret(Ret::Code::InternalError);
    }

    LOGI() << "[OutputSpec] sampleRate: " << outputSpec.sampleRate
           << ", samplesPerChannel: " << outputSpec.samplesPerChannel
           << ", audioChannelCount: " << outputSpec.audioChannelCount;

    m_outputSpec = outputSpec;

    m_operationType = OperationType::NoOperation;

    m_inited = true;

    return make_ret(Ret::Code::Ok);
}

void AudioEngine::deinit()
{
    ONLY_AUDIO_ENGINE_THREAD;
    if (m_inited) {
        m_inited = false;

        for (auto& p : m_contexts) {
            p.second->deinit();
        }
        m_contexts.clear();
    }
}

RetVal<std::shared_ptr<IAudioContext> > AudioEngine::addAudioContext(const AudioCtxId& ctxId)
{
    ONLY_AUDIO_ENGINE_THREAD;

    using RetType = RetVal<std::shared_ptr<IAudioContext> >;

    if (m_contexts.find(ctxId) != m_contexts.end()) {
        return RetType::make_ret(Err::AudioContextAlreadyExists);
    }
    auto ctx = std::make_shared<AudioContext>(ctxId);
    Ret ret = ctx->init();
    if (ret) {
        m_contexts[ctxId] = ctx;
    }

    return RetType::make_ok(ctx);
}

std::shared_ptr<IAudioContext> AudioEngine::context(const AudioCtxId& ctxId) const
{
    auto it = m_contexts.find(ctxId);
    if (it != m_contexts.end()) {
        return it->second;
    }
    return nullptr;
}

void AudioEngine::destroyContext(const AudioCtxId& ctxId)
{
    auto it = m_contexts.find(ctxId);
    if (it != m_contexts.end()) {
        it->second->deinit();
        m_contexts.erase(it);
    }
}

void AudioEngine::setOutputSpec(const OutputSpec& outputSpec)
{
    ONLY_AUDIO_ENGINE_THREAD;

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

    for (auto& p : m_contexts) {
        p.second->setOutputSpec(outputSpec);
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

    // check current operation
    switch (m_operationType) {
    case OperationType::Undefined: {
        UNREACHABLE;
        return fillSilent(buffer, samplesPerChannel);
    }
    case OperationType::NoOperation: {
        // normal playing
        //! TODO mix all contexts audio
        samples_t totalProcessedSamples = 0;
        for (auto& p : m_contexts) {
            totalProcessedSamples = p.second->process(buffer, samplesPerChannel);
        }
        return totalProcessedSamples;
    }
    case OperationType::QuickOperation: {
        // wait
        LOGD() << "wait end of quick operation";
        std::scoped_lock<std::mutex> lock(m_quickOperationWaitMutex);
        //! TODO mix all contexts audio
        samples_t totalProcessedSamples = 0;
        for (auto& p : m_contexts) {
            totalProcessedSamples = p.second->process(buffer, samplesPerChannel);
        }
        return totalProcessedSamples;
    }
    case OperationType::LongOperation: {
        return fillSilent(buffer, samplesPerChannel);
    }
    }

    return fillSilent(buffer, samplesPerChannel);
}
