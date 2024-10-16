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
#include "mixerchannel.h"

#include <algorithm>

#include "internal/dsp/audiomathutils.h"
#include "internal/audiosanitizer.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::async;

MixerChannel::MixerChannel(const TrackId trackId, IAudioSourcePtr source, const unsigned int sampleRate,
                           const modularity::ContextPtr& iocCtx)
    : Injectable(iocCtx), m_trackId(trackId),
    m_sampleRate(sampleRate),
    m_audioSource(std::move(source)),
    m_compressor(std::make_unique<dsp::Compressor>(sampleRate))
{
    ONLY_AUDIO_WORKER_THREAD;

    setSampleRate(sampleRate);
}

MixerChannel::MixerChannel(const TrackId trackId, const unsigned int sampleRate, unsigned int audioChannelsCount,
                           const modularity::ContextPtr& iocCtx)
    : MixerChannel(trackId, nullptr, sampleRate, iocCtx)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_audioChannelsCount = audioChannelsCount;
}

TrackId MixerChannel::trackId() const
{
    return m_trackId;
}

IAudioSourcePtr MixerChannel::source() const
{
    return m_audioSource;
}

bool MixerChannel::muted() const
{
    return m_params.muted;
}

async::Notification MixerChannel::mutedChanged() const
{
    return m_mutedChanged;
}

const AudioOutputParams& MixerChannel::outputParams() const
{
    return m_params;
}

void MixerChannel::applyOutputParams(const AudioOutputParams& requiredParams)
{
    ONLY_AUDIO_WORKER_THREAD;

    if (m_params == requiredParams) {
        return;
    }

    m_fxProcessors.clear();
    m_fxProcessors = fxResolver()->resolveFxList(m_trackId, requiredParams.fxChain);

    for (IFxProcessorPtr& fx : m_fxProcessors) {
        fx->setSampleRate(m_sampleRate);

        fx->paramsChanged().onReceive(this, [this](const AudioFxParams& fxParams) {
            m_params.fxChain.insert_or_assign(fxParams.chainOrder, fxParams);
            m_paramsChanges.send(m_params);
        });
    }

    AudioOutputParams resultParams = requiredParams;

    auto findFxProcessor = [this](const std::pair<AudioFxChainOrder, AudioFxParams>& params) -> IFxProcessorPtr {
        for (IFxProcessorPtr& fx : m_fxProcessors) {
            if (fx->params().chainOrder != params.first) {
                continue;
            }

            if (fx->params().resourceMeta == params.second.resourceMeta) {
                return fx;
            }
        }

        return nullptr;
    };

    for (auto it = resultParams.fxChain.begin(); it != resultParams.fxChain.end();) {
        if (IFxProcessorPtr fx = findFxProcessor(*it)) {
            fx->setActive(it->second.active);
            ++it;
        } else {
            it = resultParams.fxChain.erase(it);
        }
    }

    bool mutedChanged = m_params.muted != resultParams.muted;

    m_params = resultParams;
    m_paramsChanges.send(std::move(resultParams));

    if (mutedChanged) {
        m_mutedChanged.notify();
    }
}

async::Channel<AudioOutputParams> MixerChannel::outputParamsChanged() const
{
    return m_paramsChanges;
}

AudioSignalChanges MixerChannel::audioSignalChanges() const
{
    return m_audioSignalNotifier.audioSignalChanges;
}

bool MixerChannel::isActive() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_audioSource ? m_audioSource->isActive() : true;
}

void MixerChannel::setIsActive(bool arg)
{
    ONLY_AUDIO_WORKER_THREAD;

    if (m_audioSource) {
        m_audioSource->setIsActive(arg);
    }
}

void MixerChannel::setSampleRate(unsigned int sampleRate)
{
    ONLY_AUDIO_WORKER_THREAD;

    if (m_audioSource) {
        m_audioSource->setSampleRate(sampleRate);
    }

    for (IFxProcessorPtr fx : m_fxProcessors) {
        fx->setSampleRate(sampleRate);
    }
}

unsigned int MixerChannel::audioChannelsCount() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_audioSource ? m_audioSource->audioChannelsCount() : m_audioChannelsCount;
}

async::Channel<unsigned int> MixerChannel::audioChannelsCountChanged() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_audioSource ? m_audioSource->audioChannelsCountChanged() : async::Channel<unsigned int>();
}

samples_t MixerChannel::process(float* buffer, samples_t samplesPerChannel)
{
    ONLY_AUDIO_WORKER_THREAD;

    samples_t processedSamplesCount = samplesPerChannel;

    if (m_audioSource && !m_params.muted) {
        processedSamplesCount = m_audioSource->process(buffer, samplesPerChannel);
    }

    if (processedSamplesCount == 0 || m_params.muted) {
        std::fill(buffer, buffer + samplesPerChannel * audioChannelsCount(), 0.f);
        notifyNoAudioSignal();

        return processedSamplesCount;
    }

    for (IFxProcessorPtr fx : m_fxProcessors) {
        if (!fx->active()) {
            continue;
        }
        fx->process(buffer, samplesPerChannel);
    }

    completeOutput(buffer, samplesPerChannel);

    return processedSamplesCount;
}

void MixerChannel::completeOutput(float* buffer, unsigned int samplesCount) const
{
    unsigned int channelsCount = audioChannelsCount();
    float volume = muse::db_to_linear(m_params.volume);
    float totalSquaredSum = 0.f;

    for (audioch_t audioChNum = 0; audioChNum < channelsCount; ++audioChNum) {
        float singleChannelSquaredSum = 0.f;

        gain_t totalGain = dsp::balanceGain(m_params.balance, audioChNum) * volume;

        for (unsigned int s = 0; s < samplesCount; ++s) {
            int idx = s * channelsCount + audioChNum;

            float resultSample = buffer[idx] * totalGain;
            buffer[idx] = resultSample;

            float squaredSample = resultSample * resultSample;
            singleChannelSquaredSum += squaredSample;
            totalSquaredSum += squaredSample;
        }

        float rms = dsp::samplesRootMeanSquare(singleChannelSquaredSum, samplesCount);
        m_audioSignalNotifier.updateSignalValues(audioChNum, rms);
    }

    m_audioSignalNotifier.notifyAboutChanges();

    if (!m_compressor->isActive()) {
        return;
    }

    float totalRms = dsp::samplesRootMeanSquare(totalSquaredSum, samplesCount * channelsCount);
    m_compressor->process(totalRms, buffer, channelsCount, samplesCount);
}

void MixerChannel::notifyNoAudioSignal()
{
    unsigned int channelsCount = audioChannelsCount();

    for (audioch_t audioChNum = 0; audioChNum < channelsCount; ++audioChNum) {
        m_audioSignalNotifier.updateSignalValues(audioChNum, 0.f);
    }

    m_audioSignalNotifier.notifyAboutChanges();
}
