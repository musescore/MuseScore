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

#include "scoreElement.h"
#include "mscore.h"

namespace Ms {
//---------------------------------------------------------
//   BracketItem
//---------------------------------------------------------

class BracketItem final : public ScoreElement
{
    BracketType _bracketType { BracketType::NO_BRACKET };
    int _column              { 0 };
    int _bracketSpan        { 0 };
    Staff* _staff            { 0 };

public:
    BracketItem(Score* s)
        : ScoreElement(s) {}
    BracketItem(Score* s, BracketType a, int b)
        : ScoreElement(s), _bracketType(a), _bracketSpan(b) { }
    virtual ElementType type() const override { return ElementType::BRACKET_ITEM; }
    virtual QVariant getProperty(Pid) const override;
    virtual bool setProperty(Pid, const QVariant&) override;
    virtual QVariant propertyDefault(Pid id) const override;

//      bool selected() const              { return _selected;    }
    int bracketSpan() const { return _bracketSpan; }
    BracketType bracketType() const { return _bracketType; }
//      void setSelected(bool v)           { _selected = v;       }
    void setBracketSpan(int v) { _bracketSpan = v; }
    void setBracketType(BracketType v) { _bracketType = v; }
    Staff* staff() { return _staff; }
    void setStaff(Staff* s) { _staff = s; }
    int column() const { return _column; }
    void setColumn(int v) { _column = v; }
};
}
#endif
