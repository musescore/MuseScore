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
#include "audio/synthtypes.h"

using namespace mu;
using namespace mu::vst;
using namespace mu::mpe;

VstAudioClient::~VstAudioClient()
{
    if (!m_pluginComponent) {
        return;
    }

    m_pluginComponent->setActive(false);
    m_pluginComponent->terminate();
    m_pluginComponent->release();
}

void VstAudioClient::init(VstPluginType&& type, VstPluginPtr plugin, audio::audioch_t&& audioChannelsCount)
{
    IF_ASSERT_FAILED(plugin && type != VstPluginType::Undefined) {
        return;
    }

    m_type = type;
    m_pluginPtr = std::move(plugin);
    m_audioChannelsCount = audioChannelsCount;
}

bool VstAudioClient::handleEvent(const VstEvent& event)
{
    ensureActivity();

    if (m_eventList.addEvent(const_cast<VstEvent&>(event)) == Steinberg::kResultTrue) {
        return true;
    }

    return false;
}

bool VstAudioClient::handleParamChange(const PluginParamInfo& param)
{
    IF_ASSERT_FAILED(m_pluginPtr && m_pluginPtr->provider()) {
        return false;
    }

    ensureActivity();

    Steinberg::int32 dummyIdx = 0;
    Steinberg::Vst::IParamValueQueue* queue = m_paramChanges.addParameterData(param.id, dummyIdx);
    if (queue) {
        queue->addPoint(0, param.defaultNormalizedValue, dummyIdx);
    }

    return true;
}

void VstAudioClient::setVolumeGain(const audio::gain_t newVolumeGain)
{
    m_volumeGain = newVolumeGain;
}

audio::samples_t VstAudioClient::process(float* output, audio::samples_t samplesPerChannel)
{
    IAudioProcessorPtr processor = pluginProcessor();
    if (!processor || !output) {
        return 0;
    }

    if (!m_isActive) {
        return 0;
    }

    m_processData.numSamples = samplesPerChannel;

    //! NOTE: From the VST3 documentation:
    //!
    //! Note that the ProcessData->numSamples
    //! which indicates how many samples are used in a process call can change from call to call,
    //! but never bigger than the maxSamplesPerBlock
    if (samplesPerChannel != m_samplesInfo.samplesPerBlock) {
        setBlockSize(samplesPerChannel);
    }

    if (m_type == VstPluginType::Fx) {
        extractInputSamples(samplesPerChannel, output);
    }

    if (processor->process(m_processData) != Steinberg::kResultOk) {
        return 0;
    }

    if (m_type == VstPluginType::Instrument) {
        m_eventList.clear();
        m_paramChanges.clearQueue();
    }

    if (!fillOutputBuffer(samplesPerChannel, output)) {
        return 0;
    }

    return samplesPerChannel;
}

void VstAudioClient::flush()
{
    flushBuffers();

    disableActivity();

    m_eventList.clear();
    m_paramChanges.clearQueue();
}

