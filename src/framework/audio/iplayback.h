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
#ifndef MUSE_AUDIO_ISEQUENCER_H
#define MUSE_AUDIO_ISEQUENCER_H

#include "modularity/imoduleinterface.h"
#include "global/async/channel.h"
#include "global/async/promise.h"
#include "global/progress.h"

#include "audiotypes.h"

namespace muse::audio {
class ITracks;
class IPlayer;
class IAudioOutput;

class IPlayback : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IPlayback)

public:
    virtual ~IPlayback() = default;

    virtual void init() = 0;
    virtual void deinit() = 0;
    virtual bool isInited() const = 0;

    // A quick guide how to playback something:

    // 1. Add Sequence
    virtual async::Promise<TrackSequenceId> addSequence() = 0;
    virtual async::Promise<TrackSequenceIdList> sequenceIdList() const = 0;
    virtual void removeSequence(const TrackSequenceId id) = 0;

    virtual async::Channel<TrackSequenceId> sequenceAdded() const = 0;
    virtual async::Channel<TrackSequenceId> sequenceRemoved() const = 0;

    // 2. Setup tracks for Sequence
    virtual async::Promise<TrackIdList> trackIdList(const TrackSequenceId sequenceId) const = 0;
    virtual async::Promise<TrackName> trackName(const TrackSequenceId sequenceId, const TrackId trackId) const = 0;

    virtual async::Promise<TrackId, AudioParams> addTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                                          io::IODevice* playbackData, AudioParams&& params) = 0;
    virtual async::Promise<TrackId, AudioParams> addTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                                          const mpe::PlaybackData& playbackData, AudioParams&& params) = 0;

    virtual async::Promise<TrackId, AudioOutputParams> addAuxTrack(const TrackSequenceId sequenceId, const std::string& trackName,
                                                                   const AudioOutputParams& outputParams) = 0;

    virtual void removeTrack(const TrackSequenceId sequenceId, const TrackId trackId) = 0;
    virtual void removeAllTracks(const TrackSequenceId sequenceId) = 0;

    virtual async::Channel<TrackSequenceId, TrackId> trackAdded() const = 0;
    virtual async::Channel<TrackSequenceId, TrackId> trackRemoved() const = 0;

    virtual async::Promise<AudioResourceMetaList> availableInputResources() const = 0;
    virtual async::Promise<SoundPresetList> availableSoundPresets(const AudioResourceMeta& resourceMeta) const = 0;

    virtual async::Promise<AudioInputParams> inputParams(const TrackSequenceId sequenceId, const TrackId trackId) const = 0;
    virtual void setInputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioInputParams& params) = 0;
    virtual async::Channel<TrackSequenceId, TrackId, AudioInputParams> inputParamsChanged() const = 0;

    virtual async::Promise<InputProcessingProgress> inputProcessingProgress(const TrackSequenceId sequenceId, const TrackId id) const = 0;

    virtual void clearSources() = 0;

    // 3. Play Sequence
    virtual std::shared_ptr<IPlayer> player(const TrackSequenceId id) const = 0;

    // 4. Adjust a Sequence output
    virtual async::Promise<AudioOutputParams> outputParams(const TrackSequenceId sequenceId, const TrackId trackId) const = 0;
    virtual void setOutputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioOutputParams& params) = 0;
    virtual async::Channel<TrackSequenceId, TrackId, AudioOutputParams> outputParamsChanged() const = 0;

    virtual async::Promise<AudioOutputParams> masterOutputParams() const = 0;
    virtual void setMasterOutputParams(const AudioOutputParams& params) = 0;
    virtual void clearMasterOutputParams() = 0;
    virtual async::Channel<AudioOutputParams> masterOutputParamsChanged() const = 0;

    virtual async::Promise<AudioResourceMetaList> availableOutputResources() const = 0;

    virtual async::Promise<AudioSignalChanges> signalChanges(const TrackSequenceId sequenceId, const TrackId trackId) const = 0;
    virtual async::Promise<AudioSignalChanges> masterSignalChanges() const = 0;

    virtual async::Promise<bool> saveSoundTrack(const TrackSequenceId sequenceId, const io::path_t& destination,
                                                const SoundTrackFormat& format) = 0;
    virtual void abortSavingAllSoundTracks() = 0;

    virtual Progress saveSoundTrackProgress(const TrackSequenceId sequenceId) = 0;

    virtual void clearAllFx() = 0;
};

using IPlaybackPtr = std::shared_ptr<IPlayback>;
}
#endif // MUSE_AUDIO_ISEQUENCER_H
