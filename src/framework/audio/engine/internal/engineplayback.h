/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include <unordered_set>

#include "../iengineplayback.h"

#include "global/async/asyncable.h"

#include "global/modularity/ioc.h"
#include "../isynthresolver.h"
#include "../ifxresolver.h"
#include "iaudioengine.h"
#include "../iaudioengineconfiguration.h"
#include "../iengineplayer.h"

#include "track.h"
#include "igettracks.h"

namespace muse::audio::soundtrack {
class SoundTrackWriter;
using SoundTrackWriterPtr = std::shared_ptr<SoundTrackWriter>;
}

namespace muse::audio::engine {
class Mixer;
class EnginePlayback : public IEnginePlayback, public IGetTracks, public async::Asyncable
{
    GlobalInject<synth::ISynthResolver> synthResolver;
    GlobalInject<fx::IFxResolver> fxResolver;
    GlobalInject<IAudioEngineConfiguration> configuration;
    GlobalInject<IAudioEngine> audioEngine;

public:
    EnginePlayback() = default;

    void init() override;
    void deinit() override;

    // 2. Setup tracks
    RetVal<TrackIdList> trackIdList() const override;
    RetVal<TrackName> trackName(const TrackId trackId) const override;
    RetVal2<TrackId, AudioParams> addTrack(const std::string& trackName, io::IODevice* playbackData, const AudioParams& params) override;
    RetVal2<TrackId, AudioParams> addTrack(const std::string& trackName, const mpe::PlaybackData& playbackData,
                                           const AudioParams& params) override;
    RetVal2<TrackId, AudioOutputParams> addAuxTrack(const std::string& trackName, const AudioOutputParams& outputParams) override;

    void removeTrack(const TrackId trackId) override;
    void removeAllTracks() override;

    async::Channel<TrackId> trackAdded() const override;
    async::Channel<TrackId> trackRemoved() const override;

    AudioResourceMetaList availableInputResources() const override;
    SoundPresetList availableSoundPresets(const AudioResourceMeta& resourceMeta) const override;

    RetVal<AudioInputParams> inputParams(const TrackId trackId) const override;
    void setInputParams(const TrackId trackId, const AudioInputParams& params) override;
    async::Channel<TrackId, AudioInputParams> inputParamsChanged() const override;

    void processInput(const TrackId trackId) const override;
    RetVal<InputProcessingProgress> inputProcessingProgress(const TrackId trackId) const override;

    void clearCache(const TrackId trackId) const override;
    void clearSources() override;

    // 3. Play
    async::Promise<Ret> prepareToPlay() override;

    void play(const secs_t delay = 0.0) override;
    void seek(const secs_t newPosition, const bool flushSound = true) override;
    void stop() override;
    void pause() override;
    void resume(const secs_t delay = 0.0) override;

    void setDuration(const secs_t duration) override;
    Ret setLoop(const secs_t from, const secs_t to) override;
    void resetLoop() override;

    PlaybackStatus playbackStatus() const override;
    async::Channel<PlaybackStatus> playbackStatusChanged() const override;
    secs_t playbackPosition() const override;
    async::Channel<secs_t> playbackPositionChanged() const override;

    // 4. Adjust output
    RetVal<AudioOutputParams> outputParams(const TrackId trackId) const override;
    void setOutputParams(const TrackId trackId, const AudioOutputParams& params) override;
    async::Channel<TrackId, AudioOutputParams> outputParamsChanged() const override;

    RetVal<AudioOutputParams> masterOutputParams() const override;
    void setMasterOutputParams(const AudioOutputParams& params) override;
    void clearMasterOutputParams() override;
    async::Channel<AudioOutputParams> masterOutputParamsChanged() const override;

    AudioResourceMetaList availableOutputResources() const override;

    RetVal<AudioSignalChanges> signalChanges(const TrackId trackId) const override;
    RetVal<AudioSignalChanges> masterSignalChanges() const override;

    async::Promise<Ret> saveSoundTrack(io::IODevice& dstDevice, const SoundTrackFormat& format) override;
    void abortSavingAllSoundTracks() override;
    SaveSoundTrackProgress saveSoundTrackProgressChanged() const override;

    void clearAllFx() override;

private:

    std::shared_ptr<IAudioContext> audioContext() const;
    void ensureAudioContextSubscriptions();

    TrackId newTrackId() const;
    void doAddTrack(const TrackPtr& track);

    void onShouldProcessDuringSilenceChanged(const TrackId trackId, bool shouldProcess);

    // IGetTracks
    TrackPtr track(const TrackId id) const override;
    const TracksMap& allTracks() const override;

    bool hasPendingChunks(const TrackId id) const;
    void listenInputProcessing(std::function<void(const Ret&)> completed);
    size_t tracksBeingProcessedCount() const;

    Ret doSaveSoundTrack(io::IODevice& dstDevice, const SoundTrackFormat& format);

    async::Channel<TrackId> m_trackAdded;
    async::Channel<TrackId> m_trackRemoved;
    async::Channel<TrackId, AudioInputParams> m_inputParamsChanged;
    async::Channel<TrackId, AudioOutputParams> m_outputParamsChanged;
    async::Channel<AudioOutputParams> m_masterOutputParamsChanged;

    TracksMap m_tracks;
    IEnginePlayerPtr m_player = nullptr;
    TrackId m_prevActiveTrackId = INVALID_TRACK_ID;
    std::unordered_set<TrackId> m_tracksToProcessWhenIdle;

    struct SaveSoundTrackProgressData {
        SaveSoundTrackProgress progress;
        async::Notification aborted;
    };

    SaveSoundTrackProgressData m_saveSoundTracksProgress;
};
}
