/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "editstaffbrackets.h"

#include "../dom/bracket.h"
#include "../dom/bracketitem.h"
#include "../dom/factory.h"
#include "../dom/score.h"
#include "../dom/staff.h"

using namespace mu::engraving;

void EditStaffBrackets::undoAddBracket(Score* score, Staff* staff, size_t level, BracketType type, size_t span)
{
    staff_idx_t startStaffIdx = staff->idx();
    staff_idx_t totStaves = score->nstaves();

    // Make sure this brackets won't overlap with others sharing same column.
    // If overlaps are found, move the other brackets outwards (i.e. increase column).
    for (staff_idx_t staffIdx = startStaffIdx; staffIdx < startStaffIdx + span && staffIdx < totStaves; ++staffIdx) {
        const std::vector<BracketItem*>& scoreBrackets = score->brackets(staffIdx);

        bool collision = false;
        for (BracketItem* b : scoreBrackets) {
            if (b->bracketType() != BracketType::NO_BRACKET && b->bracketType() != BracketType::GROUP
                && b->column() == level) {
                collision = true;
                break;
            }
        }

        if (!collision) {
            continue;
        }

        for (int i = static_cast<int>(scoreBrackets.size()) - 1; i >= static_cast<int>(level); --i) {
            if (i >= static_cast<int>(scoreBrackets.size())) {
                // This might theoretically happen when a lot of brackets get cleaned up
                // after changing the column of the first bracket we see
                continue;
            }

            BracketItem* bi = scoreBrackets[i];
            if (bi->column() > level) {
                continue;
            }

            if (bi->bracketType() == BracketType::NO_BRACKET || bi->bracketType() == BracketType::GROUP) {
                continue;
            }

            bi->undoChangeProperty(Pid::BRACKET_COLUMN, bi->column() + 1);
        }
    }

    score->undo(new AddBracket(staff, level, type, span));
}

void EditStaffBrackets::undoRemoveBracket(Score* score, Bracket* b)
{
    score->undo(new RemoveBracket(b->staff(), b->column(), b->bracketType(), b->span()));
}

void EditStaffBrackets::fillBrackets(Score* score, staff_idx_t staffIdx, size_t idx)
{
    // make sure index idx is valid
    std::vector<BracketItem*>& staffBrackets = score->brackets(staffIdx);
    for (size_t i = staffBrackets.size(); i <= idx; ++i) {
        BracketItem* bi = Factory::createBracketItem(score->dummy());
        bi->setStartStaffIdx(staffIdx);
        bi->setColumn(i);
        bi->setScore(score);
        staffBrackets.push_back(bi);
    }
}

void EditStaffBrackets::cleanBrackets(Score* score, staff_idx_t staffIdx)
{
    // remove NO_BRACKET entries from the end of list
    std::vector<BracketItem*>& staffBrackets = score->brackets(staffIdx);
    while (!staffBrackets.empty() && (staffBrackets.back()->bracketType() == BracketType::NO_BRACKET)) {
        BracketItem* bi = muse::takeLast(staffBrackets);
        delete bi;
    }
}

void EditStaffBrackets::adjustBracketsDel(Score* score, size_t sidx, size_t eidx)
{
    IF_ASSERT_FAILED(sidx < eidx && eidx <= score->staves().size()) {
        return;
    }

    for (size_t staffIdx = 0; staffIdx < eidx; ++staffIdx) {
        const std::vector<BracketItem*> staffBrackets = score->brackets(staffIdx);
        for (BracketItem* bi : staffBrackets) {
            size_t span = bi->bracketSpan();
            if ((span == 0) || ((staffIdx + span) <= sidx)) {
                continue;
            }
            const bool startsOutsideDeletedRange = (staffIdx < sidx);
            const bool endsOutsideDeletedRange = ((staffIdx + span) >= eidx);
            if (startsOutsideDeletedRange && endsOutsideDeletedRange) {
                // Shorten the bracket by the number of staves deleted
                bi->undoChangeProperty(Pid::BRACKET_SPAN, int(span - (eidx - sidx)));
            } else if (startsOutsideDeletedRange) {
                // Shorten the bracket by the number of staves deleted that were spanned by it
                bi->undoChangeProperty(Pid::BRACKET_SPAN, int(sidx - staffIdx));
            } else if (endsOutsideDeletedRange) {
                const size_t column = bi->column();
                const BracketType bracketType = bi->bracketType();
                score->undo(new RemoveBracket(score->staves().at(staffIdx), bi->column(), bi->bracketType(), span));
                int newSpan = int(span - (eidx - staffIdx));
                if (eidx < score->staves().size() && newSpan > 0) {
                    // Move the bracket past the end of the deleted range,
                    // and shorten it by the number of staves deleted that were spanned by it.

                    EditStaffBrackets::undoAddBracket(score, score->staves().at(eidx), column, bracketType, newSpan);
                }
            }
        }
    }
}

