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

#include <optional>

#include "engravingitem.h"
#include "stafftype.h"

namespace mu::engraving {
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

    const StaffType* staffType() const { return m_staffType; }
    void setStaffType(StaffType* st, bool owned);

    //! Returns the staff baseline saved before this marker overrides the first measure.
    const StaffType* initialStaffType() const { return m_initialStaffType ? &*m_initialStaffType : nullptr; }
    //! Saves the baseline independently of the active first-measure staff type.
    void setInitialStaffType(const StaffType& type) { m_initialStaffType = type; }

    void startDrag(EditData&) override;
    RectF drag(EditData&) override;
    void endDrag(EditData&) override;
    std::vector<LineF> dragAnchorLines() const override;

    double lw() const { return m_lw; }

    Measure* measure() const { return toMeasure(ownershipParent()); }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

private:

    friend class Factory;
    StaffTypeChange(MeasureBase* parent = 0);
    StaffTypeChange(const StaffTypeChange&);

    void spatiumChanged(double oldValue, double newValue) override;

    StaffType* m_staffType = nullptr;
    std::optional<StaffType> m_initialStaffType;
    bool m_ownsStaffType = false;
    double m_lw = 0.0;
    Measure* m_dragTarget = nullptr;
    PointF m_dragOffset;
};
} // namespace mu::engraving

#endif
