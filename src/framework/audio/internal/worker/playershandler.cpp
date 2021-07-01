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

#include "playershandler.h"

#include "log.h"
#include "async/async.h"

#include "internal/audiosanitizer.h"
#include "internal/audiothread.h"
#include "audioerrors.h"

using namespace mu::audio;
using namespace mu::async;

PlayersHandler::PlayersHandler(IGetTrackSequence* getSequence)
    : m_getSequence(getSequence)
{
}

PlayersHandler::~PlayersHandler()
{
    m_getSequence = nullptr;
}

void PlayersHandler::play(const TrackSequenceId sequenceId)
{
    Async::call(this, [this, sequenceId]() {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);
        if (s) {
            s->player()->play();
        }
    }, AudioThread::ID);
}

void PlayersHandler::seek(const TrackSequenceId sequenceId, const msecs_t newPositionMsecs)
{
    Async::call(this, [this, sequenceId, newPositionMsecs]() {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);
        if (s) {
            s->player()->seek(newPositionMsecs);
        }
    }, AudioThread::ID);
}

void PlayersHandler::stop(const TrackSequenceId sequenceId)
{
    Async::call(this, [this, sequenceId]() {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);
        if (s) {
            s->player()->stop();
        }
    }, AudioThread::ID);
}

void PlayersHandler::pause(const TrackSequenceId sequenceId)
{
    Async::call(this, [this, sequenceId]() {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);
        if (s) {
            s->player()->pause();
        }
    }, AudioThread::ID);
}

Promise<bool> PlayersHandler::setLoop(const TrackSequenceId sequenceId, const msecs_t fromMsec, const msecs_t toMsec)
{
    return Promise<bool>([this, sequenceId, fromMsec, toMsec](Promise<bool>::Resolve resolve, Promise<bool>::Reject reject) {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);

        if (!s) {
            reject(static_cast<int>(Err::InvalidSequenceId), "invalid sequence id");
        }

        Ret result = s->player()->setLoop(fromMsec, toMsec);

        if (!result) {
            reject(result.code(), result.text());
        }

        resolve(result);
    }, AudioThread::ID);
}

void PlayersHandler::resetLoop(const TrackSequenceId sequenceId)
{
    Async::call(this, [this, sequenceId]() {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = sequence(sequenceId);
        if (s) {
            s->player()->resetLoop();
        }
    }, AudioThread::ID);
}

Channel<TrackSequenceId, msecs_t> PlayersHandler::playbackPositionMsecs() const
{
    ONLY_AUDIO_MAIN_OR_WORKER_THREAD;

    return m_playbackPositionMsecsChanged;
}

ITrackSequencePtr PlayersHandler::sequence(const TrackSequenceId id) const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_getSequence) {
        return nullptr;
    }

    ITrackSequencePtr s = m_getSequence->sequence(id);
    ensureSubscriptions(s);

    return s;
}

void PlayersHandler::ensureSubscriptions(const ITrackSequencePtr s) const
{
    ONLY_AUDIO_WORKER_THREAD;

    if (!s || s->player()->playbackPositionMSecs().isConnected()) {
        return;
    }

    s->player()->playbackPositionMSecs().onReceive(this, [this, s](const msecs_t newPosMsecs) {
        m_playbackPositionMsecsChanged.send(s->id(), newPosMsecs);
    });
}
