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

#ifndef MUSE_AUDIO_PLAYERSHANDLER_H
#define MUSE_AUDIO_PLAYERSHANDLER_H

#include "global/async/asyncable.h"

#include "iplayer.h"
#include "igettracksequence.h"

namespace muse::audio {
class PlayerHandler : public IPlayer, public async::Asyncable
{
public:
    explicit PlayerHandler(IGetTrackSequence* getSequence);
    ~PlayerHandler();

    void play(const TrackSequenceId sequenceId) override;
    void seek(const TrackSequenceId sequenceId, const msecs_t newPositionMsecs) override;
    void stop(const TrackSequenceId sequenceId) override;
    void pause(const TrackSequenceId sequenceId) override;
    void resume(const TrackSequenceId sequenceId) override;

    void setDuration(const TrackSequenceId sequenceId, const msecs_t durationMsec) override;
    async::Promise<bool> setLoop(const TrackSequenceId sequenceId, const msecs_t fromMsec, const msecs_t toMsec) override;
    void resetLoop(const TrackSequenceId sequenceId) override;

    async::Channel<TrackSequenceId, msecs_t> playbackPositionMsecs() const override;
    async::Channel<TrackSequenceId, PlaybackStatus> playbackStatusChanged() const override;

private:
    ITrackSequencePtr sequence(const TrackSequenceId id) const;
    void ensureSubscriptions(const ITrackSequencePtr s) const;

    IGetTrackSequence* m_getSequence = nullptr;

    mutable async::Channel<TrackSequenceId, msecs_t> m_playbackPositionMsecsChanged;
    mutable async::Channel<TrackSequenceId, PlaybackStatus> m_playbackStatusChanged;
};
}

#endif // MUSE_AUDIO_PLAYERSHANDLER_H
