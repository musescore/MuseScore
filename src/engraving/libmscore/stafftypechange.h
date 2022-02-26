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

namespace Ms {
class StaffType;

//---------------------------------------------------------
//   @@ StaffTypeChange
//---------------------------------------------------------

class StaffTypeChange final : public EngravingItem
{
    StaffType* m_staffType { nullptr };
    bool m_ownsStaffType = false;
    qreal lw;

    friend class mu::engraving::Factory;
    StaffTypeChange(MeasureBase* parent = 0);
    StaffTypeChange(const StaffTypeChange&);

    void layout() override;
    void spatiumChanged(qreal oldValue, qreal newValue) override;
    void draw(mu::draw::Painter*) const override;

public:
    ~StaffTypeChange() override;

    StaffTypeChange* clone() const override { return new StaffTypeChange(*this); }

    void write(XmlWriter&) const override;
    void read(XmlReader&) override;

    const StaffType* staffType() const { return m_staffType; }
    void setStaffType(StaffType* st, bool owned);

    Measure* measure() const { return toMeasure(explicitParent()); }

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid) const override;
};
}     // namespace Ms

#endif
