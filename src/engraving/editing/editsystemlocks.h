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
class SystemLock;

class AddSystemLock : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, AddSystemLock)

    const SystemLock* m_systemLock;
public:
    AddSystemLock(const SystemLock* systemLock);
    void undo(EditData*) override;
    void redo(EditData*) override;
    void cleanup(bool undo) override;

    UNDO_NAME("AddSystemLock")
    std::vector<EngravingObject*> objectItems() const override;
};

class RemoveSystemLock : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveSystemLock)

    const SystemLock* m_systemLock;
public:
    RemoveSystemLock(const SystemLock* systemLock);
    void undo(EditData*) override;
    void redo(EditData*) override;
    void cleanup(bool undo) override;

    UNDO_NAME("RemoveSystemLock")
    std::vector<EngravingObject*> objectItems() const override;
};
}
