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
#include "synthesizerstub.h"

using namespace mu;
using namespace mu::audio;
using namespace mu::audio::synth;

SynthesizerStub::SynthesizerStub(const AudioSourceParams& params)
    : m_params(params)
{
}

void SynthesizerStub::setSampleRate(unsigned int)
{
}

unsigned int SynthesizerStub::audioChannelsCount() const
{
    return 2;
}

bool SynthesizerStub::setAudioChannelsCount(unsigned int /*channels*/)
{
    return false;
}

samples_t SynthesizerStub::process(float*, size_t, samples_t)
{
    return 0;
}

std::string SynthesizerStub::name() const
{
    return std::string();
}

AudioSourceType SynthesizerStub::type() const
{
    return AudioSourceType::Undefined;
}

void SynthesizerStub::setup(const mpe::PlaybackData&)
{
}

const audio::AudioInputParams& SynthesizerStub::params() const
{
    return m_params;
}

async::Channel<audio::AudioInputParams> SynthesizerStub::paramsChanged() const
{
    static async::Channel<audio::AudioInputParams> ch;
    return ch;
}

msecs_t SynthesizerStub::playbackPosition() const
{
    return 0;
}

void SynthesizerStub::setPlaybackPosition(const msecs_t)
{
}

void SynthesizerStub::revokePlayingNotes()
{
}

void SynthesizerStub::flushSound()
{
}

bool SynthesizerStub::isValid() const
{
    return false;
}

bool SynthesizerStub::isActive() const
{
    return false;
}

void SynthesizerStub::setIsActive(bool)
{
}
