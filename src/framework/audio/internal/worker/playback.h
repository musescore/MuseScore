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
#ifndef MUSE_AUDIO_SEQUENCER_H
#define MUSE_AUDIO_SEQUENCER_H

#include <map>

#include "modularity/ioc.h"
#include "global/async/asyncable.h"

#include "iplayer.h"
#include "itracks.h"
#include "iaudiooutput.h"
#include "igettracksequence.h"
#include "iplayback.h"

namespace muse::audio {
class Playback : public IPlayback, public IGetTrackSequence, public Injectable, public async::Asyncable
{
public:
    Playback(const muse::modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init() override;
    void deinit() override;
    bool isInited() const override;

    // IPlayback
    async::Promise<TrackSequenceId> addSequence() override;
    async::Promise<TrackSequenceIdList> sequenceIdList() const override;
    void removeSequence(const TrackSequenceId id) override;

    async::Channel<TrackSequenceId> sequenceAdded() const override;
    async::Channel<TrackSequenceId> sequenceRemoved() const override;

    IPlayerPtr player(const TrackSequenceId id) const override;
    ITracksPtr tracks() const override;
    IAudioOutputPtr audioOutput() const override;

protected:
    // IGetTrackSequence
    ITrackSequencePtr sequence(const TrackSequenceId id) const override;

private:
    ITracksPtr m_trackHandlersPtr = nullptr;
    IAudioOutputPtr m_audioOutputPtr = nullptr;

    std::map<TrackSequenceId, ITrackSequencePtr> m_sequences;

    async::Channel<TrackSequenceId> m_sequenceAdded;
    async::Channel<TrackSequenceId> m_sequenceRemoved;
};
}

#endif // MUSE_AUDIO_SEQUENCER_H
