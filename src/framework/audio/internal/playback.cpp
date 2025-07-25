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

#include "muse_framework_config.h"

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
    ONLY_AUDIO_MAIN_THREAD;
    return make_promise<TrackName>([this, sequenceId, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(Method::GetTrackName, RpcPacker::pack(sequenceId, trackId));
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal<TrackName> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                return;
            }
            if (ret.ret) {
                (void)resolve(ret.val);
            } else {
                (void)reject(ret.ret.code(), ret.ret.text());
            }
        });
        return Promise<TrackName>::dummy_result();
    }, PromiseType::AsyncByBody);
}

async::Promise<TrackId, AudioParams> Playback::addTrack(const TrackSequenceId sequenceId, const TrackName& trackName,
                                                        io::IODevice* playbackData, AudioParams&& params)
{
#ifdef MUE_CONFIGURATION_IS_APPWEB
    NOT_SUPPORTED;
    return make_promise<TrackId, AudioParams>([](auto /*resolve*/, auto reject) {
        Ret ret = muse::make_ret(Ret::Code::NotSupported);
        return reject(ret.code(), ret.text());
    });
#else
    ONLY_AUDIO_MAIN_THREAD;
    return make_promise<TrackId, AudioParams>([this, sequenceId, trackName, playbackData, params](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;

        ByteArray data = RpcPacker::pack(sequenceId, trackName, reinterpret_cast<uint64_t>(playbackData), params);

        Msg msg = rpc::make_request(Method::AddTrackWithIODevice, data);
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal2<TrackId, AudioParams> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                return;
            }
            if (ret.ret) {
                (void)resolve(ret.val1, ret.val2);
            } else {
                (void)reject(ret.ret.code(), ret.ret.text());
            }
        });
        return Promise<TrackId, AudioParams>::dummy_result();
    }, PromiseType::AsyncByBody);

#endif // MUE_CONFIGURATION_IS_APPWEB
}

async::Promise<TrackId, AudioParams> Playback::addTrack(const TrackSequenceId sequenceId, const TrackName& trackName,
                                                        const mpe::PlaybackData& playbackData, AudioParams&& params)
{
    ONLY_AUDIO_MAIN_THREAD;
    return make_promise<TrackId, AudioParams>([this, sequenceId, trackName, playbackData, params](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;

        rpc::StreamId mainStreamId = channel()->addSendStream(playbackData.mainStream);
        rpc::StreamId offStreamId = channel()->addSendStream(playbackData.offStream);

        ByteArray data = RpcPacker::pack(sequenceId, trackName, playbackData, params, mainStreamId, offStreamId);

        Msg msg = rpc::make_request(Method::AddTrackWithPlaybackData, data);
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal2<TrackId, AudioParams> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                return;
            }
            if (ret.ret) {
                (void)resolve(ret.val1, ret.val2);
            } else {
                (void)reject(ret.ret.code(), ret.ret.text());
            }
        });
        return Promise<TrackId, AudioParams>::dummy_result();
    }, PromiseType::AsyncByBody);
}

async::Promise<TrackId, AudioOutputParams> Playback::addAuxTrack(const TrackSequenceId sequenceId, const TrackName& trackName,
                                                                 const AudioOutputParams& outputParams)
{
    ONLY_AUDIO_MAIN_THREAD;
    return make_promise<TrackId, AudioOutputParams>([this, sequenceId, trackName, outputParams](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(Method::AddAuxTrack, RpcPacker::pack(sequenceId, trackName, outputParams));
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal2<TrackId, AudioOutputParams> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                return;
            }
            if (ret.ret) {
                (void)resolve(ret.val1, ret.val2);
            } else {
                (void)reject(ret.ret.code(), ret.ret.text());
            }
        });
        return Promise<TrackId, AudioOutputParams>::dummy_result();
    }, PromiseType::AsyncByBody);
}

void Playback::removeTrack(const TrackSequenceId sequenceId, const TrackId trackId)
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(Method::RemoveTrack, RpcPacker::pack(sequenceId, trackId));
    channel()->send(msg);
}

void Playback::removeAllTracks(const TrackSequenceId sequenceId)
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(Method::RemoveAllTracks, RpcPacker::pack(sequenceId));
    channel()->send(msg);
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
    ONLY_AUDIO_MAIN_THREAD;
    return make_promise<AudioResourceMetaList>([this](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(Method::GetAvailableInputResources);
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            AudioResourceMetaList list;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, list)) {
                return;
            }
            (void)resolve(list);
        });
        return Promise<AudioResourceMetaList>::dummy_result();
    }, PromiseType::AsyncByBody);
}

async::Promise<SoundPresetList> Playback::availableSoundPresets(const AudioResourceMeta& resourceMeta) const
{
    ONLY_AUDIO_MAIN_THREAD;
    return make_promise<SoundPresetList>([this, resourceMeta](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(Method::GetAvailableSoundPresets, RpcPacker::pack(resourceMeta));
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            SoundPresetList list;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, list)) {
                return;
            }

            (void)resolve(list);
        });
        return Promise<SoundPresetList>::dummy_result();
    }, PromiseType::AsyncByBody);
}

