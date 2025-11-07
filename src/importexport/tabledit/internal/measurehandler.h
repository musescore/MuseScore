/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#pragma once

#include <vector>

#include "importtef.h"

namespace mu::iex::tabledit {
class MeasureHandler
{
public:
    int actualSize(const std::vector<TefMeasure>& tefMeasures, const size_t idx) const;
    void calculate(const std::vector<TefNote>& tefContents, const std::vector<TefMeasure>& tefMeasures);
    int sumPreviousGaps(const size_t idx) const;
    size_t measureIndex(int tstart, const std::vector<TefMeasure>& tefMeasures) const;
private:
    void dumpActualsAndSumGaps(const std::vector<TefMeasure>& tefMeasures) const;
    void initializeMeasureStartsAndGaps(const std::vector<TefMeasure>& tefMeasures);
    int offsetInMeasure(int tstart, const std::vector<TefMeasure>& tefMeasures);
    void updateGapLeft(std::vector<int>& gapLeft, const int position, const std::vector<TefMeasure>& tefMeasures);
    void updateGapRight(std::vector<int>& gapRight, const TefNote& note, const std::vector<TefMeasure>& tefMeasures);
    void updateGaps(const std::vector<TefNote>& tefContents, const std::vector<TefMeasure>& tefMeasures);
    std::vector<int> gapsLeft;
    std::vector<int> gapsRight;
    std::vector<int> nominalMeasureStarts;
};
} // namespace mu::iex::tabledit
