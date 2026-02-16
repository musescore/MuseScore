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

#include "tracksequence.h"

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

    ensureMixerSubscriptions();
}

void EnginePlayback::deinit()
{
    ONLY_AUDIO_ENGINE_THREAD;

    m_sequences.clear();
    m_saveSoundTracksProgressMap.clear();

    // Explicitly disconnect and clear all channel members before
    // async_disconnectAll() and before the destructor runs. This ensures
    // subscribers are disconnected while they're still alive, not during IoC
    // teardown when some may already be destroyed.
    m_masterOutputParamsChanged = async::Channel<AudioOutputParams>();
    m_outputParamsChanged = async::Channel<TrackSequenceId, TrackId, AudioOutputParams>();
    m_inputParamsChanged = async::Channel<TrackSequenceId, TrackId, AudioInputParams>();
    m_trackRemoved = async::Channel<TrackSequenceId, TrackId>();
    m_trackAdded = async::Channel<TrackSequenceId, TrackId>();

    async_disconnectAll();
}

void EnginePlayback::ensureMixerSubscriptions()
{
    ONLY_AUDIO_ENGINE_THREAD;

    if (!mixer()) {
        return;
    }

    if (!mixer()->masterOutputParamsChanged().isConnected()) {
        mixer()->masterOutputParamsChanged().onReceive(this, [this](const AudioOutputParams& params) {
            m_masterOutputParamsChanged.send(params);
        });
    }
}

TrackSequenceId EnginePlayback::addSequence()
{
    ONLY_AUDIO_ENGINE_THREAD;

    static TrackSequenceId lastId = 0;

    TrackSequenceId newId = ++lastId;
    ITrackSequencePtr s = std::make_shared<TrackSequence>(newId, iocContext());

    m_sequences.emplace(newId, s);

    ensureSubscriptions(s);

    return newId;
}

void EnginePlayback::ensureSubscriptions(const ITrackSequencePtr s)
{
    ONLY_AUDIO_ENGINE_THREAD;

    if (!s) {
        return;
    }

    TrackSequenceId sequenceId = s->id();

    if (!s->audioIO()->inputParamsChanged().isConnected()) {
        s->audioIO()->inputParamsChanged().onReceive(this, [this, sequenceId](const TrackId trackId, const AudioInputParams& params) {
            m_inputParamsChanged.send(sequenceId, trackId, params);
        });
    }

    if (!s->audioIO()->outputParamsChanged().isConnected()) {
        s->audioIO()->outputParamsChanged().onReceive(this, [this, sequenceId](const TrackId trackId, const AudioOutputParams& params) {
            m_outputParamsChanged.send(sequenceId, trackId, params);
        });
    }

    if (!s->trackAdded().isConnected()) {
        s->trackAdded().onReceive(this, [this, sequenceId](const TrackId trackId) {
            m_trackAdded.send(sequenceId, trackId);
        });
    }

    if (!s->trackRemoved().isConnected()) {
        s->trackRemoved().onReceive(this, [this, sequenceId](const TrackId trackId) {
            m_trackRemoved.send(sequenceId, trackId);
        });
    }
}

void EnginePlayback::removeSequence(const TrackSequenceId id)
{
    ONLY_AUDIO_ENGINE_THREAD;

    muse::remove(m_sequences, id);
}

TrackSequenceIdList EnginePlayback::sequenceIdList() const
{
    ONLY_AUDIO_ENGINE_THREAD;

    TrackSequenceIdList result;
    result.reserve(m_sequences.size());

    for (const auto& pair : m_sequences) {
        result.push_back(pair.first);
    }

    return result;
}

ITrackSequencePtr EnginePlayback::sequence(const TrackSequenceId id) const
{
    ONLY_AUDIO_ENGINE_THREAD;

    return muse::value(m_sequences, id, nullptr);
}

// 2. Setup tracks for Sequence
RetVal<TrackIdList> EnginePlayback::trackIdList(const TrackSequenceId sequenceId) const
{
    ONLY_AUDIO_ENGINE_THREAD;

    ITrackSequencePtr s = sequence(sequenceId);
    if (!s) {
        return RetVal<TrackIdList>::make_ret((int)Err::InvalidSequenceId, "invalid sequence id");
    }

    return RetVal<TrackIdList>::make_ok(s->trackIdList());
}

