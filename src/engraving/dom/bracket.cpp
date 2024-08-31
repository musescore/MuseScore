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

#include "bracket.h"

#include "types/typesconv.h"

#include "bracketItem.h"
#include "measure.h"
#include "score.h"
#include "staff.h"
#include "system.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace muse::draw;

//---------------------------------------------------------
//   Bracket
//---------------------------------------------------------

Bracket::Bracket(EngravingItem* parent)
    : EngravingItem(ElementType::BRACKET, parent)
{
    m_ay1          = 0;
    m_firstStaff  = 0;
    m_lastStaff   = 0;
    m_bi          = 0;
    m_braceSymbol = SymId::noSym;
    m_magx        = 1.;
    setGenerated(true);       // brackets are not saved
}

Bracket::~Bracket()
{
}

//---------------------------------------------------------
//   playTick
//---------------------------------------------------------

Fraction Bracket::playTick() const
{
    // Brackets always have a tick value of zero, so play from the start of the first measure in the system that the bracket belongs to.
    const auto sys = system();
    if (sys) {
        const auto firstMeasure = sys->firstMeasure();
        if (firstMeasure) {
            return firstMeasure->tick();
        }
    }

    return tick();
}

//---------------------------------------------------------
//   setStaffSpan
//---------------------------------------------------------

void Bracket::setStaffSpan(size_t a, size_t b)
{
    m_firstStaff = a;
    m_lastStaff = b;

    if (bracketType() == BracketType::BRACE
        && style().styleSt(Sid::musicalSymbolFont) != "Emmentaler" && style().styleSt(Sid::musicalSymbolFont) != "Gonville") {
        int v = static_cast<int>(m_lastStaff - m_firstStaff + 1);

        // if staves inner staves are hidden, decrease span
        for (size_t staffIndex = m_firstStaff; staffIndex <= m_lastStaff; ++staffIndex) {
            if (system() && !system()->staff(staffIndex)->show()) {
                --v;
            }
        }

        if (style().styleSt(Sid::musicalSymbolFont) == "Leland") {
            v = std::min(4, v);
        }

        // 1.625 is a "magic" number based on akkoladeDistance/4.0 (default value 6.5).
        m_magx = v + ((v - 1) * 1.625);

        if (v == 1) {
            m_braceSymbol = SymId::braceSmall;
        } else if (v <= 2) {
            m_braceSymbol = SymId::brace;
        } else if (v <= 3) {
            m_braceSymbol = SymId::braceLarge;
        } else {
            m_braceSymbol = SymId::braceLarger;
        }
    }
}

bool Bracket::isEditable() const
{
    return true;
}

