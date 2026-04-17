/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "engineplayback.h"

#include "audio/common/audiosanitizer.h"
#include "audio/common/audioerrors.h"
#include "audio/common/audioutils.h"

#include "clock.h"
#include "eventaudiosource.h"
#include "sequenceio.h"
#include "sequenceplayer.h"

#include "muse_framework_config.h"
#ifdef MUSE_MODULE_AUDIO_EXPORT
#include "export/soundtrackwriter.h"
#endif

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::engine;

#ifdef MUSE_MODULE_AUDIO_EXPORT
using namespace muse::audio::soundtrack;
#endif

void EnginePlayback::init()
{
    ONLY_AUDIO_ENGINE_THREAD;

    m_clock = std::make_shared<Clock>();
    m_player = std::make_shared<SequencePlayer>(this, m_clock);
    m_audioIO = std::make_shared<SequenceIO>(this);

    audioEngine()->modeChanged().onReceive(this, [this](RenderMode mode) {
        m_prevActiveTrackId = INVALID_TRACK_ID;

        if (mode == RenderMode::IdleMode) {
            mixer()->setTracksToProcessWhenIdle(m_tracksToProcessWhenIdle);
        }
    });

    mixer()->addClock(m_clock);

    m_audioIO->inputParamsChanged().onReceive(this, [this](const TrackId trackId, const AudioInputParams& params) {
        m_inputParamsChanged.send(trackId, params);
    });

    m_audioIO->outputParamsChanged().onReceive(this, [this](const TrackId trackId, const AudioOutputParams& params) {
        m_outputParamsChanged.send(trackId, params);
    });

    ensureMixerSubscriptions();
}

void EnginePlayback::deinit()
{
    ONLY_AUDIO_ENGINE_THREAD;

    if (mixer()) {
        mixer()->removeClock(m_clock);
    }

    removeAllTracks();

    m_player.reset();
    m_audioIO.reset();
    m_clock.reset();

    // Explicitly disconnect and clear all channel members before
    // async_disconnectAll() and before the destructor runs. This ensures
    // subscribers are disconnected while they're still alive, not during IoC
    // teardown when some may already be destroyed.
    m_saveSoundTracksProgress = SaveSoundTrackProgressData();
    m_masterOutputParamsChanged = async::Channel<AudioOutputParams>();
    m_outputParamsChanged = async::Channel<TrackId, AudioOutputParams>();
    m_inputParamsChanged = async::Channel<TrackId, AudioInputParams>();
    m_trackRemoved = async::Channel<TrackId>();
    m_trackAdded = async::Channel<TrackId>();

    async_disconnectAll();
}

void EnginePlayback::ensureMixerSubscriptions()
{
    ONLY_AUDIO_ENGINE_THREAD;

    if (!mixer()->masterOutputParamsChanged().isConnected()) {
        mixer()->masterOutputParamsChanged().onReceive(this, [this](const AudioOutputParams& params) {
            m_masterOutputParamsChanged.send(params);
        });
    }
}

TrackId EnginePlayback::newTrackId() const
{
    static TrackId lastId = 0;
    ++lastId;
    return lastId;
}

// 2. Setup tracks
RetVal<TrackIdList> EnginePlayback::trackIdList() const
{
    ONLY_AUDIO_ENGINE_THREAD;

    TrackIdList result;
    result.reserve(m_tracks.size());

    for (const auto& pair : m_tracks) {
        result.push_back(pair.first);
    }

    return RetVal<TrackIdList>::make_ok(result);
}

RetVal<TrackName> EnginePlayback::trackName(const TrackId trackId) const
{
    ONLY_AUDIO_ENGINE_THREAD;

    if (const TrackPtr trackPtr = track(trackId)) {
        return RetVal<TrackName>::make_ok(trackPtr->name);
    }

    return RetVal<TrackName>::make_ret((int)Err::InvalidTrackId, "no track");
}

