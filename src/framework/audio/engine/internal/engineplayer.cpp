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
#include "audio/common/audioerrors.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::engine;
using namespace muse::async;

EnginePlayer::EnginePlayer(IGetTrackSource* getTracks)
    : m_trackSource(getTracks)
{
    m_status.set(PlaybackStatus::Stopped);
    m_isActive.set(false);

    m_status.ch.onReceive(this, [this](const PlaybackStatus status) {
        onStatusChanged(status);
    });

    // Forwarding events from the processing thread to the engine thread
    m_timeEvent.onReceive(this, [this](const TimeEvent event) {
        onTimeEvent(event);
    });
}

void EnginePlayer::onStatusChanged(const PlaybackStatus status)
{
    const bool active = status == PlaybackStatus::Running;
    if (active) {
        //! NOTE If there is no countdown, activate the mixer.
        //! Otherwise, it will become active when the countdown ends.
        if (m_countDown.is_zero()) {
            m_isActive.set(true);
        }
    } else {
        m_isActive.set(false);
        flushAllTracks();
    }
}

void EnginePlayer::forward(const TimePosition& delta)
{
    ONLY_AUDIO_PROC_THREAD;

    // Check: Active
    if (m_status.val != PlaybackStatus::Running) {
        return;
    }

    const TimePosition newTime = proc_onTimeChanged(delta);

    if (m_currentPosition == newTime) {
        return;
    }

    m_currentPosition = newTime;
    m_timeChanged.send(m_currentPosition.time());
}

TimePosition EnginePlayer::proc_onTimeChanged(const TimePosition& delta)
{
    ONLY_AUDIO_PROC_THREAD;

    // Check: Count down
    if (!m_countDown.is_zero()) {
        m_countDown -= delta.time();

        if (m_countDown > 0.) {
            return m_currentPosition; // no change
        }

        m_countDown = 0.;
        m_timeEvent.send(TimeEvent { TimeEventType::CountDownEnded, m_currentPosition }); // forwarding an event to the engine thread
    }

    // Check: Loop
    const TimePosition newTime = m_currentPosition.forwarded(delta);
    if (m_timeLoopStart < m_timeLoopEnd && newTime.time() >= m_timeLoopEnd) {
        //! TODO Seek may be necessary to call this directly within the PROC thread.
        m_timeEvent.send(TimeEvent { TimeEventType::LoopEnded, newTime }); // forwarding an event to the engine thread
        const secs_t overshoot = newTime.time() - m_timeLoopEnd;
        return TimePosition::fromTime(m_timeLoopStart + overshoot, delta.sampleRate());
    }

    // Check: Duration
    if (newTime.time() >= m_timeDuration) {
        m_timeEvent.send(TimeEvent { TimeEventType::PlaybackEnded, newTime }); // forwarding an event to the engine thread
        return TimePosition::fromTime(m_timeDuration, delta.sampleRate());
    }

    return newTime;
}

const TimePosition& EnginePlayer::currentPosition() const
{
    return m_currentPosition;
}

void EnginePlayer::onTimeEvent(const TimeEvent event)
{
    ONLY_AUDIO_ENGINE_THREAD;
    switch (event.type) {
    case TimeEventType::CountDownEnded:
        m_isActive.set(m_status.val == PlaybackStatus::Running);
        break;
    case TimeEventType::LoopEnded:
        seekAllTracks(event.position);
        break;
    case TimeEventType::PlaybackEnded:
        pause();
        break;
    default:
        break;
    }
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

    if (playbackStatus() == PlaybackStatus::Running) {
        return;
    }

    m_countDown = delay;
    m_status.set(PlaybackStatus::Running);
}

void EnginePlayer::seek(const TimePosition& position, const bool flushSound)
{
    ONLY_AUDIO_ENGINE_THREAD;

    //! NOTE During export, the current time does not change, it remains at 0
    // but a seek operation is still required to reset the internal state of the sources (synthesizers).
    // if (newPosition == m_currentPosition.time()) {
    //     return;
    // }

    IF_ASSERT_FAILED(position.isValid()) {
        return;
    }

    IF_ASSERT_FAILED(m_trackSource) {
        return;
    }

    m_flushSoundOnSeek = flushSound;
    m_currentPosition = position;
    m_timeChanged.send(m_currentPosition.time());
    seekAllTracks(position);
    m_flushSoundOnSeek = true;
}

