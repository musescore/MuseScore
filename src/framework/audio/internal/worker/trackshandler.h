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

#ifndef MUSE_AUDIO_SEQUENCETRACKS_H
#define MUSE_AUDIO_SEQUENCETRACKS_H

#include "global/modularity/ioc.h"
#include "global/async/asyncable.h"

#include "isynthresolver.h"
#include "itracks.h"
#include "igettracksequence.h"

namespace muse::audio {
class TracksHandler : public ITracks, public Injectable, public async::Asyncable
{
    Inject<synth::ISynthResolver> resolver = { this };

public:
    explicit TracksHandler(IGetTrackSequence* getSequence, const modularity::ContextPtr& iocCtx);
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
    async::Promise<SoundPresetList> availableSoundPresets(const AudioResourceMeta& resourceMeta) const override;

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

#endif // MUSE_AUDIO_SEQUENCETRACKS_H
