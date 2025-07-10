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
#include "playback.h"

#include <utility>

#include "global/async/async.h"

#include "internal/audiothread.h"
#include "internal/audiosanitizer.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::async;

void Playback::init()
{
    Async::call(this, [this]() {
        ONLY_AUDIO_WORKER_THREAD;

        workerPlayback()->trackAdded().onReceive(this, [this](TrackSequenceId sequenceId, TrackId trackId) {
            m_trackAdded.send(sequenceId, trackId);
        });

        workerPlayback()->trackRemoved().onReceive(this, [this](TrackSequenceId sequenceId, TrackId trackId) {
            m_trackRemoved.send(sequenceId, trackId);
        });

        workerPlayback()->inputParamsChanged().onReceive(this, [this](TrackSequenceId sequenceId, TrackId trackId,
                                                                      const AudioInputParams& params) {
            m_inputParamsChanged.send(sequenceId, trackId, params);
        });
    }, AudioThread::ID);
}

void Playback::deinit()
{
    Async::call(this, [this]() {
        ONLY_AUDIO_WORKER_THREAD;
        workerPlayback()->trackAdded().resetOnReceive(this);
        workerPlayback()->trackRemoved().resetOnReceive(this);
        workerPlayback()->inputParamsChanged().resetOnReceive(this);
    }, AudioThread::ID);
}

Promise<TrackSequenceId> Playback::addSequence()
{
    return Promise<TrackSequenceId>([this](auto resolve, auto /*reject*/) {
        ONLY_AUDIO_WORKER_THREAD;

        TrackSequenceId newId = workerPlayback()->addSequence();
        m_sequenceAdded.send(newId);

        return resolve(newId);
    }, AudioThread::ID);
}

Promise<TrackSequenceIdList> Playback::sequenceIdList() const
{
    return Promise<TrackSequenceIdList>([this](auto resolve, auto /*reject*/) {
        ONLY_AUDIO_WORKER_THREAD;

        TrackSequenceIdList result = workerPlayback()->sequenceIdList();

        return resolve(std::move(result));
    }, AudioThread::ID);
}

void Playback::removeSequence(const TrackSequenceId id)
{
    Async::call(this, [this, id]() {
        ONLY_AUDIO_WORKER_THREAD;

        workerPlayback()->removeSequence(id);

        m_sequenceRemoved.send(id);
    }, AudioThread::ID);
}

Channel<TrackSequenceId> Playback::sequenceAdded() const
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;

    return m_sequenceAdded;
}

Channel<TrackSequenceId> Playback::sequenceRemoved() const
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;

    return m_sequenceRemoved;
}

IPlayerPtr Playback::player(const TrackSequenceId id) const
{
    return workerPlayback()->player(id);
}

// 2. Setup tracks for Sequence
async::Promise<TrackIdList> Playback::trackIdList(const TrackSequenceId sequenceId) const
{
    return Promise<TrackIdList>([this, sequenceId](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        RetVal<TrackIdList> ret = workerPlayback()->trackIdList(sequenceId);
        if (ret.ret) {
            return resolve(ret.val);
        } else {
            return reject(ret.ret.code(), ret.ret.text());
        }
    }, AudioThread::ID);
}

async::Promise<TrackName> Playback::trackName(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    return Promise<TrackName>([this, sequenceId, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        RetVal<TrackName> ret = workerPlayback()->trackName(sequenceId, trackId);
        if (ret.ret) {
            return resolve(ret.val);
        } else {
            return reject(ret.ret.code(), ret.ret.text());
        }
    }, AudioThread::ID);
}

async::Promise<TrackId, AudioParams> Playback::addTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                                        io::IODevice* playbackData, AudioParams&& params)
{
    return Promise<TrackId, AudioParams>([this, sequenceId, trackName, playbackData, params](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        RetVal2<TrackId, AudioParams> ret = workerPlayback()->addTrack(sequenceId, trackName, playbackData, params);
        if (ret.ret) {
            return resolve(ret.val1, ret.val2);
        } else {
            return reject(ret.ret.code(), ret.ret.text());
        }
    }, AudioThread::ID);
}

async::Promise<TrackId, AudioParams> Playback::addTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                                        const mpe::PlaybackData& playbackData, AudioParams&& params)
{
    return Promise<TrackId, AudioParams>([this, sequenceId, trackName, playbackData, params](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        RetVal2<TrackId, AudioParams> ret = workerPlayback()->addTrack(sequenceId, trackName, playbackData, params);
        if (ret.ret) {
            return resolve(ret.val1, ret.val2);
        } else {
            return reject(ret.ret.code(), ret.ret.text());
        }
    }, AudioThread::ID);
}

async::Promise<TrackId, AudioOutputParams> Playback::addAuxTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                                                 const AudioOutputParams& outputParams)
{
    return Promise<TrackId, AudioOutputParams>([this, sequenceId, trackName, outputParams](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        RetVal2<TrackId, AudioOutputParams> ret = workerPlayback()->addAuxTrack(sequenceId, trackName, outputParams);
        if (ret.ret) {
            return resolve(ret.val1, ret.val2);
        } else {
            return reject(ret.ret.code(), ret.ret.text());
        }
    }, AudioThread::ID);
}

