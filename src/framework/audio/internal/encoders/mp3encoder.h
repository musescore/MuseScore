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

struct LameHandler;

namespace mu::audio::encode {
class Mp3Encoder : public AbstractAudioEncoder
{
public:
    bool init(const io::path_t& path, const SoundTrackFormat& format, const samples_t totalSamplesNumber) override;

    size_t encode(samples_t samplesPerChannel, const float* input) override;
    size_t flush() override;

private:
    size_t requiredOutputBufferSize(samples_t totalSamplesNumber) const override;
    void closeDestination() override;

    LameHandler* m_handler = nullptr;
};
}

#endif // MU_AUDIO_MP3ENCODER_H