RetVal2<TrackId, AudioParams> EnginePlayback::addTrack(const std::string& trackName,
                                                       io::IODevice* playbackData,
                                                       const AudioParams& params)
{
    ONLY_AUDIO_ENGINE_THREAD;

    RetVal2<TrackId, AudioParams> result;
    result.val1 = -1;

    if (!playbackData) {
        result.ret = make_ret(Err::InvalidAudioFilePath);
        return result;
    }

    TrackId newId = newTrackId();
    SoundTrackPtr trackPtr = std::make_shared<SoundTrack>();
    trackPtr->id = newId;
    trackPtr->name = trackName;
    trackPtr->setPlaybackData(playbackData);
    trackPtr->setInputParams(params.in);
    trackPtr->setOutputParams(params.out);

    m_trackAboutToBeAdded.send(trackPtr);
    m_tracks.emplace(newId, trackPtr);
    m_trackAdded.send(newId);

    result.ret = make_ret(Err::NoError);
    result.val1 = newId;
    result.val2 = { trackPtr->inputParams(), trackPtr->outputParams() };
    return result;
}

RetVal2<TrackId, AudioParams> EnginePlayback::addTrack(const std::string& trackName,
                                                       const mpe::PlaybackData& playbackData, const AudioParams& params)
{
    ONLY_AUDIO_ENGINE_THREAD;

    RetVal2<TrackId, AudioParams> result;
    result.val1 = -1;

    if (!playbackData.setupData.isValid()) {
        result.ret = make_ret(Err::InvalidSetupData);
        return result;
    }

    TrackId newId = newTrackId();

    auto onOffStreamReceived = [this](const TrackId trackId) {
        std::unordered_set<TrackId> tracksToProcess = m_tracksToProcessWhenIdle;

        if (m_prevActiveTrackId == INVALID_TRACK_ID) {
            tracksToProcess.insert(trackId);
        } else {
            tracksToProcess.insert({ m_prevActiveTrackId, trackId });
        }

        mixer()->setTracksToProcessWhenIdle(tracksToProcess);
        m_prevActiveTrackId = trackId;
    };

    EventAudioSourcePtr source = std::make_shared<EventAudioSource>(newId, playbackData, onOffStreamReceived);
    source->setOutputSpec(audioEngine()->outputSpec());

    RetVal<MixerChannelPtr> channel = mixer()->addChannel(newId, source);
    if (!channel.ret) {
        result.ret = channel.ret;
        return result;
    }

    channel.val->shouldProcessDuringSilenceChanged().onReceive(this, [this, newId](bool shouldProcess) {
        onShouldProcessDuringSilenceChanged(newId, shouldProcess);
    });

    EventTrackPtr trackPtr = std::make_shared<EventTrack>();
    trackPtr->id = newId;
    trackPtr->name = trackName;
    trackPtr->setPlaybackData(playbackData);
    trackPtr->inputHandler = source;
    trackPtr->outputHandler = channel.val;
    trackPtr->setInputParams(params.in);
    trackPtr->setOutputParams(params.out);

    m_trackAboutToBeAdded.send(trackPtr);
    m_tracks.emplace(newId, trackPtr);
    m_trackAdded.send(newId);

    result.ret = make_ret(Err::NoError);
    result.val1 = newId;
    result.val2 = { trackPtr->inputParams(), trackPtr->outputParams() };
    return result;
}

RetVal2<TrackId, AudioOutputParams> EnginePlayback::addAuxTrack(const std::string& trackName,
                                                                const AudioOutputParams& outputParams)
{
    ONLY_AUDIO_ENGINE_THREAD;

    RetVal2<TrackId, AudioOutputParams> result;
    result.val1 = -1;

    TrackId newId = newTrackId();
    RetVal<MixerChannelPtr> channel = mixer()->addAuxChannel(newId);
    if (!channel.ret) {
        result.ret = channel.ret;
        return result;
    }

    channel.val->shouldProcessDuringSilenceChanged().onReceive(this, [this, newId](bool shouldProcess) {
        onShouldProcessDuringSilenceChanged(newId, shouldProcess);
    });

    EventTrackPtr trackPtr = std::make_shared<EventTrack>();
    trackPtr->id = newId;
    trackPtr->name = trackName;
    trackPtr->outputHandler = channel.val;
    trackPtr->setOutputParams(outputParams);

    m_trackAboutToBeAdded.send(trackPtr);
    m_tracks.emplace(newId, trackPtr);
    m_trackAdded.send(newId);

    result.ret = make_ret(Err::NoError);
    result.val1 = newId;
    result.val2 = trackPtr->outputParams();
    return result;
}

