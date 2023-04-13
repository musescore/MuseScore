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
#include "iengravingfont.h"
#include "rw/xml.h"

#include "types/symnames.h"

#include "image.h"
#include "staff.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
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

AsciiStringView Symbol::symName() const
{
    return SymNames::nameForSymId(_sym);
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Symbol::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), String::fromUtf8(SymNames::userNameForSymId(_sym)));
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
    double w = width();
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
    TRACE_ITEM_DRAW;
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
    UNREACHABLE;
    xml.startElement(this);
    xml.tag("name", SymNames::nameForSymId(_sym));
    if (_scoreFont) {
        xml.tag("font", _scoreFont->name());
    }
    BSymbol::writeProperties(xml);
    xml.endElement();
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
//   toString
// FSymbol is a single code point but code points above 2^16 cannot be
// represented by a single Char, hence we return a String instead. Char
// and String use the UTF-16 encoding internally (like QChar, QString).
//---------------------------------------------------------

String FSymbol::toString() const
{
    return String::fromUcs4(_code);
}

//---------------------------------------------------------
//   accessibleInfo
// Screen readers know how to pronounce the common font symbols so we can
// return just the character itself. Similarly, common characters should
// be rendered correctly by Braille terminals.
//---------------------------------------------------------

String FSymbol::accessibleInfo() const
{
    return toString();
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void FSymbol::draw(mu::draw::Painter* painter) const
{
    String s;
    mu::draw::Font f(_font);
    f.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
    painter->setFont(f);
    painter->setPen(curColor());
    painter->drawText(PointF(0, 0), toString());
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void FSymbol::write(XmlWriter& xml) const
{
    UNREACHABLE;
    xml.startElement(this);
    xml.tag("font",     _font.family());
    xml.tag("fontsize", _font.pointSizeF());
    xml.tag("code",     _code);
    BSymbol::writeProperties(xml);
    xml.endElement();
}

void FSymbol::layout()
{
    setbbox(mu::draw::FontMetrics::boundingRect(_font, toString()));
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
