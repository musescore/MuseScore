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

#include "rpc/rpcpacker.h"
#include "audiothread.h"
#include "audiosanitizer.h"
#include "player.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::rpc;
using namespace muse::async;

void Playback::initOnWorker()
{
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

    workerPlayback()->outputParamsChanged().onReceive(this, [this](TrackSequenceId sequenceId, TrackId trackId,
                                                                   const AudioOutputParams& params) {
        m_outputParamsChanged.send(sequenceId, trackId, params);
    });

    workerPlayback()->masterOutputParamsChanged().onReceive(this, [this](const AudioOutputParams& params) {
        m_masterOutputParamsChanged.send(params);
    });
}

void Playback::deinitOnWorker()
{
    ONLY_AUDIO_WORKER_THREAD;

    workerPlayback()->trackAdded().resetOnReceive(this);
    workerPlayback()->trackRemoved().resetOnReceive(this);
    workerPlayback()->inputParamsChanged().resetOnReceive(this);
    workerPlayback()->outputParamsChanged().resetOnReceive(this);
}

Promise<TrackSequenceId> Playback::addSequence()
{
    ONLY_AUDIO_MAIN_THREAD;
    return make_promise<TrackSequenceId>([this](auto resolve, auto /*reject*/) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(Method::AddSequence);
        channel()->send(msg, [this, resolve](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            TrackSequenceId seqId;
            RpcPacker::unpack(res.data, seqId);
            m_sequenceAdded.send(seqId);
            (void)resolve(seqId);
        });

        return Promise<TrackSequenceId>::dummy_result();
    }, PromiseType::AsyncByBody);
}

Promise<TrackSequenceIdList> Playback::sequenceIdList() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return make_promise<TrackSequenceIdList>([this](auto resolve, auto /*reject*/) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(Method::GetSequenceIdList);
        channel()->send(msg, [resolve](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            TrackSequenceIdList list;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, list)) {
                return;
            }
            (void)resolve(list);
        });
        return Promise<TrackSequenceIdList>::dummy_result();
    }, PromiseType::AsyncByBody);
}

void Playback::removeSequence(const TrackSequenceId id)
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(Method::RemoveSequence, RpcPacker::pack(id));
    channel()->send(msg, [this, id](const Msg& res) {
        ONLY_AUDIO_MAIN_THREAD;
        bool ok = false;
        IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ok)) {
            return;
        }
        if (ok) {
            m_sequenceRemoved.send(id);
        }
    });
}

Channel<TrackSequenceId> Playback::sequenceAdded() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return m_sequenceAdded;
}

Channel<TrackSequenceId> Playback::sequenceRemoved() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return m_sequenceRemoved;
}

IPlayerPtr Playback::player(const TrackSequenceId id) const
{
    std::shared_ptr<Player> p = std::make_shared<Player>(id);
    p->init();
    return p;
}

// 2. Setup tracks for Sequence
async::Promise<TrackIdList> Playback::trackIdList(const TrackSequenceId sequenceId) const
{
    ONLY_AUDIO_MAIN_THREAD;
    return make_promise<TrackIdList>([this, sequenceId](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(Method::GetTrackIdList, RpcPacker::pack(sequenceId));
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal<TrackIdList> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                return;
            }
            if (ret.ret) {
                (void)resolve(ret.val);
            } else {
                (void)reject(ret.ret.code(), ret.ret.text());
            }
        });
        return Promise<TrackSequenceIdList>::dummy_result();
    }, PromiseType::AsyncByBody);
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
    return Promise<AudioOutputParams>([this, sequenceId, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        RetVal<AudioOutputParams> ret = workerPlayback()->outputParams(sequenceId, trackId);
        if (ret.ret) {
            return resolve(ret.val);
        } else {
            return reject(ret.ret.code(), ret.ret.text());
        }
    }, AudioThread::ID);
}

void Playback::setOutputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioOutputParams& params)
{
    Async::call(this, [this, sequenceId, trackId, params]() {
        ONLY_AUDIO_WORKER_THREAD;
        workerPlayback()->setOutputParams(sequenceId, trackId, params);
    }, AudioThread::ID);
}

async::Channel<TrackSequenceId, TrackId, AudioOutputParams> Playback::outputParamsChanged() const
{
    return m_outputParamsChanged;
}

async::Promise<AudioOutputParams> Playback::masterOutputParams() const
{
    return Promise<AudioOutputParams>([this](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        RetVal<AudioOutputParams> ret = workerPlayback()->masterOutputParams();
        if (ret.ret) {
            return resolve(ret.val);
        } else {
            return reject(ret.ret.code(), ret.ret.text());
        }
    }, AudioThread::ID);
}

void Playback::setMasterOutputParams(const AudioOutputParams& params)
{
    Async::call(this, [this, params]() {
        ONLY_AUDIO_WORKER_THREAD;
        workerPlayback()->setMasterOutputParams(params);
    }, AudioThread::ID);
}

void Playback::clearMasterOutputParams()
{
    Async::call(this, [this]() {
        ONLY_AUDIO_WORKER_THREAD;
        workerPlayback()->clearMasterOutputParams();
    }, AudioThread::ID);
}

async::Channel<AudioOutputParams> Playback::masterOutputParamsChanged() const
{
    return m_masterOutputParamsChanged;
}

async::Promise<AudioResourceMetaList> Playback::availableOutputResources() const
{
    return Promise<AudioResourceMetaList>([this](auto resolve, auto /*reject*/) {
        ONLY_AUDIO_WORKER_THREAD;

        AudioResourceMetaList res = workerPlayback()->availableOutputResources();
        return resolve(res);
    }, AudioThread::ID);
}

async::Promise<AudioSignalChanges> Playback::signalChanges(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    return Promise<AudioSignalChanges>([this, sequenceId, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        RetVal<AudioSignalChanges> ret = workerPlayback()->signalChanges(sequenceId, trackId);
        if (ret.ret) {
            return resolve(ret.val);
        } else {
            return reject(ret.ret.code(), ret.ret.text());
        }
    }, AudioThread::ID);
}

async::Promise<AudioSignalChanges> Playback::masterSignalChanges() const
{
    return Promise<AudioSignalChanges>([this](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        RetVal<AudioSignalChanges> ret = workerPlayback()->masterSignalChanges();
        if (ret.ret) {
            return resolve(ret.val);
        } else {
            return reject(ret.ret.code(), ret.ret.text());
        }
    }, AudioThread::ID);
}

async::Promise<bool> Playback::saveSoundTrack(const TrackSequenceId sequenceId, const io::path_t& destination,
                                              const SoundTrackFormat& format)
{
    return Promise<bool>([this, sequenceId, destination, format](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        Ret ret = workerPlayback()->saveSoundTrack(sequenceId, destination, format);
        if (ret) {
            return resolve(true);
        } else {
            return reject(ret.code(), ret.text());
        }
    }, AudioThread::ID);
}

void Playback::abortSavingAllSoundTracks()
{
    Async::call(this, [this]() {
        ONLY_AUDIO_WORKER_THREAD;
        workerPlayback()->abortSavingAllSoundTracks();
    }, AudioThread::ID);
}

async::Channel<int64_t, int64_t> Playback::saveSoundTrackProgressChanged(const TrackSequenceId sequenceId) const
{
    //! FIXME
    return workerPlayback()->saveSoundTrackProgressChanged(sequenceId);
}

void Playback::clearAllFx()
{
    Async::call(this, [this]() {
        ONLY_AUDIO_WORKER_THREAD;
        workerPlayback()->clearAllFx();
    }, AudioThread::ID);
}
