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

namespace mu::engraving {
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

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void DeadSlapped::layout()
{
    const double deadSlappedWidth = spatium() * 2;
    m_rect = RectF(0, 0, deadSlappedWidth, staff()->height());
    setbbox(m_rect);
    fillPath();
}

//---------------------------------------------------------
//   fillPath
//---------------------------------------------------------

void DeadSlapped::fillPath()
{
    constexpr double crossThinknessPercentage = 0.1;
    double height = m_rect.height();
    double width = m_rect.width();
    double crossThickness = width * crossThinknessPercentage;

    PointF topLeft = PointF(m_rect.x(), m_rect.y());
    PointF bottomRight = topLeft + PointF(width, height);
    PointF topRight = topLeft + PointF(width, 0);
    PointF bottomLeft = topLeft + PointF(0, height);
    PointF offsetX = PointF(crossThickness, 0);

    m_path1 = mu::draw::PainterPath();

    m_path1.moveTo(topLeft);
    m_path1.lineTo(topLeft + offsetX);
    m_path1.lineTo(bottomRight);
    m_path1.lineTo(bottomRight - offsetX);
    m_path1.lineTo(topLeft);

    m_path2 = mu::draw::PainterPath();

    m_path2.moveTo(topRight);
    m_path2.lineTo(topRight - offsetX);
    m_path2.lineTo(bottomLeft);
    m_path2.lineTo(bottomLeft + offsetX);
    m_path2.lineTo(topRight);
}
}
