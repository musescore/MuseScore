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

namespace mu::engraving {
class AddElement : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, AddElement)

    EngravingItem* element = nullptr;

    void endUndoRedo(bool) const;
    void undo(EditData*) override;
    void redo(EditData*) override;

public:
    AddElement(EngravingItem*);
    EngravingItem* getElement() const { return element; }
    void cleanup(bool) override;
    const char* name() const override;

    bool isFiltered(UndoCommand::Filter f, const EngravingItem* target) const override;

    std::vector<EngravingObject*> objectItems() const override;

    UNDO_TYPE(CommandType::AddElement)
};

class RemoveElement : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveElement)

    EngravingItem* element = nullptr;

public:
    RemoveElement(EngravingItem*);
    void undo(EditData*) override;
    void redo(EditData*) override;
    void cleanup(bool) override;
    const char* name() const override;

    bool isFiltered(UndoCommand::Filter f, const EngravingItem* target) const override;

    std::vector<EngravingObject*> objectItems() const override;

    UNDO_TYPE(CommandType::RemoveElement)
};

class ChangeElement : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeElement)

    EngravingItem* oldElement = nullptr;
    EngravingItem* newElement = nullptr;

    void flip(EditData*) override;

public:
    ChangeElement(EngravingItem* oldElement, EngravingItem* newElement);

    UNDO_TYPE(CommandType::ChangeElement)
    UNDO_NAME("ChangeElement")
    UNDO_CHANGED_OBJECTS({ oldElement, newElement })
};

class ChangeParent : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeParent)

    EngravingItem* element = nullptr;
    EngravingItem* parent = nullptr;
    staff_idx_t staffIdx = muse::nidx;

    void flip(EditData*) override;

public:
    ChangeParent(EngravingItem* e, EngravingItem* p, staff_idx_t si)
        : element(e), parent(p), staffIdx(si) {}

    UNDO_TYPE(CommandType::ChangeParent)
    UNDO_NAME("ChangeParent")
    UNDO_CHANGED_OBJECTS({ element })
};

class LinkUnlink : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, LinkUnlink)

    bool mustDelete  { false };

protected:
    LinkedObjects* le = nullptr;
    EngravingObject* e = nullptr;

    void link();
    void unlink();

public:
    LinkUnlink() {}
    ~LinkUnlink();
};

class Unlink : public LinkUnlink
{
    OBJECT_ALLOCATOR(engraving, Unlink)
public:
    Unlink(EngravingObject*);
    void undo(EditData*) override { link(); }
    void redo(EditData*) override { unlink(); }

    UNDO_TYPE(CommandType::Unlink)
    UNDO_NAME("Unlink")
};

class Link : public LinkUnlink
{
    OBJECT_ALLOCATOR(engraving, Link)
public:
    Link(EngravingObject*, EngravingObject*);

    void undo(EditData*) override { unlink(); }
    void redo(EditData*) override { link(); }

    UNDO_TYPE(CommandType::Link)
    UNDO_NAME("Link")

    bool isFiltered(UndoCommand::Filter f, const EngravingItem* target) const override;
};
}
