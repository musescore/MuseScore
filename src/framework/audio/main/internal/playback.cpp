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

#include "audio/common/rpc/rpcpacker.h"
#include "audio/common/audiosanitizer.h"
#include "player.h"

#include "muse_framework_config.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::rpc;
using namespace muse::async;

void Playback::init()
{
    ONLY_AUDIO_MAIN_THREAD;

    channel()->onMethod(Method::TrackAdded, [this](const Msg& msg) {
        ONLY_AUDIO_MAIN_THREAD;
        TrackSequenceId seqId = 0;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackId)) {
            return;
        }
        m_trackAdded.send(seqId, trackId);
    });

    channel()->onMethod(Method::TrackRemoved, [this](const Msg& msg) {
        ONLY_AUDIO_MAIN_THREAD;
        TrackSequenceId seqId = 0;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackId)) {
            return;
        }
        m_trackRemoved.send(seqId, trackId);
    });

    channel()->onMethod(Method::InputParamsChanged, [this](const Msg& msg) {
        ONLY_AUDIO_MAIN_THREAD;
        TrackSequenceId seqId = 0;
        TrackId trackId = 0;
        AudioInputParams params;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackId, params)) {
            return;
        }
        m_inputParamsChanged.send(seqId, trackId, params);
    });

    channel()->onMethod(Method::OutputParamsChanged, [this](const Msg& msg) {
        ONLY_AUDIO_MAIN_THREAD;
        TrackSequenceId seqId = 0;
        TrackId trackId = 0;
        AudioOutputParams params;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackId, params)) {
            return;
        }
        m_outputParamsChanged.send(seqId, trackId, params);
    });

    channel()->onMethod(Method::MasterOutputParamsChanged, [this](const Msg& msg) {
        ONLY_AUDIO_MAIN_THREAD;
        AudioOutputParams params;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, params)) {
            return;
        }
        m_masterOutputParamsChanged.send(params);
    });

    m_saveSoundTrackProgressStream.onReceive(this, [this](TrackSequenceId seqId, int64_t current, int64_t total) {
        auto it = m_saveSoundTrackProgressChannels.find(seqId);
        if (it != m_saveSoundTrackProgressChannels.end()) {
            it->second.send(current, total);
        }
    });
}

void Playback::deinit()
{
    ONLY_AUDIO_MAIN_THREAD;

    channel()->onMethod(Method::TrackAdded, nullptr);
    channel()->onMethod(Method::TrackRemoved, nullptr);
    channel()->onMethod(Method::InputParamsChanged, nullptr);
    channel()->onMethod(Method::InputParamsChanged, nullptr);
    channel()->onMethod(Method::MasterOutputParamsChanged, nullptr);
}

Promise<TrackSequenceId> Playback::addSequence()
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<TrackSequenceId>([this](auto resolve, auto /*reject*/) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(Method::AddSequence);
        channel()->send(msg, [this, resolve](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            TrackSequenceId seqId = 0;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, seqId)) {
                return;
            }
            m_sequenceAdded.send(seqId);
            (void)resolve(seqId);
        });

        return Promise<TrackSequenceId>::dummy_result();
    }, PromiseType::AsyncByBody);
}

Promise<TrackSequenceIdList> Playback::sequenceIdList() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<TrackSequenceIdList>([this](auto resolve, auto /*reject*/) {
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

            // clear cache
            m_saveSoundTrackProgressChannels.erase(id);
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
    return async::make_promise<TrackIdList>([this, sequenceId](auto resolve, auto reject) {
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

async::Promise<RetVal<TrackName> > Playback::trackName(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<RetVal<TrackName> >([this, sequenceId, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(Method::GetTrackName, RpcPacker::pack(sequenceId, trackId));
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal<TrackName> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                return;
            }
            (void)resolve(ret);
        });
        return Promise<RetVal<TrackName> >::dummy_result();
    }, PromiseType::AsyncByBody);
}

async::Promise<TrackId, AudioParams> Playback::addTrack(const TrackSequenceId sequenceId, const TrackName& trackName,
                                                        io::IODevice* playbackData, AudioParams&& params)
{
#ifdef MUE_CONFIGURATION_IS_APPWEB
    NOT_SUPPORTED;
    return async::make_promise<TrackId, AudioParams>([](auto /*resolve*/, auto reject) {
        Ret ret = muse::make_ret(Ret::Code::NotSupported);
        return reject(ret.code(), ret.text());
    });
#else
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<TrackId, AudioParams>([this, sequenceId, trackName, playbackData, params](auto resolve, auto reject) {
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
    return async::make_promise<TrackId, AudioParams>([this, sequenceId, trackName, playbackData, params](auto resolve, auto reject) {
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
    return async::make_promise<TrackId, AudioOutputParams>([this, sequenceId, trackName, outputParams](auto resolve, auto reject) {
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
    return async::make_promise<AudioResourceMetaList>([this](auto resolve, auto reject) {
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
    return async::make_promise<SoundPresetList>([this, resourceMeta](auto resolve, auto reject) {
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
    return async::make_promise<AudioInputParams>([this, sequenceId, trackId](auto resolve, auto reject) {
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
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<InputProcessingProgress>([this, sequenceId, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(Method::GetInputProcessingProgress, RpcPacker::pack(sequenceId, trackId));
        channel()->send(msg, [this, resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            Ret ret;
            bool isStarted = false;
            StreamId streamId = 0;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret, isStarted, streamId)) {
                return;
            }

            if (ret) {
                InputProcessingProgress prog;
                prog.isStarted = isStarted;
                channel()->addReceiveStream(streamId, prog.processedChannel);
                (void)resolve(prog);
            } else {
                (void)reject(ret.code(), ret.text());
            }
        });
        return Promise<InputProcessingProgress>::dummy_result();
    }, PromiseType::AsyncByBody);
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
    return async::make_promise<AudioOutputParams>([this, sequenceId, trackId](auto resolve, auto reject) {
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
    return async::make_promise<AudioOutputParams>([this](auto resolve, auto reject) {
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
    return async::make_promise<AudioResourceMetaList>([this](auto resolve, auto reject) {
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
    return async::make_promise<AudioSignalChanges>([this, sequenceId, trackId](auto resolve, auto reject) {
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
    return async::make_promise<AudioSignalChanges>([this](auto resolve, auto reject) {
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
    return async::make_promise<bool>([this, sequenceId, destination, format](auto resolve, auto reject) {
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
    auto it = m_saveSoundTrackProgressChannels.find(sequenceId);
    if (it == m_saveSoundTrackProgressChannels.end()) {
        it = m_saveSoundTrackProgressChannels.insert({ sequenceId, async::Channel<int64_t, int64_t>() }).first;

        async::Channel<int64_t, int64_t> ch;

        Msg msg = rpc::make_request(Method::GetSaveSoundTrackProgress, RpcPacker::pack(sequenceId));
        channel()->send(msg, [this, ch](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            StreamId streamId = 0;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, streamId)) {
                return;
            }

            if (m_saveSoundTrackProgressStreamId == 0) {
                m_saveSoundTrackProgressStreamId = streamId;
                channel()->addReceiveStream(m_saveSoundTrackProgressStreamId, m_saveSoundTrackProgressStream);
            }

            assert(m_saveSoundTrackProgressStreamId == streamId);
        });
    }

    return it->second;
}

void Playback::clearAllFx()
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(Method::ClearAllFx);
    channel()->send(msg);
}
