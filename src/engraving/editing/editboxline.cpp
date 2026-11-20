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

#include "editboxline.h"

#include "../dom/boxline.h"
#include "../dom/chordrest.h"
#include "../dom/factory.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/select.h"

#include "editspanner.h"

using namespace mu::engraving;

std::vector<BoxLine*> EditBoxLine::addBoxLines(Transaction& tx, Score* score)
{
    std::vector<BoxLine*> boxLines;
    const Selection& selection = score->selection();

    if (selection.isRange()) {
        for (staff_idx_t staffIdx = selection.staffStart(); staffIdx < selection.staffEnd(); ++staffIdx) {
            BoxLine* boxLine = Factory::createBoxLine(score->dummy());
            EditSpanner::addSpanner(tx, score, boxLine, staffIdx, selection.startSegment(), selection.endSegment());
            boxLines.push_back(boxLine);
        }
        return boxLines;
    }

    ChordRest* cr1 = nullptr;
    ChordRest* cr2 = nullptr;
    score->getSelectedStartEndChordRests(cr1, cr2);
    if (!cr1) {
        return boxLines;
    }
    if (!cr2) {
        cr2 = cr1;
    }

    Segment* startSegment = cr1->segment();
    Segment* endSegment = cr2->nextSegmentAfterCR(SegmentType::ChordRest | SegmentType::EndBarLine | SegmentType::Clef);

    BoxLine* boxLine = Factory::createBoxLine(score->dummy());
    EditSpanner::addSpanner(tx, score, boxLine, cr1->staffIdx(), startSegment, endSegment);
    boxLines.push_back(boxLine);

    return boxLines;
}
