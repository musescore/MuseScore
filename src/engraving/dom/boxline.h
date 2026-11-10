/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#pragma once

#include "line.h"

namespace mu::engraving {
class BoxLine;

//---------------------------------------------------------
//  BoxLineSegment
//---------------------------------------------------------

class BoxLineSegment final : public LineSegment
{
    OBJECT_ALLOCATOR(engraving, BoxLineSegment)
    DECLARE_CLASSOF(ElementType::BOXLINE_SEGMENT)

public:
    BoxLineSegment(BoxLine* sp);

    BoxLineSegment* clone() const override { return new BoxLineSegment(*this); }

    BoxLine* boxLine() const { return toBoxLine(spanner()); }

    struct LayoutData : public EngravingItem::LayoutData {
        RectF rect; // box in segment-local coordinates
        bool closedLeft = true;
        bool closedRight = true;
    };
    DECLARE_LAYOUTDATA_METHODS(BoxLineSegment)
};

//---------------------------------------------------------
//  BoxLine
///     A general-purpose box, segment-anchored
//---------------------------------------------------------

class BoxLine final : public SLine
{
    OBJECT_ALLOCATOR(engraving, BoxLine)
    DECLARE_CLASSOF(ElementType::BOXLINE)

public:
    BoxLine(DummyParentOr<EngravingItem> parent);
    BoxLine(const BoxLine&);
    ~BoxLine() {}

    BoxLine* clone() const override { return new BoxLine(*this); }

    Anchor anchor() const override { return Anchor::SEGMENT; }

    LineSegment* createLineSegment() override;

    Segment* segment() const { return (Segment*)ownershipParent(); }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    PointF defaultPos() const override;

    Spatium boxHeight() const { return m_height; }
    void setBoxHeight(const Spatium& v) { m_height = v; }

private:
    Spatium m_height = 2.0_sp;
};
} // namespace mu::engraving
