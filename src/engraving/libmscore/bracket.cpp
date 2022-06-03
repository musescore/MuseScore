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

#include "draw/brush.h"
#include "style/style.h"
#include "rw/xml.h"

#include "factory.h"
#include "utils.h"
#include "measure.h"
#include "staff.h"
#include "system.h"
#include "score.h"
#include "system.h"
#include "mscore.h"
#include "bracketItem.h"

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
    ay1          = 0;
    h2           = 3.5 * spatium();
    _firstStaff  = 0;
    _lastStaff   = 0;
    _bi          = 0;
    _braceSymbol = SymId::noSym;
    _magx        = 1.;
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

void Bracket::setHeight(qreal h)
{
    h2 = h * .5;
}

//---------------------------------------------------------
//   width
//---------------------------------------------------------

qreal Bracket::width() const
{
    qreal w;
    switch (bracketType()) {
    case BracketType::BRACE:
        if (score()->styleSt(Sid::MusicalSymbolFont) == "Emmentaler" || score()->styleSt(Sid::MusicalSymbolFont) == "Gonville") {
            w = score()->styleMM(Sid::akkoladeWidth) + score()->styleMM(Sid::akkoladeBarDistance);
        } else {
            w = (symWidth(_braceSymbol) * _magx) + score()->styleMM(Sid::akkoladeBarDistance);
        }
        break;
    case BracketType::NORMAL:
        w = score()->styleMM(Sid::bracketWidth) + score()->styleMM(Sid::bracketDistance);
        break;
    case BracketType::SQUARE:
        w = score()->styleMM(Sid::staffLineWidth) + spatium() * .5;
        break;
    case BracketType::LINE:
        w = 0.67 * score()->styleMM(Sid::bracketWidth) + score()->styleMM(Sid::bracketDistance);
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
    _firstStaff = a;
    _lastStaff = b;

    if (bracketType() == BracketType::BRACE
        && score()->styleSt(Sid::MusicalSymbolFont) != "Emmentaler" && score()->styleSt(Sid::MusicalSymbolFont) != "Gonville") {
        int v = static_cast<int>(_lastStaff - _firstStaff + 1);

        // if staves inner staves are hidden, decrease span
        for (size_t staffIndex = _firstStaff; staffIndex <= _lastStaff; ++staffIndex) {
            if (system() && !system()->staff(staffIndex)->show()) {
                --v;
            }
        }

        if (score()->styleSt(Sid::MusicalSymbolFont) == "Leland") {
            v = qMin(4, v);
        }

        // 1.625 is a "magic" number based on akkoladeDistance/4.0 (default value 6.5).
        _magx = v + ((v - 1) * 1.625);

        if (v == 1) {
            _braceSymbol = SymId::braceSmall;
        } else if (v <= 2) {
            _braceSymbol = SymId::brace;
        } else if (v <= 3) {
            _braceSymbol = SymId::braceLarge;
        } else {
            _braceSymbol = SymId::braceLarger;
        }
    }
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Bracket::layout()
{
    path = PainterPath();
    if (h2 == 0.0) {
        return;
    }

    _shape.clear();
    switch (bracketType()) {
    case BracketType::BRACE: {
        if (score()->styleSt(Sid::MusicalSymbolFont) == "Emmentaler" || score()->styleSt(Sid::MusicalSymbolFont) == "Gonville") {
            _braceSymbol = SymId::noSym;
            qreal w = score()->styleMM(Sid::akkoladeWidth);

#define XM(a) (a + 700) * w / 700
#define YM(a) (a + 7100) * h2 / 7100

            path.moveTo(XM(-8), YM(-2048));
            path.cubicTo(XM(-8), YM(-3192), XM(-360), YM(-4304), XM(-360), YM(-5400));                 // c 0
            path.cubicTo(XM(-360), YM(-5952), XM(-264), YM(-6488), XM(32), YM(-6968));                 // c 1
            path.cubicTo(XM(36), YM(-6974), XM(38), YM(-6984), XM(38), YM(-6990));                     // c 0
            path.cubicTo(XM(38), YM(-7008), XM(16), YM(-7024), XM(0), YM(-7024));                      // c 0
            path.cubicTo(XM(-8), YM(-7024), XM(-22), YM(-7022), XM(-32), YM(-7008));                   // c 1
            path.cubicTo(XM(-416), YM(-6392), XM(-544), YM(-5680), XM(-544), YM(-4960));               // c 0
            path.cubicTo(XM(-544), YM(-3800), XM(-168), YM(-2680), XM(-168), YM(-1568));               // c 0
            path.cubicTo(XM(-168), YM(-1016), XM(-264), YM(-496), XM(-560), YM(-16));                  // c 1
            path.lineTo(XM(-560), YM(0));                    //  l 1
            path.lineTo(XM(-560), YM(16));                   //  l 1
            path.cubicTo(XM(-264), YM(496), XM(-168), YM(1016), XM(-168), YM(1568));                   // c 0
            path.cubicTo(XM(-168), YM(2680), XM(-544), YM(3800), XM(-544), YM(4960));                  // c 0
            path.cubicTo(XM(-544), YM(5680), XM(-416), YM(6392), XM(-32), YM(7008));                   // c 1
            path.cubicTo(XM(-22), YM(7022), XM(-8), YM(7024), XM(0), YM(7024));                        // c 0
            path.cubicTo(XM(16), YM(7024), XM(38), YM(7008), XM(38), YM(6990));                        // c 0
            path.cubicTo(XM(38), YM(6984), XM(36), YM(6974), XM(32), YM(6968));                        // c 1
            path.cubicTo(XM(-264), YM(6488), XM(-360), YM(5952), XM(-360), YM(5400));                  // c 0
            path.cubicTo(XM(-360), YM(4304), XM(-8), YM(3192), XM(-8), YM(2048));                      // c 0
            path.cubicTo(XM(-8), YM(1320), XM(-136), YM(624), XM(-512), YM(0));                        // c 1
            path.cubicTo(XM(-136), YM(-624), XM(-8), YM(-1320), XM(-8), YM(-2048));                    // c 0*/
            setbbox(path.boundingRect());
            _shape.add(bbox());
        } else {
            if (_braceSymbol == SymId::noSym) {
                _braceSymbol = SymId::brace;
            }
            qreal h = h2 * 2;
            qreal w = symWidth(_braceSymbol) * _magx;
            bbox().setRect(0, 0, w, h);
            _shape.add(bbox());
        }
    }
    break;
    case BracketType::NORMAL: {
        qreal _spatium = spatium();
        qreal w = score()->styleMM(Sid::bracketWidth) * .5;
        qreal x = -w;

        qreal bd   = (score()->styleSt(Sid::MusicalSymbolFont) == "Leland") ? _spatium * .5 : _spatium * .25;
        _shape.add(RectF(x, -bd, w * 2, 2 * (h2 + bd)));
        _shape.add(symBbox(SymId::bracketTop).translated(PointF(-w, -bd)));
        _shape.add(symBbox(SymId::bracketBottom).translated(PointF(-w, bd + 2 * h2)));

        w      += symWidth(SymId::bracketTop);
        qreal y = -symHeight(SymId::bracketTop) - bd;
        qreal h = (-y + h2) * 2;
        bbox().setRect(x, y, w, h);
    }
    break;
    case BracketType::SQUARE: {
        qreal w = score()->styleMM(Sid::staffLineWidth) * .5;
        qreal x = -w;
        qreal y = -w;
        qreal h = (h2 + w) * 2;
        w      += (.5 * spatium() + 3 * w);
        bbox().setRect(x, y, w, h);
        _shape.add(bbox());
    }
    break;
    case BracketType::LINE: {
        qreal _spatium = spatium();
        qreal w = 0.67 * score()->styleMM(Sid::bracketWidth) * .5;
        qreal x = -w;
        qreal bd = _spatium * .25;
        qreal y = -bd;
        qreal h = (-y + h2) * 2;
        bbox().setRect(x, y, w, h);
        _shape.add(bbox());
    }
    break;
    case BracketType::NO_BRACKET:
        break;
    }
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Bracket::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    if (h2 == 0.0) {
        return;
    }
    switch (bracketType()) {
    case BracketType::BRACE: {
        if (_braceSymbol == SymId::noSym) {
            painter->setNoPen();
            painter->setBrush(Brush(curColor()));
            painter->drawPath(path);
        } else {
            qreal h        = 2 * h2;
            qreal mag      = h / (100 * magS());
            painter->setPen(curColor());
            painter->save();
            painter->scale(_magx, mag);
            drawSymbol(_braceSymbol, painter, PointF(0, 100 * magS()));
            painter->restore();
        }
    }
    break;
    case BracketType::NORMAL: {
        qreal h        = 2 * h2;
        qreal _spatium = spatium();
        qreal w        = score()->styleMM(Sid::bracketWidth);
        qreal bd       = (score()->styleSt(Sid::MusicalSymbolFont) == "Leland") ? _spatium * .5 : _spatium * .25;
        Pen pen(curColor(), w, PenStyle::SolidLine, PenCapStyle::FlatCap);
        painter->setPen(pen);
        painter->drawLine(LineF(0.0, -bd - w * .5, 0.0, h + bd + w * .5));
        qreal x    =  -w * .5;
        qreal y1   = -bd;
        qreal y2   = h + bd;
        drawSymbol(SymId::bracketTop, painter, PointF(x, y1));
        drawSymbol(SymId::bracketBottom, painter, PointF(x, y2));
    }
    break;
    case BracketType::SQUARE: {
        qreal h = 2 * h2;
        qreal _spatium = spatium();
        qreal w = score()->styleMM(Sid::staffLineWidth);
        Pen pen(curColor(), w, PenStyle::SolidLine, PenCapStyle::SquareCap);
        painter->setPen(pen);
        painter->drawLine(LineF(0.0, 0.0, 0.0, h));
        painter->drawLine(LineF(0.0, 0.0, w + .5 * _spatium, 0.0));
        painter->drawLine(LineF(0.0, h, w + .5 * _spatium, h));
    }
    break;
    case BracketType::LINE: {
        qreal h = 2 * h2;
        qreal w = 0.67 * score()->styleMM(Sid::bracketWidth);
        Pen pen(curColor(), w, PenStyle::SolidLine, PenCapStyle::FlatCap);
        painter->setPen(pen);
        qreal bd = score()->styleMM(Sid::staffLineWidth) * 0.5;
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
    ay1 = pagePos().y();
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> Bracket::gripsPositions(const EditData&) const
{
    return { PointF(0.0, h2 * 2) + pagePos() };
}

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Bracket::endEdit(EditData& ed)
{
    triggerLayoutAll();
    ed.clear(); // score layout invalidates element
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Bracket::editDrag(EditData& ed)
{
    h2 += ed.delta.y() * .5;
    layout();
}

//---------------------------------------------------------
//   endEditDrag
//    snap to nearest staff
//---------------------------------------------------------

void Bracket::endEditDrag(EditData&)
{
    qreal ay2 = ay1 + h2 * 2;

    staff_idx_t staffIdx1 = staffIdx();
    staff_idx_t staffIdx2;
    size_t n = system()->staves().size();
    if (staffIdx1 + 1 >= n) {
        staffIdx2 = staffIdx1;
    } else {
        qreal ay  = parentItem()->pagePos().y();
        System* s = system();
        qreal y   = s->staff(staffIdx1)->y() + ay;
        qreal h1  = staff()->height();

        for (staffIdx2 = staffIdx1 + 1; staffIdx2 < n; ++staffIdx2) {
            qreal h = s->staff(staffIdx2)->y() + ay - y;
            if (ay2 < (y + (h + h1) * .5)) {
                break;
            }
            y += h;
        }
        staffIdx2 -= 1;
    }

    qreal sy = system()->staff(staffIdx1)->y();
    qreal ey = system()->staff(staffIdx2)->y() + score()->staff(staffIdx2)->height();
    h2 = (ey - sy) * .5;
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
    if (ed.key == Qt::Key_Up && span() > 1) {
        return true;
    }
    if (ed.key == Qt::Key_Down && _lastStaff < system()->staves().size() - 1) {
        return true;
    }

    if (!(ed.modifiers & Qt::ShiftModifier)) {
        return false;
    }

    if (ed.key == Qt::Key_Left) {
        return true;
    }
    if (ed.key == Qt::Key_Right) {
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

    if (ed.key == Qt::Key_Up && span() > 1) {
        bracketItem()->undoChangeProperty(Pid::BRACKET_SPAN, static_cast<int>(span()) - 1);
        return true;
    }
    if (ed.key == Qt::Key_Down && _lastStaff < system()->staves().size() - 1) {
        bracketItem()->undoChangeProperty(Pid::BRACKET_SPAN, static_cast<int>(span()) + 1);
        return true;
    }

    if (ed.key == Qt::Key_Left) {
        bracketItem()->undoChangeProperty(Pid::BRACKET_COLUMN, bracketItem()->column() + 1);
        return true;
    }
    if (ed.key == Qt::Key_Right) {
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
        v = _bi->getProperty(id);
    }
    return v;
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Bracket::setProperty(Pid id, const PropertyValue& v)
{
    return _bi->setProperty(id, v);
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
        v = _bi->propertyDefault(id);
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
    _bi->setSelected(f);
    EngravingItem::setSelected(f);
}

//---------------------------------------------------------
//   Bracket::bracketTypeName
//---------------------------------------------------------

const char* Bracket::bracketTypeName(BracketType type)
{
    switch (type) {
    case BracketType::BRACE:
        return "Brace";
    case BracketType::NORMAL:
        return "Normal";
    case BracketType::SQUARE:
        return "Square";
    case BracketType::LINE:
        return "Line";
    case BracketType::NO_BRACKET:
        return "NoBracket";
    }
    Q_UNREACHABLE();
}

//---------------------------------------------------------
//   Bracket::write
//    used only for palettes
//---------------------------------------------------------

void Bracket::write(XmlWriter& xml) const
{
    bool isStartTag = false;
    switch (_bi->bracketType()) {
    case BracketType::BRACE:
    case BracketType::SQUARE:
    case BracketType::LINE:
    {
        const char* type = bracketTypeName(_bi->bracketType());
        xml.startElement(this, { { "type", type } });
        isStartTag = true;
    }
    break;
    case BracketType::NORMAL:
        xml.startElement(this);
        isStartTag = true;
        break;
    case BracketType::NO_BRACKET:
        break;
    }

    if (isStartTag) {
        if (_bi->column()) {
            xml.tag("level", static_cast<int>(_bi->column()));
        }

        EngravingItem::writeProperties(xml);

        xml.endElement();
    }
}

//---------------------------------------------------------
//   Bracket::read
//    used only for palettes
//---------------------------------------------------------

void Bracket::read(XmlReader& e)
{
    QString t(e.attribute("type", "Normal"));
    _bi = Factory::createBracketItem(score()->dummy());

    if (t == "Normal") {
        _bi->setBracketType(BracketType::NORMAL);
    } else if (t == "Akkolade") { //compatibility, not used anymore
        _bi->setBracketType(BracketType::BRACE);
    } else if (t == "Brace") {
        _bi->setBracketType(BracketType::BRACE);
    } else if (t == "Square") {
        _bi->setBracketType(BracketType::SQUARE);
    } else if (t == "Line") {
        _bi->setBracketType(BracketType::LINE);
    } else {
        LOGD("unknown brace type <%s>", qPrintable(t));
    }

    while (e.readNextStartElement()) {
        if (e.name() == "level") {
            _bi->setColumn(e.readInt());
        } else if (!EngravingItem::readProperties(e)) {
            e.unknown();
        }
    }
}
}
