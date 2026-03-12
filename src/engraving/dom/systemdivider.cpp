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

#include "systemdivider.h"

#include "types/symnames.h"

#include "score.h"
#include "system.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   SystemDivider
//---------------------------------------------------------

static const ElementStyle dividerStyle {
    { Sid::dividerLeftSize, Pid::SYMBOLS_SIZE }, // left or right is decided later
    { Sid::musicalSymbolFont, Pid::SCORE_FONT }
};

SystemDivider::SystemDivider(System* parent)
    : Symbol(ElementType::SYSTEM_DIVIDER, parent, ElementFlag::SYSTEM | ElementFlag::MOVABLE)
{
    // default value, but not valid until setDividerType()
    m_dividerType = SystemDividerType::LEFT;
    m_sym = SymId::systemDivider;
}

//---------------------------------------------------------
//   SystemDivider
//---------------------------------------------------------

SystemDivider::SystemDivider(const SystemDivider& sd)
    : Symbol(sd)
{
    m_dividerType = sd.m_dividerType;
}

//---------------------------------------------------------
//   setDividerType
//---------------------------------------------------------

void SystemDivider::setDividerType(SystemDividerType v)
{
    m_dividerType = v;

    if (v == SystemDividerType::LEFT) {
        setSym(SymNames::symIdByName(style().styleSt(Sid::dividerLeftSym)));
    } else {
        setSym(SymNames::symIdByName(style().styleSt(Sid::dividerRightSym)));
    }

    initElementStyle(&dividerStyle);
}

void SystemDivider::styleChanged()
{
    if (m_dividerType == SystemDividerType::LEFT) {
        setSym(SymNames::symIdByName(style().styleSt(Sid::dividerLeftSym)));
    } else {
        setSym(SymNames::symIdByName(style().styleSt(Sid::dividerRightSym)));
    }

    if (isStyled(Pid::SYMBOLS_SIZE)) {
        m_symbolsSize = style().styleD(getPropertyStyle(Pid::SYMBOLS_SIZE));
    }

    Symbol::styleChanged();
}

Sid SystemDivider::getPropertyStyle(Pid id) const
{
    if (id == Pid::SYMBOLS_SIZE) {
        return m_dividerType == SystemDividerType::LEFT ? Sid::dividerLeftSize : Sid::dividerRightSize;
    }

    return Symbol::getPropertyStyle(id);
}

std::vector<LineF> SystemDivider::dragAnchorLines() const
{
    std::vector<LineF> result;

    const System* system = toSystem(parentItem());
    IF_ASSERT_FAILED(system) {
        return result;
    }

    RectF systemBBox = system->canvasBoundingRect();
    PointF p1 =  PointF(m_dividerType == SystemDividerType::LEFT
                        ? systemBBox.left() + system->leftMargin() : systemBBox.right(), systemBBox.bottom());

    RectF thisBBox = canvasBoundingRect();
    PointF p2 = 0.5 * (thisBBox.topLeft() + thisBBox.bottomRight());

    result.push_back(LineF(p1, p2));

    return result;
}

void SystemDivider::reset()
{
    Symbol::reset();
    setGenerated(true);
}

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

RectF SystemDivider::drag(EditData& ed)
{
    setGenerated(false);
    return Symbol::drag(ed);
}

PropertyValue SystemDivider::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::SYMBOLS_SIZE:
        return style().styleD(m_dividerType == SystemDividerType::LEFT ? Sid::dividerLeftSize : Sid::dividerRightSize);
    default:
        return Symbol::propertyDefault(id);
    }
}
} // namespace mu::engraving
