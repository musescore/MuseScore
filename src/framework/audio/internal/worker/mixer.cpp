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

#include "concurrency/taskscheduler.h"

#include "internal/audiosanitizer.h"
#include "internal/dsp/audiomathutils.h"
#include "audioerrors.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::async;

Mixer::Mixer(const modularity::ContextPtr& iocCtx)
    : muse::Injectable(iocCtx)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_taskScheduler = std::make_unique<TaskScheduler>(static_cast<thread_pool_size_t>(configuration()->desiredAudioThreadNumber()));

    if (!m_taskScheduler->setThreadsPriority(ThreadPriority::High)) {
        LOGE() << "Unable to change audio threads priority";
    }

    AudioSanitizer::setMixerThreads(m_taskScheduler->threadIdSet());

    m_minTrackCountForMultithreading = configuration()->minTrackCountForMultithreading();
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

RetVal<MixerChannelPtr> Mixer::addChannel(const TrackId trackId, ITrackAudioInputPtr source)
{
    ONLY_AUDIO_WORKER_THREAD;

    RetVal<MixerChannelPtr> result;

    if (!source) {
        result.val = nullptr;
        result.ret = make_ret(Err::InvalidAudioSource);
        return result;
    }

    MixerChannelPtr channel = std::make_shared<MixerChannel>(trackId, std::move(source), m_sampleRate, iocContext());
    std::weak_ptr<MixerChannel> channelWeakPtr = channel;

    m_nonMutedTrackCount++;

    channel->mutedChanged().onNotify(this, [this, channelWeakPtr]() {
        MixerChannelPtr channel = channelWeakPtr.lock();
        if (!channel) {
            return;
        }

        if (channel->muted()) {
            if (m_nonMutedTrackCount != 0) {
                m_nonMutedTrackCount--;
            }
            return;
        }

        m_nonMutedTrackCount++;

        ITrackAudioInputPtr source = std::static_pointer_cast<ITrackAudioInput>(channel->source());
        if (source) {
            source->seek(currentTime());
        }
    });

    m_trackChannels.emplace(trackId, channel);

    result.val = m_trackChannels[trackId];
    result.ret = make_ret(Ret::Code::Ok);

    return result;
}

RetVal<MixerChannelPtr> Mixer::addAuxChannel(const TrackId trackId)
{
    ONLY_AUDIO_WORKER_THREAD;

    const samples_t samplesToPreallocate = configuration()->samplesToPreallocate();
    const audioch_t audioChannelsCount = configuration()->audioChannelsCount();

    MixerChannelPtr channel = std::make_shared<MixerChannel>(trackId, m_sampleRate, audioChannelsCount, iocContext());

    AuxChannelInfo aux;
    aux.channel = channel;
    aux.buffer = std::vector<float>(samplesToPreallocate * audioChannelsCount, 0.f);

    m_auxChannelInfoList.emplace_back(std::move(aux));

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
        if (m_nonMutedTrackCount != 0) {
            m_nonMutedTrackCount--;
        }

        m_trackChannels.erase(trackId);
        return make_ret(Ret::Code::Ok);
    }

    bool removed = muse::remove_if(m_auxChannelInfoList, [trackId](const AuxChannelInfo& aux) {
        return aux.channel->trackId() == trackId;
    });

    return removed ? make_ret(Ret::Code::Ok) : make_ret(Err::InvalidTrackId);
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

    for (AuxChannelInfo& aux : m_auxChannelInfoList) {
        aux.channel->setSampleRate(sampleRate);
    }

    for (IFxProcessorPtr& fx : m_masterFxProcessors) {
        fx->setSampleRate(sampleRate);
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

    size_t outBufferSize = samplesPerChannel * m_audioChannelsCount;
    std::fill(outBuffer, outBuffer + outBufferSize, 0.f);

    if (m_isIdle && m_tracksToProcessWhenIdle.empty() && m_isSilence) {
        notifyNoAudioSignal();
        return 0;
    }

    TracksData tracksData;
    processTrackChannels(outBufferSize, samplesPerChannel, tracksData);

    prepareAuxBuffers(outBufferSize);

    for (auto& pair : tracksData) {
        auto channelIt = m_trackChannels.find(pair.first);
        if (channelIt == m_trackChannels.cend()) {
            continue;
        }

        const MixerChannelPtr channel = channelIt->second;
        if (!channel->isSilent()) {
            m_isSilence = false;
        } else if (m_isSilence) {
            continue;
        }

        const std::vector<float>& trackBuffer = pair.second;
        mixOutputFromChannel(outBuffer, trackBuffer.data(), samplesPerChannel);
        writeTrackToAuxBuffers(trackBuffer.data(), channel->outputParams().auxSends, samplesPerChannel);
    }

    if (m_masterParams.muted || samplesPerChannel == 0 || m_isSilence) {
        notifyNoAudioSignal();
        return 0;
    }

    processAuxChannels(outBuffer, samplesPerChannel);
    completeOutput(outBuffer, samplesPerChannel);

    for (IFxProcessorPtr& fxProcessor : m_masterFxProcessors) {
        if (fxProcessor->active()) {
            fxProcessor->process(outBuffer, samplesPerChannel);
        }
    }

    return samplesPerChannel;
}

