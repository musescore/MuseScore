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
#include "mixer.h"

#include "audio/common/audiosanitizer.h"
#include "audio/common/audioerrors.h"

#include "dsp/audiomathutils.h"

#include "muse_framework_config.h"

#ifdef MUSE_THREADS_SUPPORT
#include "concurrency/taskscheduler.h"
#endif

#include "log.h"

using namespace muse;
using namespace muse::async;
using namespace muse::audio;
using namespace muse::audio::engine;

constexpr size_t MIN_TRACK_COUNT_FOR_MULTITHREADING = 2;

Mixer::~Mixer()
{
    ONLY_AUDIO_MAIN_OR_ENGINE_THREAD;
    delete m_taskScheduler;
}

void Mixer::init()
{
    ONLY_AUDIO_ENGINE_THREAD;

#ifdef MUSE_THREADS_SUPPORT
    m_taskScheduler = new TaskScheduler();

    if (!m_taskScheduler->setThreadsPriority(ThreadPriority::High)) {
        LOGE() << "Unable to change audio threads priority";
    }

    AudioSanitizer::setMixerThreads(m_taskScheduler->threadIdSet());
#endif
}

IAudioSourcePtr Mixer::mixedSource()
{
    ONLY_AUDIO_ENGINE_THREAD;
    return shared_from_this();
}

Ret Mixer::addChannel(ITrackAudioOutputPtr output)
{
    MixerChannelPtr channel = std::dynamic_pointer_cast<MixerChannel>(output);
    if (!channel) {
        LOGE() << "Invalid audio output, only MixerChannel is available.";
        return make_ret(Err::InvalidAudioOutput);
    }

    channel->setPlayheadPosition(m_playhead);

    std::weak_ptr<MixerChannel> channelWeakPtr = channel;

    updateNonMutedTrackCount();

    channel->mutedChanged().onNotify(this, [this, channelWeakPtr]() {
        MixerChannelPtr channel = channelWeakPtr.lock();
        if (!channel) {
            return;
        }

        updateNonMutedTrackCount();

        if (channel->muted()) {
            channel->setMode(ProcessMode::Idle);
        } else {
            channel->setMode(mode());
            ITrackAudioInputPtr source = std::static_pointer_cast<ITrackAudioInput>(channel->source());
            if (source) {
                source->seek(playbackPosition());
            }
        }
    });

    TrackData trackData;
    trackData.trackId = channel->trackId();
    trackData.channel = channel;

    const size_t outBufferSize = m_outputSpec.samplesPerChannel * m_outputSpec.audioChannelCount;
    trackData.buffer.resize(outBufferSize);

    m_tracks.emplace_back(std::move(trackData));

    return make_ok();
}

Ret Mixer::addAuxChannel(ITrackAudioOutputPtr output)
{
    ONLY_AUDIO_ENGINE_THREAD;

    MixerChannelPtr channel = std::dynamic_pointer_cast<MixerChannel>(output);
    if (!channel) {
        LOGE() << "Invalid audio output, only MixerChannel is available.";
        return make_ret(Err::InvalidAudioOutput);
    }

    channel->setPlayheadPosition(m_playhead);

    AuxChannelInfo aux;
    aux.channel = channel;

    m_auxChannelInfoList.emplace_back(std::move(aux));

    return make_ok();
}

Ret Mixer::removeChannel(const TrackId trackId)
{
    ONLY_AUDIO_ENGINE_THREAD;

    bool removed = muse::remove_if(m_tracks, [trackId](const TrackData& track) {
        return track.trackId == trackId;
    });

    if (!removed) {
        removed = muse::remove_if(m_auxChannelInfoList, [trackId](const AuxChannelInfo& aux) {
            return aux.channel->trackId() == trackId;
        });
    }

    if (removed) {
        updateNonMutedTrackCount();
    }

    return removed ? make_ret(Ret::Code::Ok) : make_ret(Err::InvalidTrackId);
}

void Mixer::setOutputSpec(const OutputSpec& spec)
{
    ONLY_AUDIO_ENGINE_THREAD;

    m_outputSpec = spec;

    AbstractAudioSource::setOutputSpec(spec);

    for (auto& t : m_tracks) {
        t.channel->setOutputSpec(spec);
    }

    for (AuxChannelInfo& aux : m_auxChannelInfoList) {
        aux.channel->setOutputSpec(spec);
    }

    for (IFxProcessorPtr& fx : m_masterFxProcessors) {
        fx->setOutputSpec(spec);
    }
}

unsigned int Mixer::audioChannelsCount() const
{
    ONLY_AUDIO_ENGINE_THREAD;

    return m_outputSpec.audioChannelCount;
}

