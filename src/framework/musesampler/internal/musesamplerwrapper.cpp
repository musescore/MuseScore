/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#include "musesamplerwrapper.h"

#include <cstring>

#include "realfn.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::musesampler;

static constexpr int AUDIO_CHANNELS_COUNT = 2;

MuseSamplerWrapper::MuseSamplerWrapper(MuseSamplerLibHandlerPtr samplerLib,
                                       const InstrumentInfo& instrument,
                                       const AudioSourceParams& params,
                                       const modularity::ContextPtr& iocCtx)
    : AbstractSynthesizer(params, iocCtx), m_samplerLib(samplerLib), m_instrument(instrument)
{
    if (!m_samplerLib || !m_samplerLib->isValid()) {
        return;
    }

    m_sequencer.setOnOffStreamFlushed([this]() {
        m_allNotesOffRequested = true;
    });

    config()->samplesToPreallocateChanged().onReceive(this, [this](const samples_t samples) {
        initSampler(m_samplerSampleRate, samples);
    });
}

MuseSamplerWrapper::~MuseSamplerWrapper()
{
    if (!m_samplerLib || !m_sampler) {
        return;
    }

    m_samplerLib->destroy(m_sampler);
}

void MuseSamplerWrapper::setSampleRate(unsigned int sampleRate)
{
    const bool isOffline = currentRenderMode() == RenderMode::OfflineMode;
    const bool shouldUpdateSampleRate = m_samplerSampleRate != sampleRate && !isOffline;

    if (!m_sampler || shouldUpdateSampleRate) {
        if (!initSampler(sampleRate, config()->samplesToPreallocate())) {
            return;
        }

        m_samplerSampleRate = sampleRate;
    }

    m_sampleRate = sampleRate;

    if (isOffline) {
        LOGD() << "Start offline mode, sampleRate: " << m_sampleRate;
        m_samplerLib->startOfflineMode(m_sampler, m_sampleRate);
        m_offlineModeStarted = true;
    }
}

unsigned int MuseSamplerWrapper::audioChannelsCount() const
{
    return AUDIO_CHANNELS_COUNT;
}

async::Channel<unsigned int> MuseSamplerWrapper::audioChannelsCountChanged() const
{
    return m_audioChannelsCountChanged;
}

samples_t MuseSamplerWrapper::process(float* buffer, samples_t samplesPerChannel)
{
    if (!m_samplerLib || !m_sampler) {
        return 0;
    }

    if (m_allNotesOffRequested) {
        m_samplerLib->allNotesOff(m_sampler);
        m_allNotesOffRequested = false;
    }

    prepareOutputBuffer(samplesPerChannel);

    bool active = isActive();

    if (!active) {
        msecs_t nextMicros = samplesToMsecs(samplesPerChannel, m_sampleRate);
        MuseSamplerSequencer::EventSequenceMap sequences = m_sequencer.movePlaybackForward(nextMicros);

        for (const auto& pair : sequences) {
            for (const MuseSamplerSequencer::EventType& event : pair.second) {
                handleAuditionEvents(event);
            }
        }
    }

    if (currentRenderMode() == RenderMode::OfflineMode) {
        if (m_samplerLib->processOffline(m_sampler, m_bus) != ms_Result_OK) {
            return 0;
        }
    } else {
        if (m_samplerLib->process(m_sampler, m_bus, m_currentPosition) != ms_Result_OK) {
            return 0;
        }
    }

    extractOutputSamples(samplesPerChannel, buffer);

    if (active) {
        m_currentPosition += samplesPerChannel;
    }

    return samplesPerChannel;
}

std::string MuseSamplerWrapper::name() const
{
    return "musesampler";
}

AudioSourceType MuseSamplerWrapper::type() const
{
    return AudioSourceType::MuseSampler;
}

void MuseSamplerWrapper::flushSound()
{
    m_allNotesOffRequested = true;
}

