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

#ifndef MU_AUDIO_COMPRESSOR_H
#define MU_AUDIO_COMPRESSOR_H

#include <memory>

#include "audiotypes.h"

namespace mu::audio::dsp {
class Compressor
{
public:
    Compressor() = default;

    void process(float* buffer, const samples_t& samplesPerChannel, const float& audioChannelRms,const audioch_t channelNumber,
                 const audioch_t audioChannelsCount);
};

using CompressorPtr = std::unique_ptr<Compressor>;
}

#endif // MU_AUDIO_COMPRESSOR_H
