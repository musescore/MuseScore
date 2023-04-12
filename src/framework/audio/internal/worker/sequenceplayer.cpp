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

#include "log.h"

#include "internal/audiosanitizer.h"

using namespace mu;
using namespace mu::audio;
using namespace mu::async;

SequencePlayer::SequencePlayer(IGetTracks* getTracks, IClockPtr clock)
    : m_getTracks(getTracks), m_clock(clock)
{
    m_clock->seekOccurred().onNotify(this, [this]() {
        seekAllTracks(m_clock->currentTime());
    });

    m_clock->statusChanged().onReceive(this, [this](const PlaybackStatus status) {
        setAllTracksActive(status == PlaybackStatus::Running);
    });
}

void SequencePlayer::play()
{
    ONLY_AUDIO_WORKER_THREAD;

    m_clock->start();
    setAllTracksActive(true);
}

void SequencePlayer::seek(const msecs_t newPositionMsecs)
{
    ONLY_AUDIO_WORKER_THREAD;

    msecs_t newPos = newPositionMsecs * 1000;
    m_clock->seek(newPos);
    seekAllTracks(newPos);
}

void SequencePlayer::stop()
{
    ONLY_AUDIO_WORKER_THREAD;

    m_clock->stop();
    setAllTracksActive(false);
}

void SequencePlayer::pause()
{
    ONLY_AUDIO_WORKER_THREAD;

    m_clock->pause();
    setAllTracksActive(false);
}

void SequencePlayer::resume()
{
    ONLY_AUDIO_WORKER_THREAD;

    m_clock->resume();
    setAllTracksActive(true);
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

Channel<msecs_t> SequencePlayer::playbackPositionMSecs() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_clock->timeChanged();
}

Channel<PlaybackStatus> SequencePlayer::playbackStatusChanged() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_clock->statusChanged();
}

void SequencePlayer::setAllTracksActive(bool active)
{
    IF_ASSERT_FAILED(m_getTracks) {
        return;
    }

    for (const auto& pair : m_getTracks->allTracks()) {
        if (pair.second->inputHandler) {
            pair.second->inputHandler->setIsActive(active);
        }
    }
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