void Mixer::processTrackChannels(size_t outBufferSize, size_t samplesPerChannel, TracksData& outTracksData)
{
    auto processChannel = [outBufferSize, samplesPerChannel](MixerChannelPtr channel) -> std::vector<float> {
        thread_local std::vector<float> buffer(outBufferSize, 0.f);
        thread_local std::vector<float> silent_buffer(outBufferSize, 0.f);

        if (buffer.size() < outBufferSize) {
            buffer.resize(outBufferSize, 0.f);
            silent_buffer.resize(outBufferSize, 0.f);
        }

        buffer = silent_buffer;

        if (channel) {
            channel->process(buffer.data(), samplesPerChannel);
        }

        return buffer;
    };

    bool filterTracks = m_isIdle && !m_tracksToProcessWhenIdle.empty();

    if (useMultithreading()) {
        std::map<TrackId, std::future<std::vector<float> > > futures;

        for (const auto& pair : m_trackChannels) {
            if (filterTracks && !muse::contains(m_tracksToProcessWhenIdle, pair.second->trackId())) {
                continue;
            }

            if (pair.second->muted()) {
                pair.second->notifyNoAudioSignal();
                continue;
            }

            std::future<std::vector<float> > future = m_taskScheduler->submit(processChannel, pair.second);
            futures.emplace(pair.first, std::move(future));
        }

        for (auto& pair : futures) {
            outTracksData.emplace(pair.first, pair.second.get());
        }
    } else {
        for (const auto& pair : m_trackChannels) {
            if (filterTracks && !muse::contains(m_tracksToProcessWhenIdle, pair.second->trackId())) {
                continue;
            }

            if (pair.second->muted()) {
                pair.second->notifyNoAudioSignal();
                continue;
            }

            outTracksData.emplace(pair.first, processChannel(pair.second));
        }
    }
}

bool Mixer::useMultithreading() const
{
    if (m_nonMutedTrackCount < m_minTrackCountForMultithreading) {
        return false;
    }

    if (m_isIdle) {
        if (m_tracksToProcessWhenIdle.size() < m_minTrackCountForMultithreading) {
            return false;
        }
    }

    return true;
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

AudioSignalChanges Mixer::masterAudioSignalChanges() const
{
    return m_audioSignalNotifier.audioSignalChanges;
}

void Mixer::setIsIdle(bool idle)
{
    ONLY_AUDIO_WORKER_THREAD;

    if (m_isIdle == idle) {
        return;
    }

    m_isIdle = idle;
    m_tracksToProcessWhenIdle.clear();
}

void Mixer::setTracksToProcessWhenIdle(std::unordered_set<TrackId>&& trackIds)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_tracksToProcessWhenIdle = std::move(trackIds);
}

void Mixer::mixOutputFromChannel(float* outBuffer, const float* inBuffer, unsigned int samplesCount) const
{
    IF_ASSERT_FAILED(outBuffer && inBuffer) {
        return;
    }

    if (m_masterParams.muted) {
        return;
    }

    for (samples_t s = 0; s < samplesCount; ++s) {
        size_t samplePos = s * m_audioChannelsCount;

        for (audioch_t audioChNum = 0; audioChNum < m_audioChannelsCount; ++audioChNum) {
            size_t idx = samplePos + audioChNum;
            float sample = inBuffer[idx];
            outBuffer[idx] += sample;
        }
    }
}

