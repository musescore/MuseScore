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

#ifndef __PEDAL_H__
#define __PEDAL_H__

#include "textlinebase.h"

namespace Ms {
class Pedal;

//---------------------------------------------------------
//   @@ PedalSegment
//---------------------------------------------------------

class PedalSegment final : public TextLineBaseSegment
{
    Sid getPropertyStyle(Pid) const override;

public:
    PedalSegment(Spanner* sp, Score* s)
        : TextLineBaseSegment(ElementType::PEDAL_SEGMENT, sp, s, ElementFlag::MOVABLE | ElementFlag::ON_STAFF) {}

    PedalSegment* clone() const override { return new PedalSegment(*this); }
    Pedal* pedal() const { return toPedal(spanner()); }
    void layout() override;

    friend class Pedal;
};

//---------------------------------------------------------
//   @@ Pedal
//---------------------------------------------------------

class Pedal final : public TextLineBase
{
    Sid getPropertyStyle(Pid) const override;

protected:
    mu::PointF linePos(Grip, System**) const override;

public:
    Pedal(EngravingItem* parent);

    Pedal* clone() const override { return new Pedal(*this); }

    void read(XmlReader&) override;
    void write(XmlWriter& xml) const override;

    LineSegment* createLineSegment() override;
    QVariant propertyDefault(Pid propertyId) const override;

    friend class PedalLine;
};
}     // namespace Ms
#endif
