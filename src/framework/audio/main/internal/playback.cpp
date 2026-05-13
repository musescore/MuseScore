/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "audio/common/audioerrors.h"
#include "player.h"

#include "muse_framework_config.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::rpc;
using namespace muse::async;

rpc::CtxId Playback::ctxId() const
{
    return rpc::ctxId(iocContext());
}

// Init
async::Promise<Ret> Playback::init()
{
    ONLY_AUDIO_MAIN_THREAD;

    channel()->onNotification(ctxId(), MsgCode::TrackAdded, [this](const Msg& msg) {
        ONLY_AUDIO_MAIN_THREAD;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, trackId)) {
            return;
        }
        m_trackAdded.send(trackId);
    });

    channel()->onNotification(ctxId(), MsgCode::TrackRemoved, [this](const Msg& msg) {
        ONLY_AUDIO_MAIN_THREAD;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, trackId)) {
            return;
        }
        m_trackRemoved.send(trackId);
    });

    channel()->onNotification(ctxId(), MsgCode::SourceParamsChanged, [this](const Msg& msg) {
        ONLY_AUDIO_MAIN_THREAD;
        TrackId trackId = 0;
        AudioSourceParams params;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, trackId, params)) {
            return;
        }
        m_sourceParamsChanged.send(trackId, params);
    });

    channel()->onNotification(ctxId(), MsgCode::FxChainParamsChanged, [this](const Msg& msg) {
        ONLY_AUDIO_MAIN_THREAD;
        TrackId trackId = 0;
        AudioFxChain params;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, trackId, params)) {
            return;
        }
        if (trackId == MASTER_TRACK_ID) {
            m_masterFxChainParamsChanged.send(params);
        } else {
            m_fxChainParamsChanged.send(trackId, params);
        }
    });

    return async::make_promise<Ret>([this](auto resolve, auto /*reject*/) {
        ONLY_AUDIO_MAIN_THREAD;

        auto initContext = [this, resolve]() {
            //! NOTE The message context here is global, and the context ID is the data in the message
            Msg msg = rpc::make_request(rpc::GLOBAL_CTX_ID, MsgCode::ContextInit, RpcPacker::pack(ctxId()));
            channel()->send(msg, [this, resolve](const Msg& res) {
                ONLY_AUDIO_MAIN_THREAD;
                Ret ret;
                IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                    ret = audio::make_ret(Err::InvalidRpcData);
                    (void)resolve(ret);
                    return;
                }
                m_inited.set(ret.success());
                (void)resolve(ret);
            });
        };

        LOGD() << "isAudioStarted: " << startAudioController()->isAudioStarted();
        if (startAudioController()->isAudioStarted()) {
            initContext();
        } else {
            startAudioController()->isAudioStartedChanged().onReceive(this, [this, initContext](bool arg) {
                LOGD() << "isAudioStartedChanged: " << arg;
                if (arg) {
                    initContext();
                } else {
                    LOGE() << "audio not started";
                }
                startAudioController()->isAudioStartedChanged().disconnect(this);
            });
        }

        return Promise<Ret>::dummy_result();
    }, PromiseType::AsyncByPromise);
}

void Playback::deinit()
{
    ONLY_AUDIO_MAIN_THREAD;

    channel()->onNotification(ctxId(), MsgCode::TrackAdded, nullptr);
    channel()->onNotification(ctxId(), MsgCode::TrackRemoved, nullptr);
    channel()->onNotification(ctxId(), MsgCode::SourceParamsChanged, nullptr);
    channel()->onNotification(ctxId(), MsgCode::FxChainParamsChanged, nullptr);

    m_saveSoundTrackProgressStream = SaveSoundTrackProgress();
    m_saveSoundTrackProgressStreamInited = false;
    m_saveSoundTrackProgressStreamId = 0;

    channel()->send(rpc::make_request(rpc::GLOBAL_CTX_ID, MsgCode::ContextDeinit, RpcPacker::pack(ctxId())));
    m_inited.set(false);
}

bool Playback::isInited() const
{
    return m_inited.val;
}

async::Channel<bool> Playback::initedChanged() const
{
    return m_inited.ch;
}

IPlayerPtr Playback::player() const
{
    std::shared_ptr<Player> p = std::make_shared<Player>(iocContext());
    p->init();
    return p;
}

template<typename T>
static void doReject(MsgCode code, T& reject, const Ret& ret)
{
    LOGE() << "failed rpc request: " << rpc::to_string(code) << ", err: " << ret.toString();
    (void)reject(ret.code(), ret.text());
}

