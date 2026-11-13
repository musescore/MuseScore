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
     * Grips: START/END are the corners on the edge nearest the staff and behave like
     * ordinary (non-diagonal) line grips, i.e. they only resize horizontally and may
     * re-anchor. MIDDLE is the centre of the box and moves it. HEIGHT is in the middle
     * of the far edge and only changes the box height (no re-anchoring).
     */
    static constexpr Grip HEIGHT = Grip(3);

    int gripsCount() const override { return 4; }
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

    bool edit(EditData&) override;
    std::vector<LineF> gripAnchorLines(Grip) const override;

protected:
    void startDragGrip(EditData&) override;
    void dragGrip(EditData&) override;
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
    static constexpr Spatium MIN_HEIGHT = 0.5_sp;
    static constexpr Spatium MAX_HEIGHT = 900.0_sp;

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
    void setBoxHeight(const Spatium& v);

private:
    Spatium m_height = 2.0_sp;
};
} // namespace mu::engraving