bool MuseSamplerWrapper::isValid() const
{
    return m_samplerLib && m_sampler;
}

void MuseSamplerWrapper::setupSound(const mpe::PlaybackSetupData& setupData)
{
    m_tracks.clear();

    ms_Track track = addTrack();
    if (!track) {
        LOGE() << "Could not add track for instrument: " << m_instrument.instrumentId << "; falling back to MPE info";
        m_instrument = resolveInstrument(setupData);

        track = addTrack();
        if (!track) {
            LOGE() << "Could not init instrument: " << m_instrument.instrumentId;
            return;
        }
    }

    m_sequencer.init(m_samplerLib, m_sampler, this, resolveDefaultPresetCode(m_instrument));
}

void MuseSamplerWrapper::setupEvents(const mpe::PlaybackData& playbackData)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_sequencer.load(playbackData);
}

const mpe::PlaybackData& MuseSamplerWrapper::playbackData() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_sequencer.playbackData();
}

void MuseSamplerWrapper::updateRenderingMode(const RenderMode mode)
{
    ONLY_AUDIO_WORKER_THREAD;

    if (!m_samplerLib || !m_sampler) {
        return;
    }

    m_sequencer.updateMainStream();

    if (mode != RenderMode::OfflineMode && m_offlineModeStarted) {
        m_samplerLib->stopOfflineMode(m_sampler);
        m_offlineModeStarted = false;
    }
}

const TrackList& MuseSamplerWrapper::allTracks() const
{
    return m_tracks;
}

ms_Track MuseSamplerWrapper::addTrack()
{
    TRACEFUNC;

    IF_ASSERT_FAILED(m_samplerLib && m_sampler) {
        return nullptr;
    }

    ms_Track track = m_samplerLib->addTrack(m_sampler, m_instrument.instrumentId);
    if (track) {
        m_tracks.push_back(track);
    }

    return track;
}

msecs_t MuseSamplerWrapper::playbackPosition() const
{
    return samplesToMsecs(m_currentPosition, m_sampleRate);
}

void MuseSamplerWrapper::setPlaybackPosition(const msecs_t newPosition)
{
    m_sequencer.setPlaybackPosition(newPosition);

    setCurrentPosition(microSecsToSamples(newPosition, m_sampleRate));
}

bool MuseSamplerWrapper::isActive() const
{
    return m_sequencer.isActive();
}

void MuseSamplerWrapper::setIsActive(bool active)
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler) {
        return;
    }

    if (isActive() == active) {
        return;
    }

    if (active) {
        m_samplerLib->setPosition(m_sampler, m_currentPosition);
    }

    m_sequencer.setActive(active);
    m_samplerLib->setPlaying(m_sampler, active);
}

bool MuseSamplerWrapper::initSampler(const sample_rate_t sampleRate, const samples_t blockSize)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(sampleRate != 0 && blockSize != 0) {
        return false;
    }

    IF_ASSERT_FAILED(m_samplerLib) {
        return false;
    }

    const bool isFirstInit = m_sampler == nullptr;

    if (isFirstInit) {
        m_sampler = m_samplerLib->create();
        IF_ASSERT_FAILED(m_sampler) {
            return false;
        }
    }

    if (isFirstInit || m_samplerLib->supportsReinit()) {
        if (!m_samplerLib->initSampler(m_sampler, sampleRate, blockSize, AUDIO_CHANNELS_COUNT)) {
            LOGE() << "Unable to init MuseSampler, sampleRate: " << sampleRate << ", blockSize: " << blockSize;
            return false;
        } else {
            LOGI() << "Successfully initialized sampler, sampleRate: " << sampleRate << ", blockSize: " << blockSize;
        }
    }

    prepareOutputBuffer(blockSize);

    return true;
}

