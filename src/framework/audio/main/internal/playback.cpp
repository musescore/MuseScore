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
#include "player.h"

#include "muse_framework_config.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::rpc;
using namespace muse::async;

async::Promise<Ret> Playback::init()
{
    ONLY_AUDIO_MAIN_THREAD;

    channel()->onNotification(MsgCode::TrackAdded, [this](const Msg& msg) {
        ONLY_AUDIO_MAIN_THREAD;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, trackId)) {
            return;
        }
        m_trackAdded.send(trackId);
    });

    channel()->onNotification(MsgCode::TrackRemoved, [this](const Msg& msg) {
        ONLY_AUDIO_MAIN_THREAD;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, trackId)) {
            return;
        }
        m_trackRemoved.send(trackId);
    });

    channel()->onNotification(MsgCode::InputParamsChanged, [this](const Msg& msg) {
        ONLY_AUDIO_MAIN_THREAD;
        TrackId trackId = 0;
        AudioInputParams params;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, trackId, params)) {
            return;
        }
        m_inputParamsChanged.send(trackId, params);
    });

    channel()->onNotification(MsgCode::OutputParamsChanged, [this](const Msg& msg) {
        ONLY_AUDIO_MAIN_THREAD;
        TrackId trackId = 0;
        AudioOutputParams params;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, trackId, params)) {
            return;
        }
        m_outputParamsChanged.send(trackId, params);
    });

    channel()->onNotification(MsgCode::MasterOutputParamsChanged, [this](const Msg& msg) {
        ONLY_AUDIO_MAIN_THREAD;
        AudioOutputParams params;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, params)) {
            return;
        }
        m_masterOutputParamsChanged.send(params);
    });

    return async::make_promise<Ret>([this](auto resolve, auto /*reject*/) {
        ONLY_AUDIO_MAIN_THREAD;

        auto initContext = [this, resolve]() {
            Msg msg = rpc::make_request(MsgCode::ContextInit);
            channel()->send(msg, [this, resolve](const Msg& res) {
                ONLY_AUDIO_MAIN_THREAD;
                Ret ret;
                IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
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

    channel()->onNotification(MsgCode::TrackAdded, nullptr);
    channel()->onNotification(MsgCode::TrackRemoved, nullptr);
    channel()->onNotification(MsgCode::InputParamsChanged, nullptr);
    channel()->onNotification(MsgCode::OutputParamsChanged, nullptr);
    channel()->onNotification(MsgCode::MasterOutputParamsChanged, nullptr);

    m_saveSoundTrackProgressStream = SaveSoundTrackProgress();

    channel()->send(rpc::make_request(MsgCode::ContextDeinit));
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

// 2. Setup tracks
async::Promise<TrackIdList> Playback::trackIdList() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<TrackIdList>([this](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(MsgCode::GetTrackIdList);
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
        return Promise<TrackIdList>::dummy_result();
    }, PromiseType::AsyncByBody);
}

async::Promise<RetVal<TrackName> > Playback::trackName(const TrackId trackId) const
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<RetVal<TrackName> >([this, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(MsgCode::GetTrackName, RpcPacker::pack(trackId));
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

async::Promise<TrackId, AudioParams> Playback::addTrack(const TrackName& trackName,
                                                        io::IODevice* playbackData,
                                                        AudioParams&& params)
{
#ifdef MUE_CONFIGURATION_IS_APPWEB
    NOT_SUPPORTED;
    return async::make_promise<TrackId, AudioParams>([](auto /*resolve*/, auto reject) {
        Ret ret = muse::make_ret(Ret::Code::NotSupported);
        return reject(ret.code(), ret.text());
    });
#else
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<TrackId, AudioParams>([this, trackName, playbackData, params](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;

        ByteArray data = RpcPacker::pack(trackName, reinterpret_cast<uint64_t>(playbackData), params);

        Msg msg = rpc::make_request(MsgCode::AddTrackWithIODevice, data);
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

async::Promise<TrackId, AudioParams> Playback::addTrack(const TrackName& trackName,
                                                        const mpe::PlaybackData& playbackData,
                                                        AudioParams&& params)
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<TrackId, AudioParams>([this, trackName, playbackData, params](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;

        rpc::StreamId mainStreamId = channel()->addSendStream(StreamName::PlaybackDataMainStream, playbackData.mainStream);
        rpc::StreamId offStreamId = channel()->addSendStream(StreamName::PlaybackDataOffStream, playbackData.offStream);

        ByteArray data = RpcPacker::pack(trackName, playbackData, params, mainStreamId, offStreamId);

        Msg msg = rpc::make_request(MsgCode::AddTrackWithPlaybackData, data);
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

async::Promise<TrackId, AudioOutputParams> Playback::addAuxTrack(const TrackName& trackName,
                                                                 const AudioOutputParams& outputParams)
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<TrackId, AudioOutputParams>([this, trackName, outputParams](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(MsgCode::AddAuxTrack, RpcPacker::pack(trackName, outputParams));
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

void Playback::removeTrack(const TrackId trackId)
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(MsgCode::RemoveTrack, RpcPacker::pack(trackId));
    channel()->send(msg);
}

void Playback::removeAllTracks()
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(MsgCode::RemoveAllTracks);
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

async::Promise<AudioResourceMetaList> Playback::availableInputResources() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<AudioResourceMetaList>([this](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(MsgCode::GetAvailableInputResources);
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
        Msg msg = rpc::make_request(MsgCode::GetAvailableSoundPresets, RpcPacker::pack(resourceMeta));
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

async::Promise<AudioInputParams> Playback::inputParams(const TrackId trackId) const
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<AudioInputParams>([this, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(MsgCode::GetInputParams, RpcPacker::pack(trackId));
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

void Playback::setInputParams(const TrackId trackId, const AudioInputParams& params)
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(MsgCode::SetInputParams, RpcPacker::pack(trackId, params));
    channel()->send(msg);
}

async::Channel<TrackId, AudioInputParams> Playback::inputParamsChanged() const
{
    return m_inputParamsChanged;
}

void Playback::processInput(const TrackId trackId) const
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(MsgCode::ProcessInput, RpcPacker::pack(trackId));
    channel()->send(msg);
}

muse::async::Promise<InputProcessingProgress> Playback::inputProcessingProgress(const TrackId trackId) const
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<InputProcessingProgress>([this, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(MsgCode::GetInputProcessingProgress, RpcPacker::pack(trackId));
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
                channel()->addReceiveStream(StreamName::InputProcessingProgressStream, streamId, prog.processedChannel);
                (void)resolve(prog);
            } else {
                (void)reject(ret.code(), ret.text());
            }
        });
        return Promise<InputProcessingProgress>::dummy_result();
    }, PromiseType::AsyncByBody);
}

void Playback::clearCache(const TrackId trackId) const
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(MsgCode::ClearCache, RpcPacker::pack(trackId));
    channel()->send(msg);
}

void Playback::clearSources()
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(MsgCode::ClearSources);
    channel()->send(msg);
}

// 4. Adjust output

async::Promise<AudioOutputParams> Playback::outputParams(const TrackId trackId) const
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<AudioOutputParams>([this, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(MsgCode::GetOutputParams, RpcPacker::pack(trackId));
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

void Playback::setOutputParams(const TrackId trackId, const AudioOutputParams& params)
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(MsgCode::SetOutputParams, RpcPacker::pack(trackId, params));
    channel()->send(msg);
}

async::Channel<TrackId, AudioOutputParams> Playback::outputParamsChanged() const
{
    return m_outputParamsChanged;
}

async::Promise<AudioOutputParams> Playback::masterOutputParams() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<AudioOutputParams>([this](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(MsgCode::GetMasterOutputParams);
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
    Msg msg = rpc::make_request(MsgCode::SetMasterOutputParams, RpcPacker::pack(params));
    channel()->send(msg);
}

void Playback::clearMasterOutputParams()
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(MsgCode::ClearMasterOutputParams);
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
        Msg msg = rpc::make_request(MsgCode::GetAvailableOutputResources);
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

async::Promise<AudioSignalChanges> Playback::signalChanges(const TrackId trackId) const
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<AudioSignalChanges>([this, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(MsgCode::GetSignalChanges, RpcPacker::pack(trackId));
        channel()->send(msg, [this, resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal<StreamId> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                return;
            }

            if (ret.ret) {
                AudioSignalChanges ch;
                channel()->addReceiveStream(StreamName::AudioSignalStream, ret.val, ch);
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
        Msg msg = rpc::make_request(MsgCode::GetMasterSignalChanges);
        channel()->send(msg, [this, resolve, reject](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            RetVal<StreamId> ret;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, ret)) {
                return;
            }

            if (ret.ret) {
                AudioSignalChanges ch;
                channel()->addReceiveStream(StreamName::AudioMasterSignalStream, ret.val, ch);
                (void)resolve(ch);
            } else {
                (void)reject(ret.ret.code(), ret.ret.text());
            }
        });
        return Promise<AudioSignalChanges>::dummy_result();
    }, PromiseType::AsyncByBody);
}

async::Promise<bool> Playback::saveSoundTrack(const SoundTrackFormat& format, io::IODevice& dstDevice)
{
    ONLY_AUDIO_MAIN_THREAD;
    return async::make_promise<bool>([this, format, &dstDevice](auto resolve, auto reject) {
        ONLY_AUDIO_MAIN_THREAD;
        Msg msg = rpc::make_request(MsgCode::SaveSoundTrack, RpcPacker::pack(format, reinterpret_cast<uintptr_t>(&dstDevice)));
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
    Msg msg = rpc::make_request(MsgCode::AbortSavingAllSoundTracks);
    channel()->send(msg);
}

SaveSoundTrackProgress Playback::saveSoundTrackProgressChanged() const
{
    if (!m_saveSoundTrackProgressStreamInited) {
        Msg msg = rpc::make_request(MsgCode::GetSaveSoundTrackProgress);
        channel()->send(msg, [this](const Msg& res) {
            ONLY_AUDIO_MAIN_THREAD;
            StreamId streamId = 0;
            IF_ASSERT_FAILED(RpcPacker::unpack(res.data, streamId)) {
                return;
            }

            if (m_saveSoundTrackProgressStreamId == 0) {
                m_saveSoundTrackProgressStreamId = streamId;
                channel()->addReceiveStream(StreamName::SaveSoundTrackProgressStream,
                                            m_saveSoundTrackProgressStreamId,
                                            m_saveSoundTrackProgressStream);
            }

            assert(m_saveSoundTrackProgressStreamId == streamId);
        });

        m_saveSoundTrackProgressStreamInited = true;
    }

    return m_saveSoundTrackProgressStream;
}

void Playback::clearAllFx()
{
    ONLY_AUDIO_MAIN_THREAD;
    Msg msg = rpc::make_request(MsgCode::ClearAllFx);
    channel()->send(msg);
}
