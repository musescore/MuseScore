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

#ifndef MU_ENGRAVING_LETRING_H
#define MU_ENGRAVING_LETRING_H

#include "chordtextlinebase.h"

namespace mu::engraving {
class LetRing;

//---------------------------------------------------------
//   @@ LetRingSegment
//---------------------------------------------------------

class LetRingSegment final : public TextLineBaseSegment
{
    OBJECT_ALLOCATOR(engraving, LetRingSegment)
    DECLARE_CLASSOF(ElementType::LET_RING_SEGMENT)

public:
    LetRingSegment(LetRing* sp, System* parent);

    LetRingSegment* clone() const override { return new LetRingSegment(*this); }

    LetRing* letRing() const { return (LetRing*)spanner(); }

    friend class LetRing;
};

//---------------------------------------------------------
//   @@ LetRing
//---------------------------------------------------------

class LetRing final : public ChordTextLineBase
{
    OBJECT_ALLOCATOR(engraving, LetRing)
    DECLARE_CLASSOF(ElementType::LET_RING)

public:
    LetRing(EngravingItem* parent);

    LetRing* clone() const override { return new LetRing(*this); }

//      virtual void write(XmlWriter& xml) const override;
    LineSegment* createLineSegment(System* parent) override;

    PropertyValue propertyDefault(Pid propertyId) const override;
    Sid getPropertyStyle(Pid) const override;
};
} // namespace mu::engraving
#endif
