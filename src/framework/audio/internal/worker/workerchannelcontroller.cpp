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

#include "../audiosanitizer.h"
#include "../rpc/rpcpacker.h"

#include "log.h"

using namespace muse::audio::worker;
using namespace muse::audio::rpc;

void WorkerChannelController::initOnWroker(std::shared_ptr<IWorkerPlayback> playback)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_playback = playback;

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
}
