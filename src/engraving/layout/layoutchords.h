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

#include <QtGlobal>
#include <vector>

#include "layoutcontext.h"

namespace mu::engraving {
class Score;
class Segment;
class Note;
class Staff;
class MStyle;
class Measure;
class Chord;
}

namespace mu::engraving {
class LayoutChords
{
public:

    static void layoutChords1(mu::engraving::Score* score, mu::engraving::Segment* segment, staff_idx_t staffIdx);
    static qreal layoutChords2(std::vector<mu::engraving::Note*>& notes, bool up);
    static void layoutChords3(const mu::engraving::MStyle& style, std::vector<mu::engraving::Note*>&, const mu::engraving::Staff*,
                              mu::engraving::Segment*);
    static void updateGraceNotes(mu::engraving::Measure* measure);
    static void repositionGraceNotesAfter(mu::engraving::Segment* segment);
    static void appendGraceNotes(mu::engraving::Chord* chord);
};
}

#endif // MU_ENGRAVING_LAYOUTCHORDS_H