RetVal<TrackName> EnginePlayback::trackName(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    ONLY_AUDIO_ENGINE_THREAD;

    RetVal<TrackIdList> result;
    ITrackSequencePtr s = sequence(sequenceId);
    if (!s) {
        return RetVal<TrackName>::make_ret((int)Err::InvalidSequenceId, "invalid sequence id");
    }

    return RetVal<TrackName>::make_ok(s->trackName(trackId));
}

RetVal2<TrackId, AudioParams> EnginePlayback::addTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                                       io::IODevice* playbackData,
                                                       const AudioParams& params)
{
    ONLY_AUDIO_ENGINE_THREAD;

    ITrackSequencePtr s = sequence(sequenceId);
    if (!s) {
        return RetVal2<TrackId, AudioParams>::make_ret((int)Err::InvalidSequenceId, "invalid sequence id");
    }

    return s->addTrack(trackName, playbackData, params);
}

RetVal2<TrackId, AudioParams> EnginePlayback::addTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                                       const mpe::PlaybackData& playbackData, const AudioParams& params)
{
    ONLY_AUDIO_ENGINE_THREAD;

    ITrackSequencePtr s = sequence(sequenceId);
    if (!s) {
        return RetVal2<TrackId, AudioParams>::make_ret((int)Err::InvalidSequenceId, "invalid sequence id");
    }

    return s->addTrack(trackName, playbackData, params);
}

RetVal2<TrackId, AudioOutputParams> EnginePlayback::addAuxTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                                                const AudioOutputParams& outputParams)
{
    ONLY_AUDIO_ENGINE_THREAD;

    ITrackSequencePtr s = sequence(sequenceId);
    if (!s) {
        return RetVal2<TrackId, AudioOutputParams>::make_ret((int)Err::InvalidSequenceId, "invalid sequence id");
    }

    return s->addAuxTrack(trackName, outputParams);
}

void EnginePlayback::removeTrack(const TrackSequenceId sequenceId, const TrackId trackId)
{
    ONLY_AUDIO_ENGINE_THREAD;

    ITrackSequencePtr s = sequence(sequenceId);

    if (!s) {
        return;
    }

    s->removeTrack(trackId);
}

void EnginePlayback::removeAllTracks(const TrackSequenceId sequenceId)
{
    ONLY_AUDIO_ENGINE_THREAD;

    ITrackSequencePtr s = sequence(sequenceId);

    if (!s) {
        return;
    }

    for (const TrackId& id : s->trackIdList()) {
        s->removeTrack(id);
    }
}

async::Channel<TrackSequenceId, TrackId> EnginePlayback::trackAdded() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_trackAdded;
}

async::Channel<TrackSequenceId, TrackId> EnginePlayback::trackRemoved() const
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

RetVal<AudioInputParams> EnginePlayback::inputParams(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    ONLY_AUDIO_ENGINE_THREAD;

    ITrackSequencePtr s = sequence(sequenceId);
    if (!s) {
        return RetVal<AudioInputParams>::make_ret((int)Err::InvalidSequenceId, "invalid sequence id");
    }

    return s->audioIO()->inputParams(trackId);
}

void EnginePlayback::setInputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioInputParams& params)
{
    ONLY_AUDIO_ENGINE_THREAD;

    ITrackSequencePtr s = sequence(sequenceId);

    if (s) {
        s->audioIO()->setInputParams(trackId, params);
    }
}

async::Channel<TrackSequenceId, TrackId, AudioInputParams> EnginePlayback::inputParamsChanged() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_inputParamsChanged;
}

void EnginePlayback::processInput(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    ONLY_AUDIO_ENGINE_THREAD;

    const ITrackSequencePtr s = sequence(sequenceId);
    IF_ASSERT_FAILED(s) {
        return;
    }

    s->audioIO()->processInput(trackId);
}

