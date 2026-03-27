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

#ifndef MUSE_AUDIO_ENGINEPLAYER_H
#define MUSE_AUDIO_ENGINEPLAYER_H

#include "global/async/asyncable.h"

#include "modularity/ioc.h"
#include "../iaudioengine.h"

#include "../iengineplayer.h"
#include "../iclock.h"

#include "igettracks.h"

namespace muse::audio::engine {
class EnginePlayer : public IEnginePlayer, public async::Asyncable
{
    GlobalInject<engine::IAudioEngine> audioEngine;

public:
    explicit EnginePlayer(IGetTracks* getTracks, IClockPtr clock);

    async::Promise<Ret> prepareToPlay() override;

    void play(const secs_t delay = 0) override;
    void seek(const secs_t newPosition, const bool flushSound = true) override;
    void stop() override;
    void pause() override;
    void resume(const secs_t delay = 0) override;

    PlaybackStatus playbackStatus() const override;
    async::Channel<PlaybackStatus> playbackStatusChanged() const override;

    secs_t duration() const override;
    void setDuration(const secs_t duration) override;
    Ret setLoop(const secs_t from, const secs_t to) override;
    void resetLoop() override;

    secs_t playbackPosition() const override;
    async::Channel<secs_t> playbackPositionChanged() const override;

private:
    void seekAllTracks(const secs_t newPosition);
    void flushAllTracks();

    using AllTracksReadyCallback = std::function<void ()>;
    void prepareAllTracksToPlay(AllTracksReadyCallback allTracksReadyCallback);

    IGetTracks* m_getTracks = nullptr;
    IClockPtr m_clock = nullptr;

    bool m_countDownIsSet = false;
    bool m_flushSoundOnSeek = true;
    std::set<TrackId> m_notYetReadyToPlayTrackIdSet;
};
}

#endif // MUSE_AUDIO_ENGINEPLAYER_H
