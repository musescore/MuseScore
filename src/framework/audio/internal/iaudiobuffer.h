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
#ifndef MU_AUDIO_IAUDIOBUFFER_H
#define MU_AUDIO_IAUDIOBUFFER_H

#include <memory>
#include "../iaudiosource.h"

namespace mu::audio {
class IAudioBuffer
{
public:
    virtual ~IAudioBuffer() = default;

    virtual void setSource(std::shared_ptr<IAudioSource> source) = 0;
    virtual void forward() = 0;

    virtual void pop(float* dest, size_t sampleCount) = 0;
    virtual void setMinSampleLag(size_t lag) = 0;
};

using IAudioBufferPtr = std::shared_ptr<IAudioBuffer>;
}
#endif // MU_AUDIO_IAUDIOBUFFER_H
