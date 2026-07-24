/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#ifndef MU_ENGRAVING_STAFFTYPECHANGE_H
#define MU_ENGRAVING_STAFFTYPECHANGE_H

#include "engravingitem.h"

namespace mu::engraving {
class Measure;
class StaffType;

//---------------------------------------------------------
//   @@ StaffTypeChange
//---------------------------------------------------------

class StaffTypeChange final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, StaffTypeChange)
    DECLARE_CLASSOF(ElementType::STAFFTYPE_CHANGE)

public:
    ~StaffTypeChange() override;

    StaffTypeChange* clone() const override { return new StaffTypeChange(*this); }

    static Fraction insertionTick(const Measure* measure);
    static Fraction insertionTick(const Measure* measure, const PointF& dropPos);

    Fraction tick() const override { return m_tick; }
    Fraction rtick() const override;

    const StaffType* staffType() const { return m_staffType; }
    void setStaffType(StaffType* st, bool owned);
    void setStaffTypeAndTick(StaffType* st, bool owned, const Fraction& tick);
    void setTickFromMeasure(const Measure* measure);
    void setTickFromDropPosition(const Measure* measure, const PointF& dropPos);
    void moveTicks(const Fraction& diff);
    bool isAtMeasureStart() const;

    double lw() const { return m_lw; }

    Measure* measure() const { return toMeasure(explicitParent()); }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

private:

    void setTick(const Fraction& tick);

    friend class Factory;
    StaffTypeChange(MeasureBase* parent = 0);
    StaffTypeChange(const StaffTypeChange&);

    void spatiumChanged(double oldValue, double newValue) override;

    StaffType* m_staffType = nullptr;
    bool m_ownsStaffType = false;
    double m_lw = 0.0;
    Fraction m_tick = Fraction(0, 1);
};
} // namespace mu::engraving

#endif
