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

#include "log.h"

#include "internal/audiomathutils.h"
#include "internal/audiosanitizer.h"

using namespace mu;
using namespace mu::audio;
using namespace mu::async;

MixerChannel::MixerChannel(const TrackId trackId, IAudioSourcePtr source, const unsigned int sampleRate)
    : m_trackId(trackId), m_sampleRate(sampleRate), m_audioSource(std::move(source))
{
    ONLY_AUDIO_WORKER_THREAD;

    setSampleRate(sampleRate);
}

void MixerChannel::applyOutputParams(const AudioOutputParams& originParams, AudioOutputParams& resultParams)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_fxProcessors.clear();
    m_fxProcessors = fxResolver()->resolveFxList(m_trackId, originParams.fxChain);

    for (IFxProcessorPtr& fx : m_fxProcessors) {
        fx->setSampleRate(m_sampleRate);
    }

    resultParams = originParams;

    auto filterOutInvalidFxParams = [this](const std::pair<AudioFxChainOrder, AudioFxParams>& params) {
        for (IFxProcessorPtr& fx : m_fxProcessors) {
            if (fx->params() == params.second) {
                return false;
            }
        }

        return true;
    };

    for (auto it = resultParams.fxChain.begin(); it != resultParams.fxChain.end(); ) {
        if (filterOutInvalidFxParams(*it)) {
            it = resultParams.fxChain.erase(it);
        } else {
            ++it;
        }
    }
}

Channel<audioch_t, float> MixerChannel::signalAmplitudeRmsChanged() const
{
    return m_signalAmplitudeRmsChanged;
}

Channel<audioch_t, volume_dbfs_t> MixerChannel::volumePressureDbfsChanged() const
{
    return m_volumePressureDbfsChanged;
}

bool MixerChannel::isActive() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_params.muted;
}

void MixerChannel::setIsActive(bool arg)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_params.muted = !arg;
}

void MixerChannel::setSampleRate(unsigned int sampleRate)
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_audioSource) {
        return;
    }

    m_audioSource->setSampleRate(sampleRate);

    for (IFxProcessorPtr fx : m_fxProcessors) {
        fx->setSampleRate(sampleRate);
    }
}

unsigned int MixerChannel::audioChannelsCount() const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_audioSource) {
        return 0;
    }

    return m_audioSource->audioChannelsCount();
}

async::Channel<unsigned int> MixerChannel::audioChannelsCountChanged() const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_audioSource) {
        return {};
    }

    return m_audioSource->audioChannelsCountChanged();
}

void MixerChannel::process(float* buffer, unsigned int sampleCount)
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_audioSource) {
        return;
    }

    if (m_params.muted) {
        std::fill(buffer, buffer + sampleCount * audioChannelsCount(), 0.f);
        return;
    }

    m_audioSource->process(buffer, sampleCount);

    for (IFxProcessorPtr fx : m_fxProcessors) {
        if (!fx->active()) {
            continue;
        }
        fx->process(buffer, sampleCount);
    }

    completeOutput(buffer, sampleCount);
}

void MixerChannel::completeOutput(float* buffer, unsigned int samplesCount) const
{
    for (audioch_t audioChNum = 0; audioChNum < audioChannelsCount(); ++audioChNum) {
        float squaredSum = 0.f;

        gain_t totalGain = balanceGain(m_params.balance, audioChNum) * gainFromDecibels(m_params.volume);

        for (unsigned int s = 0; s < samplesCount; ++s) {
            int idx = s * audioChannelsCount() + audioChNum;

            buffer[idx] = buffer[idx] * totalGain;

            squaredSum += buffer[idx] * buffer[idx];
        }

        float rms = samplesRootMeanSquare(std::move(squaredSum), samplesCount);

        m_signalAmplitudeRmsChanged.send(audioChNum, rms);
        m_volumePressureDbfsChanged.send(audioChNum, dbFullScaleFromSample(rms));
    }
}
