/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "jianpudurationline.h"

#include "chord.h"
#include "measure.h"
#include "rest.h"
#include "staff.h"
#include "system.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   JianpuDurationLine
//---------------------------------------------------------

JianpuDurationLine::JianpuDurationLine(DummyParentOr<ChordRest> parent)
    : EngravingItem(ElementType::JIANPU_DURATION_LINE, parent)
{
    setSelectable(false);
}

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

PointF JianpuDurationLine::pagePos() const
{
    const Measure* measure = chordRest() ? chordRest()->measure() : nullptr;
    const System* system = measure ? measure->system() : nullptr;
    if (system && system->staves().size() > staffIdx()) {
        if (const SysStaff* staff = system->staff(staffIdx())) {
            const double yp = y() + staff->y() + system->y();
            return PointF(pageX(), yp);
        }
    }

    return EngravingItem::pagePos();
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

double JianpuDurationLine::mag() const
{
    const EngravingObject* parent = ownershipParent();
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

ChordRest* JianpuDurationLine::chordRest() const
{
    EngravingObject* parent = ownershipParent();
    if (parent && parent->isChordRest()) {
        return toChordRest(parent);
    } else {
        return nullptr;
    }
}
}
