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

Ret SanitySynthesizer::setupChannels(const std::vector<midi::Event>& events)
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->setupChannels(events);
}

bool SanitySynthesizer::handleEvent(const midi::Event& e)
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->handleEvent(e);
}

void SanitySynthesizer::writeBuf(float* stream, unsigned int samples)
{
    ONLY_AUDIO_WORKER_THREAD;
    m_synth->writeBuf(stream, samples);
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

void SanitySynthesizer::channelSoundsOff(midi::channel_t chan)
{
    ONLY_AUDIO_WORKER_THREAD;
    m_synth->channelSoundsOff(chan);
}

bool SanitySynthesizer::channelVolume(midi::channel_t chan, float val)
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->channelVolume(chan, val);
}

bool SanitySynthesizer::channelBalance(midi::channel_t chan, float val)
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->channelBalance(chan, val);
}

bool SanitySynthesizer::channelPitch(midi::channel_t chan, int16_t val)
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->channelPitch(chan, val);
}

// IAudioSource
void SanitySynthesizer::setSampleRate(unsigned int sampleRate)
{
    ONLY_AUDIO_WORKER_THREAD;
    m_synth->setSampleRate(sampleRate);
}

unsigned int SanitySynthesizer::streamCount() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->streamCount();
}

async::Channel<unsigned int> SanitySynthesizer::streamsCountChanged() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->streamsCountChanged();
}

void SanitySynthesizer::forward(float* buffer, unsigned int sampleCount)
{
    ONLY_AUDIO_WORKER_THREAD;
    m_synth->forward(buffer, sampleCount);
}
