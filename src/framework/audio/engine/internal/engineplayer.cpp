/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "engineplayer.h"

#include "audio/common/audiosanitizer.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::engine;
using namespace muse::async;

EnginePlayer::EnginePlayer(IGetTracks* getTracks, IClockPtr clock)
    : m_getTracks(getTracks), m_clock(clock)
{
    m_clock->seekOccurred().onNotify(this, [this]() {
        seekAllTracks(m_clock->currentTime());
    });

    m_clock->statusChanged().onReceive(this, [this](const PlaybackStatus status) {
        const bool active = status == PlaybackStatus::Running;

        if (!m_countDownIsSet) {
            audioEngine()->mixer()->setIsActive(active);
        } else if (!active) {
            flushAllTracks();
        }
    });

    m_clock->countDownEnded().onNotify(this, [this]() {
        m_countDownIsSet = false;
        audioEngine()->mixer()->setIsActive(m_clock->status() == PlaybackStatus::Running);
    });
}

async::Promise<Ret> EnginePlayer::prepareToPlay()
{
    ONLY_AUDIO_ENGINE_THREAD;

    return async::make_promise<Ret>([this](auto resolve, auto) {
        prepareAllTracksToPlay([resolve]() {
            (void)resolve(make_ok());
        });

        return Promise<Ret>::dummy_result();
    });
}

void EnginePlayer::play(const secs_t delay)
{
    ONLY_AUDIO_ENGINE_THREAD;

    m_clock->setCountDown(delay);
    m_countDownIsSet = !delay.is_zero();
    audioEngine()->setMode(RenderMode::RealTimeMode);
    m_clock->start();
}

void EnginePlayer::seek(const secs_t newPosition, const bool flushSound)
{
    ONLY_AUDIO_ENGINE_THREAD;

    m_flushSoundOnSeek = flushSound;
    m_clock->seek(newPosition);
    seekAllTracks(newPosition);
    m_flushSoundOnSeek = true;
}

void EnginePlayer::stop()
{
    ONLY_AUDIO_ENGINE_THREAD;

    audioEngine()->setMode(RenderMode::IdleMode);
    m_clock->stop();
    m_notYetReadyToPlayTrackIdSet.clear();
}

void EnginePlayer::pause()
{
    ONLY_AUDIO_ENGINE_THREAD;

    audioEngine()->setMode(RenderMode::IdleMode);
    m_clock->pause();
    m_notYetReadyToPlayTrackIdSet.clear();
}

void EnginePlayer::resume(const secs_t delay)
{
    ONLY_AUDIO_ENGINE_THREAD;

    m_clock->setCountDown(delay);
    m_countDownIsSet = !delay.is_zero();
    audioEngine()->setMode(RenderMode::RealTimeMode);
    m_clock->resume();
}

secs_t EnginePlayer::duration() const
{
    ONLY_AUDIO_ENGINE_THREAD;

    return m_clock ? m_clock->timeDuration() : secs_t { 0. };
}

void EnginePlayer::setDuration(const secs_t duration)
{
    ONLY_AUDIO_ENGINE_THREAD;

    m_clock->setTimeDuration(duration);
}

Ret EnginePlayer::setLoop(const secs_t from, const secs_t to)
{
    ONLY_AUDIO_ENGINE_THREAD;

    return m_clock->setTimeLoop(from, to);
}

void EnginePlayer::resetLoop()
{
    ONLY_AUDIO_ENGINE_THREAD;

    m_clock->resetTimeLoop();
}

secs_t EnginePlayer::playbackPosition() const
{
    ONLY_AUDIO_ENGINE_THREAD;

    return m_clock->currentTime();
}

Channel<secs_t> EnginePlayer::playbackPositionChanged() const
{
    ONLY_AUDIO_ENGINE_THREAD;

    return m_clock->timeChanged();
}

PlaybackStatus EnginePlayer::playbackStatus() const
{
    ONLY_AUDIO_ENGINE_THREAD;

    return m_clock->status();
}

Channel<PlaybackStatus> EnginePlayer::playbackStatusChanged() const
{
    ONLY_AUDIO_ENGINE_THREAD;

    return m_clock->statusChanged();
}

void EnginePlayer::seekAllTracks(const secs_t newPosition)
{
    IF_ASSERT_FAILED(m_getTracks) {
        return;
    }

    for (const auto& pair : m_getTracks->allTracks()) {
        if (pair.second->inputHandler) {
            pair.second->inputHandler->seek(secsToMicrosecs(newPosition), m_flushSoundOnSeek);
        }
    }
}

void EnginePlayer::flushAllTracks()
{
    IF_ASSERT_FAILED(m_getTracks) {
        return;
    }

    for (const auto& pair : m_getTracks->allTracks()) {
        if (pair.second->inputHandler) {
            pair.second->inputHandler->flush();
        }
    }
}

void EnginePlayer::prepareAllTracksToPlay(AllTracksReadyCallback allTracksReadyCallback)
{
    ONLY_AUDIO_ENGINE_THREAD;

    IF_ASSERT_FAILED(m_getTracks) {
        return;
    }

    std::vector<TrackPtr> notYetReadyToPlayTracks;
    m_notYetReadyToPlayTrackIdSet.clear();

    for (const auto& pair : m_getTracks->allTracks()) {
        if (!pair.second->inputHandler) {
            continue;
        }

        pair.second->inputHandler->prepareToPlay();

        if (!pair.second->inputHandler->readyToPlay()) {
            notYetReadyToPlayTracks.push_back(pair.second);
            m_notYetReadyToPlayTrackIdSet.insert(pair.first);
        }
    }

    if (notYetReadyToPlayTracks.empty()) {
        allTracksReadyCallback();
        return;
    }

    for (const TrackPtr& track : notYetReadyToPlayTracks) {
        const TrackId trackId = track->id;

        track->inputHandler->readyToPlayChanged().onNotify(this, [this, trackId, allTracksReadyCallback]() {
            muse::remove(m_notYetReadyToPlayTrackIdSet, trackId);

            if (m_notYetReadyToPlayTrackIdSet.empty()) {
                allTracksReadyCallback();
            }

            const TrackPtr ptr = m_getTracks->track(trackId);
            if (ptr && ptr->inputHandler) {
                ptr->inputHandler->readyToPlayChanged().disconnect(this);
            }
        }, Asyncable::Mode::SetReplace);
    }
}
