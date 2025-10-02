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

#include "../dom/excerpt.h"
#include "../dom/masterscore.h"
#include "../dom/part.h"

namespace mu::engraving {
class AddExcerpt : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, AddExcerpt)

    Excerpt* excerpt = nullptr;
    bool deleteExcerpt = false;

public:
    AddExcerpt(Excerpt* ex);
    ~AddExcerpt() override;

    void undo(EditData*) override;
    void redo(EditData*) override;

    std::vector<EngravingObject*> objectItems() const override;

    UNDO_TYPE(CommandType::AddExcerpt)
    UNDO_NAME("AddExcerpt")
};

class RemoveExcerpt : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveExcerpt)

    Excerpt* excerpt = nullptr;
    size_t index = muse::nidx;
    bool deleteExcerpt = false;

public:
    RemoveExcerpt(Excerpt* ex);
    ~RemoveExcerpt() override;

    void undo(EditData*) override;
    void redo(EditData*) override;

    std::vector<EngravingObject*> objectItems() const override;

    UNDO_TYPE(CommandType::RemoveExcerpt)
    UNDO_NAME("RemoveExcerpt")
};

class SwapExcerpt : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, SwapExcerpt)

    MasterScore* score = nullptr;
    int pos1 = 0;
    int pos2 = 0;

    void flip(EditData*) override;

public:
    SwapExcerpt(MasterScore* s, int p1, int p2)
        : score(s), pos1(p1), pos2(p2) {}

    UNDO_TYPE(CommandType::SwapExcerpt)
    UNDO_NAME("SwapExcerpt")
    UNDO_CHANGED_OBJECTS({ score })
};

class ChangeExcerptTitle : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeExcerptTitle)

    Excerpt* excerpt = nullptr;
    String title;

    void flip(EditData*) override;

public:
    ChangeExcerptTitle(Excerpt* x, const String& t)
        : excerpt(x), title(t) {}

    UNDO_TYPE(CommandType::ChangeExcerptTitle)
    UNDO_NAME("ChangeExcerptTitle")
};

class AddPartToExcerpt : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, AddPartToExcerpt)

    Excerpt* m_excerpt = nullptr;
    Part* m_part = nullptr;
    size_t m_targetPartIdx = 0;

public:
    AddPartToExcerpt(Excerpt* e, Part* p, size_t targetPartIdx);
    void undo(EditData*) override;
    void redo(EditData*) override;
    void cleanup(bool undo) override;

    UNDO_TYPE(CommandType::AddPartToExcerpt)
    UNDO_NAME("AddPartToExcerpt")
    UNDO_CHANGED_OBJECTS({ m_part })
};
}
