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

#include "editstaff.h"

#include "../dom/guitarbend.h"
#include "../dom/masterscore.h"
#include "../dom/measure.h"
#include "../dom/score.h"
#include "../dom/staff.h"
#include "../dom/stafflines.h"
#include "../dom/stafftype.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   InsertStaff
//---------------------------------------------------------

InsertStaff::InsertStaff(Staff* p, staff_idx_t _ridx)
{
    staff = p;
    ridx  = _ridx;
}

void InsertStaff::undo(EditData*)
{
    staff->score()->removeStaff(staff);
}

void InsertStaff::redo(EditData*)
{
    staff->score()->insertStaff(staff, ridx);
}

void InsertStaff::cleanup(bool undo)
{
    if (!undo) {
        delete staff;
        staff = nullptr;
    }
}

//---------------------------------------------------------
//   RemoveStaff
//---------------------------------------------------------

RemoveStaff::RemoveStaff(Staff* p)
{
    staff = p;
    ridx  = staff->rstaff();
    wasSystemObjectStaff = staff->isSystemObjectStaff();
}

void RemoveStaff::undo(EditData*)
{
    staff->score()->insertStaff(staff, ridx);
    if (wasSystemObjectStaff) {
        staff->score()->addSystemObjectStaff(staff);
    }
}

void RemoveStaff::redo(EditData*)
{
    staff->score()->removeStaff(staff);
    if (wasSystemObjectStaff) {
        staff->score()->removeSystemObjectStaff(staff);
    }
}

void RemoveStaff::cleanup(bool undo)
{
    if (undo) {
        delete staff;
        staff = nullptr;
    }
}

//---------------------------------------------------------
//   AddSystemObjectStaff
//---------------------------------------------------------

AddSystemObjectStaff::AddSystemObjectStaff(Staff* s)
    : staff(s)
{
}

void AddSystemObjectStaff::undo(EditData*)
{
    staff->score()->removeSystemObjectStaff(staff);
}

void AddSystemObjectStaff::redo(EditData*)
{
    staff->score()->addSystemObjectStaff(staff);
}

//---------------------------------------------------------
//   RemoveSystemObjectStaff
//---------------------------------------------------------

RemoveSystemObjectStaff::RemoveSystemObjectStaff(Staff* s)
    : staff(s)
{
}

void RemoveSystemObjectStaff::undo(EditData*)
{
    staff->score()->addSystemObjectStaff(staff);
}

void RemoveSystemObjectStaff::redo(EditData*)
{
    staff->score()->removeSystemObjectStaff(staff);
}

//---------------------------------------------------------
//   InsertMStaff
//---------------------------------------------------------

InsertMStaff::InsertMStaff(Measure* m, MStaff* ms, staff_idx_t i)
{
    measure = m;
    mstaff  = ms;
    idx     = i;
}

void InsertMStaff::undo(EditData*)
{
    measure->removeMStaff(mstaff, idx);
}

void InsertMStaff::redo(EditData*)
{
    measure->insertMStaff(mstaff, idx);
}

//---------------------------------------------------------
//   RemoveMStaff
//---------------------------------------------------------

RemoveMStaff::RemoveMStaff(Measure* m, MStaff* ms, int i)
{
    measure = m;
    mstaff  = ms;
    idx     = i;
}

void RemoveMStaff::undo(EditData*)
{
    measure->insertMStaff(mstaff, idx);
}

void RemoveMStaff::redo(EditData*)
{
    measure->removeMStaff(mstaff, idx);
}

//---------------------------------------------------------
//   InsertStaves
//---------------------------------------------------------

InsertStaves::InsertStaves(Measure* m, staff_idx_t _a, staff_idx_t _b)
{
    measure = m;
    a       = _a;
    b       = _b;
}

void InsertStaves::undo(EditData*)
{
    measure->removeStaves(a, b);
}

void InsertStaves::redo(EditData*)
{
    measure->insertStaves(a, b);
}

//---------------------------------------------------------
//   RemoveStaves
//---------------------------------------------------------

RemoveStaves::RemoveStaves(Measure* m, staff_idx_t _a, staff_idx_t _b)
{
    measure = m;
    a       = _a;
    b       = _b;
}

void RemoveStaves::undo(EditData*)
{
    measure->insertStaves(a, b);
}

void RemoveStaves::redo(EditData*)
{
    measure->removeStaves(a, b);
}

//---------------------------------------------------------
//   SortStaves
//---------------------------------------------------------

SortStaves::SortStaves(Score* s, const std::vector<staff_idx_t>& l)
{
    score = s;

    for (staff_idx_t i = 0; i < l.size(); i++) {
        rlist.push_back(muse::indexOf(l, i));
    }
    list  = l;
}

void SortStaves::redo(EditData*)
{
    score->sortStaves(list);
}

