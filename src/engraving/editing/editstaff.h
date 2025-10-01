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

#include "undo.h"

#include "../dom/measure.h"
#include "../dom/score.h"
#include "../dom/staff.h"

namespace mu::engraving {
class InsertStaff : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, InsertStaff)

    Staff* staff = nullptr;
    staff_idx_t ridx = muse::nidx;

public:
    InsertStaff(Staff*, staff_idx_t idx);
    void undo(EditData*) override;
    void redo(EditData*) override;
    void cleanup(bool) override;

    UNDO_TYPE(CommandType::InsertStaff)
    UNDO_NAME("InsertStaff")
    UNDO_CHANGED_OBJECTS({ staff })
};

class RemoveStaff : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveStaff)

    Staff* staff = nullptr;
    staff_idx_t ridx = muse::nidx;
    bool wasSystemObjectStaff = false;

public:
    RemoveStaff(Staff*);
    void undo(EditData*) override;
    void redo(EditData*) override;
    void cleanup(bool) override;

    UNDO_TYPE(CommandType::RemoveStaff)
    UNDO_NAME("RemoveStaff")
    UNDO_CHANGED_OBJECTS({ staff })
};

class AddSystemObjectStaff : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, AddSystemObjectStaff)

    Staff* staff = nullptr;

public:
    AddSystemObjectStaff(Staff*);
    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::AddSystemObjectStaff)
    UNDO_NAME("AddSystemObjectStaff")
    UNDO_CHANGED_OBJECTS({ staff })
};

class RemoveSystemObjectStaff : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveSystemObjectStaff)

    Staff* staff = nullptr;

public:
    RemoveSystemObjectStaff(Staff*);
    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::RemoveSystemObjectStaff)
    UNDO_NAME("RemoveSystemObjectStaff")
    UNDO_CHANGED_OBJECTS({ staff })
};

class InsertMStaff : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, InsertMStaff)

    Measure* measure = nullptr;
    MStaff* mstaff = nullptr;
    staff_idx_t idx = muse::nidx;

public:
    InsertMStaff(Measure*, MStaff*, staff_idx_t);
    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::InsertMStaff)
    UNDO_NAME("InsertMStaff")
    UNDO_CHANGED_OBJECTS({ measure })
};

class RemoveMStaff : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveMStaff)

    Measure* measure = nullptr;
    MStaff* mstaff = nullptr;
    int idx = 0;

public:
    RemoveMStaff(Measure*, MStaff*, int);
    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::RemoveMStaff)
    UNDO_NAME("RemoveMStaff")
    UNDO_CHANGED_OBJECTS({ measure })
};

class InsertStaves : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, InsertStaves)

    Measure* measure = nullptr;
    staff_idx_t a = muse::nidx;
    staff_idx_t b = muse::nidx;

public:
    InsertStaves(Measure*, staff_idx_t, staff_idx_t);
    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::InsertStaves)
    UNDO_NAME("InsertStaves")
    UNDO_CHANGED_OBJECTS({ measure })
};

class RemoveStaves : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveStaves)

    Measure* measure = nullptr;
    staff_idx_t a = muse::nidx;
    staff_idx_t b = muse::nidx;

public:
    RemoveStaves(Measure*, staff_idx_t, staff_idx_t);
    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::RemoveStaves)
    UNDO_NAME("RemoveStaves")
    UNDO_CHANGED_OBJECTS({ measure })
};

class SortStaves : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, SortStaves)

    Score* score = nullptr;
    std::vector<staff_idx_t> list;
    std::vector<staff_idx_t> rlist;

public:
    SortStaves(Score*, const std::vector<staff_idx_t>&);
    void undo(EditData*) override;
    void redo(EditData*) override;

    UNDO_TYPE(CommandType::SortStaves)
    UNDO_NAME("SortStaves")
    UNDO_CHANGED_OBJECTS({ score })
};

//---------------------------------------------------------
//   ChangeStaff
//---------------------------------------------------------

class ChangeStaff : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeStaff)

    Staff* staff = nullptr;

    bool visible = false;
    ClefTypeList clefType;
    Spatium userDist = Spatium(0.0);
    bool cutaway = false;
    bool hideSystemBarLine = false;
    AutoOnOff mergeMatchingRests = AutoOnOff::AUTO;
    bool reflectTranspositionInLinkedTab = false;

    void flip(EditData*) override;

public:
    ChangeStaff(Staff*);

    ChangeStaff(Staff*, bool _visible, ClefTypeList _clefType, Spatium userDist, bool _cutaway, bool _hideSystemBarLine,
                AutoOnOff _mergeRests, bool _reflectTranspositionInLinkedTab);

    UNDO_TYPE(CommandType::ChangeStaff)
    UNDO_NAME("ChangeStaff")
    UNDO_CHANGED_OBJECTS({ staff })
};

//---------------------------------------------------------
//   ChangeStaffType
//---------------------------------------------------------

class ChangeStaffType : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeStaffType)

    Staff* staff = nullptr;
    StaffType staffType;

    void flip(EditData*) override;

public:
    ChangeStaffType(Staff* s, const StaffType& t)
        : staff(s), staffType(t) {}

    UNDO_TYPE(CommandType::ChangeStaffType)
    UNDO_NAME("ChangeStaffType")
    UNDO_CHANGED_OBJECTS({ staff })
};

class ChangeMStaffProperties : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeMStaffProperties)

    Measure* measure = nullptr;
    staff_idx_t staffIdx = 0;
    bool visible = false;
    bool stemless = false;

    void flip(EditData*) override;

public:
    ChangeMStaffProperties(Measure*, staff_idx_t staffIdx, bool visible, bool stemless);

    UNDO_TYPE(CommandType::ChangeMStaffProperties)
    UNDO_NAME("ChangeMStaffProperties")
    UNDO_CHANGED_OBJECTS({ measure })
};

class ChangeMStaffHideIfEmpty : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeMStaffHideIfEmpty)

    Measure* measure = nullptr;
    staff_idx_t staffIdx = 0;
    AutoOnOff hideIfEmpty = AutoOnOff::AUTO;

    void flip(EditData*) override;

public:
    ChangeMStaffHideIfEmpty(Measure*, staff_idx_t staffIdx, AutoOnOff hideIfEmpty);

    UNDO_TYPE(CommandType::ChangeMStaffHideIfEmpty)
    UNDO_NAME("ChangeMStaffHideIfEmpty")
    UNDO_CHANGED_OBJECTS({ measure })
};
}
