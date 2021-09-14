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
#include "vstsynthesiser.h"

#include "log.h"

#include "internal/vstplugin.h"

using namespace mu;
using namespace mu::vst;

VstSynthesiser::VstSynthesiser(VstPluginPtr&& pluginPtr)
    : m_pluginPtr(pluginPtr), m_vstAudioClient(std::make_unique<VstAudioClient>())
{
}

Ret VstSynthesiser::init()
{
    m_vstAudioClient->init(VstPluginType::Instrument, m_pluginPtr);

    return make_ret(Ret::Code::Ok);
}

bool VstSynthesiser::isValid() const
{
    if (!m_pluginPtr) {
        return false;
    }

    return m_pluginPtr->isValid();
}

bool VstSynthesiser::isActive() const
{
    return m_isActive;
}

void VstSynthesiser::setIsActive(bool arg)
{
    m_isActive = arg;
}

audio::AudioSourceType VstSynthesiser::type() const
{
    return audio::AudioSourceType::Vsti;
}

std::string VstSynthesiser::name() const
{
    if (!m_pluginPtr) {
        return std::string();
    }

    return m_pluginPtr->name();
}

audio::synth::SoundFontFormats VstSynthesiser::soundFontFormats() const
{
    NOT_SUPPORTED;
    return { audio::synth::SoundFontFormat::Embedded };
}

Ret VstSynthesiser::addSoundFonts(const std::vector<io::path>& /*sfonts*/)
{
    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}

Ret VstSynthesiser::removeSoundFonts()
{
    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}

bool VstSynthesiser::handleEvent(const midi::Event& e)
{
    if (!m_vstAudioClient) {
        return false;
    }

    return m_vstAudioClient->handleEvent(e);
}

void VstSynthesiser::allSoundsOff()
{
    NOT_IMPLEMENTED;
}

void VstSynthesiser::flushSound()
{
    NOT_IMPLEMENTED;
}

Ret VstSynthesiser::setupMidiChannels(const std::vector<midi::Event>& /*events*/)
{
    NOT_IMPLEMENTED;
    return Ret(Ret::Code::Ok);
}

void VstSynthesiser::midiChannelSoundsOff(midi::channel_t /*chan*/)
{
    NOT_IMPLEMENTED;
}

bool VstSynthesiser::midiChannelVolume(midi::channel_t /*chan*/, float /*val*/)
{
    NOT_IMPLEMENTED;
    return true;
}

bool VstSynthesiser::midiChannelBalance(midi::channel_t /*chan*/, float /*val*/)
{
    NOT_IMPLEMENTED;
    return true;
}

bool VstSynthesiser::midiChannelPitch(midi::channel_t /*chan*/, int16_t /*val*/)
{
    NOT_IMPLEMENTED;
    return true;
}

void VstSynthesiser::setSampleRate(unsigned int sampleRate)
{
    m_vstAudioClient->setSampleRate(sampleRate);
}

unsigned int VstSynthesiser::audioChannelsCount() const
{
    return config()->audioChannelsCount();
}

async::Channel<unsigned int> VstSynthesiser::audioChannelsCountChanged() const
{
    return m_streamsCountChanged;
}

void VstSynthesiser::process(float* buffer, unsigned int samplelPerChannel)
{
    if (!buffer) {
        return;
    }

    m_vstAudioClient->setBlockSize(samplelPerChannel);

    m_vstAudioClient->process(buffer, samplelPerChannel);
}
