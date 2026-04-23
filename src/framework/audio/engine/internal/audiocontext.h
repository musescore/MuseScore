/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "iaudiocontext.h"

#include "global/async/asyncable.h"

#include "modularity/ioc.h"
#include "iaudioengine.h"
#include "iaudiofactory.h"
#include "../iaudioengineconfiguration.h"

#include "igettracksource.h"
#include "iengineplayer.h"
#include "mixer.h"

namespace muse::audio::engine {
class AudioContext : public IAudioContext, public IGetTrackSource, public async::Asyncable
{
    GlobalInject<IAudioEngine> audioEngine;
    GlobalInject<IAudioFactory> audioFactory;
    GlobalInject<IAudioEngineConfiguration> configuration;

public:
    AudioContext(const modularity::IoCID& ctxId);

    Ret init(const RenderConstraints& consts) override;
    void deinit() override;

    // Config
    void setMode(const ProcessMode newMode) override;
    void setOutputSpec(const OutputSpec& outputSpec) override;

    // Tracks
    RetVal2<TrackId, AudioParams> addTrack(const std::string& trackName, io::IODevice* playbackData, const AudioParams& params) override;
    RetVal2<TrackId, AudioParams> addTrack(const std::string& trackName, const mpe::PlaybackData& playbackData,
                                           const AudioParams& params) override;
    RetVal2<TrackId, AudioOutputParams> addAuxTrack(const std::string& trackName, const AudioOutputParams& outputParams) override;

    void removeTrack(const TrackId trackId) override;
    void removeAllTracks() override;

    async::Channel<TrackId> trackAdded() const override;
    async::Channel<TrackId> trackRemoved() const override;

    RetVal<TrackIdList> trackIdList() const override;
    RetVal<TrackName> trackName(const TrackId trackId) const override;

    // Sources
    AudioResourceMetaList availableInputResources() const override;
    SoundPresetList availableSoundPresets(const AudioResourceMeta& resourceMeta) const override;

    RetVal<AudioInputParams> inputParams(const TrackId trackId) const override;
    void setInputParams(const TrackId trackId, const AudioInputParams& params) override;
    async::Channel<TrackId, AudioInputParams> inputParamsChanged() const override;

    void processInput(const TrackId trackId) const override;
    RetVal<InputProcessingProgress> inputProcessingProgress(const TrackId trackId) const override;

    void clearCache(const TrackId trackId) const override;
    void clearSources() override;

    // Outputs
    AudioResourceMetaList availableOutputResources() const override;

    RetVal<AudioOutputParams> outputParams(const TrackId trackId) const override;
    void setOutputParams(const TrackId trackId, const AudioOutputParams& params) override;
    async::Channel<TrackId, AudioOutputParams> outputParamsChanged() const override;
    RetVal<AudioSignalChanges> signalChanges(const TrackId trackId) const override;

    RetVal<AudioOutputParams> masterOutputParams() const override;
    void setMasterOutputParams(const AudioOutputParams& params) override;
    void clearMasterOutputParams() override;
    async::Channel<AudioOutputParams> masterOutputParamsChanged() const override;
    RetVal<AudioSignalChanges> masterSignalChanges() const override;

    void clearAllFx() override;

    // Play
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

    // Export
    async::Promise<Ret> saveSoundTrack(io::IODevice& dstDevice, const SoundTrackFormat& format) override;
    SaveSoundTrackProgress saveSoundTrackProgressChanged() const override;
    void abortSavingAllSoundTracks() override;

    // Processing
    samples_t process(float* buffer, samples_t samplesPerChannel) override;

private:

    struct Track
    {
        TrackId id = INVALID_TRACK_ID;
        TrackType type = Undefined;
        TrackName name;
        ITrackAudioInputPtr source = nullptr;
        ITrackAudioOutputPtr output = nullptr;
    };

    TrackId newTrackId() const;
    void doAddTrack(const Track& track);
    const Track* track(const TrackId id) const;

    // IGetTrackSource
    sample_rate_t sampleRate() const override;
    ITrackAudioInputPtr trackSource(const TrackId trackId) const override;
    std::vector<ITrackAudioInputPtr> allTracksSources() const override;
    // -----

    void listenInputProcessing(std::function<void(const Ret&)> completed);
    bool hasPendingChunks(const TrackId id) const;
    size_t tracksBeingProcessedCount() const;
    Ret doSaveSoundTrack(io::IODevice& dstDevice, const SoundTrackFormat& format);

    modularity::IoCID m_ctxId = 0;

    OutputSpec m_outputSpec;
    IEnginePlayerPtr m_player = nullptr;
    std::shared_ptr<Mixer> m_mixer;

    std::vector<Track> m_tracks;
    async::Channel<TrackId> m_trackAdded;
    async::Channel<TrackId> m_trackRemoved;

    async::Channel<TrackId, AudioInputParams> m_inputParamsChanged;
    async::Channel<TrackId, AudioOutputParams> m_outputParamsChanged;

    // -----
    void onShouldProcessDuringSilenceChanged(const TrackId trackId, bool shouldProcess);
    TrackId m_prevActiveTrackId = INVALID_TRACK_ID;
    std::unordered_set<TrackId> m_tracksToProcessWhenIdle;
    // -----

    struct SaveSoundTrackProgressData {
        SaveSoundTrackProgress progress;
        async::Notification aborted;
    };
    SaveSoundTrackProgressData m_saveSoundTracksProgress;
};
}
