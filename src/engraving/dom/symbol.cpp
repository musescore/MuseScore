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

#include "symbol.h"

#include "score.h"

#include "draw/fontmetrics.h"
#include "iengravingfont.h"

#include "types/symnames.h"

#include "image.h"
#include "staff.h"

#include "log.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   Symbol
//---------------------------------------------------------

Symbol::Symbol(const ElementType& type, EngravingItem* parent, ElementFlags f)
    : BSymbol(type, parent, f)
{
    m_sym = SymId::accidentalSharp;          // arbitrary valid default
}

Symbol::Symbol(EngravingItem* parent, ElementFlags f)
    : Symbol(ElementType::SYMBOL, parent, f)
{
}

Symbol::Symbol(const Symbol& s)
    : BSymbol(s)
{
    m_sym         = s.m_sym;
    m_scoreFont   = s.m_scoreFont;
    m_symbolsSize = s.m_symbolsSize;
    m_symAngle    = s.m_symAngle;
}

//---------------------------------------------------------
//   symName
//---------------------------------------------------------

AsciiStringView Symbol::symName() const
{
    return SymNames::nameForSymId(m_sym);
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Symbol::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), SymNames::userNameForSymId(m_sym).translated());
}

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

muse::TranslatableString Symbol::subtypeUserName() const
{
    return SymNames::userNameForSymId(m_sym);
}

//---------------------------------------------------------
//   Symbol::getProperty
//---------------------------------------------------------
PropertyValue Symbol::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SYMBOL:
        return PropertyValue::fromValue(m_sym);
    case Pid::SCORE_FONT:
        if (m_scoreFont) {
            return PropertyValue::fromValue(String::fromStdString(m_scoreFont->name()));
        } else {
            return PropertyValue::fromValue(String());
        }
    case Pid::SYMBOLS_SIZE:
        return PropertyValue::fromValue(m_symbolsSize);
    case Pid::SYMBOL_ANGLE:
        return PropertyValue::fromValue(m_symAngle);
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
        m_sym = v.value<SymId>();
        break;
    case Pid::SCORE_FONT:
        m_scoreFont = score()->engravingFonts()->fontByName(v.value<String>().toStdString());
        break;
    case Pid::SYMBOLS_SIZE:
        m_symbolsSize = v.toDouble();
        break;
    case Pid::SYMBOL_ANGLE:
        m_symAngle = v.toDouble();
        break;
    default:
        break;
    }
    triggerLayout();
    return BSymbol::setProperty(propertyId, v);
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Symbol::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SYMBOL:
        return SymId::accidentalSharp;
    case Pid::SYMBOL_ANGLE:
        return 0.0;
    case Pid::SYMBOLS_SIZE:
        return 1.0;
    case Pid::SCORE_FONT:
        if (m_scoreFont) {
            return style().styleSt(Sid::musicalSymbolFont);
        } else {
            return String();
        }
    default:
        break;
    }
    return EngravingItem::propertyDefault(propertyId);
}

//---------------------------------------------------------
//   FSymbol
//---------------------------------------------------------

FSymbol::FSymbol(EngravingItem* parent)
    : BSymbol(ElementType::FSYMBOL, parent)
{
    m_code = 0;
    m_font.setNoFontMerging(true);
}

FSymbol::FSymbol(const FSymbol& s)
    : BSymbol(s)
{
    m_font = s.m_font;
    m_code = s.m_code;
}

//---------------------------------------------------------
//   toString
// FSymbol is a single code point but code points above 2^16 cannot be
// represented by a single Char, hence we return a String instead. Char
// and String use the UTF-16 encoding internally (like QChar, QString).
//---------------------------------------------------------

String FSymbol::toString() const
{
    return String::fromUcs4(m_code);
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
//   setFont
//---------------------------------------------------------

void FSymbol::setFont(const Font& f)
{
    m_font = f;
    m_font.setNoFontMerging(true);
}
}
