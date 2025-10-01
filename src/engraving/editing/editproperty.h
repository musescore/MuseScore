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
class ChangeProperty : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeProperty)
protected:
    EngravingObject* element = nullptr;
    Pid id;
    PropertyValue property;
    PropertyFlags flags;

    void flip(EditData*) override;

public:
    ChangeProperty(EngravingObject* e, Pid i, const PropertyValue& v, PropertyFlags ps = PropertyFlags::NOSTYLE)
        : element(e), id(i), property(v), flags(ps) {}

    Pid getId() const { return id; }
    EngravingObject* getElement() const { return element; }
    PropertyValue data() const { return property; }

    UNDO_TYPE(CommandType::ChangeProperty)
    UNDO_NAME("ChangeProperty")

    std::vector<EngravingObject*> objectItems() const override;

    bool isFiltered(UndoCommand::Filter f, const EngravingItem* target) const override
    {
        return f == UndoCommand::Filter::ChangePropertyLinked && muse::contains(target->linkList(), element);
    }
};

class ChangeBracketProperty : public ChangeProperty
{
    OBJECT_ALLOCATOR(engraving, ChangeBracketProperty)

    Staff* staff = nullptr;
    size_t level = 0;

    void flip(EditData*) override;

public:
    ChangeBracketProperty(Staff* s, size_t l, Pid i, const PropertyValue& v, PropertyFlags ps = PropertyFlags::NOSTYLE)
        : ChangeProperty(nullptr, i, v, ps), staff(s), level(l) {}
    UNDO_NAME("ChangeBracketProperty")
    UNDO_CHANGED_OBJECTS({ staff })
};

class ChangeTextLineProperty : public ChangeProperty
{
    OBJECT_ALLOCATOR(engraving, ChangeTextLineProperty)

    void flip(EditData*) override;

public:
    ChangeTextLineProperty(EngravingObject* e, PropertyValue v)
        : ChangeProperty(e, Pid::SYSTEM_FLAG, v, PropertyFlags::NOSTYLE) {}
    UNDO_NAME("ChangeTextLineProperty")
};
}