void EnginePlayback::removeTrack(const TrackId trackId)
{
    ONLY_AUDIO_ENGINE_THREAD;

    auto it = m_tracks.find(trackId);
    if (it == m_tracks.end() || !it->second) {
        return;
    }

    m_trackAboutToBeRemoved.send(it->second);
    mixer()->removeChannel(trackId);
    m_tracks.erase(trackId);
    muse::remove(m_tracksToProcessWhenIdle, trackId);

    if (m_prevActiveTrackId == trackId) {
        m_prevActiveTrackId = INVALID_TRACK_ID;
    }

    m_trackRemoved.send(trackId);
}

void EnginePlayback::removeAllTracks()
{
    ONLY_AUDIO_ENGINE_THREAD;

    for (const TrackId& id : trackIdList().val) {
        removeTrack(id);
    }
}

async::Channel<TrackId> EnginePlayback::trackAdded() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_trackAdded;
}

async::Channel<TrackId> EnginePlayback::trackRemoved() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_trackRemoved;
}

AudioResourceMetaList EnginePlayback::availableInputResources() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return synthResolver()->resolveAvailableResources();
}

SoundPresetList EnginePlayback::availableSoundPresets(const AudioResourceMeta& resourceMeta) const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return synthResolver()->resolveAvailableSoundPresets(resourceMeta);
}

RetVal<AudioInputParams> EnginePlayback::inputParams(const TrackId trackId) const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_audioIO->inputParams(trackId);
}

void EnginePlayback::setInputParams(const TrackId trackId, const AudioInputParams& params)
{
    ONLY_AUDIO_ENGINE_THREAD;
    m_audioIO->setInputParams(trackId, params);
}

async::Channel<TrackId, AudioInputParams> EnginePlayback::inputParamsChanged() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_inputParamsChanged;
}

void EnginePlayback::processInput(const TrackId trackId) const
{
    ONLY_AUDIO_ENGINE_THREAD;
    m_audioIO->processInput(trackId);
}

RetVal<InputProcessingProgress> EnginePlayback::inputProcessingProgress(const TrackId trackId) const
{
    ONLY_AUDIO_ENGINE_THREAD;
    if (!m_audioIO->hasTrack(trackId)) {
        return RetVal<InputProcessingProgress>::make_ret((int)Err::InvalidTrackId, "no track");
    }

    return RetVal<InputProcessingProgress>::make_ok(m_audioIO->inputProcessingProgress(trackId));
}

void EnginePlayback::clearCache(const TrackId trackId) const
{
    ONLY_AUDIO_ENGINE_THREAD;
    m_audioIO->clearCache(trackId);
}

void EnginePlayback::clearSources()
{
    ONLY_AUDIO_ENGINE_THREAD;
    synthResolver()->clearSources();
}

// 3. Play
async::Promise<Ret> EnginePlayback::prepareToPlay()
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_player->prepareToPlay();
}

void EnginePlayback::play(const secs_t delay)
{
    ONLY_AUDIO_ENGINE_THREAD;
    m_player->play(delay);
}

void EnginePlayback::seek(const secs_t newPosition, const bool flushSound)
{
    ONLY_AUDIO_ENGINE_THREAD;
    m_player->seek(newPosition, flushSound);
}

void EnginePlayback::stop()
{
    ONLY_AUDIO_ENGINE_THREAD;
    m_player->stop();
}

void EnginePlayback::pause()
{
    ONLY_AUDIO_ENGINE_THREAD;
    m_player->pause();
}

void EnginePlayback::resume(const secs_t delay)
{
    ONLY_AUDIO_ENGINE_THREAD;
    m_player->resume(delay);
}

void EnginePlayback::setDuration(const msecs_t durationMsec)
{
    ONLY_AUDIO_ENGINE_THREAD;
    m_player->setDuration(durationMsec);
}

Ret EnginePlayback::setLoop(const msecs_t fromMsec, const msecs_t toMsec)
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_player->setLoop(fromMsec, toMsec);
}

void EnginePlayback::resetLoop()
{
    ONLY_AUDIO_ENGINE_THREAD;
    m_player->resetLoop();
}

PlaybackStatus EnginePlayback::playbackStatus() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_player->playbackStatus();
}

async::Channel<PlaybackStatus> EnginePlayback::playbackStatusChanged() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_player->playbackStatusChanged();
}

secs_t EnginePlayback::playbackPosition() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_player->playbackPosition();
}

async::Channel<secs_t> EnginePlayback::playbackPositionChanged() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_player->playbackPositionChanged();
}

// 4. Adjust output
RetVal<AudioOutputParams> EnginePlayback::outputParams(const TrackId trackId) const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_audioIO->outputParams(trackId);
}

