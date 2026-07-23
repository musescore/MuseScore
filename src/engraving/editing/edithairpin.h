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

#include <vector>

#include "draw/types/geometry.h"

#include "../types/types.h"

namespace mu::engraving {
class ChordRest;
class Dynamic;
class Fraction;
class Hairpin;
class Score;
class Transaction;

enum class HairpinType : signed char;

class EditHairpin
{
public:
    static std::vector<Hairpin*> addHairpins(Transaction& tx, Score* score, HairpinType type);
    static Hairpin* addHairpin(Transaction& tx, Score* score, HairpinType type, ChordRest* cr1, ChordRest* cr2 = nullptr);
    static Hairpin* addHairpin(Transaction& tx, Score* score, HairpinType type, Fraction sTick, Fraction eTick, track_idx_t track);
    static void addHairpin(Transaction& tx, Score* score, Hairpin* hairpin, ChordRest* cr1, ChordRest* cr2 = nullptr);
    static void addHairpinToDynamic(Transaction& tx, Score* score, Hairpin* hairpin, Dynamic* dynamic);
    static Hairpin* addHairpinToDynamicOnGripDrag(Transaction& tx, Score* score, Dynamic* dynamic, bool isLeftGrip,
                                                  const muse::PointF& pos);
};
}
