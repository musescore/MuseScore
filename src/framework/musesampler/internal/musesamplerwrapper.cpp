/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited and others
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

#include "audio/common/audioerrors.h"

#include "global/serialization/json.h"
#include "global/realfn.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::musesampler;

static constexpr int AUDIO_CHANNELS_COUNT = 2;

static InputProcessingProgress::StatusInfo::StatusData parseStatusData(const std::string& json)
{
    if (json.empty()) {
        return {};
    }

    std::string err;
    JsonDocument doc = JsonDocument::fromJson(ByteArray(json.c_str()), &err);

    if (!err.empty() || !doc.isObject()) {
        LOGE() << "JSON parse error: " << err << ", json: " << json;
        return {};
    }

    JsonObject obj = doc.rootObject();
    InputProcessingProgress::StatusInfo::StatusData data;

    if (obj.contains("libraryName")) {
        data["libraryName"] = obj.value("libraryName").toStdString();
    }

    if (obj.contains("date")) {
        data["date"] = obj.value("date").toStdString();
    }

    if (obj.contains("url")) {
        data["url"] = obj.value("url").toStdString();
    }

    return data;
}

MuseSamplerWrapper::MuseSamplerWrapper(MuseSamplerLibHandlerPtr samplerLib,
                                       const InstrumentInfo& instrument,
                                       const AudioSourceParams& params,
                                       const modularity::ContextPtr& iocCtx)
    : AbstractSynthesizer(params, iocCtx),
    m_samplerLib(samplerLib),
    m_instrument(instrument)
{
    if (!m_samplerLib || !m_samplerLib->isValid()) {
        return;
    }

    m_sequencer.setOnOffStreamFlushed([this]() {
        m_allNotesOffRequested = true;
    });
}

MuseSamplerWrapper::~MuseSamplerWrapper()
{
    if (!m_samplerLib || !m_sampler) {
        return;
    }

    if (m_inputProcessingProgress.isStarted) {
        m_inputProcessingProgress.finish((int)Ret::Code::Cancel);
    }

    m_samplerLib->destroy(m_sampler);
}

void MuseSamplerWrapper::setOutputSpec(const audio::OutputSpec& spec)
{
    const bool isOffline = currentRenderMode() == RenderMode::OfflineMode;
    const bool shouldReinitSampler = !m_sampler
                                     || (m_outputSpec.sampleRate != spec.sampleRate && !isOffline)
                                     || (m_outputSpec.samplesPerChannel != spec.samplesPerChannel && !isOffline);

    if (shouldReinitSampler) {
        if (!initSampler(spec.sampleRate, spec.samplesPerChannel)) {
            return;
        }

        m_samplerSampleRate = spec.sampleRate;
    }

    m_outputSpec = spec;

    if (isOffline) {
        LOGD() << "Start offline mode, sampleRate: " << spec.sampleRate;
        m_samplerLib->startOfflineMode(m_sampler, spec.sampleRate);
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
        msecs_t nextMicros = samplesToMsecs(samplesPerChannel, m_outputSpec.sampleRate);
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
    m_sequencer.flushOffstream();
    m_allNotesOffRequested = true;
}

bool MuseSamplerWrapper::isValid() const
{
    return m_samplerLib && m_sampler;
}

void MuseSamplerWrapper::setupSound(const mpe::PlaybackSetupData& setupData)
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler) {
        return;
    }

    m_tracks.clear();

    std::string scoreId = setupData.scoreId.value_or(std::string());
    if (!scoreId.empty()) {
        m_samplerLib->setScoreId(m_sampler, scoreId.c_str());
    }

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

    if (m_instrument.isValid() && m_instrument.isOnline) {
        setupOnlineSound();
    }
}

void MuseSamplerWrapper::setupEvents(const mpe::PlaybackData& playbackData)
{
    ONLY_AUDIO_ENGINE_THREAD;

    m_sequencer.load(playbackData);
}

const mpe::PlaybackData& MuseSamplerWrapper::playbackData() const
{
    ONLY_AUDIO_ENGINE_THREAD;

    return m_sequencer.playbackData();
}

