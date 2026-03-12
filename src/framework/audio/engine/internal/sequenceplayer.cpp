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

#include "sequenceplayer.h"

#include "audio/common/audiosanitizer.h"

#include "clock.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::engine;
using namespace muse::async;

SequencePlayer::SequencePlayer(IGetTracks* getTracks, const modularity::ContextPtr& iocCtx)
    : Contextable(iocCtx), m_getTracks(getTracks), m_clock(std::make_shared<Clock>())
{
    m_clock->setOnAction([this](const IClock::ActionType type, const msecs_t time) {
        ONLY_AUDIO_PROC_THREAD;

        switch (type) {
            case IClock::ActionType::Seek: {
                if (m_tracksFollowClockSeek) {
                    seekAllTracks(time);
                }
                break;
            }
            case IClock::ActionType::LoopEndReached:
                m_clock->seek(m_loopStart);
                break;
            case IClock::ActionType::CountDownEnded:
                m_countDownIsSet = false;
                audioEngine()->mixer()->setIsActive(m_clock->status() == PlaybackStatus::Running);
                break;
        }
    });

    m_clock->statusChanged().onReceive(this, [this](const PlaybackStatus status) {
        const bool active = status == PlaybackStatus::Running;

        if (!m_countDownIsSet) {
            audioEngine()->mixer()->setIsActive(active);
        } else if (!active) {
            flushAllTracks();
        }
    });

    audioEngine()->mixer()->addClock(m_clock);
}

SequencePlayer::~SequencePlayer()
{
    m_clock->setOnAction(nullptr);
    audioEngine()->mixer()->removeClock(m_clock);
}

async::Promise<Ret> SequencePlayer::prepareToPlay()
{
    ONLY_AUDIO_ENGINE_THREAD;

    return async::make_promise<Ret>([this](auto resolve, auto) {
        prepareAllTracksToPlay([resolve]() {
            (void)resolve(make_ok());
        });

        return Promise<Ret>::dummy_result();
    });
}

void SequencePlayer::play(const secs_t delay)
{
    ONLY_AUDIO_ENGINE_THREAD;

    m_clock->setCountDown(secsToMicrosecs(delay));
    m_countDownIsSet = !delay.is_zero();
    audioEngine()->setMode(RenderMode::RealTimeMode);
    m_clock->start();
}

void SequencePlayer::seek(const secs_t newPosition, const bool flushSound)
{
    ONLY_AUDIO_ENGINE_THREAD;

    msecs_t newPos = secsToMicrosecs(newPosition);
    m_tracksFollowClockSeek = false;
    m_clock->seek(newPos);
    m_tracksFollowClockSeek = true;
    seekAllTracks(newPos, flushSound);
}

void SequencePlayer::stop()
{
    ONLY_AUDIO_ENGINE_THREAD;

    audioEngine()->setMode(RenderMode::IdleMode);
    m_clock->stop();
    m_notYetReadyToPlayTrackIdSet.clear();
}

void SequencePlayer::pause()
{
    ONLY_AUDIO_ENGINE_THREAD;

    audioEngine()->setMode(RenderMode::IdleMode);
    m_clock->pause();
    m_notYetReadyToPlayTrackIdSet.clear();
}

void SequencePlayer::resume(const secs_t delay)
{
    ONLY_AUDIO_ENGINE_THREAD;

    m_clock->setCountDown(secsToMicrosecs(delay));
    m_countDownIsSet = !delay.is_zero();
    audioEngine()->setMode(RenderMode::RealTimeMode);
    m_clock->resume();
}

msecs_t SequencePlayer::duration() const
{
    ONLY_AUDIO_ENGINE_THREAD;

    if (!m_clock) {
        return 0;
    }

    return m_clock->timeDuration();
}

void SequencePlayer::setDuration(const msecs_t duration)
{
    ONLY_AUDIO_ENGINE_THREAD;

    m_clock->setTimeDuration(duration * 1000);
}

Ret SequencePlayer::setLoop(const msecs_t fromMsec, const msecs_t toMsec)
{
    ONLY_AUDIO_ENGINE_THREAD;

    Ret ret = m_clock->setTimeLoop(m_loopStart, toMsec * 1000);
    if (ret) {
        m_loopStart = fromMsec * 1000;
    }

    return ret;
}

void SequencePlayer::resetLoop()
{
    ONLY_AUDIO_ENGINE_THREAD;

    m_clock->resetTimeLoop();
    m_loopStart = 0;
}

secs_t SequencePlayer::playbackPosition() const
{
    ONLY_AUDIO_ENGINE_THREAD;

    return microsecsToSecs(m_clock->currentTime());
}

Channel<secs_t> SequencePlayer::playbackPositionChanged() const
{
    ONLY_AUDIO_ENGINE_THREAD;

    return m_clock->timeChanged();
}

PlaybackStatus SequencePlayer::playbackStatus() const
{
    ONLY_AUDIO_ENGINE_THREAD;

    return m_clock->status();
}

Channel<PlaybackStatus> SequencePlayer::playbackStatusChanged() const
{
    ONLY_AUDIO_ENGINE_THREAD;

    return m_clock->statusChanged();
}

void SequencePlayer::seekAllTracks(const msecs_t newPositionMsecs, bool flushSound)
{
    IF_ASSERT_FAILED(m_getTracks) {
        return;
    }

    for (const auto& pair : m_getTracks->allTracks()) {
        if (pair.second->inputHandler) {
            pair.second->inputHandler->seek(newPositionMsecs, flushSound);
        }
    }
}

void SequencePlayer::flushAllTracks()
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

void SequencePlayer::prepareAllTracksToPlay(AllTracksReadyCallback allTracksReadyCallback)
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
