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

#include "stafftype.h"
#include "style/style.h"
#include "tapping.h"
#include "text.h"
#include "types/typesconv.h"

namespace mu::engraving {
Tapping::Tapping(ChordRest* parent)
    : Articulation(parent, ElementType::TAPPING)
{
}

Tapping::Tapping(const Tapping& other)
    : Articulation(other)
{
    m_hand = other.m_hand;
}

void Tapping::setSelected(bool f)
{
    if (m_text) {
        m_text->setSelected(f);
    }
    if (m_halfSlurAbove) {
        m_halfSlurAbove->setSelected(f);
    }
    if (m_halfSlurBelow) {
        m_halfSlurBelow->setSelected(f);
    }

    EngravingItem::setSelected(f);
}

LHTappingShowItems Tapping::lhShowItems() const
{
    DO_ASSERT(m_hand == TappingHand::LEFT);

    const MStyle& s = style();
    bool tabStaff = staffType()->isTabStaff();

    return s.styleV(tabStaff ? Sid::lhTappingShowItemsTab : Sid::lhTappingShowItemsNormalStave).value<LHTappingShowItems>();
}

void Tapping::styleChanged()
{
    if (m_text) {
        m_text->styleChanged();
    }
    Articulation::styleChanged();
}

double Tapping::mag() const
{
    return ldata()->mag() * Articulation::mag();
}

int Tapping::subtype() const
{
    return static_cast<int>(m_hand);
}

String Tapping::accessibleInfo() const
{
    return String(u"%1 %2").arg(translatedSubtypeUserName(), TConv::userName(ElementType::TAPPING).translated());
}

TranslatableString Tapping::typeUserName() const
{
    return EngravingItem::typeUserName();
}

TranslatableString Tapping::subtypeUserName() const
{
    return m_hand == TappingHand::LEFT ? TranslatableString("engraving", "Left-hand") : TranslatableString("engraving", "Right-hand");
}

TappingHalfSlur::TappingHalfSlur(EngravingItem* parent)
    : Slur(parent, ElementType::TAPPING_HALF_SLUR)
{
}

TappingHalfSlur::TappingHalfSlur(const TappingHalfSlur& other)
    : Slur(other)
{
}

TappingHalfSlurSegment::TappingHalfSlurSegment(System* parent)
    : SlurSegment(parent, ElementType::TAPPING_HALF_SLUR_SEGMENT)
{
}

TappingHalfSlurSegment::TappingHalfSlurSegment(const TappingHalfSlurSegment& other)
    : SlurSegment(other)
{
}

static ElementStyle tapTextStyle;

TappingText::TappingText(Tapping* parent)
    : TextBase(ElementType::TAPPING_TEXT, parent, TextStyleType::HAMMER_ON_PULL_OFF,
               ElementFlag::MOVABLE | ElementFlag::GENERATED)
{
    initElementStyle(&tapTextStyle);
}

TappingText::TappingText(const TappingText& t)
    : TextBase(t)
{
}

Color TappingText::curColor() const
{
    if (parentItem()) {
        return parentItem()->curColor();
    }

    return EngravingItem::curColor();
}
}
