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

#include "engravingobject.h"
#include "mscore.h"

namespace mu::engraving {
class Factory;
}

namespace Ms {
//---------------------------------------------------------
//   BracketItem
//---------------------------------------------------------

class BracketItem final : public EngravingItem
{
    BracketType _bracketType { BracketType::NO_BRACKET };
    int _column              { 0 };
    int _bracketSpan        { 0 };
    Staff* _staff            { 0 };

    friend class mu::engraving::Factory;

    BracketItem(EngravingItem* parent);
    BracketItem(EngravingItem* parent, BracketType a, int b);

public:
    Ms::EngravingItem* clone() const override;

    mu::engraving::PropertyValue getProperty(Pid) const override;
    bool setProperty(Pid, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid id) const override;

    int bracketSpan() const { return _bracketSpan; }
    BracketType bracketType() const { return _bracketType; }
    void setBracketSpan(int v) { _bracketSpan = v; }
    void setBracketType(BracketType v) { _bracketType = v; }
    Staff* staff() const { return _staff; }
    void setStaff(Staff* s) { _staff = s; }
    int column() const { return _column; }
    void setColumn(int v) { _column = v; }
};
}
#endif
