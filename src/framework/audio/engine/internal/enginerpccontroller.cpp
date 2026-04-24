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
#include "enginerpccontroller.h"

#include "audio/common/audiosanitizer.h"
#include "audio/common/rpc/rpcpacker.h"

#include "log.h"

//#define CHECK_METHODS_DURATION

#ifdef CHECK_METHODS_DURATION
#include <chrono>
#define BEGIN_METHOD_DURATION \
    auto _start_clock = std::chrono::high_resolution_clock::now();
#define END_METHOD_DURATION(method) \
    auto _end_clock = std::chrono::high_resolution_clock::now(); \
    auto _duration_us = std::chrono::duration_cast<std::chrono::microseconds>(_end_clock - _start_clock); \
    LOGDA() << rpc::to_string(method) << " duration: " << (_duration_us.count() / 1000.0) << " ms";
#else
#define BEGIN_METHOD_DURATION
#define END_METHOD_DURATION(method)
#endif

using namespace muse::audio::engine;
using namespace muse::audio::rpc;

std::shared_ptr<IAudioContext> EngineRpcController::audioContext(rpc::CtxId ctxId) const
{
    IF_ASSERT_FAILED(ctxId > 0) {
        return nullptr;
    }
    return audioEngine()->context(static_cast<AudioCtxId>(ctxId));
}

