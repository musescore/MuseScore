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
#include <algorithm>

#include "concurrency/taskscheduler.h"

#include "internal/audiosanitizer.h"
#include "internal/audiothread.h"
#include "internal/dsp/audiomathutils.h"
#include "audioerrors.h"

using namespace mu;
using namespace mu::audio;
using namespace mu::async;

static constexpr audioch_t AUX_AUDIO_CHANNELS_COUNT = 2;
static constexpr size_t DEFAULT_AUX_BUFFER_SIZE = 1024;

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

    m_trackChannels.emplace(trackId, std::make_shared<MixerChannel>(trackId, std::move(source), m_sampleRate));

    result.val = m_trackChannels[trackId];
    result.ret = make_ret(Ret::Code::Ok);

    return result;
}

RetVal<MixerChannelPtr> Mixer::addAuxChannel(const TrackId trackId)
{
    ONLY_AUDIO_WORKER_THREAD;

    MixerChannelPtr channel = std::make_shared<MixerChannel>(trackId, m_sampleRate, configuration()->audioChannelsCount());
    m_auxChannels.push_back(channel);
    m_auxBuffers.emplace_back(std::vector<float>(DEFAULT_AUX_BUFFER_SIZE, 0.f));

    RetVal<MixerChannelPtr> result;
    result.val = channel;
    result.ret = make_ret(Ret::Code::Ok);

    return result;
}

Ret Mixer::removeChannel(const TrackId trackId)
{
    ONLY_AUDIO_WORKER_THREAD;

    auto search = m_trackChannels.find(trackId);

    if (search != m_trackChannels.end() && search->second) {
        m_trackChannels.erase(trackId);
        return make_ret(Ret::Code::Ok);
    }

    for (auto it = m_auxChannels.begin(); it != m_auxChannels.end(); ++it) {
        if (it->get()->trackId() == trackId) {
            m_auxChannels.erase(it);
            m_auxBuffers.pop_back();
            return make_ret(Ret::Code::Ok);
        }
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

    for (auto& channel : m_trackChannels) {
        channel.second->setSampleRate(sampleRate);
    }
}

unsigned int Mixer::audioChannelsCount() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_audioChannelsCount;
}

