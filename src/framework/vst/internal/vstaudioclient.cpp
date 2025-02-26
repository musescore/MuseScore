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
#include "vstaudioclient.h"

#include "log.h"

using namespace muse;
using namespace muse::vst;
using namespace muse::mpe;
using namespace muse::audio;
using namespace muse::audioplugins;

static size_t noteEventKey(int pitch, int channel)
{
    std::size_t h1 = std::hash<int> {}(pitch);
    std::size_t h2 = std::hash<int> {}(channel);
    return h1 ^ (h2 << 1);
}

VstAudioClient::~VstAudioClient()
{
    if (!m_pluginComponent) {
        return;
    }

    m_pluginComponent->setActive(false);
    m_pluginComponent->terminate();
}

void VstAudioClient::init(AudioPluginType type, IVstPluginInstancePtr instance, audioch_t audioChannelsCount)
{
    IF_ASSERT_FAILED(instance && type != AudioPluginType::Undefined) {
        return;
    }

    m_type = type;
    m_pluginPtr = std::move(instance);
    m_audioChannelsCount = audioChannelsCount;
}

void VstAudioClient::loadSupportedParams()
{
    TRACEFUNC;

    IF_ASSERT_FAILED(m_pluginPtr) {
        return;
    }

    PluginControllerPtr controller = m_pluginPtr->controller();
    IF_ASSERT_FAILED(controller) {
        return;
    }

    int paramCount = controller->getParameterCount();

    for (int i = 0; i < paramCount; ++i) {
        PluginParamInfo info;
        controller->getParameterInfo(i, info);
        m_pluginParamInfoMap.emplace(info.id, std::move(info));
    }
}

bool VstAudioClient::handleEvent(const VstEvent& event)
{
    ensureActivity();

    if (event.type == VstEvent::kNoteOnEvent) {
        size_t key = noteEventKey(event.noteOn.pitch, event.noteOn.channel);
        m_playingNotes.insert_or_assign(key, event);
    } else if (event.type == VstEvent::kNoteOffEvent) {
        size_t key = noteEventKey(event.noteOff.pitch, event.noteOff.channel);
        m_playingNotes.erase(key);
    }

    if (m_eventList.addEvent(const_cast<VstEvent&>(event)) == Steinberg::kResultTrue) {
        return true;
    }

    return false;
}

bool VstAudioClient::handleParamChange(const ParamChangeEvent& param)
{
    ensureActivity();
    addParamChange(param);

    m_playingParams.insert(param.paramId);

    return true;
}

void VstAudioClient::setVolumeGain(const muse::audio::gain_t newVolumeGain)
{
    m_volumeGain = newVolumeGain;
}

muse::audio::samples_t VstAudioClient::process(float* output, muse::audio::samples_t samplesPerChannel,
                                               muse::audio::msecs_t playbackPosition)
{
    IAudioProcessorPtr processor = pluginProcessor();
    if (!processor || !output) {
        return 0;
    }

    if (!m_isActive) {
        return 0;
    }

    //! NOTE: From the VST3 documentation:
    //!
    //! Note that the ProcessData->numSamples
    //! which indicates how many samples are used in a process call can change from call to call,
    //! but never bigger than the maxSamplesPerBlock
    m_processData.numSamples = samplesPerChannel;

    m_processContext.projectTimeSamples = (playbackPosition / 1000000.f) * m_samplesInfo.sampleRate;

    if (samplesPerChannel > m_samplesInfo.maxSamplesPerBlock) {
        setMaxSamplesPerBlock(samplesPerChannel);
    }

    if (m_type == AudioPluginType::Fx) {
        extractInputSamples(samplesPerChannel, output);
    }

    if (processor->process(m_processData) != Steinberg::kResultOk) {
        return 0;
    }

    if (m_type == AudioPluginType::Instrument) {
        m_eventList.clear();
        m_paramChanges.clearQueue();

        if (!fillOutputBufferInstrument(samplesPerChannel, output)) {
            return 0;
        }
    } else if (!fillOutputBufferFx(samplesPerChannel, output)) {
        return 0;
    }

    return samplesPerChannel;
}

void VstAudioClient::flush()
{
    allNotesOff();

    disableActivity();
}

void VstAudioClient::allNotesOff()
{
    if (m_playingNotes.empty() && m_playingParams.empty()) {
        return;
    }

    flushBuffers();

    m_eventList.clear();
    m_paramChanges.clearQueue();

    for (const auto& pair : m_playingNotes) {
        const VstEvent& noteOn = pair.second;

        VstEvent noteOff;
        noteOff.type = VstEvent::kNoteOffEvent;
        noteOff.ppqPosition = 0;
        noteOff.sampleOffset = 0;
        noteOff.busIndex = noteOn.busIndex;
        noteOff.flags = noteOn.flags;
        noteOff.noteOff.noteId = noteOn.noteOn.noteId;
        noteOff.noteOff.channel = noteOn.noteOn.channel;
        noteOff.noteOff.pitch = noteOn.noteOn.pitch;
        noteOff.noteOff.tuning = noteOn.noteOn.tuning;
        noteOff.noteOff.velocity = noteOn.noteOn.velocity;

        m_eventList.addEvent(noteOff);
    }

    for (PluginParamId id : m_playingParams) {
        auto infoIt = m_pluginParamInfoMap.find(id);
        if (infoIt == m_pluginParamInfoMap.end()) {
            continue;
        }

        ParamChangeEvent paramOff;
        paramOff.paramId = id;
        paramOff.value = infoIt->second.defaultNormalizedValue;

        addParamChange(paramOff);
    }

    m_playingNotes.clear();
    m_playingParams.clear();
}

