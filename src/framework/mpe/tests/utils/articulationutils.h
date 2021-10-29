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

#ifndef MU_MPE_TESTS_ARTICULATIONSPROFILEEXAMPLE_H
#define MU_MPE_TESTS_ARTICULATIONSPROFILEEXAMPLE_H

#include <unordered_map>
#include "mpe/mpetypes.h"

namespace mu::mpe::tests {
inline ArrangementPattern createArrangementPattern(const duration_percentage_t durationFactor,
                                                   const msecs_t timestampOffset)
{
    ArrangementPattern result;
    result.durationFactor = durationFactor;
    result.timestampOffset = timestampOffset;
    return result;
}

inline PitchPattern createSimplePitchPattern(const pitch_level_t incrementDiff = 0.f)
{
    PitchPattern result;

    constexpr int steps = 1.f / PERCENTAGE_PRECISION_STEP;

    for (int i = 0; i < steps; ++i) {
        result.pitchOffsetMap.emplace(i * PERCENTAGE_PRECISION_STEP, i * incrementDiff);
    }

    return result;
}

inline ExpressionPattern createSimpleExpressionPattern(const dynamic_level_t amplitudeLevel)
{
    ExpressionPattern result;

    result.maxAmplitudeLevel = amplitudeLevel;

    constexpr int steps = 1.f / PERCENTAGE_PRECISION_STEP;

    float amplitudeSqrt = std::sqrt(amplitudeLevel);

    for (int i = 0; i < steps; ++i) {
        duration_percentage_t currentPos = i * PERCENTAGE_PRECISION_STEP;
        dynamic_level_t value = amplitudeLevel - std::pow((2 * amplitudeSqrt * currentPos) - amplitudeSqrt, 2);

        result.dynamicOffsetMap.emplace(currentPos, std::max(value, 0.f));
    }

    return result;
}
}

#endif // MU_MPE_TESTS_ARTICULATIONSPROFILEEXAMPLE_H
