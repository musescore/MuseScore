/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "../types/types.h"

namespace mu::engraving {
class ChordRest;
class EngravingItem;
class Lyrics;
class Note;
class Score;
class Segment;

struct ChordRestNavigateOptions {
    bool skipGrace = false;
    bool skipMeasureRepeatRests = true;
    bool disableOverRepeats = false;
};

class Navigation
{
public:
    static ChordRest* nextChordRest(const ChordRest* cr, const ChordRestNavigateOptions& options = {});
    static ChordRest* prevChordRest(const ChordRest* cr, const ChordRestNavigateOptions& options = {});
    static Lyrics* lastLyricsInMeasure(const Segment* seg, const staff_idx_t staffIdx, const int no, const PlacementV& placement);
    static Lyrics* prevLyrics(const Lyrics* lyrics);
    static Lyrics* nextLyrics(const Lyrics* lyrics);
    static EngravingItem* move(Score* score, const String& cmd);
    static EngravingItem* selectMove(Score* score, const String& cmd);

    static EngravingItem* chordNoteAbove(Score* score, EngravingItem* element);
    static EngravingItem* chordNoteBelow(Score* score, EngravingItem* element);
    static Note* topNoteInChord(Note* note);
    static Note* bottomNoteInChord(Note* note);
    static EngravingItem* firstElement(Score* score, bool frame = true);
    static EngravingItem* lastElement(Score* score, bool frame = true);
    static ChordRest* nextMeasure(Score* score, ChordRest* element, bool selectBehavior = false, bool mmRest = false);
    static ChordRest* prevMeasure(Score* score, ChordRest* element, bool mmRest = false);
    static ChordRest* upStaff(ChordRest* cr);
    static ChordRest* downStaff(Score* score, ChordRest* cr);
    static ChordRest* topStaff(Score* score, ChordRest* cr = nullptr);
};
}
