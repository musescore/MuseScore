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

using namespace mu::audio;
using namespace mu::audio::synth;

SynthesizerStub::SynthesizerStub(const AudioSourceParams& params)
    : AbstractSynthesizer(params)
{
}

void SynthesizerStub::setSampleRate(unsigned int)
{
}

unsigned int SynthesizerStub::audioChannelsCount() const
{
    return 2;
}

mu::async::Channel<unsigned int> SynthesizerStub::audioChannelsCountChanged() const
{
    return async::Channel<unsigned int>();
}

samples_t SynthesizerStub::process(float* buffer, samples_t samplesPerChannel)
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

msecs_t SynthesizerStub::playbackPosition() const
{
    return 0;
}

void SynthesizerStub::setPlaybackPosition(const msecs_t)
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

void SynthesizerStub::setupSound(const mpe::PlaybackSetupData&)
{
}

void SynthesizerStub::setupEvents(const mpe::PlaybackData&)
{
}