void EditStaffBrackets::adjustBracketsIns(Score* score, size_t sidx, size_t eidx)
{
    for (size_t staffIdx = 0; staffIdx < score->staves().size(); ++staffIdx) {
        const std::vector<BracketItem*>& staffBrackets = score->brackets(staffIdx);
        for (BracketItem* bi : staffBrackets) {
            size_t span = bi->bracketSpan();
            if ((span == 0) || ((staffIdx + span) < sidx) || (staffIdx > eidx)) {
                continue;
            }
            if ((sidx >= staffIdx) && (eidx <= (staffIdx + span))) {
                bi->undoChangeProperty(Pid::BRACKET_SPAN, int(span + (eidx - sidx)));
            }
        }
    }
}

void EditStaffBrackets::setBracketType(Score* score, staff_idx_t staffIdx, size_t idx, BracketType val)
{
    std::vector<BracketItem*>& staffBrackets = score->brackets(staffIdx);

    fillBrackets(score, staffIdx, idx);
    staffBrackets[idx]->setBracketType(val);
    cleanBrackets(score, staffIdx);
}

void EditStaffBrackets::changeBracketColumn(Score* score, staff_idx_t staffIdx, size_t oldColumn, size_t newColumn)
{
    if (oldColumn == newColumn) {
        return;
    }
    std::vector<BracketItem*>& staffBrackets = score->brackets(staffIdx);

    size_t idx = std::max(oldColumn, newColumn);
    fillBrackets(score, staffIdx, idx);
    int step = newColumn > oldColumn ? 1 : -1;
    for (size_t i = oldColumn; i != newColumn; i += step) {
        size_t oldIdx = i;
        size_t newIdx = i + step;
        staffBrackets[oldIdx]->setColumn(newIdx);
        staffBrackets[newIdx]->setColumn(oldIdx);
        muse::swapItemsAt(staffBrackets, oldIdx, newIdx);
    }
    cleanBrackets(score, staffIdx);
}

void EditStaffBrackets::setBracketSpan(Score* score, staff_idx_t staffIdx, size_t idx, size_t val)
{
    const std::vector<BracketItem*>& staffBrackets = score->brackets(staffIdx);
    fillBrackets(score, staffIdx, idx);
    staffBrackets[idx]->setBracketSpan(val);
}

void EditStaffBrackets::setBracketVisible(Score* score, staff_idx_t staffIdx, size_t idx, bool v)
{
    const std::vector<BracketItem*>& staffBrackets = score->brackets(staffIdx);
    fillBrackets(score, staffIdx, idx);
    staffBrackets[idx]->setVisible(v);
}

void EditStaffBrackets::addBracket(Score* score, staff_idx_t staffIdx, BracketItem* b)
{
    std::vector<BracketItem*>& staffBrackets = score->brackets(staffIdx);
    b->setStartStaffIdx(staffIdx);
    if (!staffBrackets.empty() && staffBrackets[0]->bracketType() == BracketType::NO_BRACKET) {
        delete staffBrackets[0];
        staffBrackets[0] = b;
    } else {
        //
        // create new bracket level
        //
        for (staff_idx_t sidx = 0; sidx < score->nstaves(); ++sidx) {
            if (sidx == staffIdx) {
                staffBrackets.push_back(b);
                b->setScore(score);
            } else {
                BracketItem* bi = Factory::createBracketItem(score->dummy());
                bi->setStartStaffIdx(sidx);
                bi->setScore(score);
                score->brackets(sidx).push_back(bi);
            }
        }
    }
}

void EditStaffBrackets::insertBracket(Score* score, staff_idx_t staffIdx, BracketItem* b)
{
    std::vector<BracketItem*>& staffBrackets = score->brackets(staffIdx);
    b->setStartStaffIdx(staffIdx);
    size_t column = b->column();
    if (column < staffBrackets.size()) {
        if (staffBrackets[column]) {
            delete staffBrackets[column];
        }
        staffBrackets[column] = b;
    } else if (column == staffBrackets.size()) {
        staffBrackets.push_back(b);
    } else {
        fillBrackets(score, staffIdx, column - 1);
        staffBrackets.push_back(b);
    }
}

//---------------------------------------------------------
//   AddBracket
//---------------------------------------------------------

void AddBracket::redo()
{
    EditStaffBrackets::setBracketType(staff->score(), staff->idx(), level, bracketType);
    EditStaffBrackets::setBracketSpan(staff->score(), staff->idx(), level, span);
    staff->triggerLayout();
}

void AddBracket::undo()
{
    EditStaffBrackets::setBracketType(staff->score(), staff->idx(), level, BracketType::NO_BRACKET);
    staff->triggerLayout();
}

//---------------------------------------------------------
//   RemoveBracket
//---------------------------------------------------------

void RemoveBracket::redo()
{
    EditStaffBrackets::setBracketType(staff->score(), staff->idx(), level, BracketType::NO_BRACKET);
    staff->triggerLayout();
}

void RemoveBracket::undo()
{
    EditStaffBrackets::setBracketType(staff->score(), staff->idx(), level, bracketType);
    EditStaffBrackets::setBracketSpan(staff->score(), staff->idx(), level, span);
    staff->triggerLayout();
}
