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
#ifndef MU_AUDIO_IAUDIOSTREAM_H
#define MU_AUDIO_IAUDIOSTREAM_H

#include "io/path.h"

namespace mu::audio {
class IAudioStream
{
public:
    virtual ~IAudioStream() = default;

    //! load stream's data from file
    virtual bool loadFile(const mu::io::path& path) = 0;

    //! return channel count for this stream
    virtual unsigned int channelsCount() const = 0;

    //! stream's sampleRate. Can be different from audioEngine
    virtual unsigned int sampleRate() const = 0;

    //! convert stream to a new sampleRate
    virtual void convertSampleRate(unsigned int sampleRate) = 0;

    //! fill samples buffer in needed sampleRate
    virtual unsigned int copySamples(float* buffer, unsigned int from, unsigned int sampleCount, unsigned int sampleRate) = 0;
};
}
#endif // MU_AUDIO_IAUDIOSTREAM_H
