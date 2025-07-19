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

Player::Player(const TrackSequenceId sequenceId)
    : m_sequenceId(sequenceId)
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

        //! NOTE Send initial state
        m_playbackStatusChanged.send(workerPlayback()->playbackStatus(m_sequenceId));
        workerPlayback()->playbackStatusChanged(m_sequenceId).onReceive(this, [this](const PlaybackStatus newStatus) {
            m_playbackStatusChanged.send(newStatus);
        });

        //! NOTE Send initial state
        m_playbackPositionChanged.send(workerPlayback()->playbackPosition(m_sequenceId));
        workerPlayback()->playbackPositionChanged(m_sequenceId).onReceive(this, [this](const secs_t newPos) {
            m_playbackPositionChanged.send(newPos);
        });
    }, AudioThread::ID);
}

TrackSequenceId Player::sequenceId() const
{
    ONLY_AUDIO_MAIN_THREAD;
    return m_sequenceId;
}

void Player::play(const secs_t delay)
{
    ONLY_AUDIO_MAIN_THREAD;

    Async::call(this, [this, delay]() {
        ONLY_AUDIO_WORKER_THREAD;
        workerPlayback()->play(m_sequenceId, delay);
    }, AudioThread::ID);
}

void Player::seek(const secs_t newPosition)
{
    ONLY_AUDIO_MAIN_THREAD;

    Async::call(this, [this, newPosition]() {
        ONLY_AUDIO_WORKER_THREAD;
        workerPlayback()->seek(m_sequenceId, newPosition);
    }, AudioThread::ID);
}

void Player::stop()
{
    ONLY_AUDIO_MAIN_THREAD;

    Async::call(this, [this]() {
        ONLY_AUDIO_WORKER_THREAD;
        workerPlayback()->stop(m_sequenceId);
    }, AudioThread::ID);
}

void Player::pause()
{
    ONLY_AUDIO_MAIN_THREAD;

    Async::call(this, [this]() {
        ONLY_AUDIO_WORKER_THREAD;
        workerPlayback()->pause(m_sequenceId);
    }, AudioThread::ID);
}

void Player::resume(const secs_t delay)
{
    ONLY_AUDIO_MAIN_THREAD;

    Async::call(this, [this, delay]() {
        ONLY_AUDIO_WORKER_THREAD;
        workerPlayback()->resume(m_sequenceId, delay);
    }, AudioThread::ID);
}

void Player::setDuration(const msecs_t durationMsec)
{
    ONLY_AUDIO_MAIN_THREAD;

    Async::call(this, [this, durationMsec]() {
        ONLY_AUDIO_WORKER_THREAD;
        workerPlayback()->setDuration(m_sequenceId, durationMsec);
    }, AudioThread::ID);
}

async::Promise<bool> Player::setLoop(const msecs_t fromMsec, const msecs_t toMsec)
{
    ONLY_AUDIO_MAIN_THREAD;

    return Promise<bool>([this, fromMsec, toMsec](auto resolve, auto reject) {
        ONLY_AUDIO_WORKER_THREAD;

        Ret ret = workerPlayback()->setLoop(m_sequenceId, fromMsec, toMsec);
        if (ret) {
            return resolve(true);
        } else {
            return reject(ret.code(), ret.text());
        }
    }, AudioThread::ID);
}

void Player::resetLoop()
{
    ONLY_AUDIO_MAIN_THREAD;

    Async::call(this, [this]() {
        ONLY_AUDIO_WORKER_THREAD;
        workerPlayback()->resetLoop(m_sequenceId);
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