InstrumentInfo MuseSamplerWrapper::resolveInstrument(const mpe::PlaybackSetupData& setupData) const
{
    IF_ASSERT_FAILED(m_samplerLib) {
        return InstrumentInfo();
    }

    if (!setupData.musicXmlSoundId.has_value()) {
        return InstrumentInfo();
    }

    String soundId = setupData.toString();

    auto matchingInstrumentList = m_samplerLib->getMatchingInstrumentList(soundId.toAscii().constChar(),
                                                                          setupData.musicXmlSoundId->c_str());

    if (matchingInstrumentList == nullptr) {
        LOGE() << "Unable to get instrument list";
        return InstrumentInfo();
    } else {
        LOGD() << "Successfully got instrument list";
    }

    // TODO: display all of these in MuseScore, and let the user choose!
    while (auto instrument = m_samplerLib->getNextInstrument(matchingInstrumentList)) {
        InstrumentInfo info;
        info.instrumentId = m_samplerLib->getInstrumentId(instrument);
        info.msInstrument = instrument;

        // For now, hack to just choose first instrument:
        if (info.isValid()) {
            return info;
        }
    }

    LOGE() << "Unable to find sound for " << soundId;

    return InstrumentInfo();
}

std::string MuseSamplerWrapper::resolveDefaultPresetCode(const InstrumentInfo& instrument) const
{
    if (!instrument.isValid()) {
        return std::string();
    }

    ms_PresetList presets = m_samplerLib->getPresetList(instrument.msInstrument);

    if (const char* presetCode = m_samplerLib->getNextPreset(presets)) {
        return presetCode;
    }

    return std::string();
}

void MuseSamplerWrapper::prepareOutputBuffer(const samples_t samples)
{
    if (m_leftChannel.size() < samples) {
        m_leftChannel.resize(samples, 0.f);
    }

    if (m_rightChannel.size() < samples) {
        m_rightChannel.resize(samples, 0.f);
    }

    m_bus._num_channels = AUDIO_CHANNELS_COUNT;
    m_bus._num_data_pts = samples;

    m_internalBuffer[0] = m_leftChannel.data();
    m_internalBuffer[1] = m_rightChannel.data();
    m_bus._channels = m_internalBuffer.data();
}

void MuseSamplerWrapper::handleAuditionEvents(const MuseSamplerSequencer::EventType& event)
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler) {
        return;
    }

    if (std::holds_alternative<AuditionStartNoteEvent>(event)) {
        const AuditionStartNoteEvent& noteOn = std::get<AuditionStartNoteEvent>(event);
        IF_ASSERT_FAILED(noteOn.msTrack) {
            return;
        }

        m_samplerLib->startAuditionNote(m_sampler, noteOn.msTrack, noteOn.msEvent);
        return;
    }

    if (std::holds_alternative<AuditionStopNoteEvent>(event)) {
        const AuditionStopNoteEvent& noteOff = std::get<AuditionStopNoteEvent>(event);
        IF_ASSERT_FAILED(noteOff.msTrack) {
            return;
        }

        m_samplerLib->stopAuditionNote(m_sampler, noteOff.msTrack, noteOff.msEvent);
        return;
    }
}

void MuseSamplerWrapper::setCurrentPosition(const samples_t samples)
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler) {
        return;
    }

    if (m_currentPosition == samples) {
        return;
    }

    m_currentPosition = samples;

    if (isActive()) {
        m_samplerLib->setPosition(m_sampler, m_currentPosition);
    }
}

void MuseSamplerWrapper::extractOutputSamples(samples_t samples, float* output)
{
    for (samples_t sampleIndex = 0; sampleIndex < samples; ++sampleIndex) {
        size_t offset = sampleIndex * m_bus._num_channels;

        for (audioch_t audioChannelIndex = 0; audioChannelIndex < m_bus._num_channels; ++audioChannelIndex) {
            float sample = m_bus._channels[audioChannelIndex][sampleIndex];
            output[offset + audioChannelIndex] += sample;
        }
    }
}

void MuseSamplerWrapper::revokePlayingNotes()
{
    m_allNotesOffRequested = true;
}