bool Bracket::needStartEditingAfterSelecting() const
{
    return true;
}

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Bracket::startEdit(EditData& ed)
{
    EngravingItem::startEdit(ed);
    m_ay1 = pagePos().y();
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> Bracket::gripsPositions(const EditData&) const
{
    return { PointF(0.0, ldata()->bracketHeight()) + pagePos() };
}

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Bracket::endEdit(EditData& ed)
{
    triggerLayoutAll();
    ed.clear(); // score layout invalidates element
}

void Bracket::editDrag(EditData& ed)
{
    double bracketHeight = ldata()->bracketHeight();
    bracketHeight += ed.delta.y();
    mutldata()->bracketHeight.set_value(bracketHeight);

    renderer()->layoutItem(this);
}

//---------------------------------------------------------
//   endEditDrag
//    snap to nearest staff
//---------------------------------------------------------

void Bracket::endEditDrag(EditData&)
{
    double ay2 = m_ay1 + ldata()->bracketHeight();

    staff_idx_t staffIdx1 = staffIdx();
    staff_idx_t staffIdx2;
    size_t n = system()->staves().size();
    if (staffIdx1 + 1 >= n) {
        staffIdx2 = staffIdx1;
    } else {
        double ay  = parentItem()->pagePos().y();
        System* s = system();
        double y   = s->staff(staffIdx1)->y() + ay;
        double h1  = staff()->staffHeight();

        for (staffIdx2 = staffIdx1 + 1; staffIdx2 < n; ++staffIdx2) {
            double h = s->staff(staffIdx2)->y() + ay - y;
            if (ay2 < (y + (h + h1) * .5)) {
                break;
            }
            y += h;
        }
        staffIdx2 -= 1;
    }

    double sy = system()->staff(staffIdx1)->y();
    double ey = system()->staff(staffIdx2)->y() + score()->staff(staffIdx2)->staffHeight();
    mutldata()->bracketHeight.set_value(ey - sy);
    bracketItem()->undoChangeProperty(Pid::BRACKET_SPAN, staffIdx2 - staffIdx1 + 1);
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Bracket::acceptDrop(EditData& data) const
{
    return data.dropElement->type() == ElementType::BRACKET;
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* Bracket::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;
    Bracket* b = 0;
    if (e->isBracket()) {
        b = toBracket(e);
        undoChangeProperty(Pid::SYSTEM_BRACKET, int(b->bracketType()));
    }
    delete e;
    return this;
}

bool Bracket::isEditAllowed(EditData& ed) const
{
    if (ed.key == Key_Up && span() > 1) {
        return true;
    }
    if (ed.key == Key_Down && m_lastStaff < system()->staves().size() - 1) {
        return true;
    }

    if (!(ed.modifiers & ShiftModifier)) {
        return false;
    }

    if (ed.key == Key_Left) {
        return true;
    }
    if (ed.key == Key_Right) {
        if (bracketItem()->column() == 0) {
            return true;
        }
        return true;
    }

    return false;
}

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool Bracket::edit(EditData& ed)
{
    if (!isEditAllowed(ed)) {
        return false;
    }

    if (ed.key == Key_Up && span() > 1) {
        bracketItem()->undoChangeProperty(Pid::BRACKET_SPAN, static_cast<int>(span()) - 1);
        return true;
    }
    if (ed.key == Key_Down && m_lastStaff < system()->staves().size() - 1) {
        bracketItem()->undoChangeProperty(Pid::BRACKET_SPAN, static_cast<int>(span()) + 1);
        return true;
    }

    if (ed.key == Key_Left) {
        bracketItem()->undoChangeProperty(Pid::BRACKET_COLUMN, bracketItem()->column() + 1);
        return true;
    }
    if (ed.key == Key_Right) {
        if (bracketItem()->column() == 0) {
            return true;
        }
        bracketItem()->undoChangeProperty(Pid::BRACKET_COLUMN, bracketItem()->column() - 1);
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Bracket::getProperty(Pid id) const
{
    PropertyValue v = EngravingItem::getProperty(id);
    if (!v.isValid()) {
        v = m_bi->getProperty(id);
    }
    return v;
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Bracket::setProperty(Pid id, const PropertyValue& v)
{
    return m_bi->setProperty(id, v);
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Bracket::propertyDefault(Pid id) const
{
    if (id == Pid::BRACKET_COLUMN) {
        return 0;
    }
    PropertyValue v = EngravingItem::propertyDefault(id);
    if (!v.isValid()) {
        v = m_bi->propertyDefault(id);
    }
    return v;
}

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

muse::TranslatableString Bracket::subtypeUserName() const
{
    return TConv::userName(bracketType());
}

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void Bracket::undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps)
{
    if (id == Pid::COLOR) {
        setColor(v.value<Color>());
    }

    // brackets do not survive layout() and therefore cannot be on
    // the undo stack; delegate to BracketItem:
    BracketItem* bi = bracketItem();
    bi->undoChangeProperty(id, v, ps);
}

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Bracket::setSelected(bool f)
{
    m_bi->setSelected(f);
    EngravingItem::setSelected(f);
}
