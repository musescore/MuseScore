/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "types/types.h"

namespace mu::engraving {
class ChordRest;
class Lyrics;
class Segment;

struct ChordRestNavigateOptions {
    bool skipGrace = false;
    bool skipMeasureRepeatRests = true;
    bool disableOverRepeats = false;
};

extern int pitch2y(int pitch, int enh, int clefOffset, int key, int& prefix, const char* tversatz);
extern ChordRest* nextChordRest(const ChordRest* cr, const ChordRestNavigateOptions& options = {});
extern ChordRest* prevChordRest(const ChordRest* cr, const ChordRestNavigateOptions& options = {});
extern Lyrics* lastLyricsInMeasure(const Segment* seg, const staff_idx_t staffIdx, const int no, const PlacementV& placement);
extern Lyrics* prevLyrics(const Lyrics* lyrics);
extern Lyrics* nextLyrics(const Lyrics* lyrics);
} // namespace mu::engraving
