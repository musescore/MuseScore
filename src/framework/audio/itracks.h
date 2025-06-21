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

#ifndef MUSE_AUDIO_ITRACKSHANDLER_H
#define MUSE_AUDIO_ITRACKSHANDLER_H

#include <memory>

#include "global/async/promise.h"
#include "global/async/channel.h"
#include "global/progress.h"

#include "mpe/events.h"

#include "audiotypes.h"

namespace muse::audio {
class ITracks
{
public:
    virtual ~ITracks() = default;

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

    virtual async::Promise<InputProcessingProgress> inputProcessingProgress(const TrackSequenceId sequenceId,
                                                                            const TrackId trackId) const = 0;

    virtual void clearSources() = 0;
};

using ITracksPtr = std::shared_ptr<ITracks>;
}

#endif // MUSE_AUDIO_ITRACKSHANDLER_H
