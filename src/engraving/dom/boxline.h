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

    /**
     * Grips: START/END are the corners on the edge nearest the staff, FAR_START/FAR_END
     * are the corners on the far edge, MIDDLE is the centre of the box. Every corner
     * resizes horizontally like START/END (including re-anchoring) and vertically by
     * changing the box height only. The center only offsets the box.
     */
    static constexpr Grip FAR_START = Grip(3);
    static constexpr Grip FAR_END = Grip(4);

    int gripsCount() const override { return 5; }
    Grip initialEditModeGrip() const override { return Grip::MIDDLE; }
    Grip defaultGrip() const override { return Grip::MIDDLE; }
    std::vector<PointF> gripsPositions(const EditData& = EditData()) const override;

    EngravingObject* propertyDelegate(Pid) const override;

    struct LayoutData : public EngravingItem::LayoutData {
        RectF rect; // box in segment-local coordinates
        bool closedLeft = true;
        bool closedRight = true;
    };
    DECLARE_LAYOUTDATA_METHODS(BoxLineSegment)

protected:
    Grip lineGripFor(Grip grip) const override;
    void startDragGrip(EditData&) override;
    void dragGrip(EditData&) override;

private:
    void resizeHeight(EditData&);
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

    void reset() override;

    Spatium boxHeight() const { return m_height; }
    void setBoxHeight(const Spatium& v) { m_height = v; }

private:
    Spatium m_height = 2.0_sp;
};
} // namespace mu::engraving
