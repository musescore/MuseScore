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
#include "vstsynthesizer.h"
#include "internal/plugininstance.h"
#include "log.h"

using namespace mu;
using namespace mu::vst;

VSTSynthesizer::VSTSynthesizer(std::string name, std::shared_ptr<PluginInstance> instance)
    : m_name(name), m_instance(instance)
{
    IF_ASSERT_FAILED(m_instance) {
        LOGE() << "valid instance must be setted";
    }
}

bool VSTSynthesizer::isValid() const
{
    return m_instance->isValid();
}

std::shared_ptr<VSTSynthesizer> VSTSynthesizer::create(std::shared_ptr<PluginInstance> instance)
{
    IF_ASSERT_FAILED(instance->id() != IVSTInstanceRegister::ID_NOT_SETTED) {
        LOGE() << "instance was not registered";
    }
    std::string name = "VST#" + std::to_string(instance->id());
    auto synth = std::static_pointer_cast<VSTSynthesizer>(synthesizersRegister()->synthesizer(name));
    if (!synth) {
        synth = std::make_shared<VSTSynthesizer>(name, instance);
        synthesizersRegister()->registerSynthesizer(name, synth);
        synth->setIsActive(instance->isActive());
    }
    return synth;
}

std::string VSTSynthesizer::name() const
{
    return m_name;
}

audio::synth::SoundFontFormats VSTSynthesizer::soundFontFormats() const
{
    NOT_SUPPORTED;
    return { audio::synth::SoundFontFormat::Undefined };
}

Ret VSTSynthesizer::addSoundFonts(const std::vector<io::path>& sfonts)
{
    UNUSED(sfonts)
    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}

Ret VSTSynthesizer::removeSoundFonts()
{
    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}

Ret VSTSynthesizer::init()
{
    return true;
}

void VSTSynthesizer::setSampleRate(unsigned int sampleRate)
{
    m_sampleRate = sampleRate;
    if (m_instance) {
        m_instance->setSampleRate(sampleRate);
    }
}

bool VSTSynthesizer::isActive() const
{
    return m_instance->isActive();
}

void VSTSynthesizer::setIsActive(bool arg)
{
    m_instance->setActive(arg);
}

Ret VSTSynthesizer::setupChannels(const std::vector<midi::Event>& events)
{
    std::vector<unsigned int> channels;
    for (const midi::Event& e: events) {
        channels.push_back(e.channel());
    }
    LOGI() << "TODO" << channels;
    return Ret(Ret::Code::Ok);
}

bool VSTSynthesizer::handleEvent(const midi::Event& e)
{
    m_instance->addMidiEvent(e);
    return true;
}

void VSTSynthesizer::writeBuf(float* stream, unsigned int samples)
{
    m_instance->process(/*input stream*/ nullptr, stream, samples);
}

void VSTSynthesizer::allSoundsOff()
{
    LOGI() << "TODO";
}

void VSTSynthesizer::flushSound()
{
    m_instance->flush();
}

void VSTSynthesizer::channelSoundsOff(midi::channel_t chan)
{
    UNUSED(chan);
    LOGI() << "TODO";
}

bool VSTSynthesizer::channelVolume(midi::channel_t chan, float val)
{
    UNUSED(chan);
    UNUSED(val);
    LOGI() << "TODO";
    return true;
}

bool VSTSynthesizer::channelBalance(midi::channel_t chan, float val)
{
    UNUSED(chan);
    UNUSED(val);
    LOGI() << "TODO";
    return true;
}

bool VSTSynthesizer::channelPitch(midi::channel_t chan, int16_t val)
{
    UNUSED(chan);
    UNUSED(val);
    LOGI() << "TODO";
    return true;
}

unsigned int VSTSynthesizer::streamCount() const
{
    return audio::synth::AUDIO_CHANNELS;
}

void VSTSynthesizer::forward(unsigned int sampleCount)
{
    writeBuf(m_buffer.data(), sampleCount);
}

async::Channel<unsigned int> VSTSynthesizer::streamsCountChanged() const
{
    return m_streamsCountChanged;
}

const float* VSTSynthesizer::data() const
{
    return m_buffer.data();
}

void VSTSynthesizer::setBufferSize(unsigned int samples)
{
    auto sc = streamCount();
    auto targetSize = samples * sc;
    if (targetSize > 0 && m_buffer.size() < targetSize) {
        m_buffer.resize(samples * streamCount());
    }
}
