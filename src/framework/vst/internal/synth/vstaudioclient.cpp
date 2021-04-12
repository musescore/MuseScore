#include "vstaudioclient.h"

#include "audio/synthtypes.h"

using namespace mu;
using namespace mu::vst;

VstAudioClient::~VstAudioClient()
{
    IAudioProcessorPtr processor = m_pluginComponent;
    if (!processor) {
        return;
    }

    processor->setProcessing(false);
    m_pluginComponent->setActive(false);
}

void VstAudioClient::init(PluginComponentPtr component)
{
    if (!component) {
        return;
    }

    m_pluginComponent = component;
}

bool VstAudioClient::handleEvent(const mu::midi::Event& e)
{
    if (m_eventList.addEvent(vstFromMidi(e)) == Steinberg::kResultTrue) {
        return true;
    }

    return false;
}

void VstAudioClient::process(float* output, unsigned int samples)
{
    IAudioProcessorPtr processor = m_pluginComponent;
    if (!processor || !output) {
        return;
    }

    m_processData.numSamples = samples;

    if (processor->process(m_processData) != Steinberg::kResultOk) {
        return;
    }

    m_eventList.clear();

    fillOutputBuffer(samples, output);
}

void VstAudioClient::setBlockSize(unsigned int samples)
{
    m_samplesInfo.samplesPerBlock = samples;

    updateProcessSetup();
}

void VstAudioClient::setSampleRate(unsigned int sampleRate)
{
    m_samplesInfo.sampleRate = sampleRate;

    updateProcessSetup();
}

void VstAudioClient::setUpProcessData()
{
    m_processContext.sampleRate = m_samplesInfo.sampleRate;

    m_processData.inputEvents = &m_eventList;
    m_processData.processContext = &m_processContext;

    m_processData.prepare(*m_pluginComponent, m_samplesInfo.samplesPerBlock, Steinberg::Vst::kSample32);
}

void VstAudioClient::updateProcessSetup()
{
    if (!m_samplesInfo.isValid()) {
        return;
    }

    IAudioProcessorPtr processor = m_pluginComponent;
    if (!processor) {
        return;
    }

    VstProcessSetup setup;
    setup.processMode = Steinberg::Vst::kRealtime;
    setup.symbolicSampleSize = Steinberg::Vst::kSample32;
    setup.maxSamplesPerBlock = m_samplesInfo.samplesPerBlock;
    setup.sampleRate = m_samplesInfo.sampleRate;

    if (processor->setupProcessing(setup) != Steinberg::kResultOk) {
        return;
    }

    processor->setProcessing(true);
    m_pluginComponent->setActive(true);

    setUpProcessData();
}

void VstAudioClient::fillOutputBuffer(unsigned int samples, float* output)
{
    for (unsigned int i = 0; i < samples; ++i) {
        for (unsigned int s = 0; s < audio::synth::AUDIO_CHANNELS; ++s) {
            auto getFromChannel = std::min<unsigned int>(s, m_processData.outputs[0].numChannels - 1);
            output[i * audio::synth::AUDIO_CHANNELS + s] = m_processData.outputs[0].channelBuffers32[getFromChannel][i];
        }
    }
}

VstEvent VstAudioClient::vstFromMidi(const mu::midi::Event& event)
{
    VstEvent result;

    if (!event.isValid()) {
        return result;
    }

    result.busIndex = event.group();
    result.sampleOffset = 0;
    result.ppqPosition = 0;
    result.flags = VstEvent::kIsLive;

    switch (event.opcode()) {
    case midi::Event::Opcode::NoteOn:
        result.type = VstEvent::kNoteOnEvent;
        result.noteOn.noteId = event.note();
        result.noteOn.channel = event.channel();
        result.noteOn.pitch = event.pitchNote();
        result.noteOn.tuning = event.pitchTuningCents();
        result.noteOn.velocity = event.velocityFraction();
        break;

    case midi::Event::Opcode::NoteOff:
        result.type = VstEvent::kNoteOffEvent;
        result.noteOff.noteId = event.note();
        result.noteOff.channel = event.channel();
        result.noteOff.pitch = event.pitchNote();
        result.noteOff.tuning = event.pitchTuningCents();
        result.noteOff.velocity = event.velocityFraction();
        break;

    default:
        break;
    }

    return result;
}
