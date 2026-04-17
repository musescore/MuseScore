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

#pragma once

#include "modularity/imoduleinterface.h"
#include "global/types/retval.h"
#include "global/async/channel.h"
#include "global/async/promise.h"

#include "../common/audiotypes.h"

namespace muse::io {
class IODevice;
}

namespace muse::audio {
class ITracks;
class IPlayer;
class IAudioOutput;

class IPlayback : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(IPlayback)

public:
    virtual ~IPlayback() = default;

    // A quick guide how to playback something:

    // 0. Check is audio system started
    virtual bool isAudioStarted() const = 0;
    virtual async::Channel<bool> isAudioStartedChanged() const = 0;

    // 1. Init playback (temporary)
    virtual async::Promise<bool> initPlayback() = 0;
    virtual void deinitPlayback() = 0;

    // 2. Setup tracks
    virtual async::Promise<TrackIdList> trackIdList() const = 0;
    virtual async::Promise<RetVal<TrackName> > trackName(const TrackId trackId) const = 0;

    virtual async::Promise<TrackId, AudioParams> addTrack(const TrackName& name, io::IODevice* data, AudioParams&& params) = 0;
    virtual async::Promise<TrackId, AudioParams> addTrack(const TrackName& name, const mpe::PlaybackData& data, AudioParams&& params) = 0;

    virtual async::Promise<TrackId, AudioOutputParams> addAuxTrack(const TrackName& trackName, const AudioOutputParams& outputParams) = 0;

    virtual void removeTrack(const TrackId trackId) = 0;
    virtual void removeAllTracks() = 0;

    virtual async::Channel<TrackId> trackAdded() const = 0;
    virtual async::Channel<TrackId> trackRemoved() const = 0;

    virtual async::Promise<AudioResourceMetaList> availableInputResources() const = 0;
    virtual async::Promise<SoundPresetList> availableSoundPresets(const AudioResourceMeta& resourceMeta) const = 0;

    virtual async::Promise<AudioInputParams> inputParams(const TrackId trackId) const = 0;
    virtual void setInputParams(const TrackId trackId, const AudioInputParams& params) = 0;
    virtual async::Channel<TrackId, AudioInputParams> inputParamsChanged() const = 0;

    virtual void processInput(const TrackId trackId) const = 0;
    virtual async::Promise<InputProcessingProgress> inputProcessingProgress(const TrackId trackId) const = 0;

    virtual void clearCache(const TrackId trackId) const = 0;
    virtual void clearSources() = 0;

    // 3. Play
    virtual std::shared_ptr<IPlayer> player() const = 0;

    // 4. Adjust output
    virtual async::Promise<AudioOutputParams> outputParams(const TrackId trackId) const = 0;
    virtual void setOutputParams(const TrackId trackId, const AudioOutputParams& params) = 0;
    virtual async::Channel<TrackId, AudioOutputParams> outputParamsChanged() const = 0;

    virtual async::Promise<AudioOutputParams> masterOutputParams() const = 0;
    virtual void setMasterOutputParams(const AudioOutputParams& params) = 0;
    virtual void clearMasterOutputParams() = 0;
    virtual async::Channel<AudioOutputParams> masterOutputParamsChanged() const = 0;

    virtual async::Promise<AudioResourceMetaList> availableOutputResources() const = 0;

    virtual async::Promise<AudioSignalChanges> signalChanges(const TrackId trackId) const = 0;
    virtual async::Promise<AudioSignalChanges> masterSignalChanges() const = 0;

    virtual async::Promise<bool> saveSoundTrack(const SoundTrackFormat& format, io::IODevice& dstDevice) = 0;
    virtual void abortSavingAllSoundTracks() = 0;
    virtual SaveSoundTrackProgress saveSoundTrackProgressChanged() const = 0;

    virtual void clearAllFx() = 0;
};

using IPlaybackPtr = std::shared_ptr<IPlayback>;
}
