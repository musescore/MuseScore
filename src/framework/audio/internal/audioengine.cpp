//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "audioengine.h"

#include "log.h"
#include "ptrutils.h"
#include "audioerrors.h"
#include "audiosanitizer.h"

using namespace mu::audio;
using namespace mu::audio::synth;

AudioEngine* AudioEngine::instance()
{
    static AudioEngine e;
    return &e;
}

AudioEngine::AudioEngine()
{
    m_buffer = std::make_shared<AudioBuffer>();
}

AudioEngine::~AudioEngine()
{
}

bool AudioEngine::isInited() const
{
    return m_inited;
}

mu::Ret AudioEngine::init(int sampleRate, uint16_t readBufferSize)
{
    ONLY_AUDIO_WORKER_THREAD;

    if (isInited()) {
        return make_ret(Ret::Code::Ok);
    }

    m_sequencer = std::make_shared<Sequencer>();

    m_mixer = std::make_shared<Mixer>();
    m_mixer->setClock(m_sequencer->clock());

    m_buffer->setSource(m_mixer->mixedSource());

    m_sequencer->audioTrackAdded().onReceive(this, [this](ISequencer::AudioTrack player) {
        m_mixer->addChannel(player->audioSource());
    });

    m_sampleRate = sampleRate;
    m_mixer->setSampleRate(sampleRate);
    m_buffer->setMinSampleLag(readBufferSize);

    //! TODO Add a subscription to add or remove synthesizers
    std::vector<ISynthesizerPtr> synths = synthesizersRegister()->synthesizers();
    for (const ISynthesizerPtr& synth : synths) {
        startSynthesizer(synth);
    }

    m_inited = true;
    m_initChanged.send(m_inited);

    return make_ret(Ret::Code::Ok);
}

void AudioEngine::deinit()
{
    if (isInited()) {
        m_inited = false;
        m_initChanged.send(m_inited);
    }
}

mu::async::Channel<bool> AudioEngine::initChanged() const
{
    return m_initChanged;
}

unsigned int AudioEngine::sampleRate() const
{
    return m_sampleRate;
}

std::shared_ptr<IAudioBuffer> AudioEngine::buffer() const
{
    return m_buffer;
}

IMixer::ChannelID AudioEngine::startSynthesizer(synth::ISynthesizerPtr synthesizer)
{
    synthesizer->setSampleRate(sampleRate());
    return m_mixer->addChannel(synthesizer);
}

std::shared_ptr<IMixer> AudioEngine::mixer() const
{
    return m_mixer;
}

std::shared_ptr<ISequencer> AudioEngine::sequencer() const
{
    return m_sequencer;
}

void AudioEngine::setBuffer(IAudioBufferPtr buffer)
{
    m_buffer = buffer;
    m_buffer->setSource(m_mixer);

    //! TODO It doesn't look obvious.
    //! Requires detailed research
    //m_worker->setAudioBuffer(m_buffer);
}
