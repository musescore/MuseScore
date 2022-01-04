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

#include "audioengine.h"

#include "log.h"
#include "ptrutils.h"

#include "internal/audiosanitizer.h"
#include "audioerrors.h"

using namespace mu::audio;

AudioEngine* AudioEngine::instance()
{
    ONLY_AUDIO_WORKER_THREAD;

    static AudioEngine e;
    return &e;
}

AudioEngine::AudioEngine()
{
    ONLY_AUDIO_WORKER_THREAD;
}

AudioEngine::~AudioEngine()
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;
}

mu::Ret AudioEngine::init(IAudioBufferPtr bufferPtr)
{
    ONLY_AUDIO_WORKER_THREAD;

    if (m_inited) {
        return make_ret(Ret::Code::Ok);
    }

    IF_ASSERT_FAILED(bufferPtr) {
        return make_ret(Ret::Code::InternalError);
    }

    m_mixer = std::make_shared<Mixer>();

    m_buffer = std::move(bufferPtr);
    m_buffer->setSource(m_mixer->mixedSource());

    m_inited = true;

    return make_ret(Ret::Code::Ok);
}

void AudioEngine::deinit()
{
    ONLY_AUDIO_WORKER_THREAD;
    if (m_inited) {
        m_buffer->setSource(nullptr);
        m_buffer = nullptr;
        m_mixer = nullptr;
        m_inited = false;
    }
}

void AudioEngine::setSampleRate(unsigned int sampleRate)
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_mixer) {
        return;
    }

    m_mixer->mixedSource()->setSampleRate(sampleRate);
}

void AudioEngine::setReadBufferSize(uint16_t readBufferSize)
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_buffer) {
        return;
    }

    m_buffer->setMinSampleLag(readBufferSize);
}

void AudioEngine::setAudioChannelsCount(const audioch_t count)
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_mixer) {
        return;
    }

    m_mixer->setAudioChannelsCount(count);
}

MixerPtr AudioEngine::mixer() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_mixer;
}
