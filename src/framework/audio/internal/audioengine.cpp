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

using namespace mu::audio;

AudioEngine::AudioEngine()
{
    m_sequencer = std::make_shared<Sequencer>();

    m_mixer = std::make_shared<Mixer>();
    m_mixer->setClock(m_sequencer->clock());

    m_buffer = std::make_shared<AudioBuffer>();
    m_buffer->setSource(m_mixer->mixedSource());

    m_sequencer->audioTrackAdded().onReceive(this, [this](ISequencer::audio_track_t player) {
        m_mixer->addChannel(player->audioSource());
    });
}

AudioEngine::~AudioEngine()
{
}

bool AudioEngine::isInited() const
{
    return m_inited;
}

mu::Ret AudioEngine::init(IAudioDriverPtr driver, uint16_t bufferSize)
{
    if (isInited()) {
        return make_ret(Ret::Code::Ok);
    }

    m_format = {
        48000,
        IAudioDriver::Format::AudioF32,
        2,
        bufferSize,
        [this](void* userdata, uint8_t* stream, int byteCount) {
            UNUSED(userdata);
            auto samples = byteCount / (2 * sizeof(float));
            m_buffer->pop(reinterpret_cast<float*>(stream), samples);
        },
        nullptr
    };

    m_driver = driver;
    if (m_driver) {
        auto audioOpened = m_driver->open(m_format, &m_format);
        if (!audioOpened) {
            LOGE() << "audioOutput open failed";
            return make_ret(Err::DriverOpenFailed);
        }
        m_mixer->setSampleRate(m_format.sampleRate);
        m_buffer->setMinSampleLag(m_format.samples);
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
        if (m_driver) {
            m_driver->close();
        }
    }
}

mu::async::Channel<bool> AudioEngine::initChanged() const
{
    return m_initChanged;
}

unsigned int AudioEngine::sampleRate() const
{
    return m_format.sampleRate;
}

std::shared_ptr<IAudioBuffer> AudioEngine::buffer() const
{
    return m_buffer;
}

IAudioDriverPtr AudioEngine::driver() const
{
    return m_driver;
}

IMixer::ChannelID AudioEngine::startSynthesizer(std::shared_ptr<midi::ISynthesizer> synthesizer)
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

void AudioEngine::resumeDriver()
{
    m_driver->resume();
}

void AudioEngine::suspendDriver()
{
    m_driver->suspend();
}
