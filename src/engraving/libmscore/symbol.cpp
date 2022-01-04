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

#include "symbol.h"

#include "draw/fontmetrics.h"
#include "rw/xml.h"
#include "types/symnames.h"

#include "scorefont.h"
#include "system.h"
#include "staff.h"
#include "measure.h"
#include "page.h"
#include "score.h"
#include "image.h"

using namespace mu;
using namespace mu::engraving;

namespace Ms {
//---------------------------------------------------------
//   Symbol
//---------------------------------------------------------

Symbol::Symbol(const ElementType& type, EngravingItem* parent, ElementFlags f)
    : BSymbol(type, parent, f)
{
    _sym = SymId::accidentalSharp;          // arbitrary valid default
}

Symbol::Symbol(EngravingItem* parent, ElementFlags f)
    : Symbol(ElementType::SYMBOL, parent, f)
{
}

Symbol::Symbol(const Symbol& s)
    : BSymbol(s)
{
    _sym       = s._sym;
    _scoreFont = s._scoreFont;
}

//---------------------------------------------------------
//   symName
//---------------------------------------------------------

QString Symbol::symName() const
{
    return SymNames::nameForSymId(_sym);
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Symbol::accessibleInfo() const
{
    return QString("%1: %2").arg(EngravingItem::accessibleInfo(), SymNames::userNameForSymId(_sym));
}

//---------------------------------------------------------
//   layout
//    height() and width() should return sensible
//    values when calling this method
//---------------------------------------------------------

void Symbol::layout()
{
    // foreach(EngravingItem* e, leafs())     done in BSymbol::layout() ?
    //      e->layout();
    setbbox(_scoreFont ? _scoreFont->bbox(_sym, magS()) : symBbox(_sym));
    qreal w = width();
    PointF p;
    if (align() == AlignV::BOTTOM) {
        p.setY(-height());
    } else if (align() == AlignV::VCENTER) {
        p.setY((-height()) * .5);
    } else if (align() == AlignV::BASELINE) {
        p.setY(-baseLine());
    }
    if (align() == AlignH::RIGHT) {
        p.setX(-w);
    } else if (align() == AlignH::HCENTER) {
        p.setX(-(w * .5));
    }
    setPos(p);
    BSymbol::layout();
}

//---------------------------------------------------------
//   Symbol::draw
//---------------------------------------------------------

void Symbol::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    if (!isNoteDot() || !staff()->isTabStaff(tick())) {
        painter->setPen(curColor());
        if (_scoreFont) {
            _scoreFont->draw(_sym, painter, magS(), PointF());
        } else {
            drawSymbol(_sym, painter);
        }
    }
}

//---------------------------------------------------------
//   Symbol::write
//---------------------------------------------------------

void Symbol::write(XmlWriter& xml) const
{
    xml.startObject(this);
    xml.tag("name", SymNames::nameForSymId(_sym));
    if (_scoreFont) {
        xml.tag("font", _scoreFont->name());
    }
    BSymbol::writeProperties(xml);
    xml.endObject();
}

//---------------------------------------------------------
//   Symbol::read
//---------------------------------------------------------

void Symbol::read(XmlReader& e)
{
    PointF pos;
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "name") {
            QString val(e.readElementText());
            SymId symId = SymNames::symIdByName(val);
            if (val != "noSym" && symId == SymId::noSym) {
                // if symbol name not found, fall back to user names
                // TODO: does it make sense? user names are probably localized
                symId = SymNames::symIdByUserName(val);
                if (symId == SymId::noSym) {
                    qDebug("unknown symbol <%s>, falling back to no symbol", qPrintable(val));
                    // set a default symbol, or layout() will crash
                    symId = SymId::noSym;
                }
            }
            setSym(symId);
        } else if (tag == "font") {
            _scoreFont = ScoreFont::fontByName(e.readElementText());
        } else if (tag == "Symbol") {
            Symbol* s = new Symbol(this);
            s->read(e);
            add(s);
        } else if (tag == "Image") {
            if (MScore::noImages) {
                e.skipCurrentElement();
            } else {
                Image* image = new Image(this);
                image->read(e);
                add(image);
            }
        } else if (tag == "small" || tag == "subtype") {    // obsolete
            e.skipCurrentElement();
        } else if (!BSymbol::readProperties(e)) {
            e.unknown();
        }
    }
    setPos(pos);
}

//---------------------------------------------------------
//   Symbol::getProperty
//---------------------------------------------------------
PropertyValue Symbol::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SYMBOL:
        return PropertyValue::fromValue(_sym);
    default:
        break;
    }
    return BSymbol::getProperty(propertyId);
}

//---------------------------------------------------------
//   Symbol::setProperty
//---------------------------------------------------------

bool Symbol::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::SYMBOL:
        _sym = v.value<SymId>();
        break;
    default:
        break;
    }
    return BSymbol::setProperty(propertyId, v);
}

//---------------------------------------------------------
//   FSymbol
//---------------------------------------------------------

FSymbol::FSymbol(EngravingItem* parent)
    : BSymbol(ElementType::FSYMBOL, parent)
{
    _code = 0;
    _font.setNoFontMerging(true);
}

FSymbol::FSymbol(const FSymbol& s)
    : BSymbol(s)
{
    _font = s._font;
    _code = s._code;
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void FSymbol::draw(mu::draw::Painter* painter) const
{
    QString s;
    mu::draw::Font f(_font);
    f.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
    painter->setFont(f);
    if (_code & 0xffff0000) {
        s = QChar(QChar::highSurrogate(_code));
        s += QChar(QChar::lowSurrogate(_code));
    } else {
        s = QChar(_code);
    }
    painter->setPen(curColor());
    painter->drawText(PointF(0, 0), s);
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void FSymbol::write(XmlWriter& xml) const
{
    xml.startObject(this);
    xml.tag("font",     _font.family());
    xml.tag("fontsize", _font.pointSizeF());
    xml.tag("code",     _code);
    BSymbol::writeProperties(xml);
    xml.endObject();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void FSymbol::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "font") {
            _font.setFamily(e.readElementText());
        } else if (tag == "fontsize") {
            _font.setPointSizeF(e.readDouble());
        } else if (tag == "code") {
            _code = e.readInt();
        } else if (!BSymbol::readProperties(e)) {
            e.unknown();
        }
    }
    setPos(PointF());
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void FSymbol::layout()
{
    QString s;
    if (_code & 0xffff0000) {
        s = QChar(QChar::highSurrogate(_code));
        s += QChar(QChar::lowSurrogate(_code));
    } else {
        s = QChar(_code);
    }

    setbbox(mu::draw::FontMetrics::boundingRect(_font, s));
}

//---------------------------------------------------------
//   setFont
//---------------------------------------------------------

void FSymbol::setFont(const mu::draw::Font& f)
{
    _font = f;
    _font.setNoFontMerging(true);
}
}