// Resources
async::Promise<AudioResourceMetaList> Playback::availableInputResources() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<AudioResourceMetaList>([this](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(ctxId(), MsgCode::GetAvailableInputResources);
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal<AudioResourceMetaList> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                doReject(MsgCode::GetAvailableInputResources, reject, audio::make_ret(Err::InvalidRpcData));
                return;
            }
            if (ret.ret) {
                (void)resolve(ret.val);
            } else {
                doReject(MsgCode::GetAvailableInputResources, reject, ret.ret);
            }
        });
        return Promise<AudioResourceMetaList>::dummy_result();
    }, PromiseType::AsyncByBody);
}

async::Promise<SoundPresetList> Playback::availableSoundPresets(const AudioResourceMeta& resourceMeta) const
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<SoundPresetList>([this, resourceMeta](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(ctxId(), MsgCode::GetAvailableSoundPresets, RpcPacker::pack(resourceMeta));
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal<SoundPresetList> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                doReject(MsgCode::GetAvailableSoundPresets, reject, audio::make_ret(Err::InvalidRpcData));
                return;
            }
            if (ret.ret) {
                (void)resolve(ret.val);
            } else {
                doReject(MsgCode::GetAvailableSoundPresets, reject, ret.ret);
            }
        });
        return Promise<SoundPresetList>::dummy_result();
    }, PromiseType::AsyncByBody);
}

async::Promise<AudioResourceMetaList> Playback::availableOutputResources() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<AudioResourceMetaList>([this](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(ctxId(), MsgCode::GetAvailableOutputResources);
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal<AudioResourceMetaList> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                doReject(MsgCode::GetAvailableOutputResources, reject, audio::make_ret(Err::InvalidRpcData));
                return;
            }
            if (ret.ret) {
                (void)resolve(ret.val);
            } else {
                doReject(MsgCode::GetAvailableOutputResources, reject, ret.ret);
            }
        });
        return Promise<AudioResourceMetaList>::dummy_result();
    }, PromiseType::AsyncByBody);
}

// Setup tracks
async::Promise<TrackIdList> Playback::trackIdList() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<TrackIdList>([this](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(ctxId(), MsgCode::GetTrackIdList);
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal<TrackIdList> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                doReject(MsgCode::GetTrackIdList, reject, audio::make_ret(Err::InvalidRpcData));
                return;
            }
            if (ret.ret) {
                (void)resolve(ret.val);
            } else {
                doReject(MsgCode::GetTrackIdList, reject, ret.ret);
            }
        });
        return Promise<TrackIdList>::dummy_result();
    }, PromiseType::AsyncByBody);
}

async::Promise<RetVal<TrackName> > Playback::trackName(const TrackId trackId) const
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<RetVal<TrackName> >([this, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(ctxId(), MsgCode::GetTrackName, RpcPacker::pack(trackId));
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal<TrackName> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                doReject(MsgCode::GetTrackName, reject, audio::make_ret(Err::InvalidRpcData));
                return;
            }
            (void)resolve(ret);
        });
        return Promise<RetVal<TrackName> >::dummy_result();
    }, PromiseType::AsyncByBody);
}

async::Promise<TrackId, TrackParams> Playback::addTrack(const TrackName& trackName,
                                                        io::IODevice* playbackData,
                                                        const TrackParams& params)
{
#ifdef MUE_CONFIGURATION_IS_APPWEB
    NOT_SUPPORTED;
    return async::make_promise<TrackId, TrackParams>([](auto /*resolve*/, auto reject) {
        doReject(MsgCode::AddTrackWithIODevice, reject, muse::make_ret(Ret::Code::NotSupported));
        return Promise<TrackId, TrackParams>::dummy_result();
    });
#else
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<TrackId, TrackParams>([this, trackName, playbackData, params](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;

        ByteArray data = RpcPacker::pack(trackName, reinterpret_cast<uint64_t>(playbackData), params);

        Msg msg = rpc::make_request(ctxId(), MsgCode::AddTrackWithIODevice, data);
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal2<TrackId, TrackParams> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                doReject(MsgCode::AddTrackWithIODevice, reject, audio::make_ret(Err::InvalidRpcData));
                return;
            }
            if (ret.ret) {
                (void)resolve(ret.val1, ret.val2);
            } else {
                doReject(MsgCode::AddTrackWithIODevice, reject, ret.ret);
            }
        });
        return Promise<TrackId, TrackParams>::dummy_result();
    }, PromiseType::AsyncByBody);

#endif // MUE_CONFIGURATION_IS_APPWEB
}

