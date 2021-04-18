#include "vstsynthesiser.h"

#include "log.h"

#include "internal/vstplugin.h"

using namespace mu;
using namespace mu::vst;

VstSynthesiser::VstSynthesiser(const VstPluginMeta& meta)
    : m_pluginMeta(meta), m_vstAudioClient(std::make_unique<VstAudioClient>())
{
}

Ret VstSynthesiser::init()
{
    RetVal plugin = repository()->findPluginById(m_pluginMeta.id);

    if (!plugin.ret) {
        return plugin.ret;
    }

    m_pluginPtr = plugin.val;

    m_vstAudioClient->init(m_pluginPtr->component());

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

std::string VstSynthesiser::name() const
{
    return m_pluginMeta.name;
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

void VstSynthesiser::writeBuf(float* stream, unsigned int samples)
{
    m_vstAudioClient->process(stream, samples);
}

void VstSynthesiser::allSoundsOff()
{
    NOT_IMPLEMENTED;
}

void VstSynthesiser::flushSound()
{
    NOT_IMPLEMENTED;
}

Ret VstSynthesiser::setupChannels(const std::vector<midi::Event>& /*events*/)
{
    NOT_IMPLEMENTED;
    return Ret(Ret::Code::Ok);
}

void VstSynthesiser::channelSoundsOff(midi::channel_t /*chan*/)
{
    NOT_IMPLEMENTED;
}

bool VstSynthesiser::channelVolume(midi::channel_t /*chan*/, float /*val*/)
{
    NOT_IMPLEMENTED;
    return true;
}

bool VstSynthesiser::channelBalance(midi::channel_t /*chan*/, float /*val*/)
{
    NOT_IMPLEMENTED;
    return true;
}

bool VstSynthesiser::channelPitch(midi::channel_t /*chan*/, int16_t /*val*/)
{
    NOT_IMPLEMENTED;
    return true;
}

void VstSynthesiser::setSampleRate(unsigned int sampleRate)
{
    m_vstAudioClient->setSampleRate(sampleRate);
}

unsigned int VstSynthesiser::streamCount() const
{
    return audio::synth::AUDIO_CHANNELS;
}

async::Channel<unsigned int> VstSynthesiser::streamsCountChanged() const
{
    return m_streamsCountChanged;
}

void VstSynthesiser::forward(unsigned int sampleCount)
{
    writeBuf(m_buffer.data(), sampleCount);
}

const float* VstSynthesiser::data() const
{
    return m_buffer.data();
}

void VstSynthesiser::setBufferSize(unsigned int samples)
{
    LOGI() << "SET BUFFER SIZE = " << samples;

    unsigned int newSize = samples * streamCount();

    if (newSize == 0 || m_buffer.size() == newSize) {
        return;
    }

    m_buffer.resize(newSize);

    m_vstAudioClient->setBlockSize(samples);
}