void Playback::removeTrack(const TrackSequenceId sequenceId, const TrackId trackId)
{
    Async::call(this, [this, sequenceId, trackId]() {
        ONLY_AUDIO_WORKER_THREAD;
        workerPlayback()->removeTrack(sequenceId, trackId);
    }, AudioThread::ID);
}

void Playback::removeAllTracks(const TrackSequenceId sequenceId)
{
    Async::call(this, [this, sequenceId]() {
        ONLY_AUDIO_WORKER_THREAD;
        workerPlayback()->removeAllTracks(sequenceId);
    }, AudioThread::ID);
}

async::Channel<TrackSequenceId, TrackId> Playback::trackAdded() const
{
    return m_trackAdded;
}

async::Channel<TrackSequenceId, TrackId> Playback::trackRemoved() const
{
    return m_trackRemoved;
}

async::Promise<AudioResourceMetaList> Playback::availableInputResources() const
{
    return Promise<AudioResourceMetaList>([this](auto resolve, auto /*reject*/) {
        ONLY_AUDIO_WORKER_THREAD;
        AudioResourceMetaList res = workerPlayback()->availableInputResources();
        return resolve(res);
    }, AudioThread::ID);
}

async::Promise<SoundPresetList> Playback::availableSoundPresets(const AudioResourceMeta& resourceMeta) const
{
    return Promise<SoundPresetList>([this, resourceMeta](auto resolve, auto /*reject*/) {
        ONLY_AUDIO_WORKER_THREAD;
        SoundPresetList res = workerPlayback()->availableSoundPresets(resourceMeta);
        return resolve(res);
    }, AudioThread::ID);
}

async::Promise<AudioInputParams> Playback::inputParams(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    return Promise<AudioInputParams>([this, sequenceId, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        RetVal<AudioInputParams> ret = workerPlayback()->inputParams(sequenceId, trackId);
        if (ret.ret) {
            return resolve(ret.val);
        } else {
            return reject(ret.ret.code(), ret.ret.text());
        }
    }, AudioThread::ID);
}

void Playback::setInputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioInputParams& params)
{
    Async::call(this, [this, sequenceId, trackId, params]() {
        ONLY_AUDIO_WORKER_THREAD;
        workerPlayback()->setInputParams(sequenceId, trackId, params);
    }, AudioThread::ID);
}

async::Channel<TrackSequenceId, TrackId, AudioInputParams> Playback::inputParamsChanged() const
{
    return m_inputParamsChanged;
}

muse::async::Promise<InputProcessingProgress> Playback::inputProcessingProgress(const TrackSequenceId sequenceId,
                                                                                const TrackId trackId) const
{
    return Promise<InputProcessingProgress>([this, sequenceId, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        RetVal<InputProcessingProgress> ret = workerPlayback()->inputProcessingProgress(sequenceId, trackId);
        if (ret.ret) {
            return resolve(ret.val);
        } else {
            return reject(ret.ret.code(), ret.ret.text());
        }
    }, AudioThread::ID);
}

void Playback::clearSources()
{
    Async::call(this, [this]() {
        ONLY_AUDIO_WORKER_THREAD;
        workerPlayback()->clearSources();
    }, AudioThread::ID);
}

// 4. Adjust a Sequence output

async::Promise<AudioOutputParams> Playback::outputParams(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    return workerPlayback()->audioOutput()->outputParams(sequenceId, trackId);
}

void Playback::setOutputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioOutputParams& params)
{
    workerPlayback()->audioOutput()->setOutputParams(sequenceId, trackId, params);
}

async::Channel<TrackSequenceId, TrackId, AudioOutputParams> Playback::outputParamsChanged() const
{
    return workerPlayback()->audioOutput()->outputParamsChanged();
}

async::Promise<AudioOutputParams> Playback::masterOutputParams() const
{
    return workerPlayback()->audioOutput()->masterOutputParams();
}

void Playback::setMasterOutputParams(const AudioOutputParams& params)
{
    workerPlayback()->audioOutput()->setMasterOutputParams(params);
}

void Playback::clearMasterOutputParams()
{
    workerPlayback()->audioOutput()->clearMasterOutputParams();
}

async::Channel<AudioOutputParams> Playback::masterOutputParamsChanged() const
{
    return workerPlayback()->audioOutput()->masterOutputParamsChanged();
}

async::Promise<AudioResourceMetaList> Playback::availableOutputResources() const
{
    return workerPlayback()->audioOutput()->availableOutputResources();
}

async::Promise<AudioSignalChanges> Playback::signalChanges(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    return workerPlayback()->audioOutput()->signalChanges(sequenceId, trackId);
}

async::Promise<AudioSignalChanges> Playback::masterSignalChanges() const
{
    return workerPlayback()->audioOutput()->masterSignalChanges();
}

async::Promise<bool> Playback::saveSoundTrack(const TrackSequenceId sequenceId, const io::path_t& destination,
                                              const SoundTrackFormat& format)
{
    return workerPlayback()->audioOutput()->saveSoundTrack(sequenceId, destination, format);
}

void Playback::abortSavingAllSoundTracks()
{
    workerPlayback()->audioOutput()->abortSavingAllSoundTracks();
}

Progress Playback::saveSoundTrackProgress(const TrackSequenceId sequenceId)
{
    return workerPlayback()->audioOutput()->saveSoundTrackProgress(sequenceId);
}

void Playback::clearAllFx()
{
    workerPlayback()->audioOutput()->clearAllFx();
}
