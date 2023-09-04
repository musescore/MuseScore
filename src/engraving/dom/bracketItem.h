/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef __BRACKET_ITEM_H__
#define __BRACKET_ITEM_H__

#include "engravingitem.h"

#include "types/types.h"

namespace mu::engraving {
class Factory;

//---------------------------------------------------------
//   BracketItem
//---------------------------------------------------------

class BracketItem final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, BracketItem)

public:
    EngravingItem* clone() const override;

    PropertyValue getProperty(Pid) const override;
    bool setProperty(Pid, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid id) const override;

    size_t bracketSpan() const { return _bracketSpan; }
    BracketType bracketType() const { return _bracketType; }
    void setBracketSpan(size_t v) { _bracketSpan = v; }
    void setBracketType(BracketType v) { _bracketType = v; }
    Staff* staff() const { return _staff; }
    void setStaff(Staff* s) { _staff = s; }
    size_t column() const { return _column; }
    void setColumn(size_t v) { _column = v; }

private:

    friend class Factory;

    BracketItem(EngravingItem* parent);
    BracketItem(EngravingItem* parent, BracketType a, int b);

    BracketType _bracketType { BracketType::NO_BRACKET };
    size_t _column = 0;
    size_t _bracketSpan = 0;
    Staff* _staff = nullptr;
};
}
#endif
