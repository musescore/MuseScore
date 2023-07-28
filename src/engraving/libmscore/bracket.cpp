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

#include "bracket.h"

#include "draw/types/brush.h"

#include "bracketItem.h"
#include "measure.h"
#include "score.h"
#include "staff.h"
#include "system.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::draw;

namespace mu::engraving {
//---------------------------------------------------------
//   Bracket
//---------------------------------------------------------

Bracket::Bracket(EngravingItem* parent)
    : EngravingItem(ElementType::BRACKET, parent)
{
    m_ay1          = 0;
    m_h2           = 3.5 * spatium();
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
//   setHeight
//---------------------------------------------------------

void Bracket::setHeight(double h)
{
    m_h2 = h * .5;
}

//---------------------------------------------------------
//   width
//---------------------------------------------------------

double Bracket::width() const
{
    double w;
    switch (bracketType()) {
    case BracketType::BRACE:
        if (style().styleSt(Sid::MusicalSymbolFont) == "Emmentaler" || style().styleSt(Sid::MusicalSymbolFont) == "Gonville") {
            w = style().styleMM(Sid::akkoladeWidth) + style().styleMM(Sid::akkoladeBarDistance);
        } else {
            w = (symWidth(m_braceSymbol) * m_magx) + style().styleMM(Sid::akkoladeBarDistance);
        }
        break;
    case BracketType::NORMAL:
        w = style().styleMM(Sid::bracketWidth) + style().styleMM(Sid::bracketDistance);
        break;
    case BracketType::SQUARE:
        w = style().styleMM(Sid::staffLineWidth) / 2 + 0.5 * spatium();
        break;
    case BracketType::LINE:
        w = 0.67 * style().styleMM(Sid::bracketWidth) + style().styleMM(Sid::bracketDistance);
        break;
    case BracketType::NO_BRACKET:
    default:
        w = 0.0;
        break;
    }
    return w;
}

//---------------------------------------------------------
//   setStaffSpan
//---------------------------------------------------------

void Bracket::setStaffSpan(size_t a, size_t b)
{
    m_firstStaff = a;
    m_lastStaff = b;

    if (bracketType() == BracketType::BRACE
        && style().styleSt(Sid::MusicalSymbolFont) != "Emmentaler" && style().styleSt(Sid::MusicalSymbolFont) != "Gonville") {
        int v = static_cast<int>(m_lastStaff - m_firstStaff + 1);

        // if staves inner staves are hidden, decrease span
        for (size_t staffIndex = m_firstStaff; staffIndex <= m_lastStaff; ++staffIndex) {
            if (system() && !system()->staff(staffIndex)->show()) {
                --v;
            }
        }

        if (style().styleSt(Sid::MusicalSymbolFont) == "Leland") {
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

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Bracket::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    if (m_h2 == 0.0) {
        return;
    }
    switch (bracketType()) {
    case BracketType::BRACE: {
        if (m_braceSymbol == SymId::noSym) {
            painter->setNoPen();
            painter->setBrush(Brush(curColor()));
            painter->drawPath(m_path);
        } else {
            double h        = 2 * m_h2;
            double mag      = h / (100 * magS());
            painter->setPen(curColor());
            painter->save();
            painter->scale(m_magx, mag);
            drawSymbol(m_braceSymbol, painter, PointF(0, 100 * magS()));
            painter->restore();
        }
    }
    break;
    case BracketType::NORMAL: {
        double h        = 2 * m_h2;
        double _spatium = spatium();
        double w        = style().styleMM(Sid::bracketWidth);
        double bd       = (style().styleSt(Sid::MusicalSymbolFont) == "Leland") ? _spatium * .5 : _spatium * .25;
        Pen pen(curColor(), w, PenStyle::SolidLine, PenCapStyle::FlatCap);
        painter->setPen(pen);
        painter->drawLine(LineF(0.0, -bd - w * .5, 0.0, h + bd + w * .5));
        double x    =  -w * .5;
        double y1   = -bd;
        double y2   = h + bd;
        drawSymbol(SymId::bracketTop, painter, PointF(x, y1));
        drawSymbol(SymId::bracketBottom, painter, PointF(x, y2));
    }
    break;
    case BracketType::SQUARE: {
        double h = 2 * m_h2;
        double lineW = style().styleMM(Sid::staffLineWidth);
        double bracketWidth = width() - lineW / 2;
        Pen pen(curColor(), lineW, PenStyle::SolidLine, PenCapStyle::FlatCap);
        painter->setPen(pen);
        painter->drawLine(LineF(0.0, 0.0, 0.0, h));
        painter->drawLine(LineF(-lineW / 2, 0.0, lineW / 2 + bracketWidth, 0.0));
        painter->drawLine(LineF(-lineW / 2, h, lineW / 2 + bracketWidth, h));
    }
    break;
    case BracketType::LINE: {
        double h = 2 * m_h2;
        double w = 0.67 * style().styleMM(Sid::bracketWidth);
        Pen pen(curColor(), w, PenStyle::SolidLine, PenCapStyle::FlatCap);
        painter->setPen(pen);
        double bd = style().styleMM(Sid::staffLineWidth) * 0.5;
        painter->drawLine(LineF(0.0, -bd, 0.0, h + bd));
    }
    break;
    case BracketType::NO_BRACKET:
        break;
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
    return { PointF(0.0, m_h2 * 2) + pagePos() };
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
    m_h2 += ed.delta.y() * .5;

    rendering()->layoutItem(this);
}

//---------------------------------------------------------
//   endEditDrag
//    snap to nearest staff
//---------------------------------------------------------

void Bracket::endEditDrag(EditData&)
{
    double ay2 = m_ay1 + m_h2 * 2;

    staff_idx_t staffIdx1 = staffIdx();
    staff_idx_t staffIdx2;
    size_t n = system()->staves().size();
    if (staffIdx1 + 1 >= n) {
        staffIdx2 = staffIdx1;
    } else {
        double ay  = parentItem()->pagePos().y();
        System* s = system();
        double y   = s->staff(staffIdx1)->y() + ay;
        double h1  = staff()->height();

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
    double ey = system()->staff(staffIdx2)->y() + score()->staff(staffIdx2)->height();
    m_h2 = (ey - sy) * .5;
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
//   undoChangeProperty
//---------------------------------------------------------

void Bracket::undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps)
{
    if (id == Pid::COLOR) {
        setColor(v.value<draw::Color>());
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
}