void EnginePlayback::setOutputParams(const TrackId trackId, const AudioOutputParams& params)
{
    ONLY_AUDIO_ENGINE_THREAD;
    m_audioIO->setOutputParams(trackId, params);
}

async::Channel<TrackId, AudioOutputParams> EnginePlayback::outputParamsChanged() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_outputParamsChanged;
}

TrackPtr EnginePlayback::track(const TrackId id) const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return muse::value(m_tracks, id, nullptr);
}

const TracksMap& EnginePlayback::allTracks() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_tracks;
}

async::Channel<TrackPtr> EnginePlayback::trackAboutToBeAdded() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_trackAboutToBeAdded;
}

async::Channel<TrackPtr> EnginePlayback::trackAboutToBeRemoved() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_trackAboutToBeRemoved;
}

void EnginePlayback::onShouldProcessDuringSilenceChanged(const TrackId trackId, bool shouldProcess)
{
    ONLY_AUDIO_ENGINE_THREAD;
    if (shouldProcess) {
        m_tracksToProcessWhenIdle.insert(trackId);
    } else {
        muse::remove(m_tracksToProcessWhenIdle, trackId);
    }

    mixer()->setTracksToProcessWhenIdle(m_tracksToProcessWhenIdle);
}

std::shared_ptr<Mixer> EnginePlayback::mixer() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return audioEngine()->mixer();
}

RetVal<AudioOutputParams> EnginePlayback::masterOutputParams() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return RetVal<AudioOutputParams>::make_ok(mixer()->masterOutputParams());
}

void EnginePlayback::setMasterOutputParams(const AudioOutputParams& params)
{
    ONLY_AUDIO_ENGINE_THREAD;
    mixer()->setMasterOutputParams(params);
}

void EnginePlayback::clearMasterOutputParams()
{
    ONLY_AUDIO_ENGINE_THREAD;
    mixer()->clearMasterOutputParams();
}

async::Channel<AudioOutputParams> EnginePlayback::masterOutputParamsChanged() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_masterOutputParamsChanged;
}

AudioResourceMetaList EnginePlayback::availableOutputResources() const
{
    ONLY_AUDIO_ENGINE_THREAD;

    return fxResolver()->resolveAvailableResources();
}

RetVal<AudioSignalChanges> EnginePlayback::signalChanges(const TrackId trackId) const
{
    ONLY_AUDIO_ENGINE_THREAD;
    if (!m_audioIO->hasTrack(trackId)) {
        return RetVal<AudioSignalChanges>::make_ret((int)Err::InvalidTrackId, "no track");
    }

    return RetVal<AudioSignalChanges>::make_ok(m_audioIO->audioSignalChanges(trackId));
}

RetVal<AudioSignalChanges> EnginePlayback::masterSignalChanges() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return RetVal<AudioSignalChanges>::make_ok(mixer()->masterAudioSignalChanges());
}

async::Promise<Ret> EnginePlayback::saveSoundTrack(io::IODevice& dstDevice, const SoundTrackFormat& format)
{
    return async::make_promise<Ret>([this, &dstDevice, format](auto resolve, auto) {
        ONLY_AUDIO_ENGINE_THREAD;

#ifdef MUSE_MODULE_AUDIO_EXPORT
        m_player->stop();
        m_player->seek(0);

        const bool lazyProcessingWasEnabled = configuration()->isLazyProcessingOfOnlineSoundsEnabled();
        configuration()->setIsLazyProcessingOfOnlineSoundsEnabled(false);

        listenInputProcessing([this, &dstDevice, format, lazyProcessingWasEnabled, resolve](Ret ret) {
            if (ret) {
                ret = doSaveSoundTrack(dstDevice, format);
            }

            configuration()->setIsLazyProcessingOfOnlineSoundsEnabled(lazyProcessingWasEnabled);
            (void)resolve(ret);
        });

        return async::Promise<Ret>::dummy_result();
#else
        return resolve(make_ret(Err::DisabledAudioExport, "audio export is disabled"));
#endif
    });
}

