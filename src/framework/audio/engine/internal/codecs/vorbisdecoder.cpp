/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#include "vorbisdecoder.h"

#include <algorithm>

#if (defined (_MSCVER) || defined (_MSC_VER))
#pragma warning(push)
#pragma warning(disable: 4456) // declaration hides previous local declaration
#endif
#include "thirdparty/stb/stb_vorbis.c"
#if (defined (_MSCVER) || defined (_MSC_VER))
#pragma warning(pop)
#endif

using namespace muse::audio::codec;

extern "C" {
//! NOTE Used in fluid
int vorbis_decode_memory(const unsigned char* mem, unsigned int len, short** output, unsigned int* channels, unsigned int* sample_rate)
{
    return VorbisDecoder::decode_memory(mem, len, output, channels, sample_rate);
}
}

int VorbisDecoder::decode_memory(const unsigned char* mem, unsigned int len, short** output, unsigned int* channels,
                                 unsigned int* sample_rate)
{
    int channels_ = 0;
    int sample_rate_ = 0;
    int samples = stb_vorbis_decode_memory(mem, len, &channels_, &sample_rate_, output);
    if (channels) {
        *channels = channels_;
    }

    if (sample_rate) {
        *sample_rate = sample_rate_;
    }

    return samples;
}

int VorbisDecoder::decode_file(const std::string& filepath, std::vector<float>& output, unsigned int* channels, unsigned int* sample_rate)
{
    int vorbis_error;
    stb_vorbis* decoder = stb_vorbis_open_filename(filepath.c_str(), &vorbis_error, NULL);

    if (!decoder) {
        return -1;
    }

    if (channels) {
        *channels = decoder->channels;
    }

    if (sample_rate) {
        *sample_rate = decoder->sample_rate;
    }

    float predata[1000];
    int total = 0;

    while (auto readed = stb_vorbis_get_samples_float_interleaved(decoder, decoder->channels, predata, 1000)) {
        output.resize(total + readed, 0.f);
        auto start = output.begin() + total;
        std::copy_n(predata, readed, start);

        total += readed;
    }

    stb_vorbis_close(decoder);

    return total;
}
