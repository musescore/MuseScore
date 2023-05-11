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

#include "ledgerline.h"

#include "layout/tlayout.h"

#include "chord.h"
#include "measure.h"
#include "score.h"
#include "staff.h"
#include "system.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   LedgerLine
//---------------------------------------------------------

LedgerLine::LedgerLine(Score* s)
    : EngravingItem(ElementType::LEDGER_LINE, s)
{
    setSelectable(false);
    _width      = 0.;
    _len        = 0.;
    _next       = 0;
}

LedgerLine::~LedgerLine()
{
}

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

PointF LedgerLine::pagePos() const
{
    System* system = chord()->measure()->system();
    double yp = y() + system->staff(staffIdx())->y() + system->y();
    return PointF(pageX(), yp);
}

//---------------------------------------------------------
//   measureXPos
//---------------------------------------------------------

double LedgerLine::measureXPos() const
{
    double xp = x();                     // chord relative
    xp += chord()->x();                  // segment relative
    xp += chord()->segment()->x();       // measure relative
    return xp;
}

//---------------------------------------------------------
<<<<<<< HEAD
=======
//   layout
//---------------------------------------------------------

void LedgerLine::layout()
{
    UNREACHABLE;
    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
}

//---------------------------------------------------------
>>>>>>> 4f8a1b6dd0... [engraving] replaced item->layout() to TLayout::layout
//   draw
//---------------------------------------------------------

void LedgerLine::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    using namespace mu::draw;
    if (chord()->crossMeasure() == CrossMeasure::SECOND) {
        return;
    }
    painter->setPen(Pen(curColor(), _width, PenStyle::SolidLine, PenCapStyle::FlatCap));
    if (m_vertical) {
        painter->drawLine(LineF(0.0, 0.0, 0.0, _len));
    } else {
        painter->drawLine(LineF(0.0, 0.0, _len, 0.0));
    }
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void LedgerLine::spatiumChanged(double oldValue, double newValue)
{
    _width = (_width / oldValue) * newValue;
    _len   = (_len / oldValue) * newValue;
    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
}
}