async::Promise<TrackId, TrackParams> Playback::addTrack(const TrackName& trackName,
                                                        const mpe::PlaybackData& playbackData,
                                                        const TrackParams& params)
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<TrackId, TrackParams>([this, trackName, playbackData, params](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;

        rpc::StreamId mainStreamId = channel()->addSendStream(StreamName::PlaybackDataMainStream, playbackData.mainStream);
        rpc::StreamId offStreamId = channel()->addSendStream(StreamName::PlaybackDataOffStream, playbackData.offStream);

        ByteArray data = RpcPacker::pack(trackName, playbackData, params, mainStreamId, offStreamId);

        Msg msg = rpc::make_request(ctxId(), MsgCode::AddTrackWithPlaybackData, data);
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal2<TrackId, TrackParams> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                doReject(MsgCode::AddTrackWithPlaybackData, reject, audio::make_ret(Err::InvalidRpcData));
                return;
            }
            if (ret.ret) {
                (void)resolve(ret.val1, ret.val2);
            } else {
                doReject(MsgCode::AddTrackWithPlaybackData, reject, ret.ret);
            }
        });
        return Promise<TrackId, TrackParams>::dummy_result();
    }, PromiseType::AsyncByBody);
}

async::Promise<TrackId, TrackParams> Playback::addAuxTrack(const TrackName& trackName, const TrackParams& params)
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<TrackId, TrackParams>([this, trackName, params](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(ctxId(), MsgCode::AddAuxTrack, RpcPacker::pack(trackName, params));
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal2<TrackId, TrackParams> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                doReject(MsgCode::AddAuxTrack, reject, audio::make_ret(Err::InvalidRpcData));
                return;
            }
            if (ret.ret) {
                (void)resolve(ret.val1, ret.val2);
            } else {
                doReject(MsgCode::AddAuxTrack, reject, ret.ret);
            }
        });
        return Promise<TrackId, TrackParams>::dummy_result();
    }, PromiseType::AsyncByBody);
}

void Playback::removeTrack(const TrackId trackId)
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(ctxId(), MsgCode::RemoveTrack, RpcPacker::pack(trackId));
    channel()->send(msg);
}

void Playback::removeAllTracks()
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(ctxId(), MsgCode::RemoveAllTracks);
    channel()->send(msg);
}

async::Channel<TrackId> Playback::trackAdded() const
{
    return m_trackAdded;
}

async::Channel<TrackId> Playback::trackRemoved() const
{
    return m_trackRemoved;
}

// Params
async::Promise<TrackParams> Playback::params(const TrackId trackId) const
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<TrackParams>([this, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(ctxId(), MsgCode::GetTrackParams, RpcPacker::pack(trackId));
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal<TrackParams> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                doReject(MsgCode::GetTrackParams, reject, audio::make_ret(Err::InvalidRpcData));
                return;
            }

            if (ret.ret) {
                (void)resolve(ret.val);
            } else {
                doReject(MsgCode::GetTrackParams, reject, ret.ret);
            }
        });
        return Promise<TrackParams>::dummy_result();
    }, PromiseType::AsyncByBody);
}

void Playback::setSourceParams(const TrackId trackId, const AudioSourceParams& params)
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(ctxId(), MsgCode::SetSourceParams, RpcPacker::pack(trackId, params));
    channel()->send(msg);
}

void Playback::setControlParams(const TrackId trackId, const ControlParams& params)
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(ctxId(), MsgCode::SetControlParams, RpcPacker::pack(trackId, params));
    channel()->send(msg);
}

void Playback::setFxChainParams(const TrackId trackId, const AudioFxChain& params)
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(ctxId(), MsgCode::SetFxChainParams, RpcPacker::pack(trackId, params));
    channel()->send(msg);
}

void Playback::setAuxSendsParams(const TrackId trackId, const AuxSendsParams& params)
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(ctxId(), MsgCode::SetAuxSendsParams, RpcPacker::pack(trackId, params));
    channel()->send(msg);
}

async::Channel<TrackId, AudioSourceParams> Playback::sourceParamsChanged() const
{
    return m_sourceParamsChanged;
}

async::Channel<TrackId, AudioFxChain> Playback::fxChainParamsChanged() const
{
    return m_fxChainParamsChanged;
}

// Same for master
async::Promise<TrackParams> Playback::masterParams() const
{
    return params(MASTER_TRACK_ID);
}

void Playback::setMasterControlParams(const ControlParams& params)
{
    setControlParams(MASTER_TRACK_ID, params);
}

void Playback::setMasterFxChainParams(const AudioFxChain& params)
{
    setFxChainParams(MASTER_TRACK_ID, params);
}

void Playback::setMasterAuxSendsParams(const AuxSendsParams& params)
{
    setAuxSendsParams(MASTER_TRACK_ID, params);
}

async::Channel<AudioFxChain> Playback::masterFxChainParamsChanged() const
{
    return m_masterFxChainParamsChanged;
}

