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

#include "ledgerline.h"

#include "chord.h"
#include "measure.h"
#include "system.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   LedgerLine
//---------------------------------------------------------

LedgerLine::LedgerLine(EngravingItem* s)
    : EngravingItem(ElementType::LEDGER_LINE, s)
{
    setSelectable(false);
    m_len        = 0.;
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
//   spatiumChanged
//---------------------------------------------------------

void LedgerLine::spatiumChanged(double oldValue, double newValue)
{
    m_len   = (m_len / oldValue) * newValue;
}
}
