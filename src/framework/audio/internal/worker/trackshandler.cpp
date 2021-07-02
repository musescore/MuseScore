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

#include "trackshandler.h"

#include "log.h"
#include "async/async.h"
#include "midi/miditypes.h"

#include "internal/audiothread.h"
#include "internal/audiosanitizer.h"
#include "audioerrors.h"

using namespace mu::audio;
using namespace mu::async;

TracksHandler::TracksHandler(IGetTrackSequence* getSequence)
    : m_getSequence(getSequence)
{
}

TracksHandler::~TracksHandler()
{
    m_getSequence = nullptr;
}

Promise<TrackIdList> TracksHandler::trackIdList(const TrackSequenceId sequenceId) const
{
    return Promise<TrackIdList>([this, sequenceId](Promise<TrackIdList>::Resolve resolve, Promise<TrackIdList>::Reject reject) {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);

        if (s) {
            resolve(s->trackIdList());
        } else {
            reject(static_cast<int>(Err::InvalidSequenceId), "invalid sequence id");
        }
    }, AudioThread::ID);
}

Promise<TrackId> TracksHandler::addTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                         midi::MidiData&& inParams, AudioOutputParams&& outParams)
{
    return Promise<TrackId>([this, sequenceId, trackName, inParams, outParams](Promise<TrackId>::Resolve resolve,
                                                                               Promise<TrackId>::Reject reject) {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);

        if (!s) {
            reject(static_cast<int>(Err::InvalidSequenceId), "invalid sequence id");
        }

        RetVal<TrackId> result = s->addTrack(trackName, inParams, outParams);

        if (!result.ret) {
            reject(result.ret.code(), result.ret.text());
        }

        resolve(result.val);
    }, AudioThread::ID);
}

Promise<TrackId> TracksHandler::addTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                         io::path&& inParams, AudioOutputParams&& outParams)
{
    return Promise<TrackId>([this, sequenceId, trackName, inParams, outParams](Promise<TrackId>::Resolve resolve,
                                                                               Promise<TrackId>::Reject reject) {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);

        if (!s) {
            reject(static_cast<int>(Err::InvalidSequenceId), "invalid sequence id");
        }

        RetVal<TrackId> result = s->addTrack(trackName, inParams, outParams);

        if (!result.ret) {
            reject(result.ret.code(), result.ret.text());
        }

        resolve(result.val);
    }, AudioThread::ID);
}

void TracksHandler::removeTrack(const TrackSequenceId sequenceId, const TrackId trackId)
{
    Async::call(this, [this, sequenceId, trackId]() {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);

        if (!s) {
            return;
        }

        s->removeTrack(trackId);
    }, AudioThread::ID);
}

void TracksHandler::removeAllTracks(const TrackSequenceId sequenceId)
{
    Async::call(this, [this, sequenceId]() {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);

        if (!s) {
            return;
        }

        for (const TrackId& id : s->trackIdList()) {
            s->removeTrack(id);
        }
    }, AudioThread::ID);
}

Channel<TrackSequenceId, TrackId> TracksHandler::trackAdded() const
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;

    return m_trackAdded;
}

Channel<TrackSequenceId, TrackId> TracksHandler::trackRemoved() const
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;

    return m_trackRemoved;
}

Promise<AudioInputParams> TracksHandler::inputParams(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    return Promise<AudioInputParams>([this, sequenceId, trackId](Promise<AudioInputParams>::Resolve resolve,
                                                                 Promise<AudioInputParams>::Reject reject) {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);

        if (!s) {
            reject(static_cast<int>(Err::InvalidSequenceId), "invalid sequence id");
        }

        RetVal<AudioInputParams> result = s->audioIO()->inputParams(trackId);

        if (!result.ret) {
            reject(result.ret.code(), result.ret.text());
        }

        resolve(result.val);
    }, AudioThread::ID);
}

void TracksHandler::setInputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioInputParams& params)
{
    Async::call(this, [this, sequenceId, trackId, params]() {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);

        if (s) {
            s->audioIO()->setInputParams(trackId, params);
        }
    }, AudioThread::ID);
}

Channel<TrackSequenceId, TrackId, AudioInputParams> TracksHandler::inputParamsChanged() const
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;

    return m_inputParamsChanged;
}

ITrackSequencePtr TracksHandler::sequence(const TrackSequenceId id) const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_getSequence) {
        return nullptr;
    }

    ITrackSequencePtr s = m_getSequence->sequence(id);
    ensureSubscriptions(s);

    return s;
}

void TracksHandler::ensureSubscriptions(const ITrackSequencePtr s) const
{
    ONLY_AUDIO_WORKER_THREAD;

    if (!s) {
        return;
    }

    if (!s->audioIO()->inputParamsChanged().isConnected()) {
        s->audioIO()->inputParamsChanged().onReceive(this, [this, s](const TrackId trackId, const AudioInputParams& params) {
            m_inputParamsChanged.send(s->id(), trackId, params);
        });
    }

    if (!s->trackAdded().isConnected()) {
        s->trackAdded().onReceive(this, [this, s](const TrackId trackId) {
            m_trackAdded.send(s->id(), trackId);
        });
    }

    if (!s->trackRemoved().isConnected()) {
        s->trackRemoved().onReceive(this, [this, s](const TrackId trackId) {
            m_trackRemoved.send(s->id(), trackId);
        });
    }
}
