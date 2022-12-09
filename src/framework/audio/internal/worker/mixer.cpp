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
#include "mixer.h"

#include "async/async.h"
#include "log.h"

#include <limits>

#include "concurrency/taskscheduler.h"

#include "internal/audiosanitizer.h"
#include "internal/audiothread.h"
#include "internal/dsp/audiomathutils.h"
#include "audioerrors.h"

using namespace mu;
using namespace mu::audio;
using namespace mu::async;

Mixer::Mixer()
{
    ONLY_AUDIO_WORKER_THREAD;
}

Mixer::~Mixer()
{
    ONLY_AUDIO_WORKER_THREAD;
}

IAudioSourcePtr Mixer::mixedSource()
{
    ONLY_AUDIO_WORKER_THREAD;
    return shared_from_this();
}

RetVal<MixerChannelPtr> Mixer::addChannel(const TrackId trackId, IAudioSourcePtr source)
{
    ONLY_AUDIO_WORKER_THREAD;

    RetVal<MixerChannelPtr> result;

    if (!source) {
        result.val = nullptr;
        result.ret = make_ret(Err::InvalidAudioSource);
        return result;
    }

    m_mixerChannels.emplace(trackId, std::make_shared<MixerChannel>(trackId, std::move(source), m_sampleRate));

    result.val = m_mixerChannels[trackId];
    result.ret = make_ret(Ret::Code::Ok);

    return result;
}

Ret Mixer::removeChannel(const TrackId id)
{
    ONLY_AUDIO_WORKER_THREAD;

    auto search = m_mixerChannels.find(id);

    if (search != m_mixerChannels.end() && search->second) {
        m_mixerChannels.erase(id);
        return make_ret(Ret::Code::Ok);
    }

    return make_ret(Err::InvalidTrackId);
}

void Mixer::setAudioChannelsCount(const audioch_t count)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_audioChannelsCount = count;
}

void Mixer::setSampleRate(unsigned int sampleRate)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_limiter = std::make_unique<dsp::Limiter>(sampleRate);

    AbstractAudioSource::setSampleRate(sampleRate);

    for (auto& channel : m_mixerChannels) {
        channel.second->setSampleRate(sampleRate);
    }
}

unsigned int Mixer::audioChannelsCount() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_audioChannelsCount;
}

samples_t Mixer::process(float* outBuffer, samples_t samplesPerChannel)
{
    ONLY_AUDIO_WORKER_THREAD;

    for (IClockPtr clock : m_clocks) {
        clock->forward((samplesPerChannel * 1000000) / m_sampleRate);
    }

    std::fill(outBuffer, outBuffer + samplesPerChannel * audioChannelsCount(), 0.f);

    if (m_writeCacheBuff.size() != samplesPerChannel * audioChannelsCount()) {
        m_writeCacheBuff.resize(samplesPerChannel * audioChannelsCount(), 0.f);
    }

    samples_t masterChannelSampleCount = 0;

    std::vector<std::future<std::vector<float> > > futureList;

    for (const auto& pair : m_mixerChannels) {
        MixerChannelPtr channel = pair.second;
        std::future<std::vector<float> > future = TaskScheduler::instance()->submit([this, samplesPerChannel,
                                                                                     channel]() -> std::vector<float> {
            thread_local std::vector<float> buffer(samplesPerChannel * audioChannelsCount(), 0.f);
            thread_local std::vector<float> silent_buffer(samplesPerChannel * audioChannelsCount(), 0.f);

            buffer = silent_buffer;

            if (channel) {
                channel->process(buffer.data(), samplesPerChannel);
            }

            return buffer;
        });

        futureList.emplace_back(std::move(future));
    }

    for (size_t i = 0; i < futureList.size(); ++i) {
        mixOutputFromChannel(outBuffer, futureList[i].get().data(), samplesPerChannel);

        masterChannelSampleCount = std::max(samplesPerChannel, masterChannelSampleCount);
    }

    if (m_masterParams.muted || masterChannelSampleCount == 0) {
        for (audioch_t audioChNum = 0; audioChNum < audioChannelsCount(); ++audioChNum) {
            notifyAboutAudioSignalChanges(audioChNum, 0);
        }
        return 0;
    }

    completeOutput(outBuffer, samplesPerChannel);

    for (IFxProcessorPtr& fxProcessor : m_masterFxProcessors) {
        if (fxProcessor->active()) {
            fxProcessor->process(outBuffer, samplesPerChannel);
        }
    }

    return masterChannelSampleCount;
}

