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
#ifndef MUSE_AUDIO_SEQUENCER_H
#define MUSE_AUDIO_SEQUENCER_H

#include "../iplayback.h"
#include "global/async/asyncable.h"

#include "modularity/ioc.h"
#include "common/rpc/icontextrpcchannel.h"
#include "../istartaudiocontroller.h"

#include "../iplayer.h"

namespace muse::audio {
//! NOTE It may not use contextual services,
// but the playback itself is contextual,
// because each context (window) has its own playback.
class Playback : public IPlayback, public async::Asyncable, public Contextable
{
    GlobalInject<IStartAudioController> startAudioController;
    ContextInject<rpc::IContextRpcChannel> channel = { this };

public:
    Playback(const muse::modularity::ContextPtr& ctx)
        : Contextable(ctx) {}

    void init();
    void deinit();

    // 0. Check is audio system started
    bool isAudioStarted() const override;
    async::Channel<bool> isAudioStartedChanged() const override;

    // 1. Init playback (temporary)
    async::Promise<bool> initPlayback() override;
    void deinitPlayback() override;

    // 2. Setup tracks for Sequence
    async::Promise<TrackIdList> trackIdList() const override;
    async::Promise<RetVal<TrackName> > trackName(const TrackId trackId) const override;

    async::Promise<TrackId, AudioParams> addTrack(const TrackName& name, io::IODevice* data, AudioParams&& params) override;
    async::Promise<TrackId, AudioParams> addTrack(const TrackName& name, const mpe::PlaybackData& data, AudioParams&& params) override;

    async::Promise<TrackId, AudioOutputParams> addAuxTrack(const TrackName& name, const AudioOutputParams& outputParams) override;

    void removeTrack(const TrackId trackId) override;
    void removeAllTracks() override;

    async::Channel<TrackId> trackAdded() const override;
    async::Channel<TrackId> trackRemoved() const override;

    async::Promise<AudioResourceMetaList> availableInputResources() const override;
    async::Promise<SoundPresetList> availableSoundPresets(const AudioResourceMeta& resourceMeta) const override;

    async::Promise<AudioInputParams> inputParams(const TrackId trackId) const override;
    void setInputParams(const TrackId trackId, const AudioInputParams& params) override;
    async::Channel<TrackId, AudioInputParams> inputParamsChanged() const override;

    void processInput(const TrackId trackId) const override;
    async::Promise<InputProcessingProgress> inputProcessingProgress(const TrackId trackId) const override;

    void clearCache(const TrackId trackId) const override;
    void clearSources() override;

    // 3. Play Sequence
    IPlayerPtr player() const override;

    // 4. Adjust a Sequence output
    async::Promise<AudioOutputParams> outputParams(const TrackId trackId) const override;
    void setOutputParams(const TrackId trackId, const AudioOutputParams& params) override;
    async::Channel<TrackId, AudioOutputParams> outputParamsChanged() const override;

    async::Promise<AudioOutputParams> masterOutputParams() const override;
    void setMasterOutputParams(const AudioOutputParams& params) override;
    void clearMasterOutputParams() override;
    async::Channel<AudioOutputParams> masterOutputParamsChanged() const override;

    async::Promise<AudioResourceMetaList> availableOutputResources() const override;

    async::Promise<AudioSignalChanges> signalChanges(const TrackId trackId) const override;
    async::Promise<AudioSignalChanges> masterSignalChanges() const override;

    async::Promise<bool> saveSoundTrack(const SoundTrackFormat& format, io::IODevice& dstDevice) override;
    void abortSavingAllSoundTracks() override;
    SaveSoundTrackProgress saveSoundTrackProgressChanged() const override;

    void clearAllFx() override;

private:

    async::Channel<TrackId> m_trackAdded;
    async::Channel<TrackId> m_trackRemoved;
    async::Channel<TrackId, AudioInputParams> m_inputParamsChanged;
    async::Channel<TrackId, AudioOutputParams> m_outputParamsChanged;
    async::Channel<AudioOutputParams> m_masterOutputParamsChanged;

    mutable std::map<TrackSequenceId, SaveSoundTrackProgress> m_saveSoundTrackProgressChannels;
    async::Channel<int64_t, int64_t, SaveSoundTrackStage> m_saveSoundTrackProgressStream;
    mutable rpc::StreamId m_saveSoundTrackProgressStreamId = 0;
};
}

#endif // MUSE_AUDIO_SEQUENCER_H
