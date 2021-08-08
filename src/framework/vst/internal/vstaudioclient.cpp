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
    if (!pluginComponent()) {
        return;
    }

    m_pluginComponent->setActive(false);

    IAudioProcessorPtr processor = pluginProcessor();
    if (!processor) {
        return;
    }
    processor->setProcessing(false);
}

void VstAudioClient::init(VstPluginPtr plugin)
{
    IF_ASSERT_FAILED(plugin) {
        return;
    }

    m_pluginPtr = std::move(plugin);
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
    if (!m_pluginComponent) {
        m_pluginComponent = m_pluginPtr->component();
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

void VstAudioClient::fillOutputBuffer(unsigned int samples, float* output)
{
    unsigned int audioChannels = config()->audioChannelsCount();

    for (unsigned int i = 0; i < samples; ++i) {
        for (unsigned int s = 0; s < audioChannels; ++s) {
            auto getFromChannel = std::min<unsigned int>(s, m_processData.outputs[0].numChannels - 1);
            output[i * audioChannels + s] = m_processData.outputs[0].channelBuffers32[getFromChannel][i];
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
