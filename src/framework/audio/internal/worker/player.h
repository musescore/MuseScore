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

#ifndef MUSE_AUDIO_PLAYER_H
#define MUSE_AUDIO_PLAYER_H

#include "iplayer.h"
#include "global/async/asyncable.h"

#include "igettracksequence.h"

namespace muse::audio {
class Player : public IPlayer, public async::Asyncable
{
public:
    Player(const IGetTrackSequence* getSeq, const TrackSequenceId sequenceId);

    void init();

    TrackSequenceId sequenceId() const override;

    void play(const secs_t delay = 0) override;
    void seek(const secs_t newPosition) override;
    void stop() override;
    void pause() override;
    void resume(const secs_t delay = 0) override;

    PlaybackStatus playbackStatus() const override;
    async::Channel<PlaybackStatus> playbackStatusChanged() const override;

    void setDuration(const msecs_t durationMsec) override;
    async::Promise<bool> setLoop(const msecs_t fromMsec, const msecs_t toMsec) override;
    void resetLoop() override;

    secs_t playbackPosition() const override;
    async::Channel<secs_t> playbackPositionChanged() const override;

private:

    ITrackSequencePtr seq() const;

    const IGetTrackSequence* m_getSequence = nullptr;
    TrackSequenceId m_sequenceId = -1;
    PlaybackStatus m_playbackStatus = PlaybackStatus::Stopped;
    async::Channel<PlaybackStatus> m_playbackStatusChanged;

    secs_t m_playbackPosition = 0.0;
    async::Channel<secs_t> m_playbackPositionChanged;
};
}

#endif // MUSE_AUDIO_PLAYER_H
