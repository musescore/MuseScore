#include "vstaudioclient.h"

#include "log.h"
#include "audio/synthtypes.h"

using namespace mu;
using namespace mu::vst;

VstAudioClient::~VstAudioClient()
{
    m_pluginComponent->setActive(false);

    IAudioProcessorPtr processor = pluginProcessor();
    if (!processor) {
        return;
    }
    processor->setProcessing(false);
}

void VstAudioClient::init(PluginComponentPtr component)
{
    IF_ASSERT_FAILED(!component) {
        return;
    }

    m_pluginComponent = component;
}

bool VstAudioClient::handleEvent(const mu::midi::Event& e)
{
    VstEvent ev;

    if (!convertMidiToVst(e, ev)) {
        return false;
    }

    if (m_eventList.addEvent(ev) == Steinberg::kResultTrue) {
        return true;
    }

    return false;
}

void VstAudioClient::process(float* output, unsigned int samples)
{
    IAudioProcessorPtr processor = pluginProcessor();
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

IAudioProcessorPtr VstAudioClient::pluginProcessor() const
{
    return static_cast<IAudioProcessorPtr>(m_pluginComponent);
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

    IAudioProcessorPtr processor = pluginProcessor();
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

bool VstAudioClient::convertMidiToVst(const mu::midi::Event& in, VstEvent& out) const
{
    if (!in.isValid()) {
        return false;
    }

    out.busIndex = in.group();
    out.sampleOffset = 0;
    out.ppqPosition = 0;
    out.flags = VstEvent::kIsLive;

    switch (in.opcode()) {
    case midi::Event::Opcode::NoteOn:
        out.type = VstEvent::kNoteOnEvent;
        out.noteOn.noteId = in.note();
        out.noteOn.channel = in.channel();
        out.noteOn.pitch = in.pitchNote();
        out.noteOn.tuning = in.pitchTuningCents();
        out.noteOn.velocity = in.velocityFraction();
        break;

    case midi::Event::Opcode::NoteOff:
        out.type = VstEvent::kNoteOffEvent;
        out.noteOff.noteId = in.note();
        out.noteOff.channel = in.channel();
        out.noteOff.pitch = in.pitchNote();
        out.noteOff.tuning = in.pitchTuningCents();
        out.noteOff.velocity = in.velocityFraction();
        break;

    default:
        break;
    }

    return true;
}