void VstAudioClient::setBlockSize(unsigned int samples)
{
    if (m_samplesInfo.samplesPerBlock == samples) {
        return;
    }

    m_samplesInfo.samplesPerBlock = samples;
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

    if (!m_pluginPtr || !m_pluginPtr->provider()) {
        return result;
    }

    auto controller = m_pluginPtr->provider()->getController();
    if (!controller) {
        return result;
    }

    Steinberg::Vst::IMidiMapping* midiMapping = nullptr;
    controller->queryInterface(Steinberg::Vst::IMidiMapping_iid, (void**)&midiMapping);

    if (!midiMapping) {
        return result;
    }

    for (const int busIdx : m_activeInputBusses) {
        for (const auto& ctrlNum : controllers) {
            PluginParamId id = 0;

            if (midiMapping->getMidiControllerAssignment(busIdx, 0, ctrlNum, id) != Steinberg::kResultOk) {
                continue;
            }

            result.emplace(ctrlNum, std::move(id));
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
    if (!m_pluginPtr || !m_pluginPtr->provider()) {
        return nullptr;
    }

    if (!m_pluginComponent) {
        m_pluginComponent = m_pluginPtr->provider()->getComponent();
    }

    return m_pluginComponent;
}

void VstAudioClient::setUpProcessData()
{
    m_processContext.sampleRate = m_samplesInfo.sampleRate;

    m_processData.inputEvents = &m_eventList;
    m_processData.inputParameterChanges = &m_paramChanges;
    m_processData.processContext = &m_processContext;

    if (m_needUnprepareProcessData) {
        m_processData.unprepare();
        m_needUnprepareProcessData = false;
    }

    if (!m_processData.outputs || !m_processData.inputs) {
        m_processData.prepare(*m_pluginComponent, m_samplesInfo.samplesPerBlock, Steinberg::Vst::kSample32);
    }

    if (!m_activeOutputBusses.empty() && !m_activeInputBusses.empty()) {
        return;
    }

    BusInfo busInfo;

    for (int busIndex = 0; busIndex < m_processData.numInputs; ++busIndex) {
        m_pluginComponent->getBusInfo(BusMediaType::kAudio, BusDirection::kInput, busIndex, busInfo);

        if (busInfo.busType == BusType::kMain && (busInfo.flags & BusInfo::kDefaultActive)) {
            m_pluginComponent->activateBus(BusMediaType::kAudio, BusDirection::kInput, busIndex, true);
            m_activeInputBusses.emplace_back(busIndex);
        }
    }

    for (int busIndex = 0; busIndex < m_processData.numOutputs; ++busIndex) {
        m_pluginComponent->getBusInfo(BusMediaType::kAudio, BusDirection::kOutput, busIndex, busInfo);

        if (busInfo.busType == BusType::kMain && (busInfo.flags & BusInfo::kDefaultActive)) {
            m_pluginComponent->activateBus(BusMediaType::kAudio, BusDirection::kOutput, busIndex, true);
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
        m_pluginComponent->activateBus(BusMediaType::kAudio, BusDirection::kInput, 0, true);
        m_activeInputBusses.emplace_back(0);
    }

    if (m_activeOutputBusses.empty()) {
        LOGI() << "0 active output buses, activating default bus";
        m_pluginComponent->activateBus(BusMediaType::kAudio, BusDirection::kOutput, 0, true);
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
    setup.maxSamplesPerBlock = m_samplesInfo.samplesPerBlock;
    setup.sampleRate = m_samplesInfo.sampleRate;

    if (processor->setupProcessing(setup) != Steinberg::kResultOk) {
        return;
    }

    processor->setProcessing(true);
    m_pluginComponent->setActive(true);
    m_isActive = true;

    setUpProcessData();
    flushBuffers();
}

void VstAudioClient::extractInputSamples(const audio::samples_t& sampleCount, const float* sourceBuffer)
{
    if (!m_processData.inputs || !sourceBuffer) {
        return;
    }

    Steinberg::Vst::AudioBusBuffers& bus = m_processData.inputs[0];

    for (unsigned int i = 0; i < sampleCount; ++i) {
        for (audio::audioch_t s = 0; s < bus.numChannels; ++s) {
            bus.channelBuffers32[s][i] = sourceBuffer[i * m_audioChannelsCount + s];
        }
    }
}

bool VstAudioClient::fillOutputBuffer(unsigned int samples, float* output)
{
    bool hasMeaningSamples = false;

    if (!m_processData.outputs) {
        return hasMeaningSamples;
    }

    for (const int busIndex : m_activeOutputBusses) {
        Steinberg::Vst::AudioBusBuffers bus = m_processData.outputs[busIndex];

        for (audio::samples_t sampleIndex = 0; sampleIndex < samples; ++sampleIndex) {
            for (audio::audioch_t audioChannelIndex = 0; audioChannelIndex < bus.numChannels; ++audioChannelIndex) {
                float sample = bus.channelBuffers32[audioChannelIndex][sampleIndex];

                if (m_type == VstPluginType::Instrument) {
                    output[sampleIndex * m_audioChannelsCount + audioChannelIndex] += sample * m_volumeGain;
                } else {
                    output[sampleIndex * m_audioChannelsCount + audioChannelIndex] = sample * m_volumeGain;
                }

                if (hasMeaningSamples) {
                    continue;
                }

                if (!RealIsEqual(sample, 0)) {
                    hasMeaningSamples = true;
                }
            }
        }
    }

    return hasMeaningSamples;
}

void VstAudioClient::ensureActivity()
{
    if (m_isActive) {
        return;
    }

    updateProcessSetup();
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

    processor->setProcessing(false);
    m_pluginComponent->setActive(false);

    m_isActive = false;
}

void VstAudioClient::flushBuffers()
{
    for (int i = 0; i < m_processData.numSamples; ++i) {
        for (int inputsNumber = 0; inputsNumber < m_processData.numInputs; ++inputsNumber) {
            Steinberg::Vst::AudioBusBuffers input = m_processData.inputs[inputsNumber];

            for (int audioChannel = 0; audioChannel < input.numChannels; ++audioChannel) {
                input.channelBuffers32[audioChannel][i] = 0.f;
            }
        }

        for (int outputsNumber = 0; outputsNumber < m_processData.numOutputs; ++outputsNumber) {
            Steinberg::Vst::AudioBusBuffers output = m_processData.outputs[outputsNumber];

            for (int audioChannel = 0; audioChannel < output.numChannels; ++audioChannel) {
                output.channelBuffers32[audioChannel][i] = 0.f;
            }
        }
    }
}
