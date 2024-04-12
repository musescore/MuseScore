/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_ENGRAVING_BRACKET_ITEM_H
#define MU_ENGRAVING_BRACKET_ITEM_H

#include "engravingitem.h"

#include "../types/types.h"

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

    size_t bracketSpan() const { return m_bracketSpan; }
    BracketType bracketType() const { return m_bracketType; }
    void setBracketSpan(size_t v) { m_bracketSpan = v; }
    void setBracketType(BracketType v) { m_bracketType = v; }
    Staff* staff() const { return m_staff; }
    void setStaff(Staff* s) { m_staff = s; }
    size_t column() const { return m_column; }
    void setColumn(size_t v) { m_column = v; }

private:

    friend class Factory;

    BracketItem(EngravingItem* parent);
    BracketItem(EngravingItem* parent, BracketType a, int b);

    BracketType m_bracketType = BracketType::NO_BRACKET;
    size_t m_column = 0;
    size_t m_bracketSpan = 0;
    Staff* m_staff = nullptr;
};
}
#endif
