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
#ifndef MUSE_AUDIO_IAUDIOSTREAM_H
#define MUSE_AUDIO_IAUDIOSTREAM_H

#include "global/io/path.h"

namespace muse::audio {
class IAudioStream
{
public:
    virtual ~IAudioStream() = default;

    //! load stream's data from file
    virtual bool loadFile(const io::path_t& path) = 0;

    //! return channel count for this stream
    virtual unsigned int channelsCount() const = 0;

    //! stream's sampleRate. Can be different from audioEngine
    virtual unsigned int sampleRate() const = 0;

    //! convert stream to a new sampleRate
    virtual void convertSampleRate(unsigned int sampleRate) = 0;

    //! fill samples buffer in needed sampleRate
    virtual unsigned int copySamplesToBuffer(float* buffer, unsigned int from, unsigned int sampleCount, unsigned int sampleRate) = 0;
};
}
#endif // MUSE_AUDIO_IAUDIOSTREAM_H
