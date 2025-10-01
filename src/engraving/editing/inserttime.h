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

#include "../dom/score.h"

namespace mu::engraving {
class InsertTime : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, InsertTime)

    Score* score = nullptr;
    Fraction tick;
    Fraction len;

    void redo(EditData*) override;
    void undo(EditData*) override;

public:
    InsertTime(Score* _score, const Fraction& _tick, const Fraction& _len)
        : score(_score), tick(_tick), len(_len) {}

    UNDO_TYPE(CommandType::InsertTime)
    UNDO_NAME("InsertTime")
    UNDO_CHANGED_OBJECTS({ score })
};

class InsertTimeUnmanagedSpanner : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, InsertTimeUnmanagedSpanner)

    Score* score = nullptr;
    Fraction tick;
    Fraction len;

    void flip(EditData*) override;

public:
    InsertTimeUnmanagedSpanner(Score* s, const Fraction& _tick, const Fraction& _len)
        : score(s), tick(_tick), len(_len) {}

    UNDO_TYPE(CommandType::InsertTimeUnmanagedSpanner)
    UNDO_NAME("InsertTimeUnmanagedSpanner")
    UNDO_CHANGED_OBJECTS({ score })
};
}
