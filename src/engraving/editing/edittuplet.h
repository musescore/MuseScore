/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <vector>

#include "../dom/tuplet.h"

namespace mu::engraving {
class Fraction;
class Score;
class Transaction;

class EditTuplet
{
public:
    // Tuplets on the selected chords/rests that use their durations as the subdivision (3:2 on a quarter: 3 quarters
    // in the time of 2). Tuplets that don't fit are left out, and so are measure rests; false (with
    // MsError::NO_SPACE_FOR_TUPLET) if none does
    static bool checkAddTupletsWithSelectedDurations(const Score* score, const Fraction& ratio);

    // Divides each selected chord/rest into a tuplet (3:2 on a quarter: 3 eighths in the time of a quarter). Keeps a range
    // selection, otherwise selects the start of the first tuplet. Returns the tuplets in score order
    // With TupletSelectedDuration::SUBDIVISION, call checkAddTupletsWithSelectedDurations() first, so that a message
    // shows if no tuplet fits
    static std::vector<Tuplet*> addTuplets(Transaction& tx, Score* score, const Fraction& ratio, TupletNumberType numberType,
                                           TupletBracketType bracketType, bool autoBaseLen,
                                           TupletSelectedDuration selectedDurationAs = TupletSelectedDuration::TUPLET_LENGTH);
};
}
