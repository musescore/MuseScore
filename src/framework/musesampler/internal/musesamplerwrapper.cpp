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
                                       const AudioSourceParams& params)
    : AbstractSynthesizer(params), m_samplerLib(samplerLib), m_instrument(instrument)
{
    if (!m_samplerLib || !m_samplerLib->isValid()) {
        return;
    }

    m_samplerLib->initLib();

    m_sequencer.setOnOffStreamFlushed([this]() {
        revokePlayingNotes();
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
    m_sampleRate = sampleRate;

    if (!m_sampler) {
        m_sampler = m_samplerLib->create();

        samples_t renderStep = config()->renderStep();

        if (m_samplerLib->initSampler(m_sampler, m_sampleRate, renderStep, AUDIO_CHANNELS_COUNT) != ms_Result_OK) {
            LOGE() << "Unable to init MuseSampler";
            return;
        } else {
            LOGD() << "Successfully initialized sampler";
        }

        m_leftChannel.resize(renderStep);
        m_rightChannel.resize(renderStep);

        m_bus._num_channels = AUDIO_CHANNELS_COUNT;
        m_bus._num_data_pts = renderStep;

        m_internalBuffer[0] = m_leftChannel.data();
        m_internalBuffer[1] = m_rightChannel.data();
        m_bus._channels = m_internalBuffer.data();
    }

    if (currentRenderMode() == RenderMode::OfflineMode) {
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

    bool active = isActive();

    if (!active) {
        msecs_t nextMicros = samplesToMsecs(samplesPerChannel, m_sampleRate);
        MuseSamplerSequencer::EventSequence sequence = m_sequencer.eventsToBePlayed(nextMicros);

        for (const MuseSamplerSequencer::EventType& event : sequence) {
            handleAuditionEvents(event);
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
    IF_ASSERT_FAILED(isValid()) {
        return;
    }

    m_samplerLib->allNotesOff(m_sampler);

    LOGD() << "ALL NOTES OFF";
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

    m_sequencer.init(m_samplerLib, m_sampler, shared_from_this(), resolveDefaultPresetCode(m_instrument));
}

void MuseSamplerWrapper::setupEvents(const mpe::PlaybackData& playbackData)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_sequencer.load(playbackData);
}

void MuseSamplerWrapper::updateRenderingMode(const RenderMode mode)
{
    ONLY_AUDIO_WORKER_THREAD;

    if (!m_samplerLib || !m_sampler) {
        return;
    }

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

    if (!m_samplerLib->supportsMultipleTracks() && !m_tracks.empty()) {
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

void MuseSamplerWrapper::setIsActive(bool arg)
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler) {
        return;
    }

    if (isActive() == arg) {
        return;
    }

    m_sequencer.setActive(arg);

    m_samplerLib->setPlaying(m_sampler, arg);

    if (!isActive()) {
        //! NOTE: restore the current position because setPlaying(m_sampler, false) resets it
        m_samplerLib->setPosition(m_sampler, m_currentPosition);
    }

    LOGD() << "Toggled playing status, isPlaying: " << arg;
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
    m_samplerLib->setPosition(m_sampler, m_currentPosition);

    LOGD() << "Seek a new playback position, newPosition: " << m_currentPosition;
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
    if (m_samplerLib) {
        m_samplerLib->allNotesOff(m_sampler);
    }
}
