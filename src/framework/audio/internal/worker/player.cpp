/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#include "player.h"

#include "global/async/async.h"

#include "internal/audiosanitizer.h"
#include "internal/audiothread.h"
#include "audioerrors.h"

#include "log.h"

using namespace muse;
using namespace muse::async;
using namespace muse::audio;

Player::Player(const IGetTrackSequence* getSeq, const TrackSequenceId sequenceId)
    : m_getSequence(getSeq), m_sequenceId(sequenceId)
{
}

ITrackSequencePtr Player::seq() const
{
    ONLY_AUDIO_WORKER_THREAD;

    if (m_seq) {
        return m_seq;
    }

    IF_ASSERT_FAILED(m_getSequence) {
        return nullptr;
    }

    m_seq = m_getSequence->sequence(m_sequenceId);
    if (!m_seq) {
        return nullptr;
    }

    m_seq->player()->playbackPositionChanged().onReceive(this, [this](const secs_t newPos) {
        m_playbackPositionChanged.send(newPos);
    });

    m_seq->player()->playbackStatusChanged().onReceive(this, [this](const PlaybackStatus newStatus) {
        m_playbackStatusChanged.send(newStatus);
    });

    return m_seq;
}

TrackSequenceId Player::sequenceId() const
{
    return m_sequenceId;
}

void Player::play()
{
    Async::call(this, [this]() {
        ONLY_AUDIO_WORKER_THREAD;
        ITrackSequencePtr s = seq();
        if (s) {
            s->player()->play();
        }
    }, AudioThread::ID);
}

void Player::seek(const msecs_t newPositionMsecs)
{
    Async::call(this, [this, newPositionMsecs]() {
        ONLY_AUDIO_WORKER_THREAD;
        ITrackSequencePtr s = seq();
        if (s) {
            s->player()->seek(newPositionMsecs);
        }
    }, AudioThread::ID);
}

void Player::stop()
{
    Async::call(this, [this]() {
        ONLY_AUDIO_WORKER_THREAD;
        ITrackSequencePtr s = seq();
        if (s) {
            s->player()->stop();
        }
    }, AudioThread::ID);
}

void Player::pause()
{
    Async::call(this, [this]() {
        ONLY_AUDIO_WORKER_THREAD;
        ITrackSequencePtr s = seq();
        if (s) {
            s->player()->pause();
        }
    }, AudioThread::ID);
}

void Player::resume()
{
    Async::call(this, [this]() {
        ONLY_AUDIO_WORKER_THREAD;
        ITrackSequencePtr s = seq();
        if (s) {
            s->player()->resume();
        }
    }, AudioThread::ID);
}

void Player::setDuration(const msecs_t durationMsec)
{
    Async::call(this, [this, durationMsec]() {
        ONLY_AUDIO_WORKER_THREAD;
        ITrackSequencePtr s = seq();
        if (s) {
            s->player()->setDuration(durationMsec);
        }
    }, AudioThread::ID);
}

async::Promise<bool> Player::setLoop(const msecs_t fromMsec, const msecs_t toMsec)
{
    return Promise<bool>([this, fromMsec, toMsec](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = seq();

        if (!s) {
            return reject(static_cast<int>(Err::InvalidSequenceId), "invalid sequence id");
        }

        Ret result = s->player()->setLoop(fromMsec, toMsec);
        if (!result) {
            return reject(result.code(), result.text());
        }

        return resolve(result);
    }, AudioThread::ID);
}

void Player::resetLoop()
{
    Async::call(this, [this]() {
        ONLY_AUDIO_WORKER_THREAD;
        ITrackSequencePtr s = seq();
        if (s) {
            s->player()->resetLoop();
        }
    }, AudioThread::ID);
}

async::Promise<secs_t> Player::playbackPosition() const
{
    return Promise<secs_t>([this](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = seq();

        if (!s) {
            return reject(static_cast<int>(Err::InvalidSequenceId), "invalid sequence id");
        }

        secs_t pos = s->player()->playbackPosition();
        return resolve(pos);
    }, AudioThread::ID);
}

async::Channel<secs_t> Player::playbackPositionChanged() const
{
    return m_playbackPositionChanged;
}

async::Channel<PlaybackStatus> Player::playbackStatusChanged() const
{
    return m_playbackStatusChanged;
}
