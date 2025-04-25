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

#include <memory>

#include "global/async/promise.h"
#include "global/async/channel.h"

#include "audiotypes.h"

namespace muse::audio {
class IAudioOutput
{
public:
    virtual ~IAudioOutput() = default;

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

using IAudioOutputPtr = std::shared_ptr<IAudioOutput>;
}