samples_t VstAudioClient::maxSamplesPerBlock() const
{
    return m_samplesInfo.maxSamplesPerBlock;
}

void VstAudioClient::setMaxSamplesPerBlock(samples_t samples)
{
    if (m_samplesInfo.maxSamplesPerBlock == samples) {
        return;
    }

    m_processData.numSamples = static_cast<Steinberg::int32>(samples);
    m_samplesInfo.maxSamplesPerBlock = samples;
    m_needUnprepareProcessData = true;

    updateProcessSetup();
}

void VstAudioClient::setSampleRate(unsigned int sampleRate)
{
    if (m_samplesInfo.sampleRate == sampleRate) {
        return;
    }

    m_samplesInfo.sampleRate = sampleRate;

    updateProcessSetup();
}

ParamsMapping VstAudioClient::paramsMapping(const std::set<Steinberg::Vst::CtrlNumber>& controllers) const
{
    ParamsMapping result;

    if (!m_pluginPtr) {
        return result;
    }

    PluginMidiMappingPtr midiMapping = m_pluginPtr->midiMapping();
    if (!midiMapping) {
        return result;
    }

    for (const int busIdx : m_activeInputBusses) {
        for (const auto& ctrlNum : controllers) {
            PluginParamId id = 0;

            if (midiMapping->getMidiControllerAssignment(busIdx, 0, ctrlNum, id) != Steinberg::kResultOk) {
                continue;
            }

            result.emplace(ctrlNum, id);
        }
    }

    return result;
}

IAudioProcessorPtr VstAudioClient::pluginProcessor() const
{
    return static_cast<IAudioProcessorPtr>(pluginComponent());
}

PluginComponentPtr VstAudioClient::pluginComponent() const
{
    if (!m_pluginComponent) {
        if (!m_pluginPtr) {
            return nullptr;
        }

        m_pluginComponent = m_pluginPtr->component();
    }

    return m_pluginComponent;
}

void VstAudioClient::setUpProcessData()
{
    PluginComponentPtr component = pluginComponent();
    if (!component) {
        return;
    }

    m_processContext.sampleRate = m_samplesInfo.sampleRate;

    m_processData.inputEvents = &m_eventList;
    m_processData.inputParameterChanges = &m_paramChanges;
    m_processData.processContext = &m_processContext;

    if (m_needUnprepareProcessData) {
        m_processData.unprepare();
        m_needUnprepareProcessData = false;
    }

    if (!m_processData.outputs || !m_processData.inputs) {
        m_processData.prepare(*component, m_samplesInfo.maxSamplesPerBlock, Steinberg::Vst::kSample32);
    }

    if (!m_activeOutputBusses.empty() && !m_activeInputBusses.empty()) {
        return;
    }

    BusInfo busInfo;

    for (int busIndex = 0; busIndex < m_processData.numInputs; ++busIndex) {
        component->getBusInfo(BusMediaType::kAudio, BusDirection::kInput, busIndex, busInfo);

        if (busInfo.busType == BusType::kMain && (busInfo.flags & BusInfo::kDefaultActive)) {
            component->activateBus(BusMediaType::kAudio, BusDirection::kInput, busIndex, true);
            m_activeInputBusses.emplace_back(busIndex);
        }
    }

    for (int busIndex = 0; busIndex < m_processData.numOutputs; ++busIndex) {
        component->getBusInfo(BusMediaType::kAudio, BusDirection::kOutput, busIndex, busInfo);

        if (busInfo.busType == BusType::kMain && (busInfo.flags & BusInfo::kDefaultActive)) {
            component->activateBus(BusMediaType::kAudio, BusDirection::kOutput, busIndex, true);
            m_activeOutputBusses.emplace_back(busIndex);
        }

        LOGI() << "BusIndex: " << busIndex;

        if (busInfo.busType == BusType::kMain) {
            LOGI() << "BusType: Main";
        } else {
            LOGI() << "BusType: Aux";
        }

        if (busInfo.flags & BusInfo::kDefaultActive) {
            LOGI() << "BusFlag: DefaultActive";
        } else {
            LOGI() << "BusFlag: ControlVoltage";
        }
    }

    if (m_activeInputBusses.empty()) {
        LOGI() << "0 active input buses, activating default bus";
        component->activateBus(BusMediaType::kAudio, BusDirection::kInput, 0, true);
        m_activeInputBusses.emplace_back(0);
    }

    if (m_activeOutputBusses.empty()) {
        LOGI() << "0 active output buses, activating default bus";
        component->activateBus(BusMediaType::kAudio, BusDirection::kOutput, 0, true);
        m_activeOutputBusses.emplace_back(0);
    }
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

    disableActivity();

    VstProcessSetup setup;
    setup.processMode = Steinberg::Vst::kRealtime;
    setup.symbolicSampleSize = Steinberg::Vst::kSample32;
    setup.maxSamplesPerBlock = m_samplesInfo.maxSamplesPerBlock;
    setup.sampleRate = m_samplesInfo.sampleRate;

    if (processor->setupProcessing(setup) != Steinberg::kResultOk) {
        return;
    }

    setUpProcessData();
    flushBuffers();

    ensureActivity();
}

