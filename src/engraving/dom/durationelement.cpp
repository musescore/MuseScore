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

#include "durationelement.h"

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
    m_tuplet = 0;
}

//---------------------------------------------------------
//   DurationElement
//---------------------------------------------------------

DurationElement::DurationElement(const DurationElement& e)
    : EngravingItem(e)
{
    m_tuplet   = 0;      // e._tuplet;
    m_duration = e.m_duration;
}

//---------------------------------------------------------
//   ~DurationElement
//---------------------------------------------------------

DurationElement::~DurationElement()
{
    if (m_tuplet) {
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
    Fraction f(m_duration);
    for (Tuplet* t = tuplet(); t; t = t->tuplet()) {
        f /= t->ratio();
    }
    return f;
}

float DurationElement::timeStretchFactor() const
{
    int nominalDuration = m_duration.ticks();
    int actualDuration = actualTicks().ticks();

    return actualDuration / static_cast<float>(nominalDuration);
}

//---------------------------------------------------------
//   actualTicks
//---------------------------------------------------------

Fraction DurationElement::actualTicksAt(const Fraction& tick) const
{
    // Use when tick() is unreliable, for example when pasting
    return globalTicks() / staff()->timeStretch(tick);
}

Fraction DurationElement::actualTicks() const
{
    return actualTicksAt(tick());
}

//---------------------------------------------------------
//   readAddTuplet
//---------------------------------------------------------

void DurationElement::readAddTuplet(Tuplet* t)
{
    setTuplet(t);
    if (!score()->undoStack()->hasActiveCommand()) { // HACK, also added in Undo::AddElement()
        t->add(this);
    }
}

void DurationElement::setTuplet(Tuplet* t)
{
    if (m_tuplet) {
        m_tuplet->removeDurationElement(this);
    }

    m_tuplet = t;

    if (m_tuplet) {
        m_tuplet->addDurationElement(this);
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue DurationElement::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::DURATION:
        return PropertyValue::fromValue(m_duration);
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
