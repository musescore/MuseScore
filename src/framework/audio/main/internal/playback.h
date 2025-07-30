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

#include "../iplayback.h"
#include "global/async/asyncable.h"

#include "modularity/ioc.h"
#include "common/rpc/irpcchannel.h"

#include "../iplayer.h"

namespace muse::audio {
class Playback : public IPlayback, public Injectable, public async::Asyncable
{
    Inject<rpc::IRpcChannel> channel;

public:
    Playback(const muse::modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();
    void deinit();

    // 1. Add Sequence
    async::Promise<TrackSequenceId> addSequence() override;
    async::Promise<TrackSequenceIdList> sequenceIdList() const override;
    void removeSequence(const TrackSequenceId id) override;

    async::Channel<TrackSequenceId> sequenceAdded() const override;
    async::Channel<TrackSequenceId> sequenceRemoved() const override;

    // 2. Setup tracks for Sequence
    async::Promise<TrackIdList> trackIdList(const TrackSequenceId sequenceId) const override;
    async::Promise<RetVal<TrackName> > trackName(const TrackSequenceId sequenceId, const TrackId trackId) const override;

    async::Promise<TrackId, AudioParams> addTrack(const TrackSequenceId sequenceId, const TrackName& trackName, io::IODevice* playbackData,
                                                  AudioParams&& params) override;
    async::Promise<TrackId, AudioParams> addTrack(const TrackSequenceId sequenceId, const TrackName& trackName,
                                                  const mpe::PlaybackData& playbackData, AudioParams&& params) override;

    async::Promise<TrackId, AudioOutputParams> addAuxTrack(const TrackSequenceId sequenceId, const TrackName& trackName,
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

    async::Promise<InputProcessingProgress> inputProcessingProgress(const TrackSequenceId sequenceId, const TrackId id) const override;

    void clearSources() override;

    // 3. Play Sequence
    IPlayerPtr player(const TrackSequenceId id) const override;

    // 4. Adjust a Sequence output
    async::Promise<AudioOutputParams> outputParams(const TrackSequenceId sequenceId, const TrackId trackId) const override;
    void setOutputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioOutputParams& params) override;
    async::Channel<TrackSequenceId, TrackId, AudioOutputParams> outputParamsChanged() const override;

    async::Promise<AudioOutputParams> masterOutputParams() const override;
    void setMasterOutputParams(const AudioOutputParams& params) override;
    void clearMasterOutputParams() override;
    async::Channel<AudioOutputParams> masterOutputParamsChanged() const override;

    async::Promise<AudioResourceMetaList> availableOutputResources() const override;

    async::Promise<AudioSignalChanges> signalChanges(const TrackSequenceId sequenceId, const TrackId trackId) const override;
    async::Promise<AudioSignalChanges> masterSignalChanges() const override;

    async::Promise<bool> saveSoundTrack(const TrackSequenceId sequenceId, const io::path_t& destination,
                                        const SoundTrackFormat& format) override;
    void abortSavingAllSoundTracks() override;
    async::Channel<int64_t, int64_t> saveSoundTrackProgressChanged(const TrackSequenceId sequenceId) const override;

    void clearAllFx() override;

private:
    async::Channel<TrackSequenceId> m_sequenceAdded;
    async::Channel<TrackSequenceId> m_sequenceRemoved;

    async::Channel<TrackSequenceId, TrackId> m_trackAdded;
    async::Channel<TrackSequenceId, TrackId> m_trackRemoved;
    async::Channel<TrackSequenceId, TrackId, AudioInputParams> m_inputParamsChanged;
    async::Channel<TrackSequenceId, TrackId, AudioOutputParams> m_outputParamsChanged;
    async::Channel<AudioOutputParams> m_masterOutputParamsChanged;

    mutable std::map<TrackSequenceId, async::Channel<int64_t, int64_t> > m_saveSoundTrackProgressChannels;
    async::Channel<TrackSequenceId, int64_t, int64_t> m_saveSoundTrackProgressStream;
    mutable rpc::StreamId m_saveSoundTrackProgressStreamId = 0;
};
}

#endif // MUSE_AUDIO_SEQUENCER_H