void Mixer::setIsActive(bool arg)
{
    ONLY_AUDIO_WORKER_THREAD;

    AbstractAudioSource::setIsActive(arg);

    for (const auto& channel : m_mixerChannels) {
        channel.second->setIsActive(arg);
    }
}

void Mixer::addClock(IClockPtr clock)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_clocks.insert(std::move(clock));
}

void Mixer::removeClock(IClockPtr clock)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_clocks.erase(clock);
}

AudioOutputParams Mixer::masterOutputParams() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_masterParams;
}

void Mixer::setMasterOutputParams(const AudioOutputParams& params)
{
    ONLY_AUDIO_WORKER_THREAD;

    if (m_masterParams == params) {
        return;
    }

    m_masterParams = params;

    m_masterFxProcessors.clear();
    m_masterFxProcessors = fxResolver()->resolveMasterFxList(params.fxChain);

    for (IFxProcessorPtr& fx : m_masterFxProcessors) {
        fx->setSampleRate(m_sampleRate);
        fx->paramsChanged().onReceive(this, [this](const AudioFxParams& fxParams) {
            m_masterParams.fxChain.insert_or_assign(fxParams.chainOrder, fxParams);
            m_masterOutputParamsChanged.send(m_masterParams);
        });
    }

    auto findFxProcessor = [this](const std::pair<AudioFxChainOrder, AudioFxParams>& params) -> IFxProcessorPtr {
        for (IFxProcessorPtr& fx : m_masterFxProcessors) {
            if (fx->params().chainOrder != params.first) {
                continue;
            }

            if (fx->params().resourceMeta == params.second.resourceMeta) {
                return fx;
            }
        }

        return nullptr;
    };

    for (auto it = params.fxChain.begin(); it != params.fxChain.end(); ++it) {
        if (IFxProcessorPtr fx = findFxProcessor(*it)) {
            fx->setActive(it->second.active);
        }
    }

    m_masterOutputParamsChanged.send(params);
}

void Mixer::clearMasterOutputParams()
{
    setMasterOutputParams(AudioOutputParams());
}

Channel<AudioOutputParams> Mixer::masterOutputParamsChanged() const
{
    return m_masterOutputParamsChanged;
}

async::Channel<audioch_t, AudioSignalVal> Mixer::masterAudioSignalChanges() const
{
    return m_audioSignalNotifier.audioSignalChanges;
}

void Mixer::mixOutputFromChannel(float* outBuffer, float* inBuffer, unsigned int samplesCount)
{
    IF_ASSERT_FAILED(outBuffer && inBuffer) {
        return;
    }

    if (m_masterParams.muted) {
        return;
    }

    for (audioch_t audioChNum = 0; audioChNum < audioChannelsCount(); ++audioChNum) {
        for (samples_t s = 0; s < samplesCount; ++s) {
            int idx = s * audioChannelsCount() + audioChNum;

            outBuffer[idx] += inBuffer[idx];
        }
    }
}

void Mixer::completeOutput(float* buffer, const samples_t& samplesPerChannel)
{
    IF_ASSERT_FAILED(buffer) {
        return;
    }

    float totalSquaredSum = 0.f;

    for (audioch_t audioChNum = 0; audioChNum < audioChannelsCount(); ++audioChNum) {
        float singleChannelSquaredSum = 0.f;

        gain_t totalGain = dsp::balanceGain(m_masterParams.balance, audioChNum) * dsp::linearFromDecibels(m_masterParams.volume);

        for (samples_t s = 0; s < samplesPerChannel; ++s) {
            int idx = s * audioChannelsCount() + audioChNum;

            float resultSample = buffer[idx] * totalGain;
            buffer[idx] = resultSample;

            float squaredSample = resultSample * resultSample;
            totalSquaredSum += squaredSample;
            singleChannelSquaredSum += squaredSample;
        }

        float rms = dsp::samplesRootMeanSquare(singleChannelSquaredSum, samplesPerChannel);
        notifyAboutAudioSignalChanges(audioChNum, rms);
    }

    if (!m_limiter->isActive()) {
        return;
    }

    float totalRms = dsp::samplesRootMeanSquare(totalSquaredSum, samplesPerChannel * audioChannelsCount());
    m_limiter->process(totalRms, buffer, audioChannelsCount(), samplesPerChannel);
}

void Mixer::notifyAboutAudioSignalChanges(const audioch_t audioChannelNumber, const float linearRms) const
{
    m_audioSignalNotifier.updateSignalValues(audioChannelNumber, linearRms, dsp::dbFromSample(linearRms));
}
