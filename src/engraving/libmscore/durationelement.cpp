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

#include "durationelement.h"

#include "rw/400/twrite.h"
#include "realfn.h"

#include "property.h"
#include "score.h"
#include "staff.h"
#include "tuplet.h"
#include "undo.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   DurationElement
//---------------------------------------------------------

DurationElement::DurationElement(const ElementType& type, EngravingItem* parent, ElementFlags f)
    : EngravingItem(type, parent, f)
{
    _tuplet = 0;
}

//---------------------------------------------------------
//   DurationElement
//---------------------------------------------------------

DurationElement::DurationElement(const DurationElement& e)
    : EngravingItem(e)
{
    _tuplet   = 0;      // e._tuplet;
    _duration = e._duration;
}

//---------------------------------------------------------
//   ~DurationElement
//---------------------------------------------------------

DurationElement::~DurationElement()
{
    if (_tuplet) {
        // Note that this sanity check is different from and unrelated to the next `if` condition.
        // See tuplet.h for the difference between `_tuplet->contains` (which involves `_tuplet->
        // _currentElements`) and `tuplet->_allElements`.
        assert(mu::contains(_tuplet->_allElements, this));

        if (_tuplet->contains(this)) {
            while (Tuplet* t = topTuplet()) { // delete tuplets from top to bottom
                delete t;   // Tuplet destructor removes references to the deleted object
            }
        }
        // else, the tuplet is in the UndoStack and will be deleted there

        setTuplet(nullptr);
    }
}

//---------------------------------------------------------
//   topTuplet
//---------------------------------------------------------

Tuplet* DurationElement::topTuplet() const
{
    Tuplet* t = tuplet();
    if (t) {
        while (t->tuplet()) {
            t = t->tuplet();
        }
    }
    return t;
}

//---------------------------------------------------------
//   globalTicks
//---------------------------------------------------------

Fraction DurationElement::globalTicks() const
{
    Fraction f(_duration);
    for (Tuplet* t = tuplet(); t; t = t->tuplet()) {
        f /= t->ratio();
    }
    return f;
}

float DurationElement::timeStretchFactor() const
{
    int nominalDuration = _duration.ticks();
    int actualDuration = actualTicks().ticks();

    return actualDuration / static_cast<float>(nominalDuration);
}

//---------------------------------------------------------
//   actualTicks
//---------------------------------------------------------

Fraction DurationElement::actualTicks() const
{
    return globalTicks() / staff()->timeStretch(tick());
}

//---------------------------------------------------------
//   readAddTuplet
//---------------------------------------------------------

void DurationElement::readAddTuplet(Tuplet* t)
{
    setTuplet(t);
    if (!score()->undoStack()->active()) {     // HACK, also added in Undo::AddElement()
        t->add(this);
    }
}

//---------------------------------------------------------
//   writeTupletStart
//---------------------------------------------------------

void DurationElement::writeTupletStart(XmlWriter& xml) const
{
    if (tuplet() && tuplet()->elements().front() == this) {
        tuplet()->writeTupletStart(xml);               // recursion
        rw400::TWrite::write(tuplet(), xml, *xml.context());
    }
}

//---------------------------------------------------------
//   writeTupletEnd
//---------------------------------------------------------

void DurationElement::writeTupletEnd(XmlWriter& xml) const
{
    if (tuplet() && tuplet()->elements().back() == this) {
        xml.tag("endTuplet");
        tuplet()->writeTupletEnd(xml);               // recursion
    }
}

void DurationElement::setTuplet(Tuplet* t)
{
    if (_tuplet) {
        _tuplet->removeDurationElement(this);
    }

    _tuplet = t;

    if (_tuplet) {
        _tuplet->addDurationElement(this);
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue DurationElement::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::DURATION:
        return PropertyValue::fromValue(_duration);
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool DurationElement::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::DURATION: {
        Fraction f(v.value<Fraction>());
        setTicks(f);
        triggerLayout();
    }
    break;
    default:
        return EngravingItem::setProperty(propertyId, v);
    }
    return true;
}
}
