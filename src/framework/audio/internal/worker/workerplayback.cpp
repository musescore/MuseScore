/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
#include "workerplayback.h"

#include "internal/audiosanitizer.h"

#include "tracksequence.h"

#include "internal/player.h"

#include "audioerrors.h"

#include "muse_framework_config.h"
#ifdef MUSE_MODULE_AUDIO_EXPORT
#include "internal/soundtracks/soundtrackwriter.h"
#endif

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::worker;

#ifdef MUSE_MODULE_AUDIO_EXPORT
using namespace muse::audio::soundtrack;
#endif

void WorkerPlayback::init()
{
    ONLY_AUDIO_WORKER_THREAD;

    ensureMixerSubscriptions();
}

void WorkerPlayback::deinit()
{
    ONLY_AUDIO_WORKER_THREAD;

    m_sequences.clear();

    disconnectAll();
}

void WorkerPlayback::ensureMixerSubscriptions()
{
    ONLY_AUDIO_WORKER_THREAD;

    if (!mixer()) {
        return;
    }

    if (!mixer()->masterOutputParamsChanged().isConnected()) {
        mixer()->masterOutputParamsChanged().onReceive(this, [this](const AudioOutputParams& params) {
            m_masterOutputParamsChanged.send(params);
        });
    }
}

TrackSequenceId WorkerPlayback::addSequence()
{
    ONLY_AUDIO_WORKER_THREAD;

    static TrackSequenceId lastId = 0;

    TrackSequenceId newId = ++lastId;
    ITrackSequencePtr s = std::make_shared<TrackSequence>(newId, iocContext());

    m_sequences.emplace(newId, s);

    ensureSubscriptions(s);

    return newId;
}

void WorkerPlayback::ensureSubscriptions(const ITrackSequencePtr s)
{
    ONLY_AUDIO_WORKER_THREAD;

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

void WorkerPlayback::removeSequence(const TrackSequenceId id)
{
    ONLY_AUDIO_WORKER_THREAD;

    auto it = m_sequences.find(id);
    if (it != m_sequences.end()) {
        m_sequences.erase(it);
    }
}

TrackSequenceIdList WorkerPlayback::sequenceIdList() const
{
    ONLY_AUDIO_WORKER_THREAD;

    TrackSequenceIdList result;
    result.reserve(m_sequences.size());

    for (const auto& pair : m_sequences) {
        result.push_back(pair.first);
    }

    return result;
}

ITrackSequencePtr WorkerPlayback::sequence(const TrackSequenceId id) const
{
    ONLY_AUDIO_WORKER_THREAD;

    auto search = m_sequences.find(id);

    if (search != m_sequences.end()) {
        return search->second;
    }

    return nullptr;
}

// 2. Setup tracks for Sequence
RetVal<TrackIdList> WorkerPlayback::trackIdList(const TrackSequenceId sequenceId) const
{
    ONLY_AUDIO_WORKER_THREAD;

    RetVal<TrackIdList> result;
    ITrackSequencePtr s = sequence(sequenceId);
    if (!s) {
        return RetVal<TrackIdList>::make_ret((int)Err::InvalidSequenceId, "invalid sequence id");
    }

    return RetVal<TrackIdList>::make_ok(s->trackIdList());
}

RetVal<TrackName> WorkerPlayback::trackName(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    ONLY_AUDIO_WORKER_THREAD;

    RetVal<TrackIdList> result;
    ITrackSequencePtr s = sequence(sequenceId);
    if (!s) {
        return RetVal<TrackName>::make_ret((int)Err::InvalidSequenceId, "invalid sequence id");
    }

    return RetVal<TrackName>::make_ok(s->trackName(trackId));
}

RetVal2<TrackId, AudioParams> WorkerPlayback::addTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                                       io::IODevice* playbackData,
                                                       const AudioParams& params)
{
    ONLY_AUDIO_WORKER_THREAD;

    ITrackSequencePtr s = sequence(sequenceId);
    if (!s) {
        return RetVal2<TrackId, AudioParams>::make_ret((int)Err::InvalidSequenceId, "invalid sequence id");
    }

    return s->addTrack(trackName, playbackData, params);
}

RetVal2<TrackId, AudioParams> WorkerPlayback::addTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                                       const mpe::PlaybackData& playbackData, const AudioParams& params)
{
    ONLY_AUDIO_WORKER_THREAD;

    ITrackSequencePtr s = sequence(sequenceId);
    if (!s) {
        return RetVal2<TrackId, AudioParams>::make_ret((int)Err::InvalidSequenceId, "invalid sequence id");
    }

    return s->addTrack(trackName, playbackData, params);
}