samples_t Mixer::process(float* outBuffer, size_t bufferSize, samples_t samplesPerChannel)
{
    ONLY_AUDIO_WORKER_THREAD;

    for (IClockPtr clock : m_clocks) {
        clock->forward((samplesPerChannel * 1000000) / m_sampleRate);
    }

    const size_t outBufferSize = samplesPerChannel * m_audioChannelsCount;
    const size_t auxBufferSize = samplesPerChannel * AUX_AUDIO_CHANNELS_COUNT;
    IF_ASSERT_FAILED(outBufferSize <= bufferSize) {
        return 0;
    }

    std::fill(outBuffer, outBuffer + outBufferSize, 0.f);

    samples_t masterChannelSampleCount = 0;

    // Schedule channel processing
    std::map < TrackId, std::pair<std::future<std::vector<float> >, audioch_t> > futures;

    for (const auto& pair : m_trackChannels) {
        MixerChannelPtr channel = pair.second;
        const audioch_t audioChannels = channel->audioChannelsCount();
        std::future<std::vector<float> > future = TaskScheduler::instance()->submit([samplesPerChannel,
                                                                                     channel, audioChannels]() -> std::vector<float> {
            // Buffers are kept for each thread instance, but potentially need to be resized if number of samples change
            thread_local std::vector<float> buffer;
            thread_local std::vector<float> silent_buffer;

            silent_buffer.resize(samplesPerChannel * audioChannels, 0.f);
            buffer = silent_buffer;

            if (channel) {
                channel->process(buffer.data(), buffer.size(), samplesPerChannel);
            }

            return buffer;
        });

        futures.emplace(pair.first, std::make_pair(std::move(future), audioChannels));
    }

    // Use 2 channels in between because fx assumes that
    const audioch_t intermediateChannelCount = 2;
    const size_t intermediateSize = samplesPerChannel * intermediateChannelCount;
    float* intermediateBuffer = outBuffer;

    if (m_audioChannelsCount != intermediateChannelCount) {
        // Use intermediate buffer for two channels
        if (m_writeCacheBuff.size() != intermediateSize) {
            m_writeCacheBuff.resize(intermediateSize, 0.f);
        }
        std::fill(m_writeCacheBuff.begin(), m_writeCacheBuff.end(), 0.f);
        intermediateBuffer = m_writeCacheBuff.data();
    } else {
        intermediateBuffer = outBuffer;
    }

    prepareAuxBuffers(auxBufferSize);

    for (auto& pair : futures) {
        const std::vector<float>& trackBuffer = pair.second.first.get();
        audioch_t trackAudioChannels = pair.second.second;

        mixOutputFromChannel(intermediateBuffer, intermediateSize, intermediateChannelCount,
                             trackBuffer.data(), samplesPerChannel, trackAudioChannels);
        masterChannelSampleCount = std::max(samplesPerChannel, masterChannelSampleCount);

        const AuxSendsParams& auxSends = m_trackChannels.at(pair.first)->outputParams().auxSends;
        writeTrackToAuxBuffers(auxSends, trackBuffer.data(), samplesPerChannel);
    }

    if (m_masterParams.muted || masterChannelSampleCount == 0) {
        for (audioch_t audioChNum = 0; audioChNum < m_audioChannelsCount; ++audioChNum) {
            notifyAboutAudioSignalChanges(audioChNum, 0);
        }
        return 0;
    }

    processAuxChannels(intermediateBuffer, intermediateSize, intermediateChannelCount, samplesPerChannel);
    completeOutput(intermediateBuffer, samplesPerChannel, intermediateChannelCount);

    for (IFxProcessorPtr& fxProcessor : m_masterFxProcessors) {
        if (fxProcessor->active()) {
            fxProcessor->process(intermediateBuffer, intermediateSize, samplesPerChannel);
        }
    }

    // Convert from intermediate buffer to output buffer if necessary
    if (m_audioChannelsCount == 1) {
        // Average for mono
        const float factor = 1.f / intermediateChannelCount;
        for (samples_t s = 0; s < samplesPerChannel; ++s) {
            const float* intermediateBegin = intermediateBuffer + s * intermediateChannelCount;
            const float* intermediateEnd = intermediateBegin + intermediateChannelCount;
            // Calculate mean over intermediate channels
            outBuffer[s] = factor * std::accumulate(intermediateBegin, intermediateEnd, 0.f);
        }
    } else if (m_audioChannelsCount != intermediateChannelCount) {
        // Fill the first channels of output (front left and front right for surround)
        audioch_t minChannels = std::min(intermediateChannelCount, m_audioChannelsCount);
        for (samples_t s = 0; s < samplesPerChannel; ++s) {
            for (audioch_t audioChNum = 0; audioChNum < minChannels; ++audioChNum) {
                int outIdx = s * m_audioChannelsCount + audioChNum;
                int inIdx = s * intermediateChannelCount + audioChNum;

                outBuffer[outIdx] = intermediateBuffer[inIdx];
            }
        }
    }

    return masterChannelSampleCount;
}