const TimePosition& Mixer::playbackPosition() const
{
    static TimePosition nullpos;
    return m_playhead ? m_playhead->currentPosition() : nullpos;
}

samples_t Mixer::process(float* outBuffer, samples_t samplesPerChannel)
{
    ONLY_AUDIO_ENGINE_THREAD;

    if (m_playhead) {
        m_playhead->forward(TimePosition::fromSamples(samplesPerChannel, m_outputSpec.sampleRate));
    }

    size_t outBufferSize = samplesPerChannel * m_outputSpec.audioChannelCount;
    std::fill(outBuffer, outBuffer + outBufferSize, 0.f);

    if (m_isIdle && m_tracksToProcessWhenIdle.empty() && (m_isSilence && !m_shouldProcessMasterFxDuringSilence)) {
        notifyNoAudioSignal();
        return 0;
    }

    processTrackChannels(outBufferSize, samplesPerChannel);

    prepareAuxBuffers(outBufferSize);

    for (auto& t : m_tracks) {
        if (!t.processed) {
            continue;
        }

        if (!t.channel->isSilent()) {
            m_isSilence = false;
        } else if (m_isSilence) {
            continue;
        }

        mixOutputFromChannel(outBuffer, t.buffer.data(), samplesPerChannel);
        writeTrackToAuxBuffers(t.buffer.data(), t.channel->outputParams().auxSends, samplesPerChannel);
    }

    if (m_masterParams.muted || samplesPerChannel == 0 || (m_isSilence && !m_shouldProcessMasterFxDuringSilence)) {
        notifyNoAudioSignal();
        return 0;
    }

    processAuxChannels(outBuffer, samplesPerChannel);
    processMasterFx(outBuffer, samplesPerChannel);
    completeOutput(outBuffer, samplesPerChannel);

    notifyAboutAudioSignalChanges();

    return samplesPerChannel;
}

void Mixer::processTrackChannels(size_t outBufferSize, size_t samplesPerChannel)
{
    auto processChannel = [outBufferSize, samplesPerChannel](TrackData& trackData) {
        IF_ASSERT_FAILED(trackData.channel) {
            return;
        }

        if (trackData.buffer.size() < outBufferSize) {
            trackData.buffer.resize(outBufferSize);
        }

        std::fill(trackData.buffer.begin(), trackData.buffer.begin() + outBufferSize, 0.f);
        trackData.channel->process(trackData.buffer.data(), samplesPerChannel);
        trackData.processed = true;
    };

    bool filterTracks = m_isIdle && !m_tracksToProcessWhenIdle.empty();

#ifdef MUSE_THREADS_SUPPORT
    if (useMultithreading()) {
        std::vector<std::future<void> > futures;

        for (auto& t : m_tracks) {
            t.processed = false;

            if (filterTracks && !muse::contains(m_tracksToProcessWhenIdle, t.trackId)) {
                continue;
            }

            if (t.channel->muted() && t.channel->isSilent()) {
                t.channel->setNoAudioSignal();
                continue;
            }

            std::future<void> future = m_taskScheduler->submit(processChannel, std::ref(t));
            futures.emplace_back(std::move(future));
        }

        for (auto& f : futures) {
            f.wait();
        }
    } else
#endif
    {
        for (auto& t : m_tracks) {
            t.processed = false;

            if (filterTracks && !muse::contains(m_tracksToProcessWhenIdle, t.trackId)) {
                continue;
            }

            if (t.channel->muted() && t.channel->isSilent()) {
                t.channel->setNoAudioSignal();
                continue;
            }

            processChannel(t);
        }
    }
}

void Mixer::updateNonMutedTrackCount()
{
    m_nonMutedTrackCount = 0;
    for (auto& t : m_tracks) {
        if (!t.channel->muted()) {
            m_nonMutedTrackCount++;
        }
    }
}

bool Mixer::useMultithreading() const
{
#ifdef MUSE_THREADS_SUPPORT
    if (m_nonMutedTrackCount < MIN_TRACK_COUNT_FOR_MULTITHREADING) {
        return false;
    }

    if (m_isIdle) {
        if (m_tracksToProcessWhenIdle.size() < MIN_TRACK_COUNT_FOR_MULTITHREADING) {
            return false;
        }
    }

    return true;
#else
    return false;
#endif
}

void Mixer::setMode(const ProcessMode mode)
{
    ONLY_AUDIO_ENGINE_THREAD;

    AbstractAudioSource::setMode(mode);

    for (auto& t : m_tracks) {
        if (!t.channel->muted()) {
            t.channel->setMode(mode);
        }
    }

    for (auto& aux : m_auxChannelInfoList) {
        if (!aux.channel->muted()) {
            aux.channel->setMode(mode);
        }
    }

    for (IFxProcessorPtr& fx : m_masterFxProcessors) {
        fx->setPlaying(isModePlaying(mode));
    }
}

