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

#include "internal/audiosanitizer.h"
#include "internal/audiothread.h"
#include "internal/audiomathutils.h"
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

RetVal<IMixerChannelPtr> Mixer::addChannel(IAudioSourcePtr source, const AudioOutputParams& params,
                                           async::Channel<AudioOutputParams> paramsChanged)
{
    ONLY_AUDIO_WORKER_THREAD;

    RetVal<IMixerChannelPtr> result;

    if (!source) {
        result.val = nullptr;
        result.ret = make_ret(Err::InvalidAudioSource);
        return result;
    }

    MixerChannelId newId = m_mixerChannels.size();
    m_mixerChannels.emplace(newId, std::make_shared<MixerChannel>(newId, std::move(source), params, paramsChanged, m_sampleRate));

    result.val = m_mixerChannels[newId];
    result.ret = make_ret(Ret::Code::Ok);

    return result;
}

Ret Mixer::removeChannel(const MixerChannelId id)
{
    ONLY_AUDIO_WORKER_THREAD;

    auto search = m_mixerChannels.find(id);

    if (search != m_mixerChannels.end() && search->second) {
        m_mixerChannels.erase(id);
        return make_ret(Ret::Code::Ok);
    }

    return make_ret(Err::InvalidMixerChannelId);
}

void Mixer::setAudioChannelsCount(const audioch_t count)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_audioChannelsCount = count;
}

void Mixer::setSampleRate(unsigned int sampleRate)
{
    ONLY_AUDIO_WORKER_THREAD;
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

void Mixer::process(float* outBuffer, unsigned int samplesPerChannel)
{
    ONLY_AUDIO_WORKER_THREAD;

    for (IClockPtr clock : m_clocks) {
        clock->forward((samplesPerChannel * 1000) / m_sampleRate);
    }

    std::fill(outBuffer, outBuffer + samplesPerChannel * audioChannelsCount(), 0.f);

    if (m_writeCacheBuff.size() != samplesPerChannel * audioChannelsCount()) {
        m_writeCacheBuff.resize(samplesPerChannel * audioChannelsCount(), 0.f);
    }

    for (auto& channel : m_mixerChannels) {
        channel.second->process(m_writeCacheBuff.data(), samplesPerChannel);
        mixOutput(outBuffer, m_writeCacheBuff.data(), samplesPerChannel);
    }

    // TODO add limiter

    for (IFxProcessorPtr& fxProcessor : m_globalFxProcessors) {
        if (fxProcessor->active()) {
            fxProcessor->process(m_writeCacheBuff.data(), outBuffer, samplesPerChannel);
        }
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

    m_globalFxProcessors.clear();

    // TODO fulfill fxProcessors using fxProvider
    /*for (const FxProcessorId& fxId : m_globalOutputParams.fxProcessors) {
        // m_masterFxProcessors.push_back();
    }*/
}

Channel<AudioOutputParams> Mixer::masterOutputParamsChanged() const
{
    return m_masterOutputParamsChanged;
}

Channel<audioch_t, float> Mixer::masterSignalAmplitudeRmsChanged() const
{
    return m_masterSignalAmplitudeRmsChanged;
}

Channel<audioch_t, float> Mixer::masterVolumePressureDbfsChanged() const
{
    return m_masterVolumePressureDbfsChanged;
}

void Mixer::mixOutput(float* outBuffer, float* inBuffer, unsigned int samplesCount)
{
    IF_ASSERT_FAILED(outBuffer && inBuffer) {
        return;
    }

    for (audioch_t audioChNum = 0; audioChNum < audioChannelsCount(); ++audioChNum) {
        float squaredSum = 0.f;

        gain_t totalGain = balanceGain(m_masterParams.balance, audioChNum) * gainFromDecibels(m_masterParams.volume);

        for (samples_t s = 0; s < samplesCount; ++s) {
            int idx = s * audioChannelsCount() + audioChNum;

            float resultSample = (outBuffer[idx] + inBuffer[idx]) * totalGain;

            outBuffer[idx] = resultSample;

            squaredSum += resultSample * resultSample;
        }

        float rms = samplesRootMeanSquare(std::move(squaredSum), samplesCount);
        m_masterSignalAmplitudeRmsChanged.send(audioChNum, rms);
        m_masterVolumePressureDbfsChanged.send(audioChNum, dbFullScaleFromSample(rms));
    }
}
