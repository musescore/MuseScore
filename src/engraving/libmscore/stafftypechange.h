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

#ifndef __STAFFTYPECHANGE_H__
#define __STAFFTYPECHANGE_H__

#include "engravingitem.h"

namespace mu::engraving {
class StaffType;

//---------------------------------------------------------
//   @@ StaffTypeChange
//---------------------------------------------------------

class StaffTypeChange final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, StaffTypeChange)
    DECLARE_CLASSOF(ElementType::STAFFTYPE_CHANGE)

    StaffType* m_staffType { nullptr };
    bool m_ownsStaffType = false;
    double lw;

    friend class Factory;
    StaffTypeChange(MeasureBase* parent = 0);
    StaffTypeChange(const StaffTypeChange&);

    void layout() override;
    void spatiumChanged(double oldValue, double newValue) override;
    void draw(mu::draw::Painter*) const override;

public:
    ~StaffTypeChange() override;

    StaffTypeChange* clone() const override { return new StaffTypeChange(*this); }

    const StaffType* staffType() const { return m_staffType; }
    void setStaffType(StaffType* st, bool owned);

    Measure* measure() const { return toMeasure(explicitParent()); }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;
};
} // namespace mu::engraving

#endif
