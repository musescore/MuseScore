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

#include "measurenumberbase.h"

#include "rw/xml.h"

#include "measure.h"
#include "score.h"
#include "staff.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   MeasureNumberBase
//---------------------------------------------------------

MeasureNumberBase::MeasureNumberBase(const ElementType& type, Measure* parent, TextStyleType tid)
    : TextBase(type, parent, tid)
{
    setFlag(ElementFlag::ON_STAFF, true);
}

//---------------------------------------------------------
//   MeasureNumberBase
//     Copy constructor
//---------------------------------------------------------

MeasureNumberBase::MeasureNumberBase(const MeasureNumberBase& other)
    : TextBase(other)
{
    setFlag(ElementFlag::ON_STAFF, true);
    setHPlacement(other.hPlacement());
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

engraving::PropertyValue MeasureNumberBase::getProperty(Pid id) const
{
    switch (id) {
    case Pid::HPLACEMENT:
        return int(hPlacement());
    default:
        return TextBase::getProperty(id);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool MeasureNumberBase::setProperty(Pid id, const PropertyValue& val)
{
    switch (id) {
    case Pid::HPLACEMENT:
        setHPlacement(val.value<PlacementH>());
        setLayoutInvalid();
        triggerLayout();
        return true;
    default:
        return TextBase::setProperty(id, val);
    }
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue MeasureNumberBase::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::DEFAULT;
    default:
        return TextBase::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void MeasureNumberBase::layout()
{
    setPos(PointF());
    if (!explicitParent()) {
        setOffset(0.0, 0.0);
    }

    // TextBase::layout1() needs to be called even if there's no measure attached to it.
    // This happens for example in the palettes.
    TextBase::layout1();
    // this could be if (!measure()) but it is the same as current and slower
    // See implementation of MeasureNumberBase::measure().
    if (!explicitParent()) {
        return;
    }

    if (placeBelow()) {
        double yoff = bbox().height();

        // If there is only one line, the barline spans outside the staff lines, so the default position is not correct.
        if (staff()->constStaffType(measure()->tick())->lines() == 1) {
            yoff += 2.0 * spatium();
        } else {
            yoff += staff()->height();
        }

        setPosY(yoff);
    } else {
        double yoff = 0.0;

        // If there is only one line, the barline spans outside the staff lines, so the default position is not correct.
        if (staff()->constStaffType(measure()->tick())->lines() == 1) {
            yoff -= 2.0 * spatium();
        }

        setPosY(yoff);
    }

    if (hPlacement() == PlacementH::CENTER) {
        // measure numbers should be centered over where there can be notes.
        // This means that header and trailing segments should be ignored,
        // which includes all timesigs, clefs, keysigs, etc.
        // This is how it should be centered:
        // |bb 4/4 notes-chords #| other measure |
        // |      ------18------ | other measure |

        //    x1 - left measure position of free space
        //    x2 - right measure position of free space

        const Measure* mea = measure();

        // find first chordrest
        Segment* chordRest = mea->first(SegmentType::ChordRest);

        Segment* s1 = chordRest->prevActive();
        // unfortunately, using !s1->header() does not work
        while (s1 && (s1->isChordRestType()
                      || s1->isBreathType()
                      || s1->isClefType()
                      || s1->isBarLineType()
                      || !s1->element(staffIdx() * VOICES))) {
            s1 = s1->prevActive();
        }

        Segment* s2 = chordRest->next();
        // unfortunately, using !s1->trailer() does not work
        while (s2 && (s2->isChordRestType()
                      || s2->isBreathType()
                      || s2->isClefType()
                      || s2->isBarLineType()
                      || !s2->element(staffIdx() * VOICES))) {
            s2 = s2->nextActive();
        }

        // if s1/s2 does not exist, it means there is no header/trailer segment. Align with start/end of measure.
        double x1 = s1 ? s1->x() + s1->minRight() : 0;
        double x2 = s2 ? s2->x() - s2->minLeft() : mea->width();

        setPosX((x1 + x2) * 0.5);
    } else if (hPlacement() == PlacementH::RIGHT) {
        setPosX(measure()->width());
    }
}
} // namespace MS