void SortStaves::undo(EditData*)
{
    score->sortStaves(rlist);
}

//---------------------------------------------------------
//   ChangeStaff
//---------------------------------------------------------

ChangeStaff::ChangeStaff(Staff* _staff)
    : staff(_staff)
{
    visible = staff->visible();
    clefType = staff->defaultClefType();
    userDist = staff->userDist();
    cutaway = staff->cutaway();
    hideSystemBarLine = staff->hideSystemBarLine();
    mergeMatchingRests = staff->mergeMatchingRests();
    reflectTranspositionInLinkedTab = staff->reflectTranspositionInLinkedTab();
}

ChangeStaff::ChangeStaff(Staff* _staff, bool _visible, ClefTypeList _clefType, Spatium _userDist, bool _cutaway, bool _hideSystemBarLine,
                         AutoOnOff _mergeMatchingRests, bool _reflectTranspositionInLinkedTab)
{
    staff       = _staff;
    visible     = _visible;
    clefType    = _clefType;
    userDist    = _userDist;
    cutaway     = _cutaway;
    hideSystemBarLine  = _hideSystemBarLine;
    mergeMatchingRests = _mergeMatchingRests;
    reflectTranspositionInLinkedTab = _reflectTranspositionInLinkedTab;
}

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeStaff::flip(EditData*)
{
    bool oldVisible = staff->visible();
    ClefTypeList oldClefType = staff->defaultClefType();
    Spatium oldUserDist   = staff->userDist();
    bool oldCutaway     = staff->cutaway();
    bool oldHideSystemBarLine  = staff->hideSystemBarLine();
    AutoOnOff oldMergeMatchingRests = staff->mergeMatchingRests();
    bool oldReflectTranspositionInLinkedTab = staff->reflectTranspositionInLinkedTab();

    staff->setVisible(visible);
    staff->setDefaultClefType(clefType);
    staff->setUserDist(userDist);
    staff->setCutaway(cutaway);
    staff->setHideSystemBarLine(hideSystemBarLine);
    staff->setMergeMatchingRests(mergeMatchingRests);
    staff->setReflectTranspositionInLinkedTab(reflectTranspositionInLinkedTab);

    visible     = oldVisible;
    clefType    = oldClefType;
    userDist    = oldUserDist;
    cutaway     = oldCutaway;
    hideSystemBarLine  = oldHideSystemBarLine;
    mergeMatchingRests = oldMergeMatchingRests;
    reflectTranspositionInLinkedTab = oldReflectTranspositionInLinkedTab;

    staff->triggerLayout();
    staff->masterScore()->rebuildMidiMapping();
    staff->score()->setPlaylistDirty();
}

//---------------------------------------------------------
//   ChangeStaffType::flip
//---------------------------------------------------------

void ChangeStaffType::flip(EditData*)
{
    StaffType oldStaffType = *staff->staffType(Fraction(0, 1));        // TODO

    staff->setStaffType(Fraction(0, 1), staffType);

    bool invisibleChanged = oldStaffType.invisible() != staffType.invisible();
    bool fromTabToStandard = oldStaffType.isTabStaff() && !staffType.isTabStaff();

    staffType = oldStaffType;

    Score* score = staff->score();
    if (invisibleChanged) {
        staff_idx_t staffIdx = staff->idx();
        for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
            m->staffLines(staffIdx)->setVisible(!staff->isLinesInvisible(Fraction(0, 1)));
        }
    }

    if (fromTabToStandard) {
        GuitarBend::adaptBendsFromTabToStandardStaff(staff);
    }

    staff->triggerLayout();
}

//---------------------------------------------------------
//   ChangeMStaffProperties
//---------------------------------------------------------

ChangeMStaffProperties::ChangeMStaffProperties(Measure* m, staff_idx_t i, bool v, bool s)
    : measure(m), staffIdx(i), visible(v), stemless(s)
{
}

void ChangeMStaffProperties::flip(EditData*)
{
    bool v = measure->visible(staffIdx);
    bool s = measure->stemless(staffIdx);
    measure->setStaffVisible(staffIdx, visible);
    measure->setStaffStemless(staffIdx, stemless);
    visible = v;
    stemless = s;
}

//---------------------------------------------------------
//   ChangeMStaffHideIfEmpty
//---------------------------------------------------------

ChangeMStaffHideIfEmpty::ChangeMStaffHideIfEmpty(Measure* m, staff_idx_t i, AutoOnOff h)
    : measure(m), staffIdx(i), hideIfEmpty(h)
{
}

void ChangeMStaffHideIfEmpty::flip(EditData*)
{
    AutoOnOff h = measure->hideStaffIfEmpty(staffIdx);
    measure->setHideStaffIfEmpty(staffIdx, hideIfEmpty);
    measure->triggerLayout(staffIdx);
    hideIfEmpty = h;
}