void Playback::processInput(const TrackId trackId) const
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(ctxId(), MsgCode::ProcessInput, RpcPacker::pack(trackId));
    channel()->send(msg);
}

muse::async::Promise<InputProcessingProgress> Playback::inputProcessingProgress(const TrackId trackId) const
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<InputProcessingProgress>([this, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(ctxId(), MsgCode::GetInputProcessingProgress, RpcPacker::pack(trackId));
        channel()->send(msg, [this, resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            Ret ret;
            bool isStarted = false;
            StreamId streamId = 0;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret, isStarted, streamId)) {
                doReject(MsgCode::GetInputProcessingProgress, reject, audio::make_ret(Err::InvalidRpcData));
                return;
            }

            if (ret) {
                InputProcessingProgress prog;
                prog.isStarted = isStarted;
                channel()->addReceiveStream(StreamName::InputProcessingProgressStream, streamId, prog.processedChannel);
                (void)resolve(prog);
            } else {
                doReject(MsgCode::GetInputProcessingProgress, reject, ret);
            }
        });
        return Promise<InputProcessingProgress>::dummy_result();
    }, PromiseType::AsyncByBody);
}

void Playback::clearCache(const TrackId trackId) const
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(ctxId(), MsgCode::ClearCache, RpcPacker::pack(trackId));
    channel()->send(msg);
}

void Playback::clearSources()
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(ctxId(), MsgCode::ClearSources);
    channel()->send(msg);
}

void Playback::clearMasterOutputParams()
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(ctxId(), MsgCode::ClearMasterOutputParams);
    channel()->send(msg);
}

void Playback::clearAllFx()
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(ctxId(), MsgCode::ClearAllFx);
    channel()->send(msg);
}

// Signals
async::Promise<AudioSignalChanges> Playback::signalChanges(const TrackId trackId) const
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<AudioSignalChanges>([this, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(ctxId(), MsgCode::GetSignalChanges, RpcPacker::pack(trackId));
        channel()->send(msg, [this, resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal<StreamId> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                doReject(MsgCode::GetSignalChanges, reject, audio::make_ret(Err::InvalidRpcData));
                return;
            }

            if (ret.ret) {
                AudioSignalChanges ch;
                channel()->addReceiveStream(StreamName::AudioSignalStream, ret.val, ch);
                (void)resolve(ch);
            } else {
                doReject(MsgCode::GetSignalChanges, reject, ret.ret);
            }
        });
        return Promise<AudioSignalChanges>::dummy_result();
    }, PromiseType::AsyncByBody);
}

async::Promise<AudioSignalChanges> Playback::masterSignalChanges() const
{
    return signalChanges(MASTER_TRACK_ID);
}

async::Promise<bool> Playback::saveSoundTrack(const SoundTrackFormat& format, io::IODevice& dstDevice)
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<bool>([this, format, &dstDevice](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg
            = rpc::make_request(ctxId(), MsgCode::SaveSoundTrack, RpcPacker::pack(format,
                                                                                  reinterpret_cast<uintptr_t>(&dstDevice)));
        channel()->send(msg, [resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            Ret ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                doReject(MsgCode::SaveSoundTrack, reject, audio::make_ret(Err::InvalidRpcData));
                return;
            }

            if (ret) {
                (void)resolve(true);
            } else {
                doReject(MsgCode::SaveSoundTrack, reject, ret);
            }
        });
        return Promise<bool>::dummy_result();
    }, PromiseType::AsyncByBody);
}

void Playback::abortSavingAllSoundTracks()
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(ctxId(), MsgCode::AbortSavingAllSoundTracks);
    channel()->send(msg);
}

SaveSoundTrackProgress Playback::saveSoundTrackProgressChanged() const
{
    if (!m_saveSoundTrackProgressStreamInited) {
        Msg msg = rpc::make_request(ctxId(), MsgCode::GetSaveSoundTrackProgress);
        channel()->send(msg, [this](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal<StreamId> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                return;
            }

            if (!ret.ret) {
                LOGE() << "GetSaveSoundTrackProgress failed: " << ret.ret.toString();
                return;
            }

            if (m_saveSoundTrackProgressStreamId == 0) {
                m_saveSoundTrackProgressStreamId = ret.val;
                channel()->addReceiveStream(StreamName::SaveSoundTrackProgressStream,
                                            m_saveSoundTrackProgressStreamId,
                                            m_saveSoundTrackProgressStream);
            }

            assert(m_saveSoundTrackProgressStreamId == ret.val);
        });

        m_saveSoundTrackProgressStreamInited = true;
    }

    return m_saveSoundTrackProgressStream;
}
