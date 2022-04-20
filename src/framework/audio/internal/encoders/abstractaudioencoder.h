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

#ifndef MU_AUDIO_ABSTRACTAUDIOENCODER_H
#define MU_AUDIO_ABSTRACTAUDIOENCODER_H

#include "log.h"

#include "audiotypes.h"

namespace mu::audio::encode {
template<class T>
class AbstractAudioEncoder
{
public:
    static size_t requiredOutputBufferSize(samples_t samplesPerChannel)
    {
        return T::outputBufferSize(samplesPerChannel);
    }

    static samples_t encode(const SoundTrackFormat& format, samples_t samplesPerChannel, float* input, char* output)
    {
        IF_ASSERT_FAILED(input && output) {
            return 0;
        }

        return T::doEncode(format, samplesPerChannel, input, output);
    }

    static samples_t flush(char* output, size_t outputSize)
    {
        IF_ASSERT_FAILED(output && outputSize > 0) {
            return 0;
        }

        return T::doFlush(output, outputSize);
    }
};
}

#endif // MU_AUDIO_ABSTRACTAUDIOENCODER_H
