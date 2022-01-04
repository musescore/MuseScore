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
#include "sanitysynthesizer.h"
#include "internal/audiosanitizer.h"

using namespace mu;
using namespace mu::audio::synth;

SanitySynthesizer::SanitySynthesizer(ISynthesizerPtr synth)
    : m_synth(synth)
{
}

bool SanitySynthesizer::isValid() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->isValid();
}

std::string SanitySynthesizer::name() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->name();
}

audio::AudioSourceType SanitySynthesizer::type() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->type();
}

SoundFontFormats SanitySynthesizer::soundFontFormats() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->soundFontFormats();
}

Ret SanitySynthesizer::init()
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->init();
}

Ret SanitySynthesizer::addSoundFonts(const std::vector<io::path>& sfonts)
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->addSoundFonts(sfonts);
}

Ret SanitySynthesizer::removeSoundFonts()
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->removeSoundFonts();
}

bool SanitySynthesizer::isActive() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->isActive();
}

void SanitySynthesizer::setIsActive(bool arg)
{
    ONLY_AUDIO_WORKER_THREAD;
    m_synth->setIsActive(arg);
}

Ret SanitySynthesizer::setupMidiChannels(const std::vector<midi::Event>& events)
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->setupMidiChannels(events);
}

bool SanitySynthesizer::handleEvent(const midi::Event& e)
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->handleEvent(e);
}

void SanitySynthesizer::allSoundsOff()
{
    ONLY_AUDIO_WORKER_THREAD;
    m_synth->allSoundsOff();
}

void SanitySynthesizer::flushSound()
{
    ONLY_AUDIO_WORKER_THREAD;
    m_synth->flushSound();
}

void SanitySynthesizer::midiChannelSoundsOff(midi::channel_t chan)
{
    ONLY_AUDIO_WORKER_THREAD;
    m_synth->midiChannelSoundsOff(chan);
}

bool SanitySynthesizer::midiChannelVolume(midi::channel_t chan, float val)
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->midiChannelVolume(chan, val);
}

bool SanitySynthesizer::midiChannelBalance(midi::channel_t chan, float val)
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->midiChannelBalance(chan, val);
}

bool SanitySynthesizer::midiChannelPitch(midi::channel_t chan, int16_t val)
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->midiChannelPitch(chan, val);
}

// IAudioSource
void SanitySynthesizer::setSampleRate(unsigned int sampleRate)
{
    ONLY_AUDIO_WORKER_THREAD;
    m_synth->setSampleRate(sampleRate);
}

unsigned int SanitySynthesizer::audioChannelsCount() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->audioChannelsCount();
}

async::Channel<unsigned int> SanitySynthesizer::audioChannelsCountChanged() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->audioChannelsCountChanged();
}

audio::samples_t SanitySynthesizer::process(float* buffer, samples_t samplesPerChannel)
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->process(buffer, samplesPerChannel);
}
