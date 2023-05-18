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

#ifndef MU_AUDIO_SEQUENCETRACKS_H
#define MU_AUDIO_SEQUENCETRACKS_H

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "isynthresolver.h"
#include "itracks.h"
#include "igettracksequence.h"

namespace mu::audio {
class TracksHandler : public ITracks, public async::Asyncable
{
    INJECT(synth::ISynthResolver, resolver)
public:
    explicit TracksHandler(IGetTrackSequence* getSequence);
    ~TracksHandler();

    async::Promise<TrackIdList> trackIdList(const TrackSequenceId sequenceId) const override;
    async::Promise<TrackName> trackName(const TrackSequenceId sequenceId, const TrackId trackId) const override;

    async::Promise<TrackId, AudioParams> addTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                                  io::IODevice* playbackData, AudioParams&& params) override;

    async::Promise<TrackId, AudioParams> addTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                                  const mpe::PlaybackData& playbackData, AudioParams&& params) override;

    async::Promise<TrackId, AudioOutputParams> addAuxTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                                           const AudioOutputParams& outputParams) override;

    void removeTrack(const TrackSequenceId sequenceId, const TrackId trackId) override;
    void removeAllTracks(const TrackSequenceId sequenceId) override;

    async::Channel<TrackSequenceId, TrackId> trackAdded() const override;
    async::Channel<TrackSequenceId, TrackId> trackRemoved() const override;

    async::Promise<AudioResourceMetaList> availableInputResources() const override;

    async::Promise<AudioInputParams> inputParams(const TrackSequenceId sequenceId, const TrackId trackId) const override;
    void setInputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioInputParams& params) override;
    async::Channel<TrackSequenceId, TrackId, AudioInputParams> inputParamsChanged() const override;

    void clearSources() override;

private:
    ITrackSequencePtr sequence(const TrackSequenceId id) const;
    void ensureSubscriptions(const ITrackSequencePtr s) const;

    mutable async::Channel<TrackSequenceId, TrackId> m_trackAdded;
    mutable async::Channel<TrackSequenceId, TrackId> m_trackRemoved;
    mutable async::Channel<TrackSequenceId, TrackId, AudioInputParams> m_inputParamsChanged;

    IGetTrackSequence* m_getSequence = nullptr;
};
}

#endif // MU_AUDIO_SEQUENCETRACKS_H
