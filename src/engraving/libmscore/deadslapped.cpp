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

#include "deadslapped.h"

#include "rest.h"
#include "staff.h"
#include "log.h"

using namespace mu;
using namespace mu::engraving;

//---------------------------------------------------------
//    DeadSlapped
//--------------------------------------------------------

DeadSlapped::DeadSlapped(Rest* rest)
    : EngravingItem(ElementType::DEAD_SLAPPED, rest)
{
}

void DeadSlapped::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    painter->setPen(draw::PenStyle::NoPen);
    painter->setBrush(curColor());
    painter->drawPath(m_path1);
    painter->drawPath(m_path2);
}
