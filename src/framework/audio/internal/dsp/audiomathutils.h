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

#ifndef MUSE_AUDIO_AUDIOMATHUTILS_H
#define MUSE_AUDIO_AUDIOMATHUTILS_H

#include <cstdlib>
#include <limits>

#include "../../audiotypes.h"

namespace muse::audio::dsp {
inline float balanceGain(const balance_t balance, const int audioChannelNumber)
{
    return (audioChannelNumber * 2 - 1) * balance + 1.f;
}

inline float samplesRootMeanSquare(const float squaredSum, const samples_t sampleCount)
{
    return std::sqrt(squaredSum / sampleCount);
}

inline float sampleAttackTimeCoefficient(const unsigned int sampleRate, const float attackTimeInSecs)
{
    return std::exp(-std::log(9) / (sampleRate * attackTimeInSecs));
}

inline float sampleReleaseTimeCoefficient(const unsigned int sampleRate, const float releaseTimeInSecs)
{
    return std::exp(-std::log(9) / (sampleRate * releaseTimeInSecs));
}

inline void multiplySamples(float* buffer, const audioch_t& audioChannelsCount,
                            const audioch_t& audioChannelNumber, const samples_t& samplesPerChannel,
                            const float& multiplier)
{
    for (samples_t i = 0; i < samplesPerChannel; ++i) {
        int idx = i * audioChannelsCount + audioChannelNumber;

        buffer[idx] *= multiplier;
    }
}

template<typename T>
constexpr T convertFloatSamples(float value)
{
    return static_cast<T>(value * std::numeric_limits<T>::max() - 1);
}
}

#endif // MUSE_AUDIO_AUDIOMATHUTILS_H
