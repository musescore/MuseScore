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

#ifndef MU_ENGRAVING_WHAMMYBAR_H
#define MU_ENGRAVING_WHAMMYBAR_H

#include "chordtextlinebase.h"

namespace mu::engraving {
class WhammyBar;

//---------------------------------------------------------
//   @@ WhammyBarSegment
//---------------------------------------------------------

class WhammyBarSegment final : public TextLineBaseSegment
{
    OBJECT_ALLOCATOR(engraving, WhammyBarSegment)
    DECLARE_CLASSOF(ElementType::WHAMMY_BAR_SEGMENT)

public:
    WhammyBarSegment(WhammyBar* sp, System* parent);

    WhammyBarSegment* clone() const override { return new WhammyBarSegment(*this); }

    WhammyBar* whammyBar() const { return (WhammyBar*)spanner(); }

    friend class WhammyBar;
};

//---------------------------------------------------------
//   @@ WhammyBar
//---------------------------------------------------------

class WhammyBar final : public ChordTextLineBase
{
    OBJECT_ALLOCATOR(engraving, WhammyBar)
    DECLARE_CLASSOF(ElementType::WHAMMY_BAR)

public:
    WhammyBar(EngravingItem* parent);

    WhammyBar* clone() const override { return new WhammyBar(*this); }

    LineSegment* createLineSegment(System* parent) override;

    PropertyValue propertyDefault(Pid propertyId) const override;
    Sid getPropertyStyle(Pid) const override;
};
} // namespace mu::engraving
#endif
