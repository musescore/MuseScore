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

#ifndef MU_AUDIO_ISOURCE_H
#define MU_AUDIO_ISOURCE_H

#include <memory>
#include <vector>

#include "async/channel.h"

#include "audiotypes.h"

namespace mu::audio {
class IAudioSource
{
public:
    virtual ~IAudioSource() = default;

    virtual bool isActive() const = 0;
    virtual void setIsActive(bool arg) = 0;

    virtual void seek(const msecs_t newPositionMsecs) { UNUSED(newPositionMsecs) }

    //! set current sample rate. Called by destination.
    virtual void setSampleRate(unsigned int sampleRate) = 0;

    //! return substream count for this source: 1 for mono, 2 for stereo, 6 for surround
    virtual unsigned int audioChannelsCount() const = 0;

    virtual async::Channel<unsigned int> audioChannelsCountChanged() const = 0;

    //! move buffer forward for sampleCount samples
    virtual void process(float* buffer, unsigned int sampleCount) = 0;
};

using IAudioSourcePtr = std::shared_ptr<IAudioSource>;
}

#endif // MU_AUDIO_ISOURCE_H
