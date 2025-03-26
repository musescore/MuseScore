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

#include "sequenceplayer.h"

#include "internal/audiosanitizer.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::async;

SequencePlayer::SequencePlayer(IGetTracks* getTracks, IClockPtr clock, const modularity::ContextPtr& iocCtx)
    : Injectable(iocCtx), m_getTracks(getTracks), m_clock(clock)
{
    m_clock->seekOccurred().onNotify(this, [this]() {
        seekAllTracks(m_clock->currentTime());
    });

    m_clock->statusChanged().onReceive(this, [this](const PlaybackStatus status) {
        if (!m_countDownIsSet) {
            audioEngine()->mixer()->setIsActive(status == PlaybackStatus::Running);
        }
    });

    m_clock->countDownEnded().onNotify(this, [this]() {
        m_countDownIsSet = false;
        audioEngine()->mixer()->setIsActive(m_clock->status() == PlaybackStatus::Running);
    });
}

void SequencePlayer::play(const secs_t delay)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_clock->setCountDown(secsToMicrosecs(delay));
    m_countDownIsSet = !delay.is_zero();
    audioEngine()->setMode(RenderMode::RealTimeMode);
    m_clock->start();
}

void SequencePlayer::seek(const secs_t newPosition)
{
    ONLY_AUDIO_WORKER_THREAD;

    msecs_t newPos = secsToMicrosecs(newPosition);
    m_clock->seek(newPos);
    seekAllTracks(newPos);
}

void SequencePlayer::stop()
{
    ONLY_AUDIO_WORKER_THREAD;

    audioEngine()->setMode(RenderMode::IdleMode);
    m_clock->stop();
}

void SequencePlayer::pause()
{
    ONLY_AUDIO_WORKER_THREAD;

    audioEngine()->setMode(RenderMode::IdleMode);
    m_clock->pause();
}

void SequencePlayer::resume(const secs_t delay)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_clock->setCountDown(secsToMicrosecs(delay));
    m_countDownIsSet = !delay.is_zero();
    audioEngine()->setMode(RenderMode::RealTimeMode);
    m_clock->resume();
}

msecs_t SequencePlayer::duration() const
{
    ONLY_AUDIO_WORKER_THREAD;

    if (!m_clock) {
        return 0;
    }

    return m_clock->timeDuration();
}

void SequencePlayer::setDuration(const msecs_t duration)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_clock->setTimeDuration(duration * 1000);
}

Ret SequencePlayer::setLoop(const msecs_t fromMsec, const msecs_t toMsec)
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_clock->setTimeLoop(fromMsec * 1000, toMsec * 1000);
}

void SequencePlayer::resetLoop()
{
    ONLY_AUDIO_WORKER_THREAD;

    m_clock->resetTimeLoop();
}

secs_t SequencePlayer::playbackPosition() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return microsecsToSecs(m_clock->currentTime());
}

Channel<secs_t> SequencePlayer::playbackPositionChanged() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_clock->timeChanged();
}

PlaybackStatus SequencePlayer::playbackStatus() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_clock->status();
}

Channel<PlaybackStatus> SequencePlayer::playbackStatusChanged() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_clock->statusChanged();
}

void SequencePlayer::seekAllTracks(const msecs_t newPositionMsecs)
{
    IF_ASSERT_FAILED(m_getTracks) {
        return;
    }

    for (const auto& pair : m_getTracks->allTracks()) {
        if (pair.second->inputHandler) {
            pair.second->inputHandler->seek(newPositionMsecs);
        }
    }
}
