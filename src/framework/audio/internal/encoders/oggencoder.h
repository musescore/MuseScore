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

#ifndef MUSE_AUDIO_OGGENCODER_H
#define MUSE_AUDIO_OGGENCODER_H

#include "abstractaudioencoder.h"

struct OggOpusEnc;

namespace muse::audio::encode {
class OggEncoder : public AbstractAudioEncoder
{
public:
    size_t encode(samples_t samplesPerChannel, const float* input) override;
    size_t flush() override;

protected:
    size_t requiredOutputBufferSize(samples_t) const override;
    bool openDestination(const io::path_t& path) override;
    void closeDestination() override;

private:
    OggOpusEnc* m_opusEncoder = nullptr;
};
}

#endif // OGGENCODER_H
