/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/

#ifndef MU_AUDIO_ISOURCE_H
#define MU_AUDIO_ISOURCE_H

#include <memory>
#include <vector>
#include "async/channel.h"

namespace mu::audio {
class IAudioSource
{
public:
    virtual ~IAudioSource() = default;

    //! set current sample rate. Called by destination.
    virtual void setSampleRate(unsigned int sampleRate) = 0;

    //! return substream count for this source: 1 for mono, 2 for stereo, 6 for surround
    virtual unsigned int streamCount() const = 0;

    virtual async::Channel<unsigned int> streamsCountChanged() const = 0;

    //! move buffer forward for sampleCount samples
    virtual void forward(unsigned int sampleCount) = 0;

    //! const access to the source's output buffer
    virtual const float* data() const = 0;

    //! set the mix step in samples
    virtual void setBufferSize(unsigned int samples) = 0;
};

using IAudioSourcePtr = std::shared_ptr<IAudioSource>;
}

#endif // MU_AUDIO_ISOURCE_H
