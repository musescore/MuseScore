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

#ifndef MU_AUDIO_ISEQUENCEIO_H
#define MU_AUDIO_ISEQUENCEIO_H

#include <memory>

#include "retval.h"

#include "audiotypes.h"

namespace mu::audio {
class ISequenceIO
{
public:
    virtual ~ISequenceIO() = default;

    virtual RetVal<AudioInputParams> inputParams(const TrackId id) const = 0;
    virtual RetVal<AudioOutputParams> outputParams(const TrackId id) const = 0;

    virtual void setInputParams(const TrackId id, const AudioInputParams& params) = 0;
    virtual void setOutputParams(const TrackId id, const AudioOutputParams& params) = 0;

    virtual async::Channel<TrackId, AudioInputParams> inputParamsChanged() const = 0;
    virtual async::Channel<TrackId, AudioOutputParams> outputParamsChanged() const = 0;

    virtual async::Channel<audioch_t, float> signalAmplitudeChanged(const TrackId id) const = 0;
    virtual async::Channel<audioch_t, volume_dbfs_t> volumePressureChanged(const TrackId id) const = 0;
};

using ISequenceIOPtr = std::shared_ptr<ISequenceIO>;
}

#endif // MU_AUDIO_ISEQUENCEIO_H