void EnginePlayer::stop()
{
    ONLY_AUDIO_ENGINE_THREAD;

    if (playbackStatus() == PlaybackStatus::Stopped) {
        return;
    }

    m_status.set(PlaybackStatus::Stopped);
    m_countDown = 0.;
    seek(TimePosition::zero(m_currentPosition.sampleRate()));
    m_notYetReadyToPlayTrackIdSet.clear();
}

void EnginePlayer::pause()
{
    ONLY_AUDIO_ENGINE_THREAD;

    if (playbackStatus() == PlaybackStatus::Paused) {
        return;
    }

    m_status.set(PlaybackStatus::Paused);
    m_notYetReadyToPlayTrackIdSet.clear();
}

void EnginePlayer::resume(const secs_t delay)
{
    ONLY_AUDIO_ENGINE_THREAD;

    if (playbackStatus() == PlaybackStatus::Running) {
        return;
    }

    m_countDown = delay;
    seek(m_currentPosition);
    m_status.set(PlaybackStatus::Running);
}

secs_t EnginePlayer::duration() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_timeDuration;
}

void EnginePlayer::setDuration(const secs_t duration)
{
    ONLY_AUDIO_ENGINE_THREAD;
    m_timeDuration = duration;
}

Ret EnginePlayer::setLoop(const secs_t from, const secs_t to)
{
    ONLY_AUDIO_ENGINE_THREAD;

    if (from >= to) {
        return make_ret(Err::InvalidTimeLoop);
    }

    m_timeLoopStart = from;
    m_timeLoopEnd = to;

    return Ret(Ret::Code::Ok);
}

void EnginePlayer::resetLoop()
{
    ONLY_AUDIO_ENGINE_THREAD;
    m_timeLoopStart = 0;
    m_timeLoopEnd = 0;
}

secs_t EnginePlayer::playbackPosition() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_currentPosition.time();
}

Channel<secs_t> EnginePlayer::playbackPositionChanged() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_timeChanged;
}

PlaybackStatus EnginePlayer::playbackStatus() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_status.val;
}

Channel<PlaybackStatus> EnginePlayer::playbackStatusChanged() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_status.ch;
}

bool EnginePlayer::isActive() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_isActive.val;
}

Channel<bool> EnginePlayer::isActiveChanged() const
{
    ONLY_AUDIO_ENGINE_THREAD;
    return m_isActive.ch;
}

void EnginePlayer::seekAllTracks(const TimePosition& position)
{
    IF_ASSERT_FAILED(m_trackSource) {
        return;
    }

    for (const auto& source : m_trackSource->allTracksSources()) {
        source->seek(position, m_flushSoundOnSeek);
    }
}

void EnginePlayer::flushAllTracks()
{
    IF_ASSERT_FAILED(m_trackSource) {
        return;
    }

    for (const auto& source : m_trackSource->allTracksSources()) {
        source->flush();
    }
}

void EnginePlayer::prepareAllTracksToPlay(AllTracksReadyCallback allTracksReadyCallback)
{
    ONLY_AUDIO_ENGINE_THREAD;

    IF_ASSERT_FAILED(m_trackSource) {
        return;
    }

    std::vector<ITrackAudioInputPtr> notYetReadyToPlayTracks;
    m_notYetReadyToPlayTrackIdSet.clear();

    for (const auto& source : m_trackSource->allTracksSources()) {
        source->prepareToPlay();

        if (!source->readyToPlay()) {
            notYetReadyToPlayTracks.push_back(source);
            m_notYetReadyToPlayTrackIdSet.insert(source->trackId());
        }
    }

    if (notYetReadyToPlayTracks.empty()) {
        allTracksReadyCallback();
        return;
    }

    for (const auto& source : notYetReadyToPlayTracks) {
        TrackId trackId = source->trackId();
        source->readyToPlayChanged().onNotify(this, [this, trackId, allTracksReadyCallback]() {
            muse::remove(m_notYetReadyToPlayTrackIdSet, trackId);

            if (m_notYetReadyToPlayTrackIdSet.empty()) {
                allTracksReadyCallback();
            }

            ITrackAudioInputPtr source = m_trackSource->trackSource(trackId);
            IF_ASSERT_FAILED(source) {
                return;
            }
            source->readyToPlayChanged().disconnect(this);
        }, Asyncable::Mode::SetReplace);
    }
}