void Mixer::setPlayhead(PlayheadPtr playhead)
{
    ONLY_AUDIO_ENGINE_THREAD;
    m_playhead = playhead;

    for (auto& track : m_tracks) {
        track.channel->setPlayheadPosition(playhead);
    }

    for (auto& aux : m_auxChannelInfoList) {
        aux.channel->setPlayheadPosition(playhead);
    }
}

AudioOutputParams Mixer::masterOutputParams() const
{
    ONLY_AUDIO_ENGINE_THREAD;

    return m_masterParams;
}

void Mixer::setMasterOutputParams(const AudioOutputParams& params)
{
    ONLY_AUDIO_ENGINE_THREAD;

    if (m_masterParams == params) {
        return;
    }

    m_masterFxProcessors.clear();
    m_masterFxProcessors = audioFactory()->makeMasterFxList(params.fxChain);

    for (IFxProcessorPtr& fx : m_masterFxProcessors) {
        fx->setOutputSpec(m_outputSpec);
        fx->setPlaying(isModePlaying(m_mode));

        fx->paramsChanged().onReceive(this, [this](const AudioFxParams& fxParams) {
            m_masterParams.fxChain.insert_or_assign(fxParams.chainOrder, fxParams);
            m_masterOutputParamsChanged.send(m_masterParams);
            updateShouldProcessMasterFxDuringSilence();
        }, async::Asyncable::Mode::SetReplace);
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
    updateShouldProcessMasterFxDuringSilence();
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
    ONLY_AUDIO_ENGINE_THREAD;

    m_isIdle = idle;
}

void Mixer::setTracksToProcessWhenIdle(const std::unordered_set<TrackId>& trackIds)
{
    ONLY_AUDIO_ENGINE_THREAD;

    m_tracksToProcessWhenIdle = trackIds;
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
        size_t samplePos = s * m_outputSpec.audioChannelCount;

        for (audioch_t audioChNum = 0; audioChNum < m_outputSpec.audioChannelCount; ++audioChNum) {
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
            size_t samplePos = s * m_outputSpec.audioChannelCount;

            for (audioch_t audioChNum = 0; audioChNum < m_outputSpec.audioChannelCount; ++audioChNum) {
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

void Mixer::processMasterFx(float* buffer, samples_t samplesPerChannel)
{
    for (IFxProcessorPtr& fxProcessor : m_masterFxProcessors) {
        if (fxProcessor->active()) {
            fxProcessor->process(buffer, samplesPerChannel, playbackPosition().samples());
        }
    }
}

void Mixer::completeOutput(float* buffer, samples_t samplesPerChannel)
{
    IF_ASSERT_FAILED(buffer) {
        return;
    }

    const float volume = muse::db_to_linear(m_masterParams.volume);
    float globalPeak = 0.f;

    for (audioch_t audioChNum = 0; audioChNum < m_outputSpec.audioChannelCount; ++audioChNum) {
        const gain_t totalGain = dsp::balanceGain(m_masterParams.balance, audioChNum) * volume;
        float peak = 0.f;

        for (samples_t s = 0; s < samplesPerChannel; ++s) {
            const size_t idx = s * m_outputSpec.audioChannelCount + audioChNum;
            const float resultSample = buffer[idx] * totalGain;
            const float absSample = std::fabs(resultSample);

            buffer[idx] = resultSample;

            if (absSample > peak) {
                peak = absSample;
            }
        }

        m_audioSignalNotifier.updateSignalValue(audioChNum, peak);

        if (peak > globalPeak) {
            globalPeak = peak;
        }
    }

    m_isSilence = RealIsNull(globalPeak);
}

void Mixer::updateShouldProcessMasterFxDuringSilence()
{
    m_shouldProcessMasterFxDuringSilence = false;
    for (const IFxProcessorPtr& fx : m_masterFxProcessors) {
        if (fx->shouldProcessDuringSilence()) {
            m_shouldProcessMasterFxDuringSilence = true;
            return;
        }
    }
}

void Mixer::notifyAboutAudioSignalChanges()
{
    for (const auto& t : m_tracks) {
        t.channel->signalNotifier().notifyAboutChanges();
    }

    for (AuxChannelInfo& aux : m_auxChannelInfoList) {
        aux.channel->signalNotifier().notifyAboutChanges();
    }

    m_audioSignalNotifier.notifyAboutChanges();
}

void Mixer::notifyNoAudioSignal()
{
    for (audioch_t audioChNum = 0; audioChNum < m_outputSpec.audioChannelCount; ++audioChNum) {
        m_audioSignalNotifier.updateSignalValue(audioChNum, 0.f);
    }

    notifyAboutAudioSignalChanges();
}