RetVal<InputProcessingProgress> EnginePlayback::inputProcessingProgress(const TrackSequenceId sequenceId,
                                                                        const TrackId trackId) const
{
    ONLY_AUDIO_ENGINE_THREAD;

    const ITrackSequencePtr s = sequence(sequenceId);
    if (!s) {
        return RetVal<InputProcessingProgress>::make_ret((int)Err::InvalidSequenceId, "invalid sequence id");
    }

    if (!s->audioIO()->hasTrack(trackId)) {
        return RetVal<InputProcessingProgress>::make_ret((int)Err::InvalidTrackId, "no track");
    }

    return RetVal<InputProcessingProgress>::make_ok(s->audioIO()->inputProcessingProgress(trackId));
}

void EnginePlayback::clearCache(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    ONLY_AUDIO_ENGINE_THREAD;

    const ITrackSequencePtr s = sequence(sequenceId);
    IF_ASSERT_FAILED(s) {
        return;
    }

    s->audioIO()->clearCache(trackId);
}

void EnginePlayback::clearSources()
{
    ONLY_AUDIO_ENGINE_THREAD;
    synthResolver()->clearSources();
}

// 3. Play Sequence
async::Promise<Ret> EnginePlayback::prepareToPlay(TrackSequenceId sequenceId)
{
    ONLY_AUDIO_ENGINE_THREAD;
    ITrackSequencePtr s = sequence(sequenceId);
    IF_ASSERT_FAILED(s) {
        return async::make_promise<Ret>([](auto resolve, auto) {
            return resolve(make_ret(Err::InvalidSequenceId, "invalid sequence id"));
        });
    }
    return s->player()->prepareToPlay();
}

void EnginePlayback::play(TrackSequenceId sequenceId, const secs_t delay)
{
    ONLY_AUDIO_ENGINE_THREAD;
    ITrackSequencePtr s = sequence(sequenceId);
    IF_ASSERT_FAILED(s) {
        return;
    }
    s->player()->play(delay);
}

void EnginePlayback::seek(TrackSequenceId sequenceId, const secs_t newPosition, const bool flushSound)
{
    ONLY_AUDIO_ENGINE_THREAD;
    ITrackSequencePtr s = sequence(sequenceId);
    IF_ASSERT_FAILED(s) {
        return;
    }
    s->player()->seek(newPosition, flushSound);
}

void EnginePlayback::stop(TrackSequenceId sequenceId)
{
    ONLY_AUDIO_ENGINE_THREAD;
    ITrackSequencePtr s = sequence(sequenceId);
    IF_ASSERT_FAILED(s) {
        return;
    }
    s->player()->stop();
}

void EnginePlayback::pause(TrackSequenceId sequenceId)
{
    ONLY_AUDIO_ENGINE_THREAD;
    ITrackSequencePtr s = sequence(sequenceId);
    IF_ASSERT_FAILED(s) {
        return;
    }
    s->player()->pause();
}

void EnginePlayback::resume(TrackSequenceId sequenceId, const secs_t delay)
{
    ONLY_AUDIO_ENGINE_THREAD;
    ITrackSequencePtr s = sequence(sequenceId);
    IF_ASSERT_FAILED(s) {
        return;
    }
    s->player()->resume(delay);
}

void EnginePlayback::setDuration(TrackSequenceId sequenceId, const msecs_t durationMsec)
{
    ONLY_AUDIO_ENGINE_THREAD;
    ITrackSequencePtr s = sequence(sequenceId);
    IF_ASSERT_FAILED(s) {
        return;
    }
    s->player()->setDuration(durationMsec);
}

Ret EnginePlayback::setLoop(TrackSequenceId sequenceId, const msecs_t fromMsec, const msecs_t toMsec)
{
    ONLY_AUDIO_ENGINE_THREAD;

    ITrackSequencePtr s = sequence(sequenceId);
    if (!s) {
        return make_ret(Err::InvalidSequenceId, "invalid sequence id");
    }

    return s->player()->setLoop(fromMsec, toMsec);
}

