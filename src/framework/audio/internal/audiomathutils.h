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

#ifndef MU_AUDIO_AUDIOMATHUTILS_H
#define MU_AUDIO_AUDIOMATHUTILS_H

#include <cmath>

#include "audiotypes.h"

namespace mu::audio {
inline float balanceGain(const balance_t balance, const int audioChannelNumber)
{
    return 0.5f * balance * ((audioChannelNumber * 2.f) - 1) + 0.5f;
}

inline float gainFromDecibels(const volume_dbfs_t volumeLevelDb)
{
    return std::pow(10.0f, volumeLevelDb * 0.05f);
}

inline volume_dbfs_t dbFullScaleFromSample(const float signalValue)
{
    return 20 * std::log10(std::abs(signalValue));
}

inline float samplesRootMeanSquare(float&& squaredSum, const samples_t sampleCount)
{
    return std::sqrt(squaredSum / sampleCount);
}
}

#endif // MU_AUDIO_AUDIOMATHUTILS_H
