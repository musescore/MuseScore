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

#include "element.h"

namespace Ms {
class StaffType;

//---------------------------------------------------------
//   @@ StaffTypeChange
//---------------------------------------------------------

class StaffTypeChange final : public Element
{
    StaffType* _staffType { 0 };
    qreal lw;

    void layout() override;
    void spatiumChanged(qreal oldValue, qreal newValue) override;
    void draw(mu::draw::Painter*) const override;

public:
    StaffTypeChange(MeasureBase* parent = 0);
    StaffTypeChange(const StaffTypeChange&);

    StaffTypeChange* clone() const override { return new StaffTypeChange(*this); }

    void write(XmlWriter&) const override;
    void read(XmlReader&) override;

    const StaffType* staffType() const { return _staffType; }
    void setStaffType(StaffType* st) { _staffType = st; }

    Measure* measure() const { return toMeasure(parent()); }

    QVariant getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const QVariant&) override;
    QVariant propertyDefault(Pid) const override;
};
}     // namespace Ms

#endif
