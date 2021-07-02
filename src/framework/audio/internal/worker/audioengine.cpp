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
#include "audioerrors.h"
#include "internal/audiosanitizer.h"

using namespace mu::audio;
using namespace mu::audio::synth;

AudioEngine* AudioEngine::instance()
{
    static AudioEngine e;
    return &e;
}

AudioEngine::AudioEngine()
{
    ONLY_AUDIO_WORKER_THREAD;
}

AudioEngine::~AudioEngine()
{
}

bool AudioEngine::isInited() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_inited;
}

mu::Ret AudioEngine::init()
{
    ONLY_AUDIO_WORKER_THREAD;

    if (isInited()) {
        return make_ret(Ret::Code::Ok);
    }

    IF_ASSERT_FAILED(m_buffer && m_mixer) {
        return make_ret(Ret::Code::InternalError);
    }

    m_buffer->setSource(m_mixer->mixedSource());

    m_synthesizerController = std::make_shared<SynthesizerController>(synthesizersRegister(), soundFontsProvider());
    m_synthesizerController->init();

    m_inited = true;
    m_initChanged.send(m_inited);

    return make_ret(Ret::Code::Ok);
}

void AudioEngine::deinit()
{
    ONLY_AUDIO_WORKER_THREAD;
    if (isInited()) {
        m_buffer->setSource(nullptr);
        m_mixer = nullptr;
        m_inited = false;
        m_initChanged.send(m_inited);
    }
}

void AudioEngine::onDriverOpened(unsigned int sampleRate, uint16_t readBufferSize)
{
    ONLY_AUDIO_WORKER_THREAD;

    setSampleRate(sampleRate);
    setReadBufferSize(readBufferSize);
}

void AudioEngine::setSampleRate(unsigned int sampleRate)
{
    ONLY_AUDIO_WORKER_THREAD;
    m_sampleRate = sampleRate;

    m_mixer->mixedSource()->setSampleRate(sampleRate);
}

void AudioEngine::setReadBufferSize(uint16_t readBufferSize)
{
    ONLY_AUDIO_WORKER_THREAD;
    m_buffer->setMinSampleLag(readBufferSize);
}

mu::async::Channel<bool> AudioEngine::initChanged() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_initChanged;
}

unsigned int AudioEngine::sampleRate() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_sampleRate;
}

IAudioBufferPtr AudioEngine::buffer() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_buffer;
}

IMixerPtr AudioEngine::mixer() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_mixer;
}

void AudioEngine::setMixer(IMixerPtr mixerPtr)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_mixer = mixerPtr;
}

void AudioEngine::setAudioBuffer(IAudioBufferPtr buffer)
{
    ONLY_AUDIO_WORKER_THREAD;
    m_buffer = buffer;
    if (m_buffer && m_mixer) {
        m_buffer->setSource(m_mixer->mixedSource());
    }
}
