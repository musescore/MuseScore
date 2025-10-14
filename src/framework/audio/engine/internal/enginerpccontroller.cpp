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

using namespace muse::audio::engine;
using namespace muse::audio::rpc;

void EngineRpcController::init()
{
    ONLY_AUDIO_RPC_THREAD;

    // AudioEngine
    onMethod(Method::SetOutputSpec, [this](const Msg& msg) {
        OutputSpec spec;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, spec)) {
            return;
        }
        engine()->setOutputSpec(spec);
    });

    // Soundfont
    onMethod(Method::LoadSoundFonts, [this](const Msg& msg) {
        std::vector<synth::SoundFontUri> uris;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, uris)) {
            return;
        }

        soundFontRepository()->loadSoundFonts(uris);
    });

    onMethod(Method::AddSoundFont, [this](const Msg& msg) {
        synth::SoundFontUri uri;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, uri)) {
            return;
        }

        soundFontRepository()->addSoundFont(uri);
    });

    onMethod(Method::AddSoundFontData, [this](const Msg& msg) {
        synth::SoundFontUri uri;
        ByteArray data;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, uri, data)) {
            return;
        }

        soundFontRepository()->addSoundFontData(uri, data);
    });

    // Engine Conf
    onMethod(Method::EngineConfigChanged, [this](const Msg& msg) {
        AudioEngineConfig conf;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, conf)) {
            return;
        }

        configuration()->setConfig(conf);
    });

    // Playback
    // Notification
    playback()->trackAdded().onReceive(this, [this](TrackSequenceId sequenceId, TrackId trackId) {
        channel()->send(rpc::make_notification(Method::TrackAdded, RpcPacker::pack(sequenceId, trackId)));
    });

    playback()->trackRemoved().onReceive(this, [this](TrackSequenceId sequenceId, TrackId trackId) {
        channel()->send(rpc::make_notification(Method::TrackRemoved, RpcPacker::pack(sequenceId, trackId)));
    });

    playback()->inputParamsChanged().onReceive(this, [this](TrackSequenceId sequenceId, TrackId trackId,
                                                            const AudioInputParams& params) {
        channel()->send(rpc::make_notification(Method::InputParamsChanged, RpcPacker::pack(sequenceId, trackId, params)));
    });

    playback()->outputParamsChanged().onReceive(this, [this](TrackSequenceId sequenceId, TrackId trackId,
                                                             const AudioOutputParams& params) {
        channel()->send(rpc::make_notification(Method::OutputParamsChanged, RpcPacker::pack(sequenceId, trackId, params)));
    });

    playback()->masterOutputParamsChanged().onReceive(this, [this](const AudioOutputParams& params) {
        channel()->send(rpc::make_notification(Method::MasterOutputParamsChanged, RpcPacker::pack(params)));
    });

    // Sequences
    onMethod(Method::AddSequence, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = playback()->addSequence();
        channel()->send(rpc::make_response(msg, RpcPacker::pack(seqId)));
    });

    onMethod(Method::RemoveSequence, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId)) {
            return;
        }
        playback()->removeSequence(seqId);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(true)));
    });

    onMethod(Method::GetSequenceIdList, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceIdList list = playback()->sequenceIdList();
        channel()->send(rpc::make_response(msg, RpcPacker::pack(list)));
    });

    // Tracks
    onMethod(Method::GetTrackIdList, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId)) {
            return;
        }
        RetVal<TrackIdList> ret = playback()->trackIdList(seqId);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    onMethod(Method::GetTrackName, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackId)) {
            return;
        }
        RetVal<TrackName> ret = playback()->trackName(seqId, trackId);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    onMethod(Method::AddTrackWithPlaybackData, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        TrackName trackName;
        mpe::PlaybackData playbackData;
        AudioParams params;
        rpc::StreamId mainStreamId = 0;
        rpc::StreamId offStreamId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackName, playbackData, params, mainStreamId, offStreamId)) {
            return;
        }

        channel()->addReceiveStream(StreamName::PlaybackDataMainStream, mainStreamId, playbackData.mainStream);
        channel()->addReceiveStream(StreamName::PlaybackDataOffStream, offStreamId, playbackData.offStream);

        auto addTrackAndSendResponce = [this](const Msg& msg, const TrackSequenceId& seqId, const TrackName& trackName,
                                              const mpe::PlaybackData& playbackData, const AudioParams& params) {
            RetVal2<TrackId, AudioParams> ret = playback()->addTrack(seqId, trackName, playbackData, params);
            channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
        };

        AudioResourceType resourceType = params.in.resourceMeta.type;
        // Not Fluid
        if (resourceType != AudioResourceType::FluidSoundfont) {
            addTrackAndSendResponce(msg, seqId, trackName, playbackData, params);
        }
        // Fluid
        else {
            AudioResourceId sfname = params.in.resourceMeta.id;
            if (soundFontRepository()->isSoundFontLoaded(sfname)) {
                addTrackAndSendResponce(msg, seqId, trackName, playbackData, params);
            }
            // Waiting for SF to load
            else {
                LOGI() << "Waiting for SF to load, trackName: " << trackName;
                m_pendingTracks[sfname].emplace_back(PendingTrack { msg, seqId, trackName, playbackData, params });
                soundFontRepository()->soundFontsChanged().onNotify(this,
                                                                    [this, sfname, addTrackAndSendResponce]() {
                    LOGD() << "isSoundFont: " << sfname << ", loaded: " << soundFontRepository()->isSoundFontLoaded(sfname);
                    if (soundFontRepository()->isSoundFontLoaded(sfname)) {
                        auto it = m_pendingTracks.find(sfname);
                        if (it != m_pendingTracks.end()) {
                            for (const PendingTrack& t : it->second) {
                                addTrackAndSendResponce(t.msg, t.seqId, t.trackName, t.playbackData, t.params);
                            }
                            it->second.clear();
                        }

                        soundFontRepository()->soundFontsChanged().resetOnNotify(this);
                    }
                });
            }
        }
    });

    onMethod(Method::AddTrackWithIODevice, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        TrackName trackName;
        uint64_t devicePtr = 0;
        AudioParams params;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackName, devicePtr, params)) {
            return;
        }
        io::IODevice* device = reinterpret_cast<io::IODevice*>(devicePtr);

        RetVal2<TrackId, AudioParams> ret = playback()->addTrack(seqId, trackName, device, params);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    onMethod(Method::AddAuxTrack, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        TrackName trackName;
        AudioOutputParams outputParams;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackName, outputParams)) {
            return;
        }
        RetVal2<TrackId, AudioOutputParams> ret = playback()->addAuxTrack(seqId, trackName, outputParams);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    onMethod(Method::RemoveTrack, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackId)) {
            return;
        }
        playback()->removeTrack(seqId, trackId);
    });

    onMethod(Method::RemoveAllTracks, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId)) {
            return;
        }
        playback()->removeAllTracks(seqId);
    });

    onMethod(Method::GetAvailableInputResources, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        AudioResourceMetaList list = playback()->availableInputResources();
        //! NOTE The list can be large.
        //! There can be many re-allocations, because of this it takes a long time to pack.
        //! Let's add a reserve. 300 is approximately the size of one element, we get empirically.
        channel()->send(rpc::make_response(msg, RpcPacker::pack(rpc::Options { list.size() * 300 }, list)));
    });

    onMethod(Method::GetAvailableSoundPresets, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        AudioResourceMeta meta;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, meta)) {
            return;
        }
        SoundPresetList list = playback()->availableSoundPresets(meta);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(list)));
    });

    onMethod(Method::GetInputParams, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackId)) {
            return;
        }
        RetVal<AudioInputParams> ret = playback()->inputParams(seqId, trackId);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    onMethod(Method::SetInputParams, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        TrackId trackId = 0;
        AudioInputParams params;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackId, params)) {
            return;
        }
        playback()->setInputParams(seqId, trackId, params);
    });

    onMethod(Method::ProcessInput, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackId)) {
            return;
        }

        playback()->processInput(seqId, trackId);
    });

    onMethod(Method::GetInputProcessingProgress, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackId)) {
            return;
        }

        RetVal<InputProcessingProgress> ret = playback()->inputProcessingProgress(seqId, trackId);
        StreamId streamId = 0;
        if (ret.ret) {
            streamId = channel()->addSendStream(StreamName::InputProcessingProgressStream, ret.val.processedChannel);
        }

        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret.ret, ret.val.isStarted, streamId)));
    });

    onMethod(Method::ClearCache, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackId)) {
            return;
        }

        playback()->clearCache(seqId, trackId);
    });

    onMethod(Method::ClearSources, [this](const Msg&) {
        ONLY_AUDIO_RPC_THREAD;
        playback()->clearSources();
    });

    // Play
    onMethod(Method::Play, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        secs_t delay = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, delay)) {
            return;
        }
        playback()->play(seqId, delay);
    });

    onMethod(Method::Seek, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        secs_t newPosition = 0;
        bool flushSound = false;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, newPosition, flushSound)) {
            return;
        }
        playback()->seek(seqId, newPosition, flushSound);
    });

    onMethod(Method::Stop, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId)) {
            return;
        }
        playback()->stop(seqId);
    });

    onMethod(Method::Pause, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId)) {
            return;
        }
        playback()->pause(seqId);
    });

    onMethod(Method::Resume, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        secs_t delay = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, delay)) {
            return;
        }
        playback()->resume(seqId, delay);
    });

    onMethod(Method::SetDuration, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        msecs_t durationMsec = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, durationMsec)) {
            return;
        }
        playback()->setDuration(seqId, durationMsec);
    });

    onMethod(Method::SetLoop, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        msecs_t fromMsec = 0;
        msecs_t toMsec = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, fromMsec, toMsec)) {
            return;
        }
        Ret ret = playback()->setLoop(seqId, fromMsec, toMsec);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    onMethod(Method::ResetLoop, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId)) {
            return;
        }
        playback()->resetLoop(seqId);
    });

    onMethod(Method::GetPlaybackStatus, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId)) {
            return;
        }

        PlaybackStatus status = playback()->playbackStatus(seqId);
        async::Channel<PlaybackStatus> ch = playback()->playbackStatusChanged(seqId);
        StreamId streamId = channel()->addSendStream(StreamName::PlaybackStatusStream, ch);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(status, streamId)));
    });

    onMethod(Method::GetPlaybackPosition, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId)) {
            return;
        }

        secs_t pos = playback()->playbackPosition(seqId);
        async::Channel<secs_t> ch = playback()->playbackPositionChanged(seqId);
        StreamId streamId = channel()->addSendStream(StreamName::PlaybackPositionStream, ch);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(pos, streamId)));
    });

    // Output

    onMethod(Method::GetOutputParams, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackId)) {
            return;
        }
        RetVal<AudioOutputParams> ret = playback()->outputParams(seqId, trackId);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    onMethod(Method::SetOutputParams, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        TrackId trackId = 0;
        AudioOutputParams params;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackId, params)) {
            return;
        }
        playback()->setOutputParams(seqId, trackId, params);
    });

    onMethod(Method::GetMasterOutputParams, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        RetVal<AudioOutputParams> ret = playback()->masterOutputParams();
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    onMethod(Method::SetMasterOutputParams, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        AudioOutputParams params;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, params)) {
            return;
        }
        playback()->setMasterOutputParams(params);
    });

    onMethod(Method::ClearMasterOutputParams, [this](const Msg&) {
        ONLY_AUDIO_RPC_THREAD;
        playback()->clearMasterOutputParams();
    });

    onMethod(Method::GetAvailableOutputResources, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        AudioResourceMetaList list = playback()->availableOutputResources();
        channel()->send(rpc::make_response(msg, RpcPacker::pack(list)));
    });

    onMethod(Method::GetSignalChanges, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackId)) {
            return;
        }

        RetVal<AudioSignalChanges> ret = playback()->signalChanges(seqId, trackId);
        StreamId streamId = 0;
        if (ret.ret) {
            streamId = channel()->addSendStream(StreamName::AudioSignalStream, ret.val);
        }

        RetVal<StreamId> res;
        res.ret = ret.ret;
        res.val = streamId;

        channel()->send(rpc::make_response(msg, RpcPacker::pack(res)));
    });

    onMethod(Method::GetMasterSignalChanges, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        RetVal<AudioSignalChanges> ret = playback()->masterSignalChanges();
        StreamId streamId = 0;
        if (ret.ret) {
            streamId = channel()->addSendStream(StreamName::AudioMasterSignalStream, ret.val);
        }

        RetVal<StreamId> res;
        res.ret = ret.ret;
        res.val = streamId;

        channel()->send(rpc::make_response(msg, RpcPacker::pack(res)));
    });

    onMethod(Method::SaveSoundTrack, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        io::path_t destination;
        SoundTrackFormat format;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, destination, format)) {
            return;
        }
        Ret ret = playback()->saveSoundTrack(seqId, destination, format);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    onMethod(Method::AbortSavingAllSoundTracks, [this](const Msg&) {
        ONLY_AUDIO_RPC_THREAD;
        playback()->abortSavingAllSoundTracks();
    });

    onMethod(Method::GetSaveSoundTrackProgress, [this](const Msg& msg) {
        ONLY_AUDIO_RPC_THREAD;
        TrackSequenceId seqId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId)) {
            return;
        }

        async::Channel<int64_t, int64_t> ch = playback()->saveSoundTrackProgressChanged(seqId);
        ch.onReceive(this, [this, seqId](int64_t current, int64_t total) {
            m_saveSoundTrackProgressStream.send(seqId, current, total);
        });

        if (m_saveSoundTrackProgressStreamId == 0) {
            m_saveSoundTrackProgressStreamId = channel()->addSendStream(StreamName::SaveSoundTrackProgressStream,
                                                                        m_saveSoundTrackProgressStream);
        }

        channel()->send(rpc::make_response(msg, RpcPacker::pack(m_saveSoundTrackProgressStreamId)));
    });

    onMethod(Method::ClearAllFx, [this](const Msg&) {
        ONLY_AUDIO_RPC_THREAD;
        playback()->clearAllFx();
    });
}

void EngineRpcController::onMethod(rpc::Method method, rpc::Handler handler)
{
    m_usedMethods.push_back(method);

    channel()->onMethod(method, [handler](const Msg& msg) {
        //! TODO pre
        handler(msg);
        //! TODO post
    });
}

void EngineRpcController::deinit()
{
    ONLY_AUDIO_RPC_THREAD;

    playback()->trackAdded().resetOnReceive(this);
    playback()->trackRemoved().resetOnReceive(this);
    playback()->inputParamsChanged().resetOnReceive(this);
    playback()->outputParamsChanged().resetOnReceive(this);

    for (const Method& m : m_usedMethods) {
        channel()->onMethod(m, nullptr);
    }
    m_usedMethods.clear();
}
