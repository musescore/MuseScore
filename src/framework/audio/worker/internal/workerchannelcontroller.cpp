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
#include "workerchannelcontroller.h"

#include "audio/common/audiosanitizer.h"
#include "audio/common/rpc/rpcpacker.h"

#include "log.h"

using namespace muse::audio::worker;
using namespace muse::audio::rpc;

void WorkerChannelController::init(std::shared_ptr<IWorkerPlayback> playback)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_playback = playback;

    // Notification
    m_playback->trackAdded().onReceive(this, [this](TrackSequenceId sequenceId, TrackId trackId) {
        channel()->send(rpc::make_notification(Method::TrackAdded, RpcPacker::pack(sequenceId, trackId)));
    });

    m_playback->trackRemoved().onReceive(this, [this](TrackSequenceId sequenceId, TrackId trackId) {
        channel()->send(rpc::make_notification(Method::TrackRemoved, RpcPacker::pack(sequenceId, trackId)));
    });

    m_playback->inputParamsChanged().onReceive(this, [this](TrackSequenceId sequenceId, TrackId trackId,
                                                            const AudioInputParams& params) {
        channel()->send(rpc::make_notification(Method::InputParamsChanged, RpcPacker::pack(sequenceId, trackId, params)));
    });

    m_playback->outputParamsChanged().onReceive(this, [this](TrackSequenceId sequenceId, TrackId trackId,
                                                             const AudioOutputParams& params) {
        channel()->send(rpc::make_notification(Method::OutputParamsChanged, RpcPacker::pack(sequenceId, trackId, params)));
    });

    m_playback->masterOutputParamsChanged().onReceive(this, [this](const AudioOutputParams& params) {
        channel()->send(rpc::make_notification(Method::MasterOutputParamsChanged, RpcPacker::pack(params)));
    });

    // Sequences
    channel()->onMethod(Method::AddSequence, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = m_playback->addSequence();
        channel()->send(rpc::make_response(msg, RpcPacker::pack(seqId)));
    });

    channel()->onMethod(Method::RemoveSequence, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId)) {
            return;
        }
        m_playback->removeSequence(seqId);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(true)));
    });

    channel()->onMethod(Method::GetSequenceIdList, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceIdList list = m_playback->sequenceIdList();
        channel()->send(rpc::make_response(msg, RpcPacker::pack(list)));
    });

    // Tracks
    channel()->onMethod(Method::GetTrackIdList, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId)) {
            return;
        }
        RetVal<TrackIdList> ret = m_playback->trackIdList(seqId);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    channel()->onMethod(Method::GetTrackName, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackId)) {
            return;
        }
        RetVal<TrackName> ret = m_playback->trackName(seqId, trackId);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    channel()->onMethod(Method::AddTrackWithPlaybackData, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
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

        RetVal2<TrackId, AudioParams> ret = m_playback->addTrack(seqId, trackName, playbackData, params);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    channel()->onMethod(Method::AddTrackWithIODevice, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        TrackName trackName;
        uint64_t devicePtr = 0;
        AudioParams params;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackName, devicePtr, params)) {
            return;
        }
        io::IODevice* device = reinterpret_cast<io::IODevice*>(devicePtr);

        RetVal2<TrackId, AudioParams> ret = m_playback->addTrack(seqId, trackName, device, params);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    channel()->onMethod(Method::AddAuxTrack, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        TrackName trackName;
        AudioOutputParams outputParams;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackName, outputParams)) {
            return;
        }
        RetVal2<TrackId, AudioOutputParams> ret = m_playback->addAuxTrack(seqId, trackName, outputParams);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    channel()->onMethod(Method::RemoveTrack, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackId)) {
            return;
        }
        m_playback->removeTrack(seqId, trackId);
    });

    channel()->onMethod(Method::RemoveAllTracks, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId)) {
            return;
        }
        m_playback->removeAllTracks(seqId);
    });

    channel()->onMethod(Method::GetAvailableInputResources, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        AudioResourceMetaList list = m_playback->availableInputResources();
        channel()->send(rpc::make_response(msg, RpcPacker::pack(list)));
    });

    channel()->onMethod(Method::GetAvailableSoundPresets, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        AudioResourceMeta meta;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, meta)) {
            return;
        }
        SoundPresetList list = m_playback->availableSoundPresets(meta);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(list)));
    });

    channel()->onMethod(Method::GetInputParams, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackId)) {
            return;
        }
        RetVal<AudioInputParams> ret = m_playback->inputParams(seqId, trackId);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    channel()->onMethod(Method::SetInputParams, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        TrackId trackId = 0;
        AudioInputParams params;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackId, params)) {
            return;
        }
        m_playback->setInputParams(seqId, trackId, params);
    });

    channel()->onMethod(Method::GetInputProcessingProgress, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackId)) {
            return;
        }

        RetVal<InputProcessingProgress> ret = m_playback->inputProcessingProgress(seqId, trackId);
        StreamId streamId = 0;
        if (ret.ret) {
            streamId = channel()->addSendStream(StreamName::InputProcessingProgressStream, ret.val.processedChannel);
        }

        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret.ret, ret.val.isStarted, streamId)));
    });

    channel()->onMethod(Method::ClearSources, [this](const Msg&) {
        ONLY_AUDIO_WORKER_THREAD;
        m_playback->clearSources();
    });

    // Play
    channel()->onMethod(Method::Play, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        secs_t delay = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, delay)) {
            return;
        }
        m_playback->play(seqId, delay);
    });

    channel()->onMethod(Method::Seek, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        secs_t newPosition = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, newPosition)) {
            return;
        }
        m_playback->seek(seqId, newPosition);
    });

    channel()->onMethod(Method::Stop, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId)) {
            return;
        }
        m_playback->stop(seqId);
    });

    channel()->onMethod(Method::Pause, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId)) {
            return;
        }
        m_playback->pause(seqId);
    });

    channel()->onMethod(Method::Resume, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        secs_t delay = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, delay)) {
            return;
        }
        m_playback->resume(seqId, delay);
    });

    channel()->onMethod(Method::SetDuration, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        msecs_t durationMsec = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, durationMsec)) {
            return;
        }
        m_playback->setDuration(seqId, durationMsec);
    });

    channel()->onMethod(Method::SetLoop, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        msecs_t fromMsec = 0;
        msecs_t toMsec = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, fromMsec, toMsec)) {
            return;
        }
        Ret ret = m_playback->setLoop(seqId, fromMsec, toMsec);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    channel()->onMethod(Method::ResetLoop, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId)) {
            return;
        }
        m_playback->resetLoop(seqId);
    });

    channel()->onMethod(Method::GetPlaybackStatus, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId)) {
            return;
        }

        PlaybackStatus status = m_playback->playbackStatus(seqId);
        async::Channel<PlaybackStatus> ch = m_playback->playbackStatusChanged(seqId);
        StreamId streamId = channel()->addSendStream(StreamName::PlaybackStatusStream, ch);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(status, streamId)));
    });

    channel()->onMethod(Method::GetPlaybackPosition, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId)) {
            return;
        }

        secs_t pos = m_playback->playbackPosition(seqId);
        async::Channel<secs_t> ch = m_playback->playbackPositionChanged(seqId);
        StreamId streamId = channel()->addSendStream(StreamName::PlaybackPositionStream, ch);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(pos, streamId)));
    });

    // Output

    channel()->onMethod(Method::GetOutputParams, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackId)) {
            return;
        }
        RetVal<AudioOutputParams> ret = m_playback->outputParams(seqId, trackId);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    channel()->onMethod(Method::SetOutputParams, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        TrackId trackId = 0;
        AudioOutputParams params;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackId, params)) {
            return;
        }
        m_playback->setOutputParams(seqId, trackId, params);
    });

    channel()->onMethod(Method::GetMasterOutputParams, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        RetVal<AudioOutputParams> ret = m_playback->masterOutputParams();
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    channel()->onMethod(Method::SetMasterOutputParams, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        AudioOutputParams params;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, params)) {
            return;
        }
        m_playback->setMasterOutputParams(params);
    });

    channel()->onMethod(Method::ClearMasterOutputParams, [this](const Msg&) {
        ONLY_AUDIO_WORKER_THREAD;
        m_playback->clearMasterOutputParams();
    });

    channel()->onMethod(Method::GetAvailableOutputResources, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        AudioResourceMetaList list = m_playback->availableOutputResources();
        channel()->send(rpc::make_response(msg, RpcPacker::pack(list)));
    });

    channel()->onMethod(Method::GetSignalChanges, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        TrackId trackId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, trackId)) {
            return;
        }

        RetVal<AudioSignalChanges> ret = m_playback->signalChanges(seqId, trackId);
        StreamId streamId = 0;
        if (ret.ret) {
            streamId = channel()->addSendStream(StreamName::AudioSignalStream, ret.val);
        }

        RetVal<StreamId> res;
        res.ret = ret.ret;
        res.val = streamId;

        channel()->send(rpc::make_response(msg, RpcPacker::pack(res)));
    });

    channel()->onMethod(Method::GetMasterSignalChanges, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        RetVal<AudioSignalChanges> ret = m_playback->masterSignalChanges();
        StreamId streamId = 0;
        if (ret.ret) {
            streamId = channel()->addSendStream(StreamName::AudioMasterSignalStream, ret.val);
        }

        RetVal<StreamId> res;
        res.ret = ret.ret;
        res.val = streamId;

        channel()->send(rpc::make_response(msg, RpcPacker::pack(res)));
    });

    channel()->onMethod(Method::SaveSoundTrack, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        io::path_t destination;
        SoundTrackFormat format;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId, destination, format)) {
            return;
        }
        Ret ret = m_playback->saveSoundTrack(seqId, destination, format);
        channel()->send(rpc::make_response(msg, RpcPacker::pack(ret)));
    });

    channel()->onMethod(Method::AbortSavingAllSoundTracks, [this](const Msg&) {
        ONLY_AUDIO_WORKER_THREAD;
        m_playback->abortSavingAllSoundTracks();
    });

    channel()->onMethod(Method::GetSaveSoundTrackProgress, [this](const Msg& msg) {
        ONLY_AUDIO_WORKER_THREAD;
        TrackSequenceId seqId = 0;
        IF_ASSERT_FAILED(RpcPacker::unpack(msg.data, seqId)) {
            return;
        }

        async::Channel<int64_t, int64_t> ch = m_playback->saveSoundTrackProgressChanged(seqId);
        ch.onReceive(this, [this, seqId](int64_t current, int64_t total) {
            m_saveSoundTrackProgressStream.send(seqId, current, total);
        });

        if (m_saveSoundTrackProgressStreamId == 0) {
            m_saveSoundTrackProgressStreamId = channel()->addSendStream(StreamName::SaveSoundTrackProgressStream,
                                                                        m_saveSoundTrackProgressStream);
        }

        channel()->send(rpc::make_response(msg, RpcPacker::pack(m_saveSoundTrackProgressStreamId)));
    });

    channel()->onMethod(Method::ClearAllFx, [this](const Msg&) {
        ONLY_AUDIO_WORKER_THREAD;
        m_playback->clearAllFx();
    });
}

