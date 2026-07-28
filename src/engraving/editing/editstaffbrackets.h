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

#pragma once

#include "transaction/undoablecommand.h"

#include "../dom/staff.h"

namespace mu::engraving {
class Score;
class Transaction;

class EditStaffBrackets
{
public:
    static void undoAddBracket(Score* score, Staff* staff, size_t level, BracketType type, size_t span);
    static void undoRemoveBracket(Score* score, Bracket* bracket);

    static void fillBrackets(Score* score, staff_idx_t staffIdx, size_t idx);
    static void cleanBrackets(Score* score, staff_idx_t staffIdx);

    static void adjustBracketsDel(Score* score, size_t sidx, size_t eidx);
    static void adjustBracketsIns(Score* score, size_t sidx, size_t eidx);

    static void setBracketType(Score* score, staff_idx_t staffIdx, size_t idx, BracketType val);
    static void setBracketSpan(Score* score, staff_idx_t staffIdx, size_t idx, size_t val);
    static void setBracketVisible(Score* score, staff_idx_t staffIdx, size_t idx, bool v);
    static void changeBracketColumn(Score* score, staff_idx_t staffIdx, size_t oldColumn, size_t newColumn);
    static void addBracket(Score* score, staff_idx_t staffIdx, BracketItem*);
    static void insertBracket(Score* score, staff_idx_t staffIdx, BracketItem* b);
};

class RemoveBracket : public UndoableCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveBracket)

    Staff* staff = nullptr;
    size_t level = 0;
    BracketType bracketType = BracketType::NORMAL;
    size_t span = 0;

    void undo() override;
    void redo() override;

public:
    RemoveBracket(Staff* s, size_t l, BracketType t, size_t sp)
        : staff(s), level(l), bracketType(t), span(sp) {}

    UNDO_TYPE(CommandType::RemoveBracket)
    UNDO_NAME("RemoveBracket")
};

class AddBracket : public UndoableCommand
{
    OBJECT_ALLOCATOR(engraving, AddBracket)

    Staff* staff = nullptr;
    size_t level = 0;
    BracketType bracketType = BracketType::NORMAL;
    size_t span = 0;

    void undo() override;
    void redo() override;

public:
    AddBracket(Staff* s, size_t l, BracketType t, size_t sp)
        : staff(s), level(l), bracketType(t), span(sp) {}

    UNDO_TYPE(CommandType::AddBracket)
    UNDO_NAME("AddBracket")
};
}
