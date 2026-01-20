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

#include "durationline.h"

#include "chord.h"
#include "measure.h"
#include "rest.h"
#include "staff.h"
#include "system.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   DurationLine
//---------------------------------------------------------

DurationLine::DurationLine(EngravingItem* s)
    : EngravingItem(ElementType::DURATION_LINE, s)
{
    setSelectable(false);
    m_len = 0.;
}

DurationLine::~DurationLine()
{
}

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

PointF DurationLine::pagePos() const
{
    System* system = chordRest()->measure()->system();
    double yp = y() + system->staff(staffIdx())->y() + system->y();
    return PointF(pageX(), yp);
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void DurationLine::spatiumChanged(double oldValue, double newValue)
{
    m_len = (m_len / oldValue) * newValue;
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

double DurationLine::mag() const
{
    auto* parent = explicitParent();
    if (parent && parent->isRest()) {
        return toRest(parent)->mag();
    }
    if (parent && parent->isChord()) {
        return toChord(parent)->mag();
    }
    const Staff* st = staff();
    return st ? st->staffMag(this) : 1.0;
}

//---------------------------------------------------------
//   chord
//---------------------------------------------------------

ChordRest* DurationLine::chordRest() const
{
    auto* parent = explicitParent();
    if (parent && parent->isChordRest()) {
        return toChordRest(parent);
    } else {
        return nullptr;
    }
}
}
