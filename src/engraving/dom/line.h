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

#ifndef MU_ENGRAVING_LINE_H
#define MU_ENGRAVING_LINE_H

#include "draw/types/color.h"
#include "spanner.h"

namespace mu::engraving {
class SLine;
class System;

//---------------------------------------------------------
//   @@ LineSegment
///    Virtual base class for segmented lines segments
///    (OttavaSegment, HairpinSegment, TrillSegment...)
///
///    This class describes one segment of an segmented
///    line object. Line objects can span multiple staves.
///    For every staff a segment is created.
//---------------------------------------------------------

class LineSegment : public SpannerSegment
{
    OBJECT_ALLOCATOR(engraving, LineSegment)
protected:
    virtual void editDrag(EditData&) override;
    void updateAnchors(EditData& ed) const;
    virtual bool isEditAllowed(EditData&) const override;
    virtual bool edit(EditData&) override;
    std::vector<LineF> gripAnchorLines(Grip) const override;
    virtual void startEditDrag(EditData&) override;
    void startDrag(EditData&) override;

    LineSegment(const ElementType& type, Spanner* sp, System* parent, ElementFlags f = ElementFlag::NOTHING);
    LineSegment(const ElementType& type, System* parent, ElementFlags f = ElementFlag::NOTHING);

public:

    LineSegment(const LineSegment&);
    SLine* line() const { return (SLine*)spanner(); }
    virtual void spatiumChanged(double, double) override;
    virtual void localSpatiumChanged(double, double) override;

    friend class SLine;

    virtual EngravingItem* propertyDelegate(Pid) override;

    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return 3; }
    Grip initialEditModeGrip() const override { return Grip::END; }
    Grip defaultGrip() const override { return Grip::MIDDLE; }
    std::vector<PointF> gripsPositions(const EditData& = EditData()) const override;

    std::vector<LineF> dragAnchorLines() const override;
    RectF drag(EditData& ed) override;

    Spatium lineWidth() const;

    double absoluteFromSpatium(const Spatium& sp) const override;

private:
    Segment* findNewAnchorSegment(const EditData& ed, const Segment* curSeg);
    void undoMoveStartEndAndSnappedItems(bool moveStart, bool moveEnd, Segment* s1, Segment* s2);
    PointF leftAnchorPosition(const double& systemPositionY) const;
    PointF rightAnchorPosition(const double& systemPositionY) const;

    Segment* findSegmentForGrip(Grip grip, PointF pos) const;
    static PointF deltaRebaseLeft(const Segment* oldSeg, const Segment* newSeg);
    static PointF deltaRebaseRight(const Segment* oldSeg, const Segment* newSeg);
    static Fraction lastSegmentEndTick(const Segment* lastSeg, const Spanner* s);
    LineSegment* rebaseAnchor(Grip grip, Segment* newSeg);
    void rebaseOffsetsOnAnchorChanged(Grip grip, const PointF& oldPos, System* sys);
    void rebaseAnchors(EditData&, Grip);
};

//---------------------------------------------------------
//   @@ SLine
///    virtual base class for Hairpin, Trill and TextLine
//---------------------------------------------------------

class SLine : public Spanner
{
    OBJECT_ALLOCATOR(engraving, SLine)

public:
    SLine(const ElementType& type, EngravingItem* parent, ElementFlags = ElementFlag::NOTHING);
    SLine(const SLine&);

    virtual LineSegment* createLineSegment(System* parent) = 0;
    void setLen(double l);

    bool diagonal() const { return m_diagonal; }
    void setDiagonal(bool v) { m_diagonal = v; }

    Spatium lineWidth() const { return m_lineWidth; }
    Color lineColor() const { return m_lineColor; }
    LineType lineStyle() const { return m_lineStyle; }
    void setLineWidth(const Spatium& v) { m_lineWidth = v; }
    void setLineColor(const Color& v) { m_lineColor = v; }
    void setLineStyle(LineType v) { m_lineStyle = v; }

    double dashLineLen() const { return m_dashLineLen; }
    void setDashLineLen(double val) { m_dashLineLen = val; }
    double dashGapLen() const { return m_dashGapLen; }
    void setDashGapLen(double val) { m_dashGapLen = val; }

    LineSegment* frontSegment() { return toLineSegment(Spanner::frontSegment()); }
    const LineSegment* frontSegment() const { return toLineSegment(Spanner::frontSegment()); }
    LineSegment* backSegment() { return toLineSegment(Spanner::backSegment()); }
    const LineSegment* backSegment() const { return toLineSegment(Spanner::backSegment()); }
    LineSegment* segmentAt(int n) { return toLineSegment(Spanner::segmentAt(n)); }
    const LineSegment* segmentAt(int n) const { return toLineSegment(Spanner::segmentAt(n)); }

    PropertyValue getProperty(Pid id) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid id) const override;

    virtual PointF linePos(Grip grip, System** system) const;
    virtual bool allowTimeAnchor() const override { return true; }

    void undoMoveStart(Fraction tickDiff);
    void undoMoveEnd(Fraction tickDiff);

    static Note* guessFinalNote(Note* startNote);

private:

    friend class LineSegment;

    Spatium m_lineWidth;
    Color m_lineColor;
    LineType m_lineStyle = LineType::SOLID;
    double m_dashLineLen = 5.0;
    double m_dashGapLen = 5.0;
    bool m_diagonal = false;
};
} // namespace mu::engraving
#endif
