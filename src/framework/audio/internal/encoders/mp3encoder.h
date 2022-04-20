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

#ifndef MU_AUDIO_MP3ENCODER_H
#define MU_AUDIO_MP3ENCODER_H

#include "abstractaudioencoder.h"

namespace mu::audio::encode {
class Mp3Encoder : public AbstractAudioEncoder<Mp3Encoder>
{
protected:
    friend class AbstractAudioEncoder;

    static size_t outputBufferSize(samples_t samplesPerChannel);
    static samples_t doEncode(const SoundTrackFormat& format, samples_t samplesPerChannel, float* input, char* output);
    static samples_t doFlush(char* output, size_t outputSize);
};
}

#endif // MU_AUDIO_MP3ENCODER_H