void Mixer::setIsActive(bool arg)
{
    ONLY_AUDIO_WORKER_THREAD;

    AbstractAudioSource::setIsActive(arg);

    for (const auto& channel : m_trackChannels) {
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

    m_masterFxProcessors.clear();
    m_masterFxProcessors = fxResolver()->resolveMasterFxList(params.fxChain);

    for (IFxProcessorPtr& fx : m_masterFxProcessors) {
        fx->setSampleRate(m_sampleRate);
        fx->paramsChanged().onReceive(this, [this](const AudioFxParams& fxParams) {
            m_masterParams.fxChain.insert_or_assign(fxParams.chainOrder, fxParams);
            m_masterOutputParamsChanged.send(m_masterParams);
        });
    }

    AudioOutputParams resultParams = params;

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

    for (auto it = resultParams.fxChain.begin(); it != resultParams.fxChain.end();) {
        if (IFxProcessorPtr fx = findFxProcessor(*it)) {
            fx->setActive(it->second.active);
            ++it;
        } else {
            it = resultParams.fxChain.erase(it);
        }
    }

    m_masterParams = resultParams;
    m_masterOutputParamsChanged.send(resultParams);
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

void Mixer::mixOutputFromChannel(float* outBuffer, size_t outBufferSize, audioch_t outChannels, const float* inBuffer,
                                 unsigned int samplesCount, audioch_t inChannels, gain_t signalAmount)
{
    IF_ASSERT_FAILED(outBuffer && inBuffer) {
        return;
    }
    IF_ASSERT_FAILED(outBufferSize >= outChannels * samplesCount) {
        return;
    }

    if (m_masterParams.muted) {
        return;
    }

    audioch_t minChannels = std::min(inChannels, outChannels);
    if (RealIsEqual(signalAmount, 1.f)) {
        for (samples_t s = 0; s < samplesCount; ++s) {
            for (audioch_t audioChNum = 0; audioChNum < minChannels; ++audioChNum) {
                int outIdx = s * outChannels + audioChNum;
                int inIdx = s * inChannels + audioChNum;

                outBuffer[outIdx] += inBuffer[inIdx];
            }
        }
    } else {
        for (samples_t s = 0; s < samplesCount; ++s) {
            for (audioch_t audioChNum = 0; audioChNum < minChannels; ++audioChNum) {
                int outIdx = s * outChannels + audioChNum;
                int inIdx = s * inChannels + audioChNum;

                outBuffer[outIdx] += inBuffer[inIdx] * signalAmount;
            }
        }
    }
}

void Mixer::prepareAuxBuffers(size_t auxBufferSize)
{
    IF_ASSERT_FAILED(m_auxChannels.size() == m_auxBuffers.size()) {
        return;
    }

    for (aux_channel_idx_t i = 0; i < m_auxBuffers.size(); ++i) {
        MixerChannelPtr auxChannel = m_auxChannels.at(i);

        if (auxChannel->outputParams().fxChain.empty()) {
            continue;
        }

        std::vector<float>& auxBuffer = m_auxBuffers.at(i);

        if (auxBuffer.size() < auxBufferSize) {
            auxBuffer.resize(auxBufferSize);
        }

        std::fill(auxBuffer.begin(), auxBuffer.begin() + auxBufferSize, 0.f);
    }
}

void Mixer::writeTrackToAuxBuffers(const AuxSendsParams& auxSends, const float* trackBuffer, samples_t samplesPerChannel)
{
    IF_ASSERT_FAILED(m_auxChannels.size() == m_auxBuffers.size()) {
        return;
    }

    for (aux_channel_idx_t auxIdx = 0; auxIdx < auxSends.size(); ++auxIdx) {
        if (auxIdx >= m_auxBuffers.size()) {
            break;
        }

        MixerChannelPtr auxChannel = m_auxChannels.at(auxIdx);
        if (auxChannel->outputParams().fxChain.empty()) {
            continue;
        }

        const AuxSendParams& auxSend = auxSends.at(auxIdx);
        if (auxSend.active && !RealIsNull(auxSend.signalAmount)) {
            mixOutputFromChannel(m_auxBuffers.at(auxIdx).data(), m_auxBuffers.at(auxIdx).size(), AUX_AUDIO_CHANNELS_COUNT,
                                 trackBuffer, samplesPerChannel, AUX_AUDIO_CHANNELS_COUNT, auxSend.signalAmount);
        }
    }
}

void Mixer::processAuxChannels(float* buffer, size_t bufferSize, audioch_t bufferChannels, samples_t samplesPerChannel)
{
    IF_ASSERT_FAILED(m_auxChannels.size() == m_auxBuffers.size()) {
        return;
    }

    for (aux_channel_idx_t i = 0; i < m_auxChannels.size(); ++i) {
        MixerChannelPtr auxChannel = m_auxChannels.at(i);

        if (auxChannel->outputParams().fxChain.empty()) {
            continue;
        }

        float* auxBuffer = m_auxBuffers.at(i).data();
        size_t auxBufferSize = m_auxBuffers.at(i).size();
        auxChannel->process(auxBuffer, auxBufferSize, samplesPerChannel);
        mixOutputFromChannel(buffer, bufferSize, bufferChannels, auxBuffer, samplesPerChannel, AUX_AUDIO_CHANNELS_COUNT);
    }
}

void Mixer::completeOutput(float* buffer, samples_t samplesPerChannel, audioch_t channels)
{
    IF_ASSERT_FAILED(buffer) {
        return;
    }

    float totalSquaredSum = 0.f;
    float volume = dsp::linearFromDecibels(m_masterParams.volume);

    for (audioch_t audioChNum = 0; audioChNum < channels; ++audioChNum) {
        float singleChannelSquaredSum = 0.f;

        gain_t totalGain = dsp::balanceGain(m_masterParams.balance, audioChNum) * volume;

        for (samples_t s = 0; s < samplesPerChannel; ++s) {
            int idx = s * channels + audioChNum;

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

    float totalRms = dsp::samplesRootMeanSquare(totalSquaredSum, samplesPerChannel * m_audioChannelsCount);
    m_limiter->process(totalRms, buffer, channels, samplesPerChannel);
}

void Mixer::notifyAboutAudioSignalChanges(const audioch_t audioChannelNumber, const float linearRms) const
{
    m_audioSignalNotifier.updateSignalValues(audioChannelNumber, linearRms, dsp::dbFromSample(linearRms));
}
