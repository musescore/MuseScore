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
#include "ifxresolver.h"
#include "iaudioengine.h"

#include "iplayer.h"
#include "igettracksequence.h"

namespace muse::audio {
class Mixer;

namespace soundtrack {
class SoundTrackWriter;
using SoundTrackWriterPtr = std::shared_ptr<SoundTrackWriter>;
}
}

namespace muse::audio::worker {
class WorkerPlayback : public IWorkerPlayback, public IGetTrackSequence, public Injectable, public async::Asyncable
{
    Inject<synth::ISynthResolver> synthResolver = { this };
    Inject<fx::IFxResolver> fxResolver = { this };
    Inject<IAudioEngine> audioEngine = { this };

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

    // 3. Play Sequence
    IPlayerPtr player(const TrackSequenceId id) const override;

    // 4. Adjust a Sequence output
    RetVal<AudioOutputParams> outputParams(const TrackSequenceId sequenceId, const TrackId trackId) const override;
    void setOutputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioOutputParams& params) override;
    async::Channel<TrackSequenceId, TrackId, AudioOutputParams> outputParamsChanged() const override;

    RetVal<AudioOutputParams> masterOutputParams() const override;
    void setMasterOutputParams(const AudioOutputParams& params) override;
    void clearMasterOutputParams() override;
    async::Channel<AudioOutputParams> masterOutputParamsChanged() const override;

    AudioResourceMetaList availableOutputResources() const override;

    RetVal<AudioSignalChanges> signalChanges(const TrackSequenceId sequenceId, const TrackId trackId) const override;
    RetVal<AudioSignalChanges> masterSignalChanges() const override;

    Ret saveSoundTrack(const TrackSequenceId sequenceId, const io::path_t& destination, const SoundTrackFormat& format) override;
    void abortSavingAllSoundTracks() override;
    Progress saveSoundTrackProgress(const TrackSequenceId sequenceId) override;

    void clearAllFx() override;

private:

    std::shared_ptr<Mixer> mixer() const;

    void ensureSubscriptions(const ITrackSequencePtr s);
    void ensureMixerSubscriptions();

    async::Channel<TrackSequenceId, TrackId> m_trackAdded;
    async::Channel<TrackSequenceId, TrackId> m_trackRemoved;
    async::Channel<TrackSequenceId, TrackId, AudioInputParams> m_inputParamsChanged;
    async::Channel<TrackSequenceId, TrackId, AudioOutputParams> m_outputParamsChanged;
    async::Channel<AudioOutputParams> m_masterOutputParamsChanged;

    std::map<TrackSequenceId, ITrackSequencePtr> m_sequences;

    std::unordered_map<TrackSequenceId, Progress> m_saveSoundTracksProgressMap;
    std::unordered_map<TrackSequenceId, soundtrack::SoundTrackWriterPtr> m_saveSoundTracksWritersMap;
};
}
