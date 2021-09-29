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

VstAudioClient::~VstAudioClient()
{
    if (!m_pluginComponent) {
        return;
    }

    m_pluginComponent->setActive(false);

    IAudioProcessorPtr processor = pluginProcessor();
    if (!processor) {
        return;
    }
    processor->setProcessing(false);
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

audio::samples_t VstAudioClient::process(float* output, audio::samples_t samplesPerChannel)
{
    IAudioProcessorPtr processor = pluginProcessor();
    if (!processor || !output) {
        return 0;
    }

    m_processData.numSamples = samplesPerChannel;

    if (m_type == VstPluginType::Fx) {
        extractInputSamples(samplesPerChannel, output);
    }

    if (processor->process(m_processData) != Steinberg::kResultOk) {
        return 0;
    }

    if (m_type == VstPluginType::Instrument) {
        m_eventList.clear();
    }

    fillOutputBuffer(samplesPerChannel, output);

    return samplesPerChannel;
}

void VstAudioClient::flush()
{
    for (int i = 0; i < m_processData.numSamples; ++i) {
        for (audio::audioch_t s = 0; s < m_audioChannelsCount; ++s) {
            for (int inputsNumber = 0; inputsNumber < m_processData.numInputs; ++inputsNumber) {
                m_processData.inputs[inputsNumber].channelBuffers32[s][i] = 0.f;
            }

            for (int outputsNumber = 0; outputsNumber < m_processData.numOutputs; ++outputsNumber) {
                m_processData.outputs[outputsNumber].channelBuffers32[s][i] = 0.f;
            }
        }
    }
}

void VstAudioClient::setBlockSize(unsigned int samples)
{
    if (m_samplesInfo.samplesPerBlock == samples) {
        return;
    }

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
    return static_cast<IAudioProcessorPtr>(pluginComponent());
}

PluginComponentPtr VstAudioClient::pluginComponent() const
{
    IF_ASSERT_FAILED(m_pluginPtr && m_pluginPtr->provider()) {
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

void VstAudioClient::extractInputSamples(const audio::samples_t& sampleCount, const float* sourceBuffer)
{
    for (unsigned int i = 0; i < sampleCount; ++i) {
        for (audio::audioch_t s = 0; s < m_audioChannelsCount; ++s) {
            m_processData.inputs[0].channelBuffers32[s][i] = sourceBuffer[i * m_audioChannelsCount + s];
        }
    }
}

void VstAudioClient::fillOutputBuffer(unsigned int samples, float* output)
{
    for (unsigned int i = 0; i < samples; ++i) {
        for (unsigned int s = 0; s < m_audioChannelsCount; ++s) {
            auto getFromChannel = std::min<unsigned int>(s, m_processData.outputs[0].numChannels - 1);
            output[i * m_audioChannelsCount + s] = m_processData.outputs[0].channelBuffers32[getFromChannel][i];
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