void EnginePlayback::listenInputProcessing(std::function<void(const Ret&)> completed)
{
#ifdef MUSE_MODULE_AUDIO_EXPORT

    auto soundsInProgress = std::make_shared<size_t>(0);

    for (TrackId trackId : trackIdList().val) {
        if (!isOnlineAudioResource(m_audioIO->inputParams(trackId).val.resourceMeta)) {
            continue;
        }

        InputProcessingProgress inputProgress = m_audioIO->inputProcessingProgress(trackId);
        if (!inputProgress.isStarted && !m_audioIO->hasPendingChunks(trackId)) {
            continue;
        }

        (*soundsInProgress)++;

        if (inputProgress.isStarted) {
            m_saveSoundTracksProgress.progress.send(0, 100, SaveSoundTrackStage::ProcessingOnlineSounds);
        }

        using StatusInfo = InputProcessingProgress::StatusInfo;
        using ChunkInfoList = InputProcessingProgress::ChunkInfoList;
        using ProgressInfo = InputProcessingProgress::ProgressInfo;

        auto ch = inputProgress.processedChannel;
        ch.onReceive(this, [this, trackId, soundsInProgress, completed](const StatusInfo& status,
                                                                        const ChunkInfoList&,
                                                                        const ProgressInfo& info) {
            const size_t tracksBeingProcessedCount = this->tracksBeingProcessedCount();

            if (status.status == InputProcessingProgress::Status::Finished) {
                m_audioIO->inputProcessingProgress(trackId).processedChannel.disconnect(this);
            }

            if (tracksBeingProcessedCount == 0) {
                m_saveSoundTracksProgress.aborted.disconnect(this);
                completed(make_ok());
            } else if (tracksBeingProcessedCount == 1) {
                m_saveSoundTracksProgress.progress.send(info.current, info.total, SaveSoundTrackStage::ProcessingOnlineSounds);
            } else {
                const int64_t percentage = 100 - (100 / *soundsInProgress) * tracksBeingProcessedCount;
                m_saveSoundTracksProgress.progress.send(percentage, 100, SaveSoundTrackStage::ProcessingOnlineSounds);
            }
        });
    }

    if ((*soundsInProgress) == 0) {
        completed(make_ok());
        return;
    }

    m_saveSoundTracksProgress.aborted.onNotify(this, [this, completed]() {
        m_saveSoundTracksProgress.aborted.disconnect(this);

        for (TrackId trackId : trackIdList().val) {
            if (isOnlineAudioResource(m_audioIO->inputParams(trackId).val.resourceMeta)) {
                m_audioIO->inputProcessingProgress(trackId).processedChannel.disconnect(this);
            }
        }

        completed(make_ret(Ret::Code::Cancel));
    });
#else
    UNUSED(completed);
#endif
}

size_t EnginePlayback::tracksBeingProcessedCount() const
{
    size_t count = 0;

    for (TrackId trackId : trackIdList().val) {
        if (m_audioIO->inputProcessingProgress(trackId).isStarted || m_audioIO->hasPendingChunks(trackId)) {
            count++;
        }
    }

    return count;
}

Ret EnginePlayback::doSaveSoundTrack(io::IODevice& dstDevice, const SoundTrackFormat& format)
{
#ifdef MUSE_MODULE_AUDIO_EXPORT
    const msecs_t totalDuration = m_player->duration();
    SoundTrackWriterPtr writer = std::make_shared<SoundTrackWriter>(dstDevice, format, totalDuration, mixer());

    writer->progress().progressChanged().onReceive(this, [this](int64_t current, int64_t total, std::string /*title*/) {
        m_saveSoundTracksProgress.progress.send(current, total, SaveSoundTrackStage::WritingSoundTrack);
    });

    std::weak_ptr<SoundTrackWriter> weakPtr = writer;
    m_saveSoundTracksProgress.aborted.onNotify(this, [weakPtr]() {
        if (auto writer = weakPtr.lock()) {
            writer->abort();
        }
    });

    Ret ret = writer->write();
    m_player->seek(0);

    m_saveSoundTracksProgress.aborted.disconnect(this);

    return ret;
#else
    return make_ret(Err::DisabledAudioExport, "audio export is disabled");
#endif
}

void EnginePlayback::abortSavingAllSoundTracks()
{
#ifdef MUSE_MODULE_AUDIO_EXPORT
    m_saveSoundTracksProgress.aborted.notify();
#endif
}

SaveSoundTrackProgress EnginePlayback::saveSoundTrackProgressChanged() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_saveSoundTracksProgress.progress;
}

void EnginePlayback::clearAllFx()
{
    ONLY_AUDIO_ENGINE_THREAD;
    fxResolver()->clearAllFx();
}
