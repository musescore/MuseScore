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

#include "global/types/retval.h"
#include "audio/common/audiotypes.h"
#include "../iplayhead.h"
#include "mixerchannel.h"

namespace muse::audio::engine {
class IAudioContext
{
public:
    virtual ~IAudioContext() = default;

    virtual void setIsActive(bool active) = 0;

    virtual samples_t process(float* buffer, samples_t samplesPerChannel) = 0;

    virtual std::shared_ptr<IAudioSource> output() const = 0;

    virtual void setPlayhead(std::shared_ptr<IPlayhead> playhead) = 0;
    virtual void setTracksToProcessWhenIdle(const std::unordered_set<TrackId>& trackIds) = 0;

    virtual AudioOutputParams masterOutputParams() const = 0;
    virtual void setMasterOutputParams(const AudioOutputParams& params) = 0;
    virtual void clearMasterOutputParams() = 0;
    virtual async::Channel<AudioOutputParams> masterOutputParamsChanged() const = 0;

    virtual Ret addChannel(ITrackAudioOutputPtr output) = 0;
    virtual Ret addAuxChannel(ITrackAudioOutputPtr output) = 0;
    virtual Ret removeChannel(const TrackId trackId) = 0;

    virtual AudioSignalChanges masterAudioSignalChanges() const = 0;
};
}
