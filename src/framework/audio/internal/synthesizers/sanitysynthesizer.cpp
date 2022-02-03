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

Ret SanitySynthesizer::init()
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->init();
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

Ret SanitySynthesizer::setupSound(const std::vector<midi::Event>& events)
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->setupSound(events);
}

bool SanitySynthesizer::handleEvent(const midi::Event& e)
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_synth->handleEvent(e);
}

void SanitySynthesizer::flushSound()
{
    ONLY_AUDIO_WORKER_THREAD;
    m_synth->flushSound();
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
