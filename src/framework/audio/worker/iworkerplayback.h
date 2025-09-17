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

#include "global/modularity/imoduleinterface.h"
#include "global/types/retval.h"
#include "global/io/path.h"

#include "audio/common/audiotypes.h"

namespace muse::audio::worker {
class IWorkerPlayback : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IWorkerPlayback)
public:
    virtual ~IWorkerPlayback() = default;

    // 1. Add Sequence
    virtual TrackSequenceId addSequence() = 0;
    virtual void removeSequence(const TrackSequenceId id) = 0;
    virtual TrackSequenceIdList sequenceIdList() const = 0;

    // 2. Setup tracks for Sequence
    virtual RetVal<TrackIdList> trackIdList(const TrackSequenceId sequenceId) const = 0;
    virtual RetVal<TrackName> trackName(const TrackSequenceId sequenceId, const TrackId trackId) const = 0;
    virtual RetVal2<TrackId, AudioParams> addTrack(const TrackSequenceId sequenceId, const TrackName& trackName, io::IODevice* playbackData,
                                                   const AudioParams& params) = 0;
    virtual RetVal2<TrackId, AudioParams> addTrack(const TrackSequenceId sequenceId, const TrackName& trackName,
                                                   const mpe::PlaybackData& playbackData, const AudioParams& params) = 0;
    virtual RetVal2<TrackId, AudioOutputParams> addAuxTrack(const TrackSequenceId sequenceId, const TrackName& trackName,
                                                            const AudioOutputParams& outputParams) = 0;

    virtual void removeTrack(const TrackSequenceId sequenceId, const TrackId trackId) = 0;
    virtual void removeAllTracks(const TrackSequenceId sequenceId) = 0;

    virtual async::Channel<TrackSequenceId, TrackId> trackAdded() const = 0;
    virtual async::Channel<TrackSequenceId, TrackId> trackRemoved() const = 0;

    virtual AudioResourceMetaList availableInputResources() const = 0;
    virtual SoundPresetList availableSoundPresets(const AudioResourceMeta& resourceMeta) const = 0;

    virtual RetVal<AudioInputParams> inputParams(const TrackSequenceId sequenceId, const TrackId trackId) const = 0;
    virtual void setInputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioInputParams& params) = 0;
    virtual async::Channel<TrackSequenceId, TrackId, AudioInputParams> inputParamsChanged() const = 0;

    virtual void processInput(const TrackSequenceId sequenceId, const TrackId trackId) const = 0;
    virtual RetVal<InputProcessingProgress> inputProcessingProgress(const TrackSequenceId sequenceId, const TrackId trackId) const = 0;

    virtual void clearCache(const TrackSequenceId sequenceId, const TrackId trackId) const = 0;
    virtual void clearSources() = 0;

    // 3. Play Sequence
    virtual void play(TrackSequenceId sequenceId, const secs_t delay = 0.0) = 0;
    virtual void seek(TrackSequenceId sequenceId, const secs_t newPosition, const bool flushSound = true) = 0;
    virtual void stop(TrackSequenceId sequenceId) = 0;
    virtual void pause(TrackSequenceId sequenceId) = 0;
    virtual void resume(TrackSequenceId sequenceId, const secs_t delay = 0.0) = 0;

    virtual void setDuration(TrackSequenceId sequenceId, const msecs_t durationMsec) = 0;
    virtual Ret setLoop(TrackSequenceId sequenceId, const msecs_t fromMsec, const msecs_t toMsec) = 0;
    virtual void resetLoop(TrackSequenceId sequenceId) = 0;

    virtual PlaybackStatus playbackStatus(TrackSequenceId sequenceId) const = 0;
    virtual async::Channel<PlaybackStatus> playbackStatusChanged(TrackSequenceId sequenceId) const = 0;
    virtual secs_t playbackPosition(TrackSequenceId sequenceId) const = 0;
    virtual async::Channel<secs_t> playbackPositionChanged(TrackSequenceId sequenceId) const = 0;

    // 4. Adjust a Sequence output
    virtual RetVal<AudioOutputParams> outputParams(const TrackSequenceId sequenceId, const TrackId trackId) const = 0;
    virtual void setOutputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioOutputParams& params) = 0;
    virtual async::Channel<TrackSequenceId, TrackId, AudioOutputParams> outputParamsChanged() const = 0;

    virtual RetVal<AudioOutputParams> masterOutputParams() const = 0;
    virtual void setMasterOutputParams(const AudioOutputParams& params) = 0;
    virtual void clearMasterOutputParams() = 0;
    virtual async::Channel<AudioOutputParams> masterOutputParamsChanged() const = 0;

    virtual AudioResourceMetaList availableOutputResources() const = 0;

    virtual RetVal<AudioSignalChanges> signalChanges(const TrackSequenceId sequenceId, const TrackId trackId) const = 0;
    virtual RetVal<AudioSignalChanges> masterSignalChanges() const = 0;

    virtual Ret saveSoundTrack(const TrackSequenceId sequenceId, const io::path_t& destination, const SoundTrackFormat& format) = 0;
    virtual void abortSavingAllSoundTracks() = 0;
    virtual async::Channel<int64_t /*current*/, int64_t /*total*/>
    saveSoundTrackProgressChanged(const TrackSequenceId sequenceId) const = 0;

    virtual void clearAllFx() = 0;
};
}
