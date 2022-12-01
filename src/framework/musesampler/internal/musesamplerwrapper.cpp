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

using namespace mu;
using namespace mu::audio;
using namespace mu::musesampler;

static constexpr int AUDIO_CHANNELS_COUNT = 2;

MuseSamplerWrapper::MuseSamplerWrapper(MuseSamplerLibHandlerPtr samplerLib, const audio::AudioSourceParams& params)
    : AbstractSynthesizer(params), m_samplerLib(samplerLib)
{
    if (!m_samplerLib || !m_samplerLib->isValid()) {
        return;
    }

    m_samplerLib->initLib();

    m_sequencer.flushedOffStreamEvents().onNotify(this, [this]() {
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

    if (currentRenderMode() == audio::RenderMode::OfflineMode) {
        m_samplerLib->startOfflineMode(m_sampler, m_sampleRate);
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

samples_t MuseSamplerWrapper::process(float* buffer, audio::samples_t samplesPerChannel)
{
    if (!m_samplerLib || !m_sampler || !m_track) {
        return 0;
    }

    if (!isActive()) {
        msecs_t nextMicros = samplesToMsecs(samplesPerChannel, m_sampleRate);

        const MuseSamplerSequencer::EventSequence& sequence = m_sequencer.eventsToBePlayed(nextMicros);
        for (const MuseSamplerSequencer::EventType& event : sequence) {
            handleAuditionEvents(event);
        }
    }

    if (currentRenderMode() == audio::RenderMode::OfflineMode) {
        if (m_samplerLib->processOffline(m_sampler, m_bus) != ms_Result_OK) {
            return 0;
        }
    } else {
        if (m_samplerLib->process(m_sampler, m_bus, m_currentPosition) != ms_Result_OK) {
            return 0;
        }
    }
    extractOutputSamples(samplesPerChannel, buffer);

    if (isActive()) {
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

    LOGI() << "ALL NOTES OFF";
}

bool MuseSamplerWrapper::isValid() const
{
    return m_samplerLib && m_sampler;
}

void MuseSamplerWrapper::setupSound(const mpe::PlaybackSetupData& setupData)
{
    // Check by exact info:
    int unique_id = params().resourceMeta.attributeVal(u"museUID").toInt();
    m_track = m_samplerLib->addTrack(m_sampler, unique_id);
    if (m_track != nullptr) {
        m_sequencer.init(m_samplerLib, m_sampler, m_track);
        return;
    } else {
        LOGE() << "Could not add instrument with ID of " << unique_id;
    }

    LOGE() << "Something went wrong; falling back to MPE info.";
    IF_ASSERT_FAILED(m_samplerLib) {
        return;
    }

    if (!setupData.musicXmlSoundId.has_value()) {
        LOGE() << "Unable to setup MuseSampler";
        return;
    }

    String soundId = setupData.toString();

    auto matchingInstrumentList = m_samplerLib->getMatchingInstrumentList(soundId.toAscii().constChar(),
                                                                          setupData.musicXmlSoundId->c_str());

    if (matchingInstrumentList == nullptr) {
        LOGE() << "Unable to get instrument list";
        return;
    } else {
        LOGD() << "Successfully got instrument list";
    }

    int firstInternalId = -1;
    int internalId = -1;

    // TODO: display all of these in MuseScore, and let the user choose!
    while (auto instrument = m_samplerLib->getNextInstrument(matchingInstrumentList))
    {
        internalId = m_samplerLib->getInstrumentId(instrument);
        const char* internalName = m_samplerLib->getInstrumentName(instrument);
        const char* internalCategory = m_samplerLib->getInstrumentCategory(instrument);
        const char* instrumentPack = m_samplerLib->getInstrumentPackage(instrument);
        const char* musicXmlId = m_samplerLib->getMusicXmlSoundId(instrument);

        LOGD() << internalId
               << ": " << instrumentPack
               << ": " << internalCategory
               << ": " << internalName
               << " - " << musicXmlId;

        // For now, hack to just choose first instrument:
        if (firstInternalId == -1) {
            firstInternalId = internalId;
        }
    }

    if (firstInternalId == -1) {
        LOGE() << "Unable to find sound for " << soundId;
        return;
    }

    m_track = m_samplerLib->addTrack(m_sampler, firstInternalId);

    m_sequencer.init(m_samplerLib, m_sampler, m_track);
}

void MuseSamplerWrapper::setupEvents(const mpe::PlaybackData& playbackData)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_sequencer.load(playbackData);
}

void MuseSamplerWrapper::updateRenderingMode(const audio::RenderMode mode)
{
    ONLY_AUDIO_WORKER_THREAD;

    if (!m_samplerLib || !m_sampler) {
        return;
    }

    if (mode != audio::RenderMode::OfflineMode) {
        m_samplerLib->stopOfflineMode(m_sampler);
    }
}

msecs_t MuseSamplerWrapper::playbackPosition() const
{
    return m_sequencer.playbackPosition();
}

void MuseSamplerWrapper::setPlaybackPosition(const audio::msecs_t newPosition)
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
    IF_ASSERT_FAILED(m_samplerLib && m_sampler && m_track) {
        return;
    }

    if (isActive() == arg) {
        return;
    }

    m_sequencer.setActive(arg);

    m_samplerLib->setPlaying(m_sampler, arg);

    if (!isActive()) {
        setCurrentPosition(m_currentPosition);
    }

    LOGD() << "Toggled playing status, isPlaying: " << arg;
}

void MuseSamplerWrapper::handleAuditionEvents(const MuseSamplerSequencer::EventType& event)
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler && m_track) {
        return;
    }

    if (std::holds_alternative<ms_AuditionStartNoteEvent>(event)) {
        m_samplerLib->startAuditionNote(m_sampler, m_track, std::get<ms_AuditionStartNoteEvent>(event));
        return;
    }

    if (std::holds_alternative<ms_AuditionStopNoteEvent>(event)) {
        m_samplerLib->stopAuditionNote(m_sampler, m_track, std::get<ms_AuditionStopNoteEvent>(event));
        return;
    }
}

void MuseSamplerWrapper::setCurrentPosition(const audio::samples_t samples)
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler && m_track) {
        return;
    }

    if (m_currentPosition == samples) {
        return;
    }

    m_currentPosition = samples;
    m_samplerLib->setPosition(m_sampler, m_currentPosition);

    LOGD() << "Seek a new playback position, newPosition: " << m_currentPosition;
}

void MuseSamplerWrapper::extractOutputSamples(audio::samples_t samples, float* output)
{
    for (audio::samples_t sampleIndex = 0; sampleIndex < samples; ++sampleIndex) {
        for (audio::audioch_t audioChannelIndex = 0; audioChannelIndex < m_bus._num_channels; ++audioChannelIndex) {
            float sample = m_bus._channels[audioChannelIndex][sampleIndex];
            output[sampleIndex * m_bus._num_channels + audioChannelIndex] += sample;
        }
    }
}

void MuseSamplerWrapper::revokePlayingNotes()
{
    if (m_samplerLib) {
        m_samplerLib->allNotesOff(m_sampler);
    }
}