void EngineRpcController::init()
{
    ONLY_AUDIO_RPC_THREAD;

    // AudioEngine
    onLongRequest(MsgCode::SetOutputSpec, [this](const Msg& msg) {
        OutputSpec spec;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, spec)) {
            return;
        }
        audioEngine()->setOutputSpec(spec);
    });

    // Soundfont
    onLongRequest(MsgCode::LoadSoundFonts, [this](const Msg& msg) {
        std::vector<synth::SoundFontUri> uris;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, uris)) {
            return;
        }

        soundFontRepository()->loadSoundFonts(uris);
    });

    onLongRequest(MsgCode::AddSoundFont, [this](const Msg& msg) {
        synth::SoundFontUri uri;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, uri)) {
            return;
        }

        soundFontRepository()->addSoundFont(uri);
    });

    onLongRequest(MsgCode::AddSoundFontData, [this](const Msg& msg) {
        synth::SoundFontUri uri;
        ByteArray data;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, uri, data)) {
            return;
        }

        soundFontRepository()->addSoundFontData(uri, data);
    });

    // Engine Conf
    onLongRequest(MsgCode::EngineConfigChanged, [this](const Msg& msg) {
        AudioEngineConfig conf;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, conf)) {
            return;
        }

        configuration()->setConfig(conf);
    });

    // AudioContext

    // Init / Deinit
    onQuickRequest(MsgCode::ContextInit, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;

        RetVal<std::shared_ptr<IAudioContext> > ret = audioEngine()->addAudioContext(msg.ctxId);
        if (ret.ret) {
            const std::shared_ptr<IAudioContext>& audioContext = ret.val;
            // Notification
            audioContext->trackAdded().onReceive(this, [this](TrackId trackId) {
                channel()->send(rpc::make_notification(MsgCode::TrackAdded, RpcPacker::pack(trackId)));
            });

            audioContext->trackRemoved().onReceive(this, [this](TrackId trackId) {
                channel()->send(rpc::make_notification(MsgCode::TrackRemoved, RpcPacker::pack(trackId)));
            });

            audioContext->inputParamsChanged().onReceive(this, [this](TrackId trackId, const AudioInputParams& params) {
                channel()->send(rpc::make_notification(MsgCode::InputParamsChanged, RpcPacker::pack(trackId, params)));
            });

            audioContext->outputParamsChanged().onReceive(this, [this](TrackId trackId, const AudioOutputParams& params) {
                channel()->send(rpc::make_notification(MsgCode::OutputParamsChanged, RpcPacker::pack(trackId, params)));
            });

            audioContext->masterOutputParamsChanged().onReceive(this, [this](const AudioOutputParams& params) {
                channel()->send(rpc::make_notification(MsgCode::MasterOutputParamsChanged, RpcPacker::pack(params)));
            });
        }

        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret.ret)));
    });

    onQuickRequest(MsgCode::ContextDeinit, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        auto audioContext = this->audioContext(msg.ctxId);
        IF_ASSERT_FAILED(audioContext) {
            return;
        }

        audioContext->trackAdded().disconnect(this);
        audioContext->trackRemoved().disconnect(this);
        audioContext->inputParamsChanged().disconnect(this);
        audioContext->outputParamsChanged().disconnect(this);
        audioContext->masterOutputParamsChanged().disconnect(this);

        audioEngine()->destroyContext(msg.ctxId);

        Ret ret = make_ok();
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    // Tracks
    onQuickRequest(MsgCode::GetTrackIdList, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        RetVal<TrackIdList> ret = audioContext(msg.ctxId)->trackIdList();
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    onQuickRequest(MsgCode::GetTrackName, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, trackId)) {
            return;
        }
        RetVal<TrackName> ret = audioContext(msg.ctxId)->trackName(trackId);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    onLongRequest(MsgCode::AddTrackWithPlaybackData, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackName trackName;
        mpe::PlaybackData playbackData;
        AudioParams params;
        rpc::StreamId mainStreamId = 0;
        rpc::StreamId offStreamId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, trackName, playbackData, params, mainStreamId, offStreamId)) {
            return;
        }

        RpcStreamExec mainExec = [this](const std::function<void()>& func) {
            audioEngine()->execOperation(OperationType::QuickOperation, func);
        };

        RpcStreamExec offExec = [this](const std::function<void()>& func) {
            audioEngine()->execOperation(OperationType::QuickOperation, func);
        };

        channel()->addReceiveStream(StreamName::PlaybackDataMainStream, mainStreamId, playbackData.mainStream, mainExec);
        channel()->addReceiveStream(StreamName::PlaybackDataOffStream, offStreamId, playbackData.offStream, offExec);

        auto addTrackAndSendResponse = [this](const Msg& msg, const TrackName& trackName,
                                              const mpe::PlaybackData& playbackData, const AudioParams& params) {
            RetVal2<TrackId, AudioParams> ret = audioContext(msg.ctxId)->addTrack(trackName, playbackData, params);
            channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
        };

        AudioResourceType resourceType = params.in.resourceMeta.type;

        // Not Fluid
        if (resourceType != AudioResourceType::FluidSoundfont) {
            addTrackAndSendResponse(msg, trackName, playbackData, params);
            return;
        }

        // Fluid
        std::string sfname = params.in.resourceMeta.attributeVal(synth::SOUNDFONT_NAME_ATTRIBUTE).toStdString();
        if (sfname.empty()) {
            sfname = params.in.resourceMeta.id;
        }

        if (soundFontRepository()->isSoundFontLoaded(sfname)) {
            addTrackAndSendResponse(msg, trackName, playbackData, params);
        }
        // Waiting for SF to load
        else if (soundFontRepository()->isLoadingSoundFonts()) {
            LOGI() << "Waiting for SF to load, trackName: " << trackName << ", SF name: " << sfname;
            m_pendingTracks[sfname].emplace_back(PendingTrack { msg, trackName, playbackData, params });

            //! NOTE We subscribe for the first track for which a soundfont is not found.
            //! When the notification is triggered, processing will be called for all tracks.
            if (!m_soundFontsChangedSubscribed) {
                m_soundFontsChangedSubscribed = true;
                soundFontRepository()->soundFontsChanged().onNotify(this,
                                                                    [this, addTrackAndSendResponse]() {
                    std::vector<std::string> toRemove;
                    for (auto& p : m_pendingTracks) {
                        const std::string& sfname = p.first;
                        if (soundFontRepository()->isSoundFontLoaded(sfname)) {
                            for (const PendingTrack& t : p.second) {
                                addTrackAndSendResponse(t.msg, t.trackName, t.playbackData, t.params);
                            }
                            toRemove.push_back(sfname);
                        }
                    }

                    for (const std::string& sf : toRemove) {
                        m_pendingTracks.erase(sf);
                    }

                    if (m_pendingTracks.empty()) {
                        soundFontRepository()->soundFontsChanged().disconnect(this);
                        m_soundFontsChangedSubscribed = false;
                    }
                });
            }
        } else { // Attempt to add it anyway (most likely fallback will be used)
            addTrackAndSendResponse(msg, trackName, playbackData, params);
        }
    });

    onLongRequest(MsgCode::AddTrackWithIODevice, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackName trackName;
        uint64_t devicePtr = 0;
        AudioParams params;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, trackName, devicePtr, params)) {
            return;
        }
        io::IODevice* device = reinterpret_cast<io::IODevice*>(devicePtr);

        RetVal2<TrackId, AudioParams> ret = audioContext(msg.ctxId)->addTrack(trackName, device, params);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    onLongRequest(MsgCode::AddAuxTrack, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackName trackName;
        AudioOutputParams outputParams;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, trackName, outputParams)) {
            return;
        }
        RetVal2<TrackId, AudioOutputParams> ret = audioContext(msg.ctxId)->addAuxTrack(trackName, outputParams);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    onLongRequest(MsgCode::RemoveTrack, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, trackId)) {
            return;
        }
        audioContext(msg.ctxId)->removeTrack(trackId);
    });

    onLongRequest(MsgCode::RemoveAllTracks, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        audioContext(msg.ctxId)->removeAllTracks();
    });

    onQuickRequest(MsgCode::GetAvailableInputResources, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        AudioResourceMetaList list = audioContext(msg.ctxId)->availableInputResources();
        //! NOTE The list can be large.
        //! There can be many re-allocations, because of this it takes a long time to pack.
        //! Let's add a reserve. 300 is approximately the size of one element, we get empirically.
        channel()->send(rpc::make_response(msg, RpcPacker::pack(rpc::Options { list.size() * 300 }, list)));
    });

    onQuickRequest(MsgCode::GetAvailableSoundPresets, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        AudioResourceMeta meta;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, meta)) {
            return;
        }
        SoundPresetList list = audioContext(msg.ctxId)->availableSoundPresets(meta);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(list)));
    });

    onQuickRequest(MsgCode::GetInputParams, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, trackId)) {
            return;
        }
        RetVal<AudioInputParams> ret = audioContext(msg.ctxId)->inputParams(trackId);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    onLongRequest(MsgCode::SetInputParams, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackId trackId = 0;
        AudioInputParams params;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, trackId, params)) {
            return;
        }
        audioContext(msg.ctxId)->setInputParams(trackId, params);
    });

    onLongRequest(MsgCode::ProcessInput, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, trackId)) {
            return;
        }

        audioContext(msg.ctxId)->processInput(trackId);
    });

    onQuickRequest(MsgCode::GetInputProcessingProgress, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, trackId)) {
            return;
        }

        RetVal<InputProcessingProgress> ret = audioContext(msg.ctxId)->inputProcessingProgress(trackId);
        StreamId streamId = 0;
        if (ret.ret) {
            streamId = channel()->addSendStream(StreamName::InputProcessingProgressStream, ret.val.processedChannel);
        }

        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret.ret, ret.val.isStarted, streamId)));
    });

    onLongRequest(MsgCode::ClearCache, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, trackId)) {
            return;
        }

        audioContext(msg.ctxId)->clearCache(trackId);
    });

    onLongRequest(MsgCode::ClearSources, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        audioContext(msg.ctxId)->clearSources();
    });

    // Play
    onQuickRequest(MsgCode::PrepareToPlay, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        audioContext(msg.ctxId)->prepareToPlay().onResolve(this, [this, msg](const Ret& ret) {
            channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
        });
    });

    onQuickRequest(MsgCode::Play, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        secs_t delay = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, delay)) {
            return;
        }
        audioContext(msg.ctxId)->play(delay);
    });

    onQuickRequest(MsgCode::Seek, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        secs_t newPosition = 0;
        bool flushSound = false;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, newPosition, flushSound)) {
            return;
        }
        audioContext(msg.ctxId)->seek(newPosition, flushSound);
    });

    onQuickRequest(MsgCode::Stop, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        audioContext(msg.ctxId)->stop();
    });

    onQuickRequest(MsgCode::Pause, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        audioContext(msg.ctxId)->pause();
    });

    onQuickRequest(MsgCode::Resume, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        secs_t delay = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, delay)) {
            return;
        }
        audioContext(msg.ctxId)->resume(delay);
    });

    onQuickRequest(MsgCode::SetDuration, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        secs_t duration = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, duration)) {
            return;
        }
        audioContext(msg.ctxId)->setDuration(duration);
    });

    onQuickRequest(MsgCode::SetLoop, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        secs_t from = 0;
        secs_t to = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, from, to)) {
            return;
        }
        Ret ret = audioContext(msg.ctxId)->setLoop(from, to);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    onQuickRequest(MsgCode::ResetLoop, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        audioContext(msg.ctxId)->resetLoop();
    });

    onQuickRequest(MsgCode::GetPlaybackStatus, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;

        PlaybackStatus status = audioContext(msg.ctxId)->playbackStatus();
        async::Channel<PlaybackStatus> ch = audioContext(msg.ctxId)->playbackStatusChanged();
        StreamId streamId = channel()->addSendStream(StreamName::PlaybackStatusStream, ch);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(status, streamId)));
    });

    onQuickRequest(MsgCode::GetPlaybackPosition, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;

        secs_t pos = audioContext(msg.ctxId)->playbackPosition();
        async::Channel<secs_t> ch = audioContext(msg.ctxId)->playbackPositionChanged();
        StreamId streamId = channel()->addSendStream(StreamName::PlaybackPositionStream, ch);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(pos, streamId)));
    });

    // Output

    onQuickRequest(MsgCode::GetOutputParams, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, trackId)) {
            return;
        }
        RetVal<AudioOutputParams> ret = audioContext(msg.ctxId)->outputParams(trackId);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    onQuickRequest(MsgCode::SetOutputParams, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackId trackId = 0;
        AudioOutputParams params;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, trackId, params)) {
            return;
        }
        audioContext(msg.ctxId)->setOutputParams(trackId, params);
    });

    onQuickRequest(MsgCode::GetMasterOutputParams, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        RetVal<AudioOutputParams> ret = audioContext(msg.ctxId)->masterOutputParams();
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    onQuickRequest(MsgCode::SetMasterOutputParams, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        AudioOutputParams params;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, params)) {
            return;
        }
        audioContext(msg.ctxId)->setMasterOutputParams(params);
    });

    onQuickRequest(MsgCode::ClearMasterOutputParams, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        audioContext(msg.ctxId)->clearMasterOutputParams();
    });

    onQuickRequest(MsgCode::GetAvailableOutputResources, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        AudioResourceMetaList list = audioContext(msg.ctxId)->availableOutputResources();
        channel()->send(rpc::make_response(msg, RpcPacker::pack(list)));
    });

    onQuickRequest(MsgCode::GetSignalChanges, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, trackId)) {
            return;
        }

        RetVal<AudioSignalChanges> ret = audioContext(msg.ctxId)->signalChanges(trackId);
        StreamId streamId = 0;
        if (ret.ret) {
            streamId = channel()->addSendStream(StreamName::AudioSignalStream, ret.val);
        }

        RetVal<StreamId> res;
        res.ret = ret.ret;
        res.val = streamId;

        channel()->send(rpc::make_response(msg, RpcPacker::pack(res)));
    });

    onQuickRequest(MsgCode::GetMasterSignalChanges, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        RetVal<AudioSignalChanges> ret = audioContext(msg.ctxId)->masterSignalChanges();
        StreamId streamId = 0;
        if (ret.ret) {
            streamId = channel()->addSendStream(StreamName::AudioMasterSignalStream, ret.val);
        }

        RetVal<StreamId> res;
        res.ret = ret.ret;
        res.val = streamId;

        channel()->send(rpc::make_response(msg, RpcPacker::pack(res)));
    });

    onLongRequest(MsgCode::SaveSoundTrack, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        SoundTrackFormat format;
        uintptr_t dstDevicePtr = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, format, dstDevicePtr)) {
            return;
        }
        io::IODevice& dstDevice = *reinterpret_cast<io::IODevice*>(dstDevicePtr);
        audioContext(msg.ctxId)->saveSoundTrack(dstDevice, format).onResolve(this, [this, msg](const Ret& ret) {
            channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
        });
    });

    onLongRequest(MsgCode::AbortSavingAllSoundTracks, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        audioContext(msg.ctxId)->abortSavingAllSoundTracks();
    });

    onQuickRequest(MsgCode::GetSaveSoundTrackProgress, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        SaveSoundTrackProgress ch = audioContext(msg.ctxId)->saveSoundTrackProgressChanged();
        ch.onReceive(this, [this](int64_t current, int64_t total, SaveSoundTrackStage stage) {
            ONLY_AUDIO_RPC_THREAD;
            m_saveSoundTrackProgressStream.send(current, total, stage);
        });

        if (m_saveSoundTrackProgressStreamId == 0) {
            m_saveSoundTrackProgressStreamId = channel()->addSendStream(StreamName::SaveSoundTrackProgressStream,
                                                                        m_saveSoundTrackProgressStream);
        }

        channel()->send(rpc::make_response(msg, RpcPacker::pack(m_saveSoundTrackProgressStreamId)));
    });

    onQuickRequest(MsgCode::ClearAllFx, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        audioContext(msg.ctxId)->clearAllFx();
    });
}

void EngineRpcController::onLongRequest(rpc::MsgCode code, const rpc::Handler& h)
{
    onRequest(OperationType::LongOperation, code, h);
}

void EngineRpcController::onQuickRequest(rpc::MsgCode code, const Handler& h)
{
    onRequest(OperationType::QuickOperation, code, h);
}

void EngineRpcController::onRequest(OperationType type, rpc::MsgCode code, const Handler& handler)
{
    m_usedRequests.push_back(code);

    channel()->onRequest(code, [this, type, code, handler](const Msg& msg) {
        IAudioEngine::Operation func = [this, code, handler, msg]() {
            if (m_terminated) {
                return;
            }

            UNUSED(code);
            BEGIN_METHOD_DURATION
            handler(msg);
            END_METHOD_DURATION(code)
        };
        audioEngine()->execOperation(type, func);
    });
}

void EngineRpcController::deinit()
{
    ONLY_AUDIO_RPC_THREAD;

    m_terminated = true;

    for (const MsgCode& m : m_usedRequests) {
        channel()->onRequest(m, nullptr);
    }
    m_usedRequests.clear();
}
