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

#ifndef __LETRING_H__
#define __LETRING_H__

#include "chordtextlinebase.h"

namespace mu::engraving {
class LetRing;

//---------------------------------------------------------
//   @@ LetRingSegment
//---------------------------------------------------------

class LetRingSegment final : public TextLineBaseSegment
{
    OBJECT_ALLOCATOR(engraving, LetRingSegment)
public:
    LetRingSegment(LetRing* sp, System* parent);

    LetRingSegment* clone() const override { return new LetRingSegment(*this); }

    LetRing* letRing() const { return (LetRing*)spanner(); }

    void layout() override;

    friend class LetRing;
};

//---------------------------------------------------------
//   @@ LetRing
//---------------------------------------------------------

class LetRing final : public ChordTextLineBase
{
    OBJECT_ALLOCATOR(engraving, LetRing)
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