void WorkerChannelController::deinit()
{
    ONLY_AUDIO_WORKER_THREAD;

    m_playback->trackAdded().resetOnReceive(this);
    m_playback->trackRemoved().resetOnReceive(this);
    m_playback->inputParamsChanged().resetOnReceive(this);
    m_playback->outputParamsChanged().resetOnReceive(this);

    channel()->onMethod(Method::AddSequence, nullptr);
    channel()->onMethod(Method::RemoveSequence, nullptr);
    channel()->onMethod(Method::GetSequenceIdList, nullptr);

    channel()->onMethod(Method::GetTrackIdList, nullptr);
    channel()->onMethod(Method::GetTrackName, nullptr);
    channel()->onMethod(Method::AddTrackWithPlaybackData, nullptr);
    channel()->onMethod(Method::AddAuxTrack, nullptr);
    channel()->onMethod(Method::RemoveTrack, nullptr);
    channel()->onMethod(Method::RemoveAllTracks, nullptr);

    channel()->onMethod(Method::GetAvailableInputResources, nullptr);
    channel()->onMethod(Method::GetAvailableSoundPresets, nullptr);

    channel()->onMethod(Method::GetInputParams, nullptr);
    channel()->onMethod(Method::SetInputParams, nullptr);

    channel()->onMethod(Method::ClearSources, nullptr);

    channel()->onMethod(Method::Play, nullptr);
    channel()->onMethod(Method::Seek, nullptr);
    channel()->onMethod(Method::Stop, nullptr);
    channel()->onMethod(Method::Pause, nullptr);
    channel()->onMethod(Method::Resume, nullptr);
    channel()->onMethod(Method::SetDuration, nullptr);
    channel()->onMethod(Method::SetLoop, nullptr);
    channel()->onMethod(Method::ResetLoop, nullptr);

    channel()->onMethod(Method::GetOutputParams, nullptr);
    channel()->onMethod(Method::SetOutputParams, nullptr);
    channel()->onMethod(Method::GetMasterOutputParams, nullptr);
    channel()->onMethod(Method::SetMasterOutputParams, nullptr);
    channel()->onMethod(Method::ClearMasterOutputParams, nullptr);

    channel()->onMethod(Method::GetAvailableOutputResources, nullptr);
    channel()->onMethod(Method::SaveSoundTrack, nullptr);

    channel()->onMethod(Method::AbortSavingAllSoundTracks, nullptr);

    channel()->onMethod(Method::ClearAllFx, nullptr);
}
