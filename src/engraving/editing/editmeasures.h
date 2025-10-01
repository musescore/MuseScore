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

namespace mu::engraving {
class InsertRemoveMeasures : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, InsertRemoveMeasures)

    MeasureBase* fm = nullptr;
    MeasureBase* lm = nullptr;

    static std::vector<Clef*> getCourtesyClefs(Measure* m);

    bool moveStc = true;

protected:
    void removeMeasures();
    void insertMeasures();

public:
    InsertRemoveMeasures(MeasureBase* _fm, MeasureBase* _lm, bool _moveStc)
        : fm(_fm), lm(_lm), moveStc(_moveStc) {}
    virtual void undo(EditData*) override = 0;
    virtual void redo(EditData*) override = 0;
    UNDO_CHANGED_OBJECTS({ fm, lm })
};

class RemoveMeasures : public InsertRemoveMeasures
{
    OBJECT_ALLOCATOR(engraving, RemoveMeasures)
public:
    RemoveMeasures(MeasureBase* m1, MeasureBase* m2, bool moveStc = true)
        : InsertRemoveMeasures(m1, m2, moveStc) {}
    void undo(EditData*) override { insertMeasures(); }
    void redo(EditData*) override { removeMeasures(); }

    UNDO_TYPE(CommandType::RemoveMeasures)
    UNDO_NAME("RemoveMeasures")
};

class InsertMeasures : public InsertRemoveMeasures
{
    OBJECT_ALLOCATOR(engraving, InsertMeasures)
public:
    InsertMeasures(MeasureBase* m1, MeasureBase* m2, bool moveStc = true)
        : InsertRemoveMeasures(m1, m2, moveStc) {}
    void redo(EditData*) override { insertMeasures(); }
    void undo(EditData*) override { removeMeasures(); }

    UNDO_TYPE(CommandType::InsertMeasures)
    UNDO_NAME("InsertMeasures")
};

class ChangeMeasureLen : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeMeasureLen)

    Measure* measure = nullptr;
    Fraction len;

    void flip(EditData*) override;

public:
    ChangeMeasureLen(Measure*, Fraction);

    UNDO_TYPE(CommandType::ChangeMeasureLen)
    UNDO_NAME("ChangeMeasureLen")
    UNDO_CHANGED_OBJECTS({ measure })
};

class ChangeMMRest : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeMMRest)

    Measure* m;
    Measure* mmrest;

    void flip(EditData*) override;

public:
    ChangeMMRest(Measure* _m, Measure* _mmr)
        : m(_m), mmrest(_mmr) {}

    UNDO_TYPE(CommandType::ChangeMMRest)
    UNDO_NAME("ChangeMMRest")
    UNDO_CHANGED_OBJECTS({ m, mmrest })
};

class ChangeMeasureRepeatCount : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeMeasureRepeatCount)

    Measure* m = nullptr;
    int count = 0;
    staff_idx_t staffIdx = muse::nidx;

    void flip(EditData*) override;

public:
    ChangeMeasureRepeatCount(Measure* _m, int _count, staff_idx_t _staffIdx)
        : m(_m), count(_count), staffIdx(_staffIdx) {}

    UNDO_TYPE(CommandType::ChangeMeasureRepeatCount)
    UNDO_NAME("ChangeMeasureRepeatCount")
    UNDO_CHANGED_OBJECTS({ m })
};
}
