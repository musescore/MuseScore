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

#include "../dom/staff.h"

namespace mu::engraving {
class RemoveBracket : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveBracket)

    Staff* staff = nullptr;
    size_t level = 0;
    BracketType bracketType = BracketType::NORMAL;
    size_t span = 0;

    void undo(EditData*) override;
    void redo(EditData*) override;

public:
    RemoveBracket(Staff* s, size_t l, BracketType t, size_t sp)
        : staff(s), level(l), bracketType(t), span(sp) {}

    UNDO_TYPE(CommandType::RemoveBracket)
    UNDO_NAME("RemoveBracket")
    UNDO_CHANGED_OBJECTS({ staff })
};

class AddBracket : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, AddBracket)

    Staff* staff = nullptr;
    size_t level = 0;
    BracketType bracketType = BracketType::NORMAL;
    size_t span = 0;

    void undo(EditData*) override;
    void redo(EditData*) override;

public:
    AddBracket(Staff* s, size_t l, BracketType t, size_t sp)
        : staff(s), level(l), bracketType(t), span(sp) {}

    UNDO_TYPE(CommandType::AddBracket)
    UNDO_NAME("AddBracket")
    UNDO_CHANGED_OBJECTS({ staff })
};
}
