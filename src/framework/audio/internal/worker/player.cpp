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

void Player::init()
{
    ONLY_AUDIO_MAIN_THREAD;

    //! NOTE Subscribe and request initial state

    m_playbackStatusChanged.onReceive(this, [this](PlaybackStatus st) {
        m_playbackStatus = st;
    });

    m_playbackPositionChanged.onReceive(this, [this](const secs_t newPos) {
        m_playbackPosition = newPos;
    });

    Async::call(this, [this]() {
        ONLY_AUDIO_WORKER_THREAD;

        ITrackSequencePtr s = seq();

        //! NOTE Send initial state
        m_playbackStatusChanged.send(s->player()->playbackStatus());
        s->player()->playbackStatusChanged().onReceive(this, [this](const PlaybackStatus newStatus) {
            m_playbackStatusChanged.send(newStatus);
        });

        //! NOTE Send initial state
        m_playbackPositionChanged.send(s->player()->playbackPosition());
        s->player()->playbackPositionChanged().onReceive(this, [this](const secs_t newPos) {
            m_playbackPositionChanged.send(newPos);
        });
    }, AudioThread::ID);
}

ITrackSequencePtr Player::seq() const
{
    ONLY_AUDIO_WORKER_THREAD;

    IF_ASSERT_FAILED(m_getSequence) {
        return nullptr;
    }

    return m_getSequence->sequence(m_sequenceId);
}

TrackSequenceId Player::sequenceId() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return m_sequenceId;
}

void Player::play()
{
    ONLY_AUDIO_MAIN_THREAD;

    Async::call(this, [this]() {
        ONLY_AUDIO_WORKER_THREAD;
        ITrackSequencePtr s = seq();
        if (s) {
            s->player()->play();
        }
    }, AudioThread::ID);
}

void Player::seek(const secs_t newPosition)
{
    ONLY_AUDIO_MAIN_THREAD;

    Async::call(this, [this, newPosition]() {
        ONLY_AUDIO_WORKER_THREAD;
        ITrackSequencePtr s = seq();
        if (s) {
            s->player()->seek(newPosition);
        }
    }, AudioThread::ID);
}

void Player::stop()
{
    ONLY_AUDIO_MAIN_THREAD;

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
    ONLY_AUDIO_MAIN_THREAD;

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
    ONLY_AUDIO_MAIN_THREAD;

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
    ONLY_AUDIO_MAIN_THREAD;

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
    ONLY_AUDIO_MAIN_THREAD;

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
    ONLY_AUDIO_MAIN_THREAD;

    Async::call(this, [this]() {
        ONLY_AUDIO_WORKER_THREAD;
        ITrackSequencePtr s = seq();
        if (s) {
            s->player()->resetLoop();
        }
    }, AudioThread::ID);
}

secs_t Player::playbackPosition() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return m_playbackPosition;
}

async::Channel<secs_t> Player::playbackPositionChanged() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return m_playbackPositionChanged;
}

PlaybackStatus Player::playbackStatus() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return m_playbackStatus;
}

async::Channel<PlaybackStatus> Player::playbackStatusChanged() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return m_playbackStatusChanged;
}
