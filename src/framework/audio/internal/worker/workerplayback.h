/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#include "iworkerplayback.h"

#include "global/async/asyncable.h"

#include "global/modularity/ioc.h"
#include "isynthresolver.h"

#include "iplayer.h"
#include "iaudiooutput.h"
#include "igettracksequence.h"

namespace muse::audio::worker {
class WorkerPlayback : public IWorkerPlayback, public IGetTrackSequence, public Injectable, public async::Asyncable
{
    Inject<synth::ISynthResolver> synthResolver = { this };

public:
    WorkerPlayback(const muse::modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();
    void deinit();

    // 1. Add Sequence
    TrackSequenceId addSequence() override;
    void removeSequence(const TrackSequenceId id) override;
    TrackSequenceIdList sequenceIdList() const override;

    // IGetTrackSequence
    ITrackSequencePtr sequence(const TrackSequenceId id) const override;

    // 2. Setup tracks for Sequence
    RetVal<TrackIdList> trackIdList(const TrackSequenceId sequenceId) const override;
    RetVal<TrackName> trackName(const TrackSequenceId sequenceId, const TrackId trackId) const override;
    RetVal2<TrackId, AudioParams> addTrack(const TrackSequenceId sequenceId, const std::string& trackName, io::IODevice* playbackData,
                                           const AudioParams& params) override;
    RetVal2<TrackId, AudioParams> addTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                           const mpe::PlaybackData& playbackData, const AudioParams& params) override;
    RetVal2<TrackId, AudioOutputParams> addAuxTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                                    const AudioOutputParams& outputParams) override;

    void removeTrack(const TrackSequenceId sequenceId, const TrackId trackId) override;
    void removeAllTracks(const TrackSequenceId sequenceId) override;

    async::Channel<TrackSequenceId, TrackId> trackAdded() const override;
    async::Channel<TrackSequenceId, TrackId> trackRemoved() const override;

    AudioResourceMetaList availableInputResources() const override;
    SoundPresetList availableSoundPresets(const AudioResourceMeta& resourceMeta) const override;

    RetVal<AudioInputParams> inputParams(const TrackSequenceId sequenceId, const TrackId trackId) const override;
    void setInputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioInputParams& params) override;
    async::Channel<TrackSequenceId, TrackId, AudioInputParams> inputParamsChanged() const override;

    RetVal<InputProcessingProgress> inputProcessingProgress(const TrackSequenceId sequenceId, const TrackId trackId) const override;

    void clearSources() override;

    // temporary
    IPlayerPtr player(const TrackSequenceId id) const override;
    IAudioOutputPtr audioOutput() const override;

private:

    void ensureSubscriptions(const ITrackSequencePtr s);

    async::Channel<TrackSequenceId, TrackId> m_trackAdded;
    async::Channel<TrackSequenceId, TrackId> m_trackRemoved;
    async::Channel<TrackSequenceId, TrackId, AudioInputParams> m_inputParamsChanged;

    IAudioOutputPtr m_audioOutputPtr = nullptr;

    std::map<TrackSequenceId, ITrackSequencePtr> m_sequences;
};
}
