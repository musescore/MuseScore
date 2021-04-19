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
#ifndef MU_AUDIO_AUDIOSTREAM_H
#define MU_AUDIO_AUDIOSTREAM_H

#include <vector>
#include "audio/iaudiostream.h"
#include "samplerateconvertor.h"

namespace mu::audio {
class AudioStream : public IAudioStream
{
public:
    AudioStream();

    //! load data from file wav, mp3 or ogg (automatically checked)
    bool loadFile(const mu::io::path& path) override;

    bool loadMP3FromMemory(const void* pData, size_t dataSize);

    //!convert full source to a new sample rate
    void convertSampleRate(unsigned int sampleRate) override;

    unsigned int channelsCount() const override;

    //! return sample rate of loaded data. Can be different than application's sample rate
    unsigned int sampleRate() const override;

    //! copy samples with real time sample rate convertion if needed
    unsigned int copySamplesToBuffer(float* buffer, unsigned int fromSample, unsigned int sampleCount, unsigned int sampleRate) override;

private:
    bool loadWAV(mu::io::path path);
    bool loadMP3(mu::io::path path);
    bool loadOGG(mu::io::path path);

    unsigned int m_channels = 1;
    unsigned int m_sampleRate = 1;
    std::vector<float> m_data = {};
    SampleRateConvertor m_src;
};
}

#endif // MU_AUDIO_AUDIOSTREAM_H