RetVal2<TrackId, AudioOutputParams> WorkerPlayback::addAuxTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                                                const AudioOutputParams& outputParams)
{
    ONLY_AUDIO_WORKER_THREAD;

    ITrackSequencePtr s = sequence(sequenceId);
    if (!s) {
        return RetVal2<TrackId, AudioOutputParams>::make_ret((int)Err::InvalidSequenceId, "invalid sequence id");
    }

    return s->addAuxTrack(trackName, outputParams);
}

void WorkerPlayback::removeTrack(const TrackSequenceId sequenceId, const TrackId trackId)
{
    ONLY_AUDIO_WORKER_THREAD;

    ITrackSequencePtr s = sequence(sequenceId);

    if (!s) {
        return;
    }

    s->removeTrack(trackId);
}

void WorkerPlayback::removeAllTracks(const TrackSequenceId sequenceId)
{
    ONLY_AUDIO_WORKER_THREAD;

    ITrackSequencePtr s = sequence(sequenceId);

    if (!s) {
        return;
    }

    for (const TrackId& id : s->trackIdList()) {
        s->removeTrack(id);
    }
}

async::Channel<TrackSequenceId, TrackId> WorkerPlayback::trackAdded() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_trackAdded;
}

async::Channel<TrackSequenceId, TrackId> WorkerPlayback::trackRemoved() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_trackRemoved;
}

AudioResourceMetaList WorkerPlayback::availableInputResources() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return synthResolver()->resolveAvailableResources();
}

SoundPresetList WorkerPlayback::availableSoundPresets(const AudioResourceMeta& resourceMeta) const
{
    ONLY_AUDIO_WORKER_THREAD;
    return synthResolver()->resolveAvailableSoundPresets(resourceMeta);
}

RetVal<AudioInputParams> WorkerPlayback::inputParams(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    ONLY_AUDIO_WORKER_THREAD;

    ITrackSequencePtr s = sequence(sequenceId);
    if (!s) {
        return RetVal<AudioInputParams>::make_ret((int)Err::InvalidSequenceId, "invalid sequence id");
    }

    return s->audioIO()->inputParams(trackId);
}

void WorkerPlayback::setInputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioInputParams& params)
{
    ONLY_AUDIO_WORKER_THREAD;

    ITrackSequencePtr s = sequence(sequenceId);

    if (s) {
        s->audioIO()->setInputParams(trackId, params);
    }
}

async::Channel<TrackSequenceId, TrackId, AudioInputParams> WorkerPlayback::inputParamsChanged() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_inputParamsChanged;
}

RetVal<InputProcessingProgress> WorkerPlayback::inputProcessingProgress(const TrackSequenceId sequenceId,
                                                                        const TrackId trackId) const
{
    ONLY_AUDIO_WORKER_THREAD;

    const ITrackSequencePtr s = sequence(sequenceId);
    if (!s) {
        return RetVal<InputProcessingProgress>::make_ret((int)Err::InvalidSequenceId, "invalid sequence id");
    }

    if (!s->audioIO()->hasTrack(trackId)) {
        return RetVal<InputProcessingProgress>::make_ret((int)Err::InvalidTrackId, "no track");
    }

    return RetVal<InputProcessingProgress>::make_ok(s->audioIO()->inputProcessingProgress(trackId));
}

void WorkerPlayback::clearSources()
{
    ONLY_AUDIO_WORKER_THREAD;
    synthResolver()->clearSources();
}

// 3. Play Sequence
IPlayerPtr WorkerPlayback::player(const TrackSequenceId id) const
{
    std::shared_ptr<Player> p = std::make_shared<Player>(this, id);
    p->init();
    return p;
}

// 4. Adjust a Sequence output
RetVal<AudioOutputParams> WorkerPlayback::outputParams(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    ONLY_AUDIO_WORKER_THREAD;

    const ITrackSequencePtr s = sequence(sequenceId);
    if (!s) {
        return RetVal<AudioOutputParams>::make_ret((int)Err::InvalidSequenceId, "invalid sequence id");
    }

    return s->audioIO()->outputParams(trackId);
}

