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
        setSym(SymNames::symIdByName(style().styleSt(Sid::dividerLeftSym)), score()->engravingFont());
        m_symbolsSize = style().styleD(Sid::dividerLeftSize);
    } else {
        setSym(SymNames::symIdByName(style().styleSt(Sid::dividerRightSym)), score()->engravingFont());
        m_symbolsSize = style().styleD(Sid::dividerRightSize);
    }
}

void SystemDivider::styleChanged()
{
    setDividerType(m_dividerType);
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
