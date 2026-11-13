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

namespace mu::engraving {
class EngravingItem;
class Score;
class Slur;
class Transaction;

class EditSlur
{
public:
    struct AddedSlur {
        Slur* slur = nullptr;
        // A hammer-on/pull-off replaced the slur that was there
        bool replacedSlur = false;
        // Added without an end, so it goes to the next chord
        bool toNextChord = false;
    };

    // Adds slurs to the selection: in each track of a range, from a selected note to the next chord, or across a list.
    // A hammer-on/pull-off also replaces a selected slur
    static std::vector<AddedSlur> addSlurs(Transaction& tx, Score* score, const Slur* slurTemplate);

    // Adds a slur from firstItem to secondItem, or to the next chord without secondItem; a barline or a measure's start at
    // one end makes it partial. Returns the slur already between them, unless a hammer-on/pull-off replaces it, and no slur
    // when a partial slur would cross into another repeat section
    static AddedSlur addSlur(Transaction& tx, Score* score, EngravingItem* firstItem, EngravingItem* secondItem, const Slur* slurTemplate);
};
}
