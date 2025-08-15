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

#include "importtef.h"

#include "engraving/dom/chordrest.h"

using namespace std;
namespace mu::iex::tabledit {
class TupletHandler
{
public:
    engraving::Fraction doTuplet(const TefNote* const tefNote); // todo rename
    void addCr(engraving::Measure* measure, engraving::ChordRest* cr);
private:
    int count { 0 };  // support overly simple algorithm: simply count notes
    bool inTuplet { false };
    int totalLength { 0 }; // sum of note duration in TablEdit units
    engraving::Tuplet* tuplet { nullptr };
};
} // namespace mu::iex::tabledit
