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

#ifndef MUSE_MPE_TESTS_ARTICULATIONSPROFILEEXAMPLE_H
#define MUSE_MPE_TESTS_ARTICULATIONSPROFILEEXAMPLE_H

#include "mpe/mpetypes.h"

namespace muse::mpe::tests {
inline ArrangementPattern createArrangementPattern(const duration_percentage_t durationFactor,
                                                   const duration_percentage_t timestampOffset)
{
    ArrangementPattern result;
    result.durationFactor = durationFactor;
    result.timestampOffset = timestampOffset;
    return result;
}

inline PitchPattern createSimplePitchPattern(const pitch_level_t incrementDiff = 0)
{
    PitchPattern result;

    for (size_t i = 0; i < EXPECTED_SIZE; ++i) {
        result.pitchOffsetMap.insert_or_assign(static_cast<int>(i) * TEN_PERCENT, static_cast<int>(i) * incrementDiff);
    }

    return result;
}

inline ExpressionPattern createSimpleExpressionPattern(const dynamic_level_t amplitudeLevel)
{
    ExpressionPattern result;

    double amplitudeSqrt = std::sqrt(amplitudeLevel);

    for (size_t i = 0; i < EXPECTED_SIZE; ++i) {
        duration_percentage_t currentPos = static_cast<int>(i) * TEN_PERCENT;
        dynamic_level_t value = amplitudeLevel - static_cast<dynamic_level_t>(std::pow(
                                                                                  (2
                                                                                   * (amplitudeSqrt / static_cast<float>(HUNDRED_PERCENT))
                                                                                   * currentPos) - amplitudeSqrt, 2));

        result.dynamicOffsetMap.insert_or_assign(currentPos, value);
    }

    return result;
}
}

#endif // MUSE_MPE_TESTS_ARTICULATIONSPROFILEEXAMPLE_H
