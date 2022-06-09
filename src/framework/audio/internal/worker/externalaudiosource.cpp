/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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
#include "externalaudiosource.h"
using namespace mu;
using namespace mu::audio;

ExternalAudioSource::ExternalAudioSource(const TrackId trackId, io::Device* playbackData)
    : m_trackId(trackId), m_playbackData(playbackData)
{
    ONLY_AUDIO_WORKER_THREAD;
    playbackData->open(mu::io::Device::ReadOnly);
    b = playbackData->readAll();

    drwav_init_memory(&m_wav, b.constData(), b.size(), nullptr);
    if (m_wav.fmt.bitsPerSample != 32) {
        LOGW() << "The wave bitsample should be 32 it is now " << m_wav.fmt.bitsPerSample;
    }

    m_sampleRate = m_wav.sampleRate;
    m_channels = m_wav.channels;
}

bool ExternalAudioSource::isActive() const
{
    return m_active;
}

void ExternalAudioSource::setIsActive(const bool active)
{
    m_active = active;
}

void ExternalAudioSource::setSampleRate(unsigned int sampleRate)
{
    ONLY_AUDIO_WORKER_THREAD;

    if (m_sampleRate != sampleRate) {
        LOGW() << "Your wave file has different samplerate " << m_sampleRate << " than samplerate set " << sampleRate;
    }
}

unsigned int ExternalAudioSource::audioChannelsCount() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_channels;
}

async::Channel<unsigned int> ExternalAudioSource::audioChannelsCountChanged() const
{
    return m_audioChannelsCountChange;
}

samples_t ExternalAudioSource::process(float* buffer, samples_t samplesPerChannel)
{
    ONLY_AUDIO_WORKER_THREAD;

    if (m_active) {
        return drwav_read_pcm_frames_f32(&m_wav, samplesPerChannel, buffer);
    }
    return 0;
}

void ExternalAudioSource::seek(const msecs_t newPositionMsecs)
{
    drwav_init_memory(&m_wav, b.constData(), b.size(), NULL);
    drwav_read_pcm_frames_f32(&m_wav, newPositionMsecs / 1000 * m_wav.sampleRate, nullptr);
}

void ExternalAudioSource::applyInputParams(const AudioInputParams& requiredParams)
{
    if (m_params == requiredParams) {
        return;
    }
    m_params = requiredParams;
    m_paramsChanges.send(m_params);
}

async::Channel<AudioInputParams> ExternalAudioSource::inputParamsChanged() const
{
    return m_paramsChanges;
}

const AudioInputParams& ExternalAudioSource::inputParams() const
{
    return m_params;
}
