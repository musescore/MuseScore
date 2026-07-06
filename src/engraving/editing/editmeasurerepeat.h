/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "../types/types.h"

namespace mu::engraving {
class Fraction;
class Measure;
class MeasureRepeat;
class Score;
class Transaction;

class EditMeasureRepeat
{
public:
    static void addMeasureRepeat(Transaction& tx, Score* score, Measure* firstMeasure, int numMeasures, staff_idx_t staffIdx);
    static bool makeMeasureRepeatGroup(Transaction& tx, Score* score, Measure* firstMeasure, int numMeasures, staff_idx_t staffIdx);

    // Creates one MeasureRepeat at tick of subtype numMeasures, creating a segment if necessary.
    // Does NOT set measureRepeatCount or do anything else with the measure(s)!
    static MeasureRepeat* addMeasureRepeat(Transaction& tx, Score* score, const Fraction& tick, track_idx_t track, int numMeasures);
};
}
