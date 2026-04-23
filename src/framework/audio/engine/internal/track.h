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

#include <memory>

#include "global/async/asyncable.h"
#include "global/async/channel.h"
#include "global/async/notification.h"

#include "audio/common/audiotypes.h"

#include "../iaudiosource.h"

namespace muse::audio::engine {
enum TrackType {
    Undefined = -1,
    Event_track,
    Sound_track
};

class ITrackAudioInput : public IAudioSource
{
public:
    virtual ~ITrackAudioInput() = default;

    virtual TrackId trackId() const = 0;

    virtual void seek(const TimePosition& position, const bool flushSound = true) = 0;
    virtual void flush() = 0;

    virtual const AudioInputParams& inputParams() const = 0;
    virtual void applyInputParams(const AudioInputParams& requiredParams) = 0;
    virtual async::Channel<AudioInputParams> inputParamsChanged() const = 0;

    virtual void prepareToPlay() = 0;
    virtual bool readyToPlay() const = 0;
    virtual async::Notification readyToPlayChanged() const = 0;

    virtual bool hasPendingChunks() const = 0;
    virtual void processInput() = 0;
    virtual InputProcessingProgress inputProcessingProgress() const = 0;

    virtual void clearCache() = 0;
};

class ITrackAudioOutput : public IAudioSource
{
public:
    virtual ~ITrackAudioOutput() = default;

    virtual const AudioOutputParams& outputParams() const = 0;
    virtual void applyOutputParams(const AudioOutputParams& requiredParams) = 0;
    virtual async::Channel<AudioOutputParams> outputParamsChanged() const = 0;

    virtual AudioSignalChanges audioSignalChanges() const = 0;
};

using ITrackAudioInputPtr = std::shared_ptr<ITrackAudioInput>;
using ITrackAudioOutputPtr = std::shared_ptr<ITrackAudioOutput>;
}
