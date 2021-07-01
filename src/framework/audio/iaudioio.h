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

#ifndef MU_AUDIO_IAUDIOIO_H
#define MU_AUDIO_IAUDIOIO_H

#include <memory>

#include "async/promise.h"
#include "async/channel.h"

#include "audiotypes.h"

namespace mu::audio {
class IAudioIO
{
public:
    virtual ~IAudioIO() = default;

    virtual async::Promise<AudioInputParams> inputParams(const TrackSequenceId sequenceId, const TrackId trackId) const = 0;
    virtual void setInputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioInputParams& params) = 0;
    virtual async::Channel<TrackSequenceId, TrackId, AudioInputParams> inputParamsChanged() const = 0;

    virtual async::Promise<AudioOutputParams> outputParams(const TrackSequenceId sequenceId, const TrackId trackId) const = 0;
    virtual void setOutputParams(const TrackSequenceId sequenceId, const TrackId trackId, const AudioOutputParams& params) = 0;
    virtual async::Channel<TrackSequenceId, TrackId, AudioOutputParams> outputParamsChanged() const = 0;

    virtual async::Promise<AudioOutputParams> globalOutputParams() const = 0;
    virtual void setGlobalOutputParams(const AudioOutputParams& params) = 0;
    virtual async::Channel<AudioOutputParams> globalOutputParamsChanged() const = 0;

    virtual async::Channel<audioch_t, float> masterSignalAmplitudeChanged() const = 0;
    virtual async::Channel<audioch_t, volume_dbfs_t> masterVolumePressureChanged() const = 0;
};

using IAudioIOPtr = std::shared_ptr<IAudioIO>;
}

#endif // MU_AUDIO_IAUDIOIO_H
