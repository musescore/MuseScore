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
#ifndef MU_AUDIO_IMIXER_H
#define MU_AUDIO_IMIXER_H

#include <memory>

#include "modularity/imoduleexport.h"
#include "async/promise.h"
#include "retval.h"

#include "iaudiosource.h"
#include "internal/worker/clock.h"
#include "internal/worker/imixerchannel.h"
#include "audiotypes.h"

namespace mu::audio {
class IMixer : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMixer)

public:
    virtual ~IMixer() = default;

    virtual IAudioSourcePtr mixedSource() = 0;

    virtual RetVal<IMixerChannelPtr> addChannel(IAudioSourcePtr source, const AudioOutputParams& params,
                                                async::Channel<AudioOutputParams> paramsChanged) = 0;
    virtual Ret removeChannel(const MixerChannelId id) = 0;

    virtual AudioOutputParams masterOutputParams() const = 0;
    virtual void setMasterOutputParams(const AudioOutputParams& params) = 0;
    virtual async::Channel<AudioOutputParams> masterOutputParamsChanged() const = 0;

    virtual void addClock(IClockPtr clock) = 0;
    virtual void removeClock(IClockPtr clock) = 0;

    // root mean square of a processed sample block
    virtual async::Channel<audioch_t, float> masterSignalAmplitudeRmsChanged() const = 0;

    // root mean square of a processed sample block in the "decibels relative to full scale" units
    virtual async::Channel<audioch_t, volume_dbfs_t> masterVolumePressureDbfsChanged() const = 0;
};

using IMixerPtr = std::shared_ptr<IMixer>;
}

#endif // MU_AUDIO_IMIXER_H
