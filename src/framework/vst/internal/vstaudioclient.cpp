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

bool VstAudioClient::handleNoteOnEvents(const mpe::PlaybackEvent& event, const audio::msecs_t from, const audio::msecs_t to)
{
    if (!std::holds_alternative<NoteEvent>(event)) {
        return false;
    }

    const NoteEvent& noteEvent = std::get<NoteEvent>(event);

    timestamp_t timestampFrom = noteEvent.arrangementCtx().actualTimestamp;

    if (timestampFrom < from || timestampFrom >= to) {
        return false;
    }

    ensureActivity();

    VstEvent vstEvent;

    vstEvent.busIndex = 0;
    vstEvent.sampleOffset = 0;
    vstEvent.ppqPosition = 0;
    vstEvent.flags = VstEvent::kIsLive;

    int noteId = noteIndex(noteEvent.pitchCtx().nominalPitchLevel);

    vstEvent.type = VstEvent::kNoteOnEvent;
    vstEvent.noteOn.noteId = noteId;
    vstEvent.noteOn.channel = 0;
    vstEvent.noteOn.pitch = noteId;
    vstEvent.noteOn.tuning = 0;
    vstEvent.noteOn.velocity = noteVelocityFraction(noteEvent.expressionCtx().expressionCurve.maxAmplitudeLevel());

    if (m_eventList.addEvent(vstEvent) == Steinberg::kResultTrue) {
        return true;
    }

    return false;
}

bool VstAudioClient::handleNoteOffEvents(const mpe::PlaybackEvent& event, const audio::msecs_t from, const audio::msecs_t to)
{
    if (!std::holds_alternative<NoteEvent>(event)) {
        return false;
    }

    const NoteEvent& noteEvent = std::get<NoteEvent>(event);

    timestamp_t timestampFrom = noteEvent.arrangementCtx().actualTimestamp;
    timestamp_t timestampTo = timestampFrom + noteEvent.arrangementCtx().actualDuration;

    if (timestampTo <= from || timestampTo > to) {
        return false;
    }

    VstEvent vstEvent;

    vstEvent.busIndex = 0;
    vstEvent.sampleOffset = 0;
    vstEvent.ppqPosition = 0;
    vstEvent.flags = VstEvent::kIsLive;

    int noteId = noteIndex(noteEvent.pitchCtx().nominalPitchLevel);

    vstEvent.type = VstEvent::kNoteOffEvent;
    vstEvent.noteOn.noteId = noteId;
    vstEvent.noteOn.channel = 0;
    vstEvent.noteOn.pitch = noteId;
    vstEvent.noteOn.tuning = 0;
    vstEvent.noteOn.velocity = noteVelocityFraction(noteEvent.expressionCtx().expressionCurve.maxAmplitudeLevel());

    if (m_eventList.addEvent(vstEvent) == Steinberg::kResultTrue) {
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
    m_samplesInfo.sampleRate = sampleRate;

    updateProcessSetup();
}

bool VstAudioClient::isPluginInputAvailable() const
{
    return m_isPluginInputAvailable;
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
        m_isPluginInputAvailable = m_pluginPtr->isAbleForInput();
    }

    return m_pluginComponent;
}

void VstAudioClient::setUpProcessData()
{
    m_processContext.sampleRate = m_samplesInfo.sampleRate;

    m_processData.inputEvents = &m_eventList;
    m_processData.processContext = &m_processContext;

    if (m_needUnprepareProcessData) {
        m_processData.unprepare();
        m_needUnprepareProcessData = false;
    }

    m_processData.prepare(*m_pluginComponent, m_samplesInfo.samplesPerBlock, Steinberg::Vst::kSample32);

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

    for (unsigned int i = 0; i < sampleCount; ++i) {
        for (audio::audioch_t s = 0; s < m_audioChannelsCount; ++s) {
            m_processData.inputs[0].channelBuffers32[s][i] = sourceBuffer[i * m_audioChannelsCount + s];
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
                output[sampleIndex * m_audioChannelsCount + audioChannelIndex] += sample;

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

int VstAudioClient::noteIndex(const mpe::pitch_level_t pitchLevel) const
{
    static constexpr mpe::pitch_level_t MIN_SUPPORTED_LEVEL = mpe::pitchLevel(PitchClass::C, 0);
    static constexpr int MIN_SUPPORTED_NOTE = 12; // VST equivalent for C0
    static constexpr mpe::pitch_level_t MAX_SUPPORTED_LEVEL = mpe::pitchLevel(PitchClass::C, 8);
    static constexpr int MAX_SUPPORTED_NOTE = 108; // VST equivalent for C8

    if (pitchLevel <= MIN_SUPPORTED_LEVEL) {
        return MIN_SUPPORTED_NOTE;
    }

    if (pitchLevel >= MAX_SUPPORTED_LEVEL) {
        return MAX_SUPPORTED_NOTE;
    }

    float stepCount = MIN_SUPPORTED_NOTE + ((pitchLevel - MIN_SUPPORTED_LEVEL) / static_cast<float>(mpe::PITCH_LEVEL_STEP));

    return RealRound(stepCount, 0);
}

float VstAudioClient::noteVelocityFraction(const mpe::dynamic_level_t dynamicLevel) const
{
    return RealRound(dynamicLevel / static_cast<float>(mpe::MAX_PITCH_LEVEL), 2);
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