void Mixer::prepareAuxBuffers(size_t outBufferSize)
{
    for (AuxChannelInfo& aux : m_auxChannelInfoList) {
        aux.receivedAudioSignal = false;

        if (aux.channel->outputParams().fxChain.empty()) {
            continue;
        }

        if (aux.buffer.size() < outBufferSize) {
            aux.buffer.resize(outBufferSize);
        }

        std::fill(aux.buffer.begin(), aux.buffer.begin() + outBufferSize, 0.f);
    }
}

void Mixer::writeTrackToAuxBuffers(const float* trackBuffer, const AuxSendsParams& auxSends, samples_t samplesPerChannel)
{
    for (aux_channel_idx_t auxIdx = 0; auxIdx < auxSends.size(); ++auxIdx) {
        if (auxIdx >= m_auxChannelInfoList.size()) {
            break;
        }

        AuxChannelInfo& aux = m_auxChannelInfoList.at(auxIdx);
        if (aux.channel->outputParams().fxChain.empty()) {
            continue;
        }

        const AuxSendParams& auxSend = auxSends.at(auxIdx);
        if (!auxSend.active || RealIsNull(auxSend.signalAmount)) {
            continue;
        }

        float* auxBuffer = aux.buffer.data();
        float signalAmount = auxSend.signalAmount;

        for (samples_t s = 0; s < samplesPerChannel; ++s) {
            size_t samplePos = s * m_audioChannelsCount;

            for (audioch_t audioChNum = 0; audioChNum < m_audioChannelsCount; ++audioChNum) {
                size_t idx = samplePos + audioChNum;
                auxBuffer[idx] += trackBuffer[idx] * signalAmount;
            }
        }

        aux.receivedAudioSignal = true;
    }
}

void Mixer::processAuxChannels(float* buffer, samples_t samplesPerChannel)
{
    for (AuxChannelInfo& aux : m_auxChannelInfoList) {
        if (!aux.receivedAudioSignal) {
            continue;
        }

        float* auxBuffer = aux.buffer.data();
        aux.channel->process(auxBuffer, samplesPerChannel);

        if (!aux.channel->isSilent()) {
            mixOutputFromChannel(buffer, auxBuffer, samplesPerChannel);
        }
    }
}

void Mixer::completeOutput(float* buffer, samples_t samplesPerChannel)
{
    IF_ASSERT_FAILED(buffer) {
        return;
    }

    float totalSquaredSum = 0.f;
    float volume = muse::db_to_linear(m_masterParams.volume);

    for (audioch_t audioChNum = 0; audioChNum < m_audioChannelsCount; ++audioChNum) {
        float singleChannelSquaredSum = 0.f;
        gain_t totalGain = dsp::balanceGain(m_masterParams.balance, audioChNum) * volume;

        for (samples_t s = 0; s < samplesPerChannel; ++s) {
            int idx = s * m_audioChannelsCount + audioChNum;

            float resultSample = buffer[idx] * totalGain;
            buffer[idx] = resultSample;

            float squaredSample = resultSample * resultSample;
            totalSquaredSum += squaredSample;
            singleChannelSquaredSum += squaredSample;
        }

        float rms = dsp::samplesRootMeanSquare(singleChannelSquaredSum, samplesPerChannel);
        m_audioSignalNotifier.updateSignalValues(audioChNum, rms);
    }

    m_isSilence = RealIsNull(totalSquaredSum);
    m_audioSignalNotifier.notifyAboutChanges();

    if (!m_limiter->isActive()) {
        return;
    }

    float totalRms = dsp::samplesRootMeanSquare(totalSquaredSum, samplesPerChannel * m_audioChannelsCount);
    m_limiter->process(totalRms, buffer, m_audioChannelsCount, samplesPerChannel);
}

void Mixer::notifyNoAudioSignal()
{
    for (audioch_t audioChNum = 0; audioChNum < m_audioChannelsCount; ++audioChNum) {
        m_audioSignalNotifier.updateSignalValues(audioChNum, 0.f);
    }

    m_audioSignalNotifier.notifyAboutChanges();
}

msecs_t Mixer::currentTime() const
{
    if (m_clocks.empty()) {
        return 0;
    }

    IClockPtr clock = *m_clocks.begin();
    return clock->currentTime();
}
