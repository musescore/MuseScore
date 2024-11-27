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

#ifndef MU_ENGRAVING_PEDAL_H
#define MU_ENGRAVING_PEDAL_H

#include "textlinebase.h"

namespace mu::engraving {
class Pedal;

//---------------------------------------------------------
//   @@ PedalSegment
//---------------------------------------------------------

class PedalSegment final : public TextLineBaseSegment
{
    OBJECT_ALLOCATOR(engraving, PedalSegment)
    DECLARE_CLASSOF(ElementType::PEDAL_SEGMENT)

    Sid getPropertyStyle(Pid) const override;

public:
    PedalSegment(Pedal* sp, System* parent);

    PedalSegment* clone() const override { return new PedalSegment(*this); }
    Pedal* pedal() const { return toPedal(spanner()); }

    int subtype() const override;
    TranslatableString subtypeUserName() const override;

    friend class Pedal;
};

//---------------------------------------------------------
//   @@ Pedal
//---------------------------------------------------------

class Pedal final : public TextLineBase
{
    OBJECT_ALLOCATOR(engraving, Pedal)
    DECLARE_CLASSOF(ElementType::PEDAL)

    Sid getPropertyStyle(Pid) const override;

protected:
    PointF linePos(Grip, System**) const override;

public:
    static const String PEDAL_SYMBOL;
    static const String STAR_SYMBOL;

    Pedal(EngravingItem* parent);

    Pedal* clone() const override { return new Pedal(*this); }

    LineSegment* createLineSegment(System* parent) override;
    PropertyValue propertyDefault(Pid propertyId) const override;

    Pedal* findNextInStaff() const;
    bool connect45HookToNext() const;

    void setPedalType(PedalType pp);
    PedalType pedalType() const { return m_pedalType; }

    int subtype() const override { return int(m_pedalType); }
    TranslatableString subtypeUserName() const override;
    String pedalTypeUserName() const;
    String accessibleInfo() const override;

    friend class PedalLine;

private:
    PedalType m_pedalType = PedalType::PED_AND_LINE;
};
} // namespace mu::engraving
#endif