async::Promise<AudioInputParams> Playback::inputParams(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    ONLY_AUDIO_MAIN_THREAD;
    return make_promise<AudioInputParams>([this, sequenceId, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(Method::GetInputParams, RpcPacker::pack(sequenceId, trackId));
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal<AudioInputParams> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                return;
            }

            if (ret.ret) {
                (void)resolve(ret.val);
            } else {
                (void)reject(ret.ret.code(), ret.ret.text());
            }
        });
        return Promise<AudioInputParams>::dummy_result();
    }, PromiseType::AsyncByBody);
}

void Playback::setInputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioInputParams& params)
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(Method::SetInputParams, RpcPacker::pack(sequenceId, trackId, params));
    channel()->send(msg);
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
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(Method::ClearSources);
    channel()->send(msg);
}

// 4. Adjust a Sequence output

async::Promise<AudioOutputParams> Playback::outputParams(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    ONLY_AUDIO_MAIN_THREAD;
    return make_promise<AudioOutputParams>([this, sequenceId, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(Method::GetOutputParams, RpcPacker::pack(sequenceId, trackId));
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal<AudioOutputParams> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                return;
            }

            if (ret.ret) {
                (void)resolve(ret.val);
            } else {
                (void)reject(ret.ret.code(), ret.ret.text());
            }
        });
        return Promise<AudioOutputParams>::dummy_result();
    }, PromiseType::AsyncByBody);
}

void Playback::setOutputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioOutputParams& params)
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(Method::SetOutputParams, RpcPacker::pack(sequenceId, trackId, params));
    channel()->send(msg);
}

async::Channel<TrackSequenceId, TrackId, AudioOutputParams> Playback::outputParamsChanged() const
{
    return m_outputParamsChanged;
}

async::Promise<AudioOutputParams> Playback::masterOutputParams() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return make_promise<AudioOutputParams>([this](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(Method::GetMasterOutputParams);
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal<AudioOutputParams> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                return;
            }

            if (ret.ret) {
                (void)resolve(ret.val);
            } else {
                (void)reject(ret.ret.code(), ret.ret.text());
            }
        });
        return Promise<AudioOutputParams>::dummy_result();
    }, PromiseType::AsyncByBody);
}

void Playback::setMasterOutputParams(const AudioOutputParams& params)
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(Method::SetMasterOutputParams, RpcPacker::pack(params));
    channel()->send(msg);
}

void Playback::clearMasterOutputParams()
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(Method::ClearMasterOutputParams);
    channel()->send(msg);
}

async::Channel<AudioOutputParams> Playback::masterOutputParamsChanged() const
{
    return m_masterOutputParamsChanged;
}

async::Promise<AudioResourceMetaList> Playback::availableOutputResources() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return make_promise<AudioResourceMetaList>([this](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(Method::GetAvailableOutputResources);
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            AudioResourceMetaList list;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, list)) {
                return;
            }
            (void)resolve(list);
        });
        return Promise<AudioResourceMetaList>::dummy_result();
    }, PromiseType::AsyncByBody);
}

async::Promise<AudioSignalChanges> Playback::signalChanges(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    ONLY_AUDIO_MAIN_THREAD;
    return make_promise<AudioSignalChanges>([this, sequenceId, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(Method::GetSignalChanges, RpcPacker::pack(sequenceId, trackId));
        channel()->send(msg, [this, resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal<StreamId> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                return;
            }

            if (ret.ret) {
                AudioSignalChanges ch;
                channel()->addReceiveStream(ret.val, ch);
                (void)resolve(ch);
            } else {
                (void)reject(ret.ret.code(), ret.ret.text());
            }
        });
        return Promise<AudioSignalChanges>::dummy_result();
    }, PromiseType::AsyncByBody);
}

async::Promise<AudioSignalChanges> Playback::masterSignalChanges() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return make_promise<AudioSignalChanges>([this](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(Method::GetMasterSignalChanges);
        channel()->send(msg, [this, resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal<StreamId> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                return;
            }

            if (ret.ret) {
                AudioSignalChanges ch;
                channel()->addReceiveStream(ret.val, ch);
                (void)resolve(ch);
            } else {
                (void)reject(ret.ret.code(), ret.ret.text());
            }
        });
        return Promise<AudioSignalChanges>::dummy_result();
    }, PromiseType::AsyncByBody);
}

async::Promise<bool> Playback::saveSoundTrack(const TrackSequenceId sequenceId, const io::path_t& destination,
                                              const SoundTrackFormat& format)
{
    ONLY_AUDIO_MAIN_THREAD;
    return make_promise<bool>([this, sequenceId, destination, format](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(Method::SaveSoundTrack, RpcPacker::pack(sequenceId, destination, format));
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            Ret ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                return;
            }

            if (ret) {
                (void)resolve(true);
            } else {
                (void)reject(ret.code(), ret.text());
            }
        });
        return Promise<bool>::dummy_result();
    }, PromiseType::AsyncByBody);
}

void Playback::abortSavingAllSoundTracks()
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(Method::AbortSavingAllSoundTracks);
    channel()->send(msg);
}

async::Channel<int64_t, int64_t> Playback::saveSoundTrackProgressChanged(const TrackSequenceId sequenceId) const
{
    //! FIXME
    return workerPlayback()->saveSoundTrackProgressChanged(sequenceId);
}

void Playback::clearAllFx()
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(Method::ClearAllFx);
    channel()->send(msg);
}
