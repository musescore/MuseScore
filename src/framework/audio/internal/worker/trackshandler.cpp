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

#include "trackshandler.h"

#include "global/async/async.h"

#include "internal/audiothread.h"
#include "internal/audiosanitizer.h"
#include "audioerrors.h"

#include "log.h"

using namespace muse::audio;
using namespace muse::async;

TracksHandler::TracksHandler(IGetTrackSequence* getSequence, const modularity::ContextPtr& iocCtx)
    : Injectable(iocCtx), m_getSequence(getSequence)
{
}

TracksHandler::~TracksHandler()
{
    m_getSequence = nullptr;
}

Promise<TrackIdList> TracksHandler::trackIdList(const TrackSequenceId sequenceId) const
{
    return Promise<TrackIdList>([this, sequenceId](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);

        if (s) {
            return resolve(s->trackIdList());
        } else {
            return reject(static_cast<int>(Err::InvalidSequenceId), "invalid sequence id");
        }
    }, AudioThread::ID);
}

Promise<TrackName> TracksHandler::trackName(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    return Promise<TrackName>([this, sequenceId, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);

        if (s) {
            return resolve(s->trackName(trackId));
        } else {
            return reject(static_cast<int>(Err::InvalidSequenceId), "invalid sequence id");
        }
    }, AudioThread::ID);
}

Promise<TrackId, AudioParams> TracksHandler::addTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                                      io::IODevice* playbackData,
                                                      AudioParams&& params)
{
    return Promise<TrackId, AudioParams>([this, sequenceId, trackName, playbackData, params](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);

        if (!s) {
            return reject(static_cast<int>(Err::InvalidSequenceId), "invalid sequence id");
        }

        RetVal2<TrackId, AudioParams> result = s->addTrack(trackName, playbackData, params);

        if (!result.ret) {
            return reject(result.ret.code(), result.ret.text());
        }

        return resolve(result.val1, result.val2);
    }, AudioThread::ID);
}

Promise<TrackId, AudioParams> TracksHandler::addTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                                      const mpe::PlaybackData& playbackData, AudioParams&& params)
{
    return Promise<TrackId, AudioParams>([this, sequenceId, trackName, playbackData, params](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);

        if (!s) {
            return reject(static_cast<int>(Err::InvalidSequenceId), "invalid sequence id");
        }

        RetVal2<TrackId, AudioParams> result = s->addTrack(trackName, playbackData, params);

        if (!result.ret) {
            return reject(result.ret.code(), result.ret.text());
        }

        return resolve(result.val1, result.val2);
    }, AudioThread::ID);
}

Promise<TrackId, AudioOutputParams> TracksHandler::addAuxTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                                               const AudioOutputParams& outputParams)
{
    return Promise<TrackId, AudioOutputParams>([this, sequenceId, trackName, outputParams](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);

        if (!s) {
            return reject(static_cast<int>(Err::InvalidSequenceId), "invalid sequence id");
        }

        RetVal2<TrackId, AudioOutputParams> result = s->addAuxTrack(trackName, outputParams);

        if (!result.ret) {
            return reject(result.ret.code(), result.ret.text());
        }

        return resolve(result.val1, result.val2);
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

Promise<AudioResourceMetaList> TracksHandler::availableInputResources() const
{
    return Promise<AudioResourceMetaList>([this](auto resolve, auto /*reject*/) {
        ONLY_AUDIO_WORKER_THREAD;

        return resolve(resolver()->resolveAvailableResources());
    }, AudioThread::ID);
}

Promise<SoundPresetList> TracksHandler::availableSoundPresets(const AudioResourceMeta& resourceMeta) const
{
    return Promise<SoundPresetList>([this, resourceMeta](auto resolve, auto /*reject*/) {
        ONLY_AUDIO_WORKER_THREAD;

        return resolve(resolver()->resolveAvailableSoundPresets(resourceMeta));
    }, AudioThread::ID);
}

Promise<AudioInputParams> TracksHandler::inputParams(const TrackSequenceId sequenceId, const TrackId trackId) const
{
    return Promise<AudioInputParams>([this, sequenceId, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);

        if (!s) {
            return reject(static_cast<int>(Err::InvalidSequenceId), "invalid sequence id");
        }

        RetVal<AudioInputParams> result = s->audioIO()->inputParams(trackId);

        if (!result.ret) {
            return reject(result.ret.code(), result.ret.text());
        }

        return resolve(result.val);
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

muse::async::Promise<InputProcessingProgress> TracksHandler::inputProcessingProgress(const TrackSequenceId sequenceId,
                                                                                     const TrackId trackId) const
{
    return Promise<InputProcessingProgress>([this, sequenceId, trackId](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        const ITrackSequencePtr s = sequence(sequenceId);
        if (!s) {
            return reject(static_cast<int>(Err::InvalidSequenceId), "invalid sequence id");
        }

        if (!s->audioIO()->hasTrack(trackId)) {
            return reject(static_cast<int>(Err::InvalidTrackId), "no track");
        }

        return resolve(s->audioIO()->inputProcessingProgress(trackId));
    }, AudioThread::ID);
}

void TracksHandler::clearSources()
{
    resolver()->clearSources();
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

    TrackSequenceId sequenceId = s->id();

    if (!s->audioIO()->inputParamsChanged().isConnected()) {
        s->audioIO()->inputParamsChanged().onReceive(this, [this, sequenceId](const TrackId trackId, const AudioInputParams& params) {
            m_inputParamsChanged.send(sequenceId, trackId, params);
        });
    }

    if (!s->trackAdded().isConnected()) {
        s->trackAdded().onReceive(this, [this, sequenceId](const TrackId trackId) {
            m_trackAdded.send(sequenceId, trackId);
        });
    }

    if (!s->trackRemoved().isConnected()) {
        s->trackRemoved().onReceive(this, [this, sequenceId](const TrackId trackId) {
            m_trackRemoved.send(sequenceId, trackId);
        });
    }
}
