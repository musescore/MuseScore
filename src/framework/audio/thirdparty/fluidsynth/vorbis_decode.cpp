/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

 #include "vorbis_decode.h"

extern "C" {
//! NOTE Used in fluid
int vorbis_decode_memory(const unsigned char* mem, unsigned int len, short** output, unsigned int* channels, unsigned int* sample_rate)
{
    if (!muse::audio::fluid::FluidVorbisDecoder::decoder) {
        return -1;
    }
    return muse::audio::fluid::FluidVorbisDecoder::decoder->decode_memory(mem, len, output, channels, sample_rate);
}
}

namespace muse::audio::fluid {
IVorbisDecoder* FluidVorbisDecoder::decoder = nullptr;
}
