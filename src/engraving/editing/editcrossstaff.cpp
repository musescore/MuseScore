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

#include "editcrossstaff.h"

#include "editchord.h"

#include "../dom/chordrest.h"
#include "../dom/part.h"
#include "../dom/score.h"
#include "../dom/staff.h"
#include "../dom/stafftype.h"

using namespace mu::engraving;

void EditCrossStaff::moveUp(Transaction&, Score* score, ChordRest* cr)
{
    Staff* staff  = cr->staff();
    Part* part    = staff->part();
    staff_idx_t rstaff    = staff->rstaff();
    int staffMove = cr->staffMove();

    if ((staffMove == -1) || (static_cast<int>(rstaff) + staffMove <= 0)) {
        return;
    }

    const std::vector<Staff*>& staves = part->staves();
    // we know that staffMove+rstaff-1 index exists due to the previous condition.
    if (staff->staffType(cr->tick())->group() != StaffGroup::STANDARD
        || staves.at(rstaff + staffMove - 1)->staffType(cr->tick())->group() != StaffGroup::STANDARD) {
        LOGD("User attempted to move a note from/to a staff which does not use standard notation - ignoring.");
    } else {
        // move the chord up a staff
        score->undo(new ChangeChordStaffMove(cr, staffMove - 1));
    }
}

void EditCrossStaff::moveDown(Transaction&, Score* score, ChordRest* cr)
{
    Staff* staff  = cr->staff();
    Part* part    = staff->part();
    staff_idx_t rstaff = staff->rstaff();
    int staffMove = cr->staffMove();
    // calculate the number of staves available so that we know whether there is another staff to move down to
    size_t rstaves = part->nstaves();

    if ((staffMove == 1) || (rstaff + staffMove >= rstaves - 1)) {
        LOGD("moveDown staffMove==%d  rstaff %zu rstaves %zu", staffMove, rstaff, rstaves);
        return;
    }

    const std::vector<Staff*>& staves = part->staves();
    // we know that staffMove+rstaff+1 index exists due to the previous condition.
    if (staff->staffType(cr->tick())->group() != StaffGroup::STANDARD
        || staves.at(staffMove + rstaff + 1)->staffType(cr->tick())->group() != StaffGroup::STANDARD) {
        LOGD("User attempted to move a note from/to a staff which does not use standard notation - ignoring.");
    } else {
        // move the chord down a staff
        score->undo(new ChangeChordStaffMove(cr, staffMove + 1));
    }
}