void WorkerPlayback::setOutputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioOutputParams& params)
{
    ONLY_AUDIO_WORKER_THREAD;

    ITrackSequencePtr s = sequence(sequenceId);
    if (s) {
        s->audioIO()->setOutputParams(trackId, params);
    }
}

async::Channel<TrackSequenceId, TrackId, AudioOutputParams> WorkerPlayback::outputParamsChanged() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_outputParamsChanged;
}

std::shared_ptr<Mixer> WorkerPlayback::mixer() const
{
    return audioEngine()->mixer();
}

RetVal<AudioOutputParams> WorkerPlayback::masterOutputParams() const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(mixer()) {
        return RetVal<AudioOutputParams>::make_ret((int)Err::Undefined, "undefined reference to a mixer");
    }

    return RetVal<AudioOutputParams>::make_ok(mixer()->masterOutputParams());
}

void WorkerPlayback::setMasterOutputParams(const AudioOutputParams& params)
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(mixer()) {
        return;
    }

    mixer()->setMasterOutputParams(params);
}

void WorkerPlayback::clearMasterOutputParams()
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(mixer()) {
        return;
    }

    mixer()->clearMasterOutputParams();
}

async::Channel<AudioOutputParams> WorkerPlayback::masterOutputParamsChanged() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_masterOutputParamsChanged;
}

AudioResourceMetaList WorkerPlayback::availableOutputResources() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return fxResolver()->resolveAvailableResources();
}

RetVal<AudioSignalChanges> WorkerPlayback::signalChanges(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    ONLY_AUDIO_WORKER_THREAD;

    const ITrackSequencePtr s = sequence(sequenceId);
    if (!s) {
        return RetVal<AudioSignalChanges>::make_ret((int)Err::InvalidSequenceId, "invalid sequence id");
    }

    if (!s->audioIO()->hasTrack(trackId)) {
        return RetVal<AudioSignalChanges>::make_ret((int)Err::InvalidTrackId, "no track");
    }

    return RetVal<AudioSignalChanges>::make_ok(s->audioIO()->audioSignalChanges(trackId));
}

RetVal<AudioSignalChanges> WorkerPlayback::masterSignalChanges() const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(mixer()) {
        return RetVal<AudioSignalChanges>::make_ret((int)Err::Undefined, "undefined reference to a mixer");
    }

    return RetVal<AudioSignalChanges>::make_ok(mixer()->masterAudioSignalChanges());
}

Ret WorkerPlayback::saveSoundTrack(const TrackSequenceId sequenceId, const io::path_t& destination, const SoundTrackFormat& format)
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(mixer()) {
        return make_ret(Err::Undefined, "undefined reference to a mixer");
    }

    ITrackSequencePtr s = sequence(sequenceId);
    if (!s) {
        return make_ret(Err::InvalidSequenceId, "invalid sequence id");
    }

#ifdef MUSE_MODULE_AUDIO_EXPORT
    s->player()->stop();
    s->player()->seek(0);
    msecs_t totalDuration = s->player()->duration();

    SoundTrackWriterPtr writer = std::make_shared<SoundTrackWriter>(destination, format, totalDuration, mixer(), iocContext());
    m_saveSoundTracksWritersMap[sequenceId] = writer;

    Progress progress = saveSoundTrackProgress(sequenceId);
    writer->progress().progressChanged().onReceive(this, [&progress](int64_t current, int64_t total, std::string title) {
        progress.progress(current, total, title);
    });

    Ret ret = writer->write();
    s->player()->seek(0);

    m_saveSoundTracksWritersMap.erase(sequenceId);

    return ret;
#else
    return make_ret(Err::DisabledAudioExport, "audio export is disabled");
#endif
}

void WorkerPlayback::abortSavingAllSoundTracks()
{
#ifdef MUSE_MODULE_AUDIO_EXPORT
    for (auto& writer : m_saveSoundTracksWritersMap) {
        writer.second->abort();
    }
#endif
}

Progress WorkerPlayback::saveSoundTrackProgress(const TrackSequenceId sequenceId)
{
    //! FIXME
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;
    if (!muse::contains(m_saveSoundTracksProgressMap, sequenceId)) {
        m_saveSoundTracksProgressMap.emplace(sequenceId, Progress());
    }

    return m_saveSoundTracksProgressMap[sequenceId];
}

void WorkerPlayback::clearAllFx()
{
    ONLY_AUDIO_WORKER_THREAD;
    fxResolver()->clearAllFx();
}
