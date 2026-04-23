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

#pragma once

#include "global/types/retval.h"
#include "global/async/channel.h"
#include "global/async/asyncable.h"

#include "../iengineplayer.h"
#include "../iplayhead.h"

#include "igettracksource.h"

namespace muse::audio::engine {
class EnginePlayer : public IEnginePlayer, public IPlayhead, public async::Asyncable
{
public:
    explicit EnginePlayer(IGetTrackSource* getTracks);

    async::Promise<Ret> prepareToPlay() override;

    void play(const secs_t delay = 0) override;
    void seek(const TimePosition& position, const bool flushSound = true) override;
    void stop() override;
    void pause() override;
    void resume(const secs_t delay = 0) override;

    PlaybackStatus playbackStatus() const override;
    async::Channel<PlaybackStatus> playbackStatusChanged() const override;
    bool isActive() const override;
    async::Channel<bool> isActiveChanged() const override;

    secs_t duration() const override;
    void setDuration(const secs_t duration) override;

    Ret setLoop(const secs_t from, const secs_t to) override;
    void resetLoop() override;

    secs_t playbackPosition() const override;
    async::Channel<secs_t> playbackPositionChanged() const override;

    // IPlayhead interface
    void forward(const TimePosition& delta) override;
    const TimePosition& currentPosition() const override;

private:

    void onStatusChanged(const PlaybackStatus status);

    // Processing thread functions
    TimePosition proc_onTimeChanged(const TimePosition& delta);
    // ----------------------------

    enum class TimeEventType {
        Undefined,
        PlaybackEnded,
        CountDownEnded,
        LoopEnded,
    };

    struct TimeEvent {
        TimeEventType type = TimeEventType::Undefined;
        TimePosition position;
    };

    void onTimeEvent(const TimeEvent event);

    void seekAllTracks(const TimePosition& position);
    void flushAllTracks();

    using AllTracksReadyCallback = std::function<void ()>;
    void prepareAllTracksToPlay(AllTracksReadyCallback allTracksReadyCallback);

    IGetTrackSource* m_trackSource = nullptr;

    ValCh<PlaybackStatus> m_status;
    ValCh<bool> m_isActive;

    TimePosition m_currentPosition;
    async::Channel<secs_t> m_timeChanged;

    secs_t m_timeDuration = 0.;
    secs_t m_countDown = 0.;
    secs_t m_timeLoopStart = 0.;
    secs_t m_timeLoopEnd = 0.;

    async::Channel<TimeEvent> m_timeEvent;

    bool m_flushSoundOnSeek = true;
    std::set<TrackId> m_notYetReadyToPlayTrackIdSet;
};
}