void VstAudioClient::extractInputSamples(samples_t sampleCount, const float* sourceBuffer)
{
    if (!m_processData.inputs || !sourceBuffer) {
        return;
    }

    Steinberg::Vst::AudioBusBuffers& bus = m_processData.inputs[0];

    for (samples_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
        size_t offset = sampleIndex * m_audioChannelsCount;

        for (audioch_t audioChannelIndex = 0; audioChannelIndex < bus.numChannels; ++audioChannelIndex) {
            bus.channelBuffers32[audioChannelIndex][sampleIndex] = sourceBuffer[offset + audioChannelIndex];
        }
    }
}

bool VstAudioClient::fillOutputBufferInstrument(samples_t sampleCount, float* output)
{
    if (!m_processData.outputs) {
        return false;
    }

    bool isSilence = true;

    for (const int busIndex : m_activeOutputBusses) {
        Steinberg::Vst::AudioBusBuffers bus = m_processData.outputs[busIndex];

        for (samples_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
            size_t offset = sampleIndex * m_audioChannelsCount;

            for (audioch_t audioChannelIndex = 0; audioChannelIndex < bus.numChannels; ++audioChannelIndex) {
                float sample = bus.channelBuffers32[audioChannelIndex][sampleIndex];
                output[offset + audioChannelIndex] += sample * m_volumeGain;

                if (isSilence && !RealIsNull(sample)) {
                    isSilence = false;
                }
            }
        }
    }

    return !isSilence;
}

bool VstAudioClient::fillOutputBufferFx(samples_t sampleCount, float* output)
{
    if (!m_processData.outputs) {
        return false;
    }

    bool isSilence = true;

    for (const int busIndex : m_activeOutputBusses) {
        Steinberg::Vst::AudioBusBuffers bus = m_processData.outputs[busIndex];

        for (samples_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex) {
            size_t offset = sampleIndex * m_audioChannelsCount;

            for (audioch_t audioChannelIndex = 0; audioChannelIndex < bus.numChannels; ++audioChannelIndex) {
                float sample = bus.channelBuffers32[audioChannelIndex][sampleIndex];
                output[offset + audioChannelIndex] = sample * m_volumeGain;

                if (isSilence && !RealIsNull(sample)) {
                    isSilence = false;
                }
            }
        }
    }

    return !isSilence;
}

void VstAudioClient::ensureActivity()
{
    if (m_isActive) {
        return;
    }

    IAudioProcessorPtr processor = pluginProcessor();
    if (!processor) {
        return;
    }

    PluginComponentPtr component = pluginComponent();
    if (!component) {
        return;
    }

    component->setActive(true);
    processor->setProcessing(true);

    m_isActive = true;
}

void VstAudioClient::disableActivity()
{
    if (!m_isActive) {
        return;
    }

    IAudioProcessorPtr processor = pluginProcessor();
    if (!processor) {
        return;
    }

    PluginComponentPtr component = pluginComponent();
    if (!component) {
        return;
    }

    processor->setProcessing(false);
    component->setActive(false);

    m_isActive = false;
}

void VstAudioClient::flushBuffers()
{
    for (int inputsNumber = 0; inputsNumber < m_processData.numInputs; ++inputsNumber) {
        Steinberg::Vst::AudioBusBuffers input = m_processData.inputs[inputsNumber];

        for (int i = 0; i < m_processData.numSamples; ++i) {
            for (int audioChannel = 0; audioChannel < input.numChannels; ++audioChannel) {
                input.channelBuffers32[audioChannel][i] = 0.f;
            }
        }
    }

    for (int outputsNumber = 0; outputsNumber < m_processData.numOutputs; ++outputsNumber) {
        Steinberg::Vst::AudioBusBuffers output = m_processData.outputs[outputsNumber];

        for (int i = 0; i < m_processData.numSamples; ++i) {
            for (int audioChannel = 0; audioChannel < output.numChannels; ++audioChannel) {
                output.channelBuffers32[audioChannel][i] = 0.f;
            }
        }
    }
}

void VstAudioClient::addParamChange(const ParamChangeEvent& param)
{
    Steinberg::int32 dummyIdx = 0;
    Steinberg::Vst::IParamValueQueue* queue = m_paramChanges.addParameterData(param.paramId, dummyIdx);
    if (queue) {
        queue->addPoint(0, param.value, dummyIdx);
    }
}
