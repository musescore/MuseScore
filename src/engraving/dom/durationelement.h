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

#ifndef MU_ENGRAVING_DURATION_H
#define MU_ENGRAVING_DURATION_H

#include "engravingitem.h"
#include "durationtype.h"

namespace mu::engraving {
class Beam;
class Tuplet;

//---------------------------------------------------------
//   @@ DurationElement
///    Virtual base class for Chord, Rest and Tuplet.
//
//   @P duration       Fraction  duration (as written)
//   @P globalDuration Fraction  played duration
//---------------------------------------------------------

class DurationElement : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, DurationElement)
public:
    ~DurationElement();

    virtual Measure* measure() const { return (Measure*)(explicitParent()); }

    void readAddTuplet(Tuplet* t);

    void setTuplet(Tuplet* t);
    Tuplet* tuplet() const { return m_tuplet; }
    Tuplet* topTuplet() const;
    virtual Beam* beam() const { return nullptr; }

    Fraction actualTicksAt(const Fraction& tick) const;
    Fraction actualTicks() const;

    // Length expressed as a fraction of a whole note
    virtual Fraction ticks() const { return m_duration; }
    Fraction globalTicks() const;
    float timeStretchFactor() const;
    void setTicks(const Fraction& f) { m_duration = f; }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;

protected:
    DurationElement(const ElementType& type, EngravingItem* parent = nullptr, ElementFlags = ElementFlag::MOVABLE | ElementFlag::ON_STAFF);
    DurationElement(const DurationElement& e);

private:
    Fraction m_duration;
    Tuplet* m_tuplet = nullptr;
};
} // namespace mu::engraving

#endif
