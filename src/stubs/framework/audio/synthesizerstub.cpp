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
using namespace mu;
using namespace mu::audio;

void SynthesizerStub::setSampleRate(unsigned int)
{
}

unsigned int SynthesizerStub::streamCount() const
{
    return 0;
}

mu::async::Channel<unsigned int> SynthesizerStub::streamsCountChanged() const
{
    return mu::async::Channel<unsigned int>();
}

void SynthesizerStub::forward(unsigned int)
{
}

const float* SynthesizerStub::data() const
{
    return new float();
}

void SynthesizerStub::setBufferSize(unsigned int)
{
}

bool SynthesizerStub::isValid() const
{
    return false;
}

std::string SynthesizerStub::name() const
{
    return std::string();
}

SoundFontFormats SynthesizerStub::soundFontFormats() const
{
    return {};
}

Ret SynthesizerStub::init(float)
{
    return make_ret(Ret::Code::NotSupported);
}

Ret SynthesizerStub::addSoundFonts(const std::vector<io::path_t>&)
{
    return make_ret(Ret::Code::NotSupported);
}

Ret SynthesizerStub::removeSoundFonts()
{
    return make_ret(Ret::Code::NotSupported);
}

bool SynthesizerStub::isActive() const
{
    return false;
}

void SynthesizerStub::setIsActive(bool)
{
}

Ret SynthesizerStub::setupChannels(const std::vector<midi::Event>&)
{
    return make_ret(Ret::Code::NotSupported);
}

bool SynthesizerStub::handleEvent(const midi::Event&)
{
    return false;
}

void SynthesizerStub::writeBuf(float*, unsigned int)
{
}

void SynthesizerStub::allSoundsOff()
{
}

void SynthesizerStub::flushSound()
{
}

void SynthesizerStub::channelSoundsOff(midi::channel_t)
{
}

bool SynthesizerStub::channelVolume(midi::channel_t, float)
{
    return false;
}

bool SynthesizerStub::channelBalance(midi::channel_t, float)
{
    return false;
}

bool SynthesizerStub::channelPitch(midi::channel_t, int16_t)
{
    return false;
}
