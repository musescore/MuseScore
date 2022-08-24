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
#ifndef MU_ENGRAVING_LAYOUTCHORDS_H
#define MU_ENGRAVING_LAYOUTCHORDS_H

#include <vector>

#include "layoutcontext.h"

namespace mu::engraving {
class Chord;
class MStyle;
class Measure;
class Note;
class Score;
class Segment;
class Staff;

class LayoutChords
{
public:

    static void layoutChords1(Score* score, Segment* segment, staff_idx_t staffIdx);
    static double layoutChords2(std::vector<Note*>& notes, bool up);
    static void layoutChords3(const MStyle& style, std::vector<Note*>&, const Staff*, Segment*);
    static void updateGraceNotes(Measure* measure);
    static void repositionGraceNotesAfter(Segment* segment);
    static void appendGraceNotes(Chord* chord);
    static void updateLineAttachPoints(Measure* measure);
    static void doUpdateLineAttachPoints(Chord* chord, bool isFirstInMeasure);
};
}

#endif // MU_ENGRAVING_LAYOUTCHORDS_H
