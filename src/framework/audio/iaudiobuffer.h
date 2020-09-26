//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_AUDIO_IAUDIOBUFFER_H
#define MU_AUDIO_IAUDIOBUFFER_H

#include <memory>
#include "iaudiosource.h"

namespace mu {
namespace audio {
class IAudioBuffer
{
public:
    virtual ~IAudioBuffer() = default;

    virtual void setSource(std::shared_ptr<IAudioSource> source) = 0;
    virtual void forward() = 0;

    virtual void push(const float* source, int sampleCount) = 0;
    virtual void pop(float* dest, unsigned int sampleCount) = 0;
    virtual void setMinSampleLag(unsigned int lag) = 0;
};
using IAudioBufferPtr = std::shared_ptr<IAudioBuffer>;
} // namespace audio
} // namespace mu
#endif // MU_AUDIO_IAUDIOBUFFER_H
