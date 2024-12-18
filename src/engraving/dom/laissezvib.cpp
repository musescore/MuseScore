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

#include "laissezvib.h"
#include "chord.h"
#include "dom/measure.h"
#include "dom/mscoreview.h"
#include "dom/score.h"
#include "note.h"
#include "staff.h"
#include "style/style.h"

namespace mu::engraving {
static const ElementStyle laissezVibStyle {
    { Sid::minLaissezVibLength, Pid::MIN_LENGTH }
};

LaissezVib::LaissezVib(Note* parent)
    : Tie(ElementType::LAISSEZ_VIB, parent)
{
    initElementStyle(&laissezVibStyle);
}

PropertyValue LaissezVib::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::MIN_LENGTH:
        return minLength();
    default:
        return Tie::getProperty(propertyId);
    }
}

PropertyValue LaissezVib::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::MIN_LENGTH:
        return Spatium(2.0);
    default:
        return Tie::propertyDefault(propertyId);
    }
}

bool LaissezVib::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::MIN_LENGTH:
        setMinLength(v.value<Spatium>());
        break;
    default:
        return Tie::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

SymId LaissezVib::symId() const
{
    return up() ? SymId::articLaissezVibrerAbove : SymId::articLaissezVibrerBelow;
}

void LaissezVib::setEndNote(Note* note)
{
    setEndElement(note);
}

void LaissezVib::setEndElement(EngravingItem* e)
{
    UNUSED(e);
    ASSERT_X("Laissez vibrer ties do not have an end note");
}

LaissezVibSegment::LaissezVibSegment(System* parent)
    : TieSegment(ElementType::LAISSEZ_VIB_SEGMENT, parent)
{
}

LaissezVibSegment::LaissezVibSegment(const LaissezVibSegment& s)
    : TieSegment(s)
{
}

void LaissezVibSegment::editDrag(EditData& ed)
{
    consolidateAdjustmentOffsetIntoUserOffset();

    ups(Grip::DRAG).off = PointF();
    roffset() += ed.delta;
}

String LaissezVibSegment::formatBarsAndBeats() const
{
    const Spanner* spanner = this->spanner();
    const Segment* startSeg = spanner ? spanner->startSegment() : nullptr;

    if (!startSeg) {
        return EngravingItem::formatBarsAndBeats();
    }

    return startSeg->formatBarsAndBeats();
}
}
