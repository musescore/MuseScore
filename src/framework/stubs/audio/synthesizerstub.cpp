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

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::synth;

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

async::Channel<unsigned int> SynthesizerStub::audioChannelsCountChanged() const
{
    return async::Channel<unsigned int>();
}

samples_t SynthesizerStub::process(float*, samples_t)
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

const mpe::PlaybackData& SynthesizerStub::playbackData() const
{
    static const mpe::PlaybackData dummyData;
    return dummyData;
}

const AudioInputParams& SynthesizerStub::params() const
{
    return m_params;
}

async::Channel<AudioInputParams> SynthesizerStub::paramsChanged() const
{
    static async::Channel<AudioInputParams> ch;
    return ch;
}

msecs_t SynthesizerStub::playbackPosition() const
{
    return 0;
}

void SynthesizerStub::setPlaybackPosition(const msecs_t)
{
}

void SynthesizerStub::prepareToPlay()
{
}

bool SynthesizerStub::readyToPlay() const
{
    return false;
}

async::Notification SynthesizerStub::readyToPlayChanged() const
{
    return async::Notification();
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