void EnginePlayback::resetLoop(TrackSequenceId sequenceId)
{
    ONLY_AUDIO_ENGINE_THREAD;
    ITrackSequencePtr s = sequence(sequenceId);
    IF_ASSERT_FAILED(s) {
        return;
    }
    s->player()->resetLoop();
}

PlaybackStatus EnginePlayback::playbackStatus(TrackSequenceId sequenceId) const
{
    ONLY_AUDIO_ENGINE_THREAD;
    ITrackSequencePtr s = sequence(sequenceId);
    IF_ASSERT_FAILED(s) {
        return PlaybackStatus::Stopped;
    }

    return s->player()->playbackStatus();
}

async::Channel<PlaybackStatus> EnginePlayback::playbackStatusChanged(TrackSequenceId sequenceId) const
{
    ONLY_AUDIO_ENGINE_THREAD;
    ITrackSequencePtr s = sequence(sequenceId);
    IF_ASSERT_FAILED(s) {
        return async::Channel<PlaybackStatus>();
    }

    return s->player()->playbackStatusChanged();
}

secs_t EnginePlayback::playbackPosition(TrackSequenceId sequenceId) const
{
    ONLY_AUDIO_ENGINE_THREAD;
    ITrackSequencePtr s = sequence(sequenceId);
    IF_ASSERT_FAILED(s) {
        return 0.0;
    }

    return s->player()->playbackPosition();
}

async::Channel<secs_t> EnginePlayback::playbackPositionChanged(TrackSequenceId sequenceId) const
{
    ONLY_AUDIO_ENGINE_THREAD;
    ITrackSequencePtr s = sequence(sequenceId);
    IF_ASSERT_FAILED(s) {
        return async::Channel<secs_t>();
    }

    return s->player()->playbackPositionChanged();
}

// 4. Adjust a Sequence output
RetVal<AudioOutputParams> EnginePlayback::outputParams(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    ONLY_AUDIO_ENGINE_THREAD;

    const ITrackSequencePtr s = sequence(sequenceId);
    if (!s) {
        return RetVal<AudioOutputParams>::make_ret((int)Err::InvalidSequenceId, "invalid sequence id");
    }

    return s->audioIO()->outputParams(trackId);
}

void EnginePlayback::setOutputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioOutputParams& params)
{
    ONLY_AUDIO_ENGINE_THREAD;

    ITrackSequencePtr s = sequence(sequenceId);
    if (s) {
        s->audioIO()->setOutputParams(trackId, params);
    }
}

async::Channel<TrackSequenceId, TrackId, AudioOutputParams> EnginePlayback::outputParamsChanged() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_outputParamsChanged;
}

std::shared_ptr<Mixer> EnginePlayback::mixer() const
{
    return audioEngine()->mixer();
}

RetVal<AudioOutputParams> EnginePlayback::masterOutputParams() const
{
    ONLY_AUDIO_ENGINE_THREAD;

    IF_ASSERT_FAILED(mixer()) {
        return RetVal<AudioOutputParams>::make_ret((int)Err::Undefined, "undefined reference to a mixer");
    }

    return RetVal<AudioOutputParams>::make_ok(mixer()->masterOutputParams());
}

void EnginePlayback::setMasterOutputParams(const AudioOutputParams& params)
{
    ONLY_AUDIO_ENGINE_THREAD;

    IF_ASSERT_FAILED(mixer()) {
        return;
    }

    mixer()->setMasterOutputParams(params);
}

