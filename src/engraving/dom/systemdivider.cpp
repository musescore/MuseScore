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

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   SystemDivider
//---------------------------------------------------------

SystemDivider::SystemDivider(System* parent)
    : Symbol(ElementType::SYSTEM_DIVIDER, parent, ElementFlag::SYSTEM | ElementFlag::NOT_SELECTABLE)
{
    // default value, but not valid until setDividerType()
    m_dividerType = SystemDivider::Type::LEFT;
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

void SystemDivider::setDividerType(SystemDivider::Type v)
{
    m_dividerType = v;

    if (v == SystemDivider::Type::LEFT) {
        setOffset(PointF(style().styleD(Sid::dividerLeftX), style().styleD(Sid::dividerLeftY)));
        setSym(SymNames::symIdByName(style().styleSt(Sid::dividerLeftSym)), score()->engravingFont());
    } else {
        setOffset(PointF(style().styleD(Sid::dividerRightX), style().styleD(Sid::dividerRightY)));
        setSym(SymNames::symIdByName(style().styleSt(Sid::dividerRightSym)), score()->engravingFont());
    }
}

void SystemDivider::styleChanged()
{
    setDividerType(m_dividerType);
}

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

RectF SystemDivider::drag(EditData& ed)
{
    setGenerated(false);
    return Symbol::drag(ed);
}
} // namespace mu::engraving
