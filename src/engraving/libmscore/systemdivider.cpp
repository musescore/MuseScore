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

#include "systemdivider.h"

#include "types/symnames.h"
#include "layout/tlayout.h"

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
    _dividerType = SystemDivider::Type::LEFT;
    _sym = SymId::systemDivider;
}

//---------------------------------------------------------
//   SystemDivider
//---------------------------------------------------------

SystemDivider::SystemDivider(const SystemDivider& sd)
    : Symbol(sd)
{
    _dividerType = sd._dividerType;
}

//---------------------------------------------------------
<<<<<<< HEAD
<<<<<<< HEAD
=======
//   layout
//---------------------------------------------------------

void SystemDivider::layout()
{
    UNREACHABLE;
    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
}

//---------------------------------------------------------
>>>>>>> 4f8a1b6dd0... [engraving] replaced item->layout() to TLayout::layout
=======
>>>>>>> 11610ff2b5... [engraving] removed item->layout method
//   setDividerType
//---------------------------------------------------------

void SystemDivider::setDividerType(SystemDivider::Type v)
{
    _dividerType = v;
    if (v == SystemDivider::Type::LEFT) {
        setOffset(PointF(score()->styleD(Sid::dividerLeftX), score()->styleD(Sid::dividerLeftY)));
    } else {
        setOffset(PointF(score()->styleD(Sid::dividerRightX), score()->styleD(Sid::dividerRightY)));
    }
}

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

mu::RectF SystemDivider::drag(EditData& ed)
{
    setGenerated(false);
    return Symbol::drag(ed);
}
} // namespace mu::engraving
