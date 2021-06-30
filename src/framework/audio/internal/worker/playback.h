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
#ifndef MU_AUDIO_SEQUENCER_H
#define MU_AUDIO_SEQUENCER_H

#include <map>

#include "async/asyncable.h"

#include "iplayers.h"
#include "itracks.h"
#include "iaudioio.h"
#include "igettracksequence.h"
#include "iplayback.h"

namespace mu::audio {
class Playback : public IPlayback, public IGetTrackSequence, public async::Asyncable
{
public:
    Playback();

    // IPlayback
    async::Promise<TrackSequenceId> addSequence() override;
    async::Promise<TrackSequenceIdList> sequenceIdList() const override;
    void removeSequence(const TrackSequenceId id) override;

    IPlayersPtr players() const override;
    ITracksPtr tracks() const override;
    IAudioIOPtr audioIO() const override;

protected:
    // IGetTrackSequence
    ITrackSequencePtr sequence(const TrackSequenceId id) const override;

private:
    IPlayersPtr m_playerHandlersPtr = nullptr;
    ITracksPtr m_trackHandlersPtr = nullptr;
    IAudioIOPtr m_audioIOPtr = nullptr;

    std::map<TrackSequenceId, ITrackSequencePtr> m_sequences;
};
}

#endif // MU_AUDIO_SEQUENCER_H