void EnginePlayback::clearMasterOutputParams()
{
    ONLY_AUDIO_ENGINE_THREAD;

    IF_ASSERT_FAILED(mixer()) {
        return;
    }

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

RetVal<AudioSignalChanges> EnginePlayback::signalChanges(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    ONLY_AUDIO_ENGINE_THREAD;

    const ITrackSequencePtr s = sequence(sequenceId);
    if (!s) {
        return RetVal<AudioSignalChanges>::make_ret((int)Err::InvalidSequenceId, "invalid sequence id");
    }

    if (!s->audioIO()->hasTrack(trackId)) {
        return RetVal<AudioSignalChanges>::make_ret((int)Err::InvalidTrackId, "no track");
    }

    return RetVal<AudioSignalChanges>::make_ok(s->audioIO()->audioSignalChanges(trackId));
}

RetVal<AudioSignalChanges> EnginePlayback::masterSignalChanges() const
{
    ONLY_AUDIO_ENGINE_THREAD;

    IF_ASSERT_FAILED(mixer()) {
        return RetVal<AudioSignalChanges>::make_ret((int)Err::Undefined, "undefined reference to a mixer");
    }

    return RetVal<AudioSignalChanges>::make_ok(mixer()->masterAudioSignalChanges());
}

async::Promise<Ret> EnginePlayback::saveSoundTrack(const TrackSequenceId sequenceId, io::IODevice& dstDevice,
                                                   const SoundTrackFormat& format)
{
    return async::make_promise<Ret>([this, sequenceId, &dstDevice, format](auto resolve, auto) {
        ONLY_AUDIO_ENGINE_THREAD;

        IF_ASSERT_FAILED(mixer()) {
            return resolve(make_ret(Err::Undefined, "undefined reference to a mixer"));
        }

        ITrackSequencePtr s = sequence(sequenceId);
        if (!s) {
            return resolve(make_ret(Err::InvalidSequenceId, "invalid sequence id"));
        }

#ifdef MUSE_MODULE_AUDIO_EXPORT
        s->player()->stop();
        s->player()->seek(0);

        const bool lazyProcessingWasEnabled = configuration()->isLazyProcessingOfOnlineSoundsEnabled();
        configuration()->setIsLazyProcessingOfOnlineSoundsEnabled(false);

        listenInputProcessing(s, [this, sequenceId, &dstDevice, format, lazyProcessingWasEnabled, resolve](Ret ret) {
            if (ret) {
                ret = doSaveSoundTrack(sequenceId, dstDevice, format);
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

void EnginePlayback::listenInputProcessing(ITrackSequencePtr s, std::function<void(const Ret&)> completed)
{
#ifdef MUSE_MODULE_AUDIO_EXPORT
    const TrackSequenceId sequenceId = s->id();
    const ISequenceIOPtr audioIO = s->audioIO();
    SaveSoundTrackProgressData& progressData = m_saveSoundTracksProgressMap[sequenceId];
    auto soundsInProgress = std::make_shared<size_t>(0);

    for (TrackId trackId : s->trackIdList()) {
        if (!isOnlineAudioResource(audioIO->inputParams(trackId).val.resourceMeta)) {
            continue;
        }

        InputProcessingProgress inputProgress = audioIO->inputProcessingProgress(trackId);
        if (!inputProgress.isStarted && !audioIO->hasPendingChunks(trackId)) {
            continue;
        }

        (*soundsInProgress)++;

        if (inputProgress.isStarted) {
            progressData.progress.send(0, 100, SaveSoundTrackStage::ProcessingOnlineSounds);
        }

        using StatusInfo = InputProcessingProgress::StatusInfo;
        using ChunkInfoList = InputProcessingProgress::ChunkInfoList;
        using ProgressInfo = InputProcessingProgress::ProgressInfo;

        auto ch = inputProgress.processedChannel;
        ch.onReceive(this, [this, sequenceId, trackId, soundsInProgress, completed](const StatusInfo& status,
                                                                                    const ChunkInfoList&,
                                                                                    const ProgressInfo& info) {
            ITrackSequencePtr s = sequence(sequenceId);
            if (!s) {
                completed(make_ret(Err::InvalidSequenceId, "invalid sequence id"));
                return;
            }

            const size_t tracksBeingProcessedCount = this->tracksBeingProcessedCount(s);

            if (status.status == InputProcessingProgress::Status::Finished) {
                s->audioIO()->inputProcessingProgress(trackId).processedChannel.disconnect(this);
            }

            if (tracksBeingProcessedCount == 0) {
                m_saveSoundTracksProgressMap[sequenceId].aborted.disconnect(this);
                completed(make_ok());
            } else if (tracksBeingProcessedCount == 1) {
                SaveSoundTrackProgress& progress = m_saveSoundTracksProgressMap[sequenceId].progress;
                progress.send(info.current, info.total, SaveSoundTrackStage::ProcessingOnlineSounds);
            } else {
                const int64_t percentage = 100 - (100 / *soundsInProgress) * tracksBeingProcessedCount;
                SaveSoundTrackProgress& progress = m_saveSoundTracksProgressMap[sequenceId].progress;
                progress.send(percentage, 100, SaveSoundTrackStage::ProcessingOnlineSounds);
            }
        });
    }

    if ((*soundsInProgress) == 0) {
        completed(make_ok());
        return;
    }

    progressData.aborted.onNotify(this, [this, sequenceId, completed]() {
        m_saveSoundTracksProgressMap[sequenceId].aborted.disconnect(this);

        ITrackSequencePtr s = sequence(sequenceId);
        if (!s) {
            completed(make_ret(Ret::Code::Cancel));
            return;
        }

        for (TrackId trackId : s->trackIdList()) {
            if (isOnlineAudioResource(s->audioIO()->inputParams(trackId).val.resourceMeta)) {
                s->audioIO()->inputProcessingProgress(trackId).processedChannel.disconnect(this);
            }
        }

        completed(make_ret(Ret::Code::Cancel));
    });
#else
    UNUSED(s);
    UNUSED(completed);
#endif
}

size_t EnginePlayback::tracksBeingProcessedCount(const ITrackSequencePtr s) const
{
    const ISequenceIOPtr audioIO = s->audioIO();
    size_t count = 0;

    for (TrackId trackId : s->trackIdList()) {
        if (audioIO->inputProcessingProgress(trackId).isStarted || audioIO->hasPendingChunks(trackId)) {
            count++;
        }
    }

    return count;
}

Ret EnginePlayback::doSaveSoundTrack(const TrackSequenceId sequenceId, io::IODevice& dstDevice, const SoundTrackFormat& format)
{
#ifdef MUSE_MODULE_AUDIO_EXPORT
    ITrackSequencePtr s = sequence(sequenceId);
    if (!s) {
        return make_ret(Err::InvalidSequenceId, "invalid sequence id");
    }

    const msecs_t totalDuration = s->player()->duration();
    SoundTrackWriterPtr writer = std::make_shared<SoundTrackWriter>(dstDevice, format, totalDuration, mixer(), iocContext());
    SaveSoundTrackProgressData progressData = m_saveSoundTracksProgressMap[s->id()];

    writer->progress().progressChanged().onReceive(this, [progressData](int64_t current, int64_t total, std::string /*title*/) {
        SaveSoundTrackProgress mutProgress = progressData.progress;
        mutProgress.send(current, total, SaveSoundTrackStage::WritingSoundTrack);
    });

    std::weak_ptr<SoundTrackWriter> weakPtr = writer;
    progressData.aborted.onNotify(this, [weakPtr]() {
        if (auto writer = weakPtr.lock()) {
            writer->abort();
        }
    });

    Ret ret = writer->write();
    s->player()->seek(0);

    progressData.aborted.disconnect(this);

    return ret;
#else
    return make_ret(Err::DisabledAudioExport, "audio export is disabled");
#endif
}

void EnginePlayback::abortSavingAllSoundTracks()
{
#ifdef MUSE_MODULE_AUDIO_EXPORT
    for (auto& pair : m_saveSoundTracksProgressMap) {
        pair.second.aborted.notify();
    }
#endif
}

SaveSoundTrackProgress EnginePlayback::saveSoundTrackProgressChanged(const TrackSequenceId sequenceId) const
{
    //! FIXME
    ONLY_AUDIO_MAIN_OR_ENGINE_THREAD;
    if (!muse::contains(m_saveSoundTracksProgressMap, sequenceId)) {
        m_saveSoundTracksProgressMap.emplace(sequenceId, SaveSoundTrackProgressData());
    }

    return m_saveSoundTracksProgressMap.at(sequenceId).progress;
}

void EnginePlayback::clearAllFx()
{
    ONLY_AUDIO_ENGINE_THREAD;
    fxResolver()->clearAllFx();
}
