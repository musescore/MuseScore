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

#include "global/modularity/imoduleinterface.h"
#include "global/types/retval.h"
#include "global/async/promise.h"

#include "audio/common/audiotypes.h"

namespace muse::io {
class IODevice;
}

namespace muse::audio::engine {
class IEnginePlayback : MODULE_GLOBAL_INTERFACE
{
    INTERFACE_ID(IEnginePlayback)
public:
    virtual ~IEnginePlayback() = default;

    virtual void init() = 0;
    virtual void deinit() = 0;

    // 2. Setup tracks
    virtual RetVal<TrackIdList> trackIdList() const = 0;
    virtual RetVal<TrackName> trackName(const TrackId trackId) const = 0;
    virtual RetVal2<TrackId, AudioParams> addTrack(const TrackName& trackName, io::IODevice* playbackData, const AudioParams& params) = 0;
    virtual RetVal2<TrackId, AudioParams> addTrack(const TrackName& trackName, const mpe::PlaybackData& playbackData,
                                                   const AudioParams& params) = 0;
    virtual RetVal2<TrackId, AudioOutputParams> addAuxTrack(const TrackName& trackName, const AudioOutputParams& outputParams) = 0;

    virtual void removeTrack(const TrackId trackId) = 0;
    virtual void removeAllTracks() = 0;

    virtual async::Channel<TrackId> trackAdded() const = 0;
    virtual async::Channel<TrackId> trackRemoved() const = 0;

    virtual AudioResourceMetaList availableInputResources() const = 0;
    virtual SoundPresetList availableSoundPresets(const AudioResourceMeta& resourceMeta) const = 0;

    virtual RetVal<AudioInputParams> inputParams(const TrackId trackId) const = 0;
    virtual void setInputParams(const TrackId trackId, const AudioInputParams& params) = 0;
    virtual async::Channel<TrackId, AudioInputParams> inputParamsChanged() const = 0;

    virtual void processInput(const TrackId trackId) const = 0;
    virtual RetVal<InputProcessingProgress> inputProcessingProgress(const TrackId trackId) const = 0;

    virtual void clearCache(const TrackId trackId) const = 0;
    virtual void clearSources() = 0;

    // 3. Play
    virtual async::Promise<Ret> prepareToPlay() = 0;

    virtual void play(const secs_t delay = 0.0) = 0;
    virtual void seek(const secs_t newPosition, const bool flushSound = true) = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume(const secs_t delay = 0.0) = 0;

    virtual void setDuration(const msecs_t durationMsec) = 0;
    virtual Ret setLoop(const msecs_t fromMsec, const msecs_t toMsec) = 0;
    virtual void resetLoop() = 0;

    virtual PlaybackStatus playbackStatus() const = 0;
    virtual async::Channel<PlaybackStatus> playbackStatusChanged() const = 0;
    virtual secs_t playbackPosition() const = 0;
    virtual async::Channel<secs_t> playbackPositionChanged() const = 0;

    // 4. Adjust a output
    virtual RetVal<AudioOutputParams> outputParams(const TrackId trackId) const = 0;
    virtual void setOutputParams(const TrackId trackId, const AudioOutputParams& params) = 0;
    virtual async::Channel<TrackId, AudioOutputParams> outputParamsChanged() const = 0;

    virtual RetVal<AudioOutputParams> masterOutputParams() const = 0;
    virtual void setMasterOutputParams(const AudioOutputParams& params) = 0;
    virtual void clearMasterOutputParams() = 0;
    virtual async::Channel<AudioOutputParams> masterOutputParamsChanged() const = 0;

    virtual AudioResourceMetaList availableOutputResources() const = 0;

    virtual RetVal<AudioSignalChanges> signalChanges(const TrackId trackId) const = 0;
    virtual RetVal<AudioSignalChanges> masterSignalChanges() const = 0;

    virtual async::Promise<Ret> saveSoundTrack(io::IODevice& dstDevice, const SoundTrackFormat& format) = 0;
    virtual void abortSavingAllSoundTracks() = 0;

    virtual SaveSoundTrackProgress saveSoundTrackProgressChanged() const = 0;

    virtual void clearAllFx() = 0;
};
}
