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

#ifndef __TRIPLETFEEL_H__
#define __TRIPLETFEEL_H__

#include "systemtext.h"

namespace mu::engraving {
//---------------------------------------------------------
//   TripletFeel
//---------------------------------------------------------

class TripletFeel final : public SystemText
{
    OBJECT_ALLOCATOR(engraving, TripletFeel)
    DECLARE_CLASSOF(ElementType::TRIPLET_FEEL)

    constexpr static int eightDivision = 2;
    constexpr static int sixteenthDivision = 4;
    constexpr static int tripletRatio = 66;
    constexpr static int dottedRatio = 75;
    constexpr static int scottishRatio = 25;
    constexpr static int zeroRatio = 0;

    TripletFeelType m_tripletFeelType;

public:
    TripletFeel(Segment* parent = nullptr, TripletFeelType tripletFillType = TripletFeelType::NONE);
    void setTripletProperty();

    TripletFeel* clone() const override { return new TripletFeel(*this); }
    TripletFeelType getTripletFeelType() const { return m_tripletFeelType; }
    PropertyValue propertyDefault(Pid propertyId) const override;
    Sid getPropertyStyle(Pid id) const override;
    TranslatableString subtypeUserName() const override;
    String accessibleInfo() const override;
};
}     // namespace mu::engraving
#endif