void MuseSamplerWrapper::updateRenderingMode(const RenderMode mode)
{
    ONLY_AUDIO_ENGINE_THREAD;

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
    return samplesToMsecs(m_currentPosition, m_outputSpec.sampleRate);
}

void MuseSamplerWrapper::setPlaybackPosition(const msecs_t newPosition)
{
    m_sequencer.setPlaybackPosition(newPosition);

    setCurrentPosition(microSecsToSamples(newPosition, m_outputSpec.sampleRate));
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

    if (active && m_pendingSetPosition) {
        doCurrentSetPosition();
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

    if (!m_sampler) {
        m_sampler = m_samplerLib->create();
        IF_ASSERT_FAILED(m_sampler) {
            return false;
        }
    }

    if (!m_samplerLib->initSampler(m_sampler, sampleRate, blockSize, AUDIO_CHANNELS_COUNT)) {
        LOGE() << "Unable to init MuseSampler, sampleRate: " << sampleRate << ", blockSize: " << blockSize;
        return false;
    } else {
        LOGI() << "Successfully initialized sampler, sampleRate: " << sampleRate << ", blockSize: " << blockSize;
    }

    prepareOutputBuffer(blockSize);

    return true;
}

void MuseSamplerWrapper::setupOnlineSound()
{
    constexpr double AUTO_PROCESS_INTERVAL = 3.0;
    constexpr double NO_AUTO_PROCESS = -1.0; // interval < 0 -> no auto process

    const bool autoProcess = config()->autoProcessOnlineSoundsInBackground();

    m_sequencer.setUpdateMainStreamWhenInactive(autoProcess);
    m_samplerLib->setAutoRenderInterval(m_sampler, autoProcess ? AUTO_PROCESS_INTERVAL : NO_AUTO_PROCESS);

    //! NOTE: update progress on the worker thread
    m_renderingStateChanged.onReceive(this, [this](ms_RenderingRangeList list, int size) {
        updateRenderingProgress(list, size);
    });

    m_samplerLib->setRenderingStateChangedCallback(m_sampler, [](void* data, ms_RenderingRangeList list, int size) {
        //! NOTE: move call to the worker thread
        RenderingStateChangedChannel* channel = reinterpret_cast<RenderingStateChangedChannel*>(data);
        channel->send(list, size);
    }, &m_renderingStateChanged);

    config()->autoProcessOnlineSoundsInBackgroundChanged().onReceive(this, [this](bool on) {
        m_sequencer.setUpdateMainStreamWhenInactive(on);
        m_sequencer.updateMainStream();
        m_samplerLib->setAutoRenderInterval(m_sampler, on ? AUTO_PROCESS_INTERVAL : NO_AUTO_PROCESS);
    });
}

void MuseSamplerWrapper::updateRenderingProgress(ms_RenderingRangeList list, int size)
{
    ONLY_AUDIO_ENGINE_THREAD;

    IF_ASSERT_FAILED(m_samplerLib && m_sampler) {
        return;
    }

    InputProcessingProgress::ChunkInfoList chunks;
    chunks.reserve(size);

    long long chunksDurationUs = 0;
    bool isRendering = false;

    for (int i = 0; i < size; ++i) {
        const RenderRangeInfo info = m_samplerLib->getNextRenderProgressInfo(list);

        switch (info._state) {
        case ms_RenderingState_Rendering:
            isRendering = true;
            break;
        case ms_RenderingState_ErrorNetwork:
            m_renderingInfo.errorCode = (int)Err::OnlineSoundsProcessingError;
            m_renderingInfo.errorText = "Network error";
            break;
        case ms_RenderingState_ErrorRendering:
            m_renderingInfo.errorCode = (int)Err::OnlineSoundsProcessingError;
            m_renderingInfo.errorText = "Rendering error";
            break;
        case ms_RenderingState_ErrorFileIO:
            m_renderingInfo.errorCode = (int)Err::OnlineSoundsProcessingError;
            m_renderingInfo.errorText = "File IO error";
            break;
        case ms_RenderingState_ErrorTimeOut:
            m_renderingInfo.errorCode = (int)Err::OnlineSoundsProcessingError;
            m_renderingInfo.errorText = "Timeout";
            break;
        case ms_RenderingState_ErrorLimitReached:
            m_renderingInfo.errorCode = (int)Err::OnlineSoundsLimitReached;
            m_renderingInfo.errorText = "Limit reached";
            break;
        }

        if (info._error_message) {
            m_renderingInfo.errorData = info._error_message;
        }

        // Failed regions remain in the list, but should be excluded when
        // calculating the total remaining rendering duration
        if (info._state != ms_RenderingState_Rendering) {
            continue;
        }

        chunksDurationUs += info._end_us - info._start_us;
        chunks.push_back({ audio::microsecsToSecs(info._start_us), audio::microsecsToSecs(info._end_us) });
    }

    // Start progress
    if (!m_inputProcessingProgress.isStarted) {
        // Rendering has started on the sampler side, but it is not yet ready to report progress
        if ((chunksDurationUs <= 0 && isRendering) || size == 0) {
            return;
        }

        m_inputProcessingProgress.start();
    }

    m_renderingInfo.maxChunksDurationUs = std::max(m_renderingInfo.maxChunksDurationUs, chunksDurationUs);

    bool isChanged = false;
    if (m_renderingInfo.lastReceivedChunks != chunks) {
        m_renderingInfo.lastReceivedChunks = chunks;
        isChanged = true;
    }

    // Update percentage
    int64_t percentage = 0;
    if (m_renderingInfo.maxChunksDurationUs != 0) {
        percentage = std::lround(100.f - (float)chunksDurationUs / (float)m_renderingInfo.maxChunksDurationUs * 100.f);
    }

    if (percentage != m_renderingInfo.percentage) {
        m_renderingInfo.percentage = percentage;
        isChanged = true;
    }

    if (isChanged) {
        m_inputProcessingProgress.process(chunks, std::lround(percentage), 100);
    }

    // Finish progress
    if (chunksDurationUs <= 0) {
        m_inputProcessingProgress.finish(m_renderingInfo.errorCode,
                                         m_renderingInfo.errorText,
                                         parseStatusData(m_renderingInfo.errorData));
        m_renderingInfo.clear();
    }
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

    if (std::holds_alternative<AuditionCCEvent>(event)) {
        const AuditionCCEvent& ccEvent = std::get<AuditionCCEvent>(event);
        IF_ASSERT_FAILED(ccEvent.msTrack) {
            return;
        }

        m_samplerLib->addAuditionCCEvent(m_sampler, ccEvent.msTrack, ccEvent.cc, ccEvent.value);
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
    m_pendingSetPosition = true;

    if (isActive()) {
        doCurrentSetPosition();
    }
}

void MuseSamplerWrapper::doCurrentSetPosition()
{
    //! NOTE: very CPU-intensive operation; should be called as infrequently as possible
    m_samplerLib->setPosition(m_sampler, m_currentPosition);
    m_pendingSetPosition = false;
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

void MuseSamplerWrapper::prepareToPlay()
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler) {
        return;
    }

    m_sequencer.updateMainStream();
    doCurrentSetPosition();

    if (readyToPlay()) {
        return;
    }

    if (!m_checkReadyToPlayTimer) {
        m_checkReadyToPlayTimer = std::make_unique<Timer>(std::chrono::microseconds(10000)); // every 10ms
    }

    m_checkReadyToPlayTimer->stop();

    m_checkReadyToPlayTimer->onTimeout(this, [this]() {
        if (readyToPlay()) {
            m_readyToPlayChanged.notify();
            m_checkReadyToPlayTimer->stop();
        }
    });

    m_checkReadyToPlayTimer->start();
}

bool MuseSamplerWrapper::readyToPlay() const
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler) {
        return false;
    }

    return m_samplerLib->readyToPlay(m_sampler);
}

void MuseSamplerWrapper::processInput()
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler) {
        return;
    }

    m_sequencer.updateMainStream();
    m_samplerLib->triggerRender(m_sampler);
}

void MuseSamplerWrapper::clearCache()
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler) {
        return;
    }

    m_samplerLib->clearOnlineCache(m_sampler);
    m_sequencer.updateMainStream();
    m_samplerLib->triggerRender(m_sampler);
}
