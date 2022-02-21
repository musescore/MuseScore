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

#ifndef __LINE_H__
#define __LINE_H__

#include "infrastructure/draw/color.h"
#include "spanner.h"

namespace Ms {
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
protected:
    virtual void editDrag(EditData&) override;
    virtual bool isEditAllowed(EditData&) const override;
    virtual bool edit(EditData&) override;
    QVector<mu::LineF> gripAnchorLines(Grip) const override;
    virtual void startEditDrag(EditData&) override;
    void startDrag(EditData&) override;

    LineSegment(const ElementType& type, Spanner* sp, System* parent, ElementFlags f = ElementFlag::NOTHING);
    LineSegment(const ElementType& type, System* parent, ElementFlags f = ElementFlag::NOTHING);

public:

    LineSegment(const LineSegment&);
    SLine* line() const { return (SLine*)spanner(); }
    virtual void spatiumChanged(qreal, qreal) override;
    virtual void localSpatiumChanged(qreal, qreal) override;

    friend class SLine;
    virtual void read(XmlReader&) override;
    bool readProperties(XmlReader&) override;

    virtual EngravingItem* propertyDelegate(Pid) override;

    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return 3; }
    Grip initialEditModeGrip() const override { return Grip::END; }
    Grip defaultGrip() const override { return Grip::MIDDLE; }
    std::vector<mu::PointF> gripsPositions(const EditData& = EditData()) const override;

    QVector<mu::LineF> dragAnchorLines() const override;
    mu::RectF drag(EditData& ed) override;
private:
    mu::PointF leftAnchorPosition(const qreal& systemPositionY) const;
    mu::PointF rightAnchorPosition(const qreal& systemPositionY) const;

    Segment* findSegmentForGrip(Grip grip, mu::PointF pos) const;
    static mu::PointF deltaRebaseLeft(const Segment* oldSeg, const Segment* newSeg);
    static mu::PointF deltaRebaseRight(const Segment* oldSeg, const Segment* newSeg, int staffIdx);
    static Fraction lastSegmentEndTick(const Segment* lastSeg, const Spanner* s);
    LineSegment* rebaseAnchor(Grip grip, Segment* newSeg);
    void rebaseAnchors(EditData&, Grip);
};

//---------------------------------------------------------
//   @@ SLine
///    virtual base class for Hairpin, Trill and TextLine
//---------------------------------------------------------

class SLine : public Spanner
{
    Millimetre _lineWidth;
    mu::draw::Color _lineColor { engravingConfiguration()->defaultColor() };
    mu::draw::PenStyle _lineStyle { mu::draw::PenStyle::SolidLine };
    qreal _dashLineLen      { 5.0 };
    qreal _dashGapLen       { 5.0 };
    bool _diagonal          { false };

protected:
    virtual mu::PointF linePos(Grip, System** system) const;

public:
    SLine(const ElementType& type, EngravingItem* parent, ElementFlags = ElementFlag::NOTHING);
    SLine(const SLine&);

    virtual void layout() override;
    virtual SpannerSegment* layoutSystem(System*) override;

    bool readProperties(XmlReader& node) override;
    void writeProperties(XmlWriter& xml) const override;
    virtual LineSegment* createLineSegment(System* parent) = 0;
    void setLen(qreal l);
    using EngravingItem::bbox;
    const mu::RectF& bbox() const override;

    virtual void write(XmlWriter&) const override;
    virtual void read(XmlReader&) override;

    bool diagonal() const { return _diagonal; }
    void setDiagonal(bool v) { _diagonal = v; }

    Millimetre lineWidth() const { return _lineWidth; }
    mu::draw::Color lineColor() const { return _lineColor; }
    mu::draw::PenStyle lineStyle() const { return _lineStyle; }
    void setLineWidth(const Millimetre& v) { _lineWidth = v; }
    void setLineColor(const mu::draw::Color& v) { _lineColor = v; }
    void setLineStyle(mu::draw::PenStyle v) { _lineStyle = v; }

    qreal dashLineLen() const { return _dashLineLen; }
    void setDashLineLen(qreal val) { _dashLineLen = val; }
    qreal dashGapLen() const { return _dashGapLen; }
    void setDashGapLen(qreal val) { _dashGapLen = val; }

    LineSegment* frontSegment() { return toLineSegment(Spanner::frontSegment()); }
    const LineSegment* frontSegment() const { return toLineSegment(Spanner::frontSegment()); }
    LineSegment* backSegment() { return toLineSegment(Spanner::backSegment()); }
    const LineSegment* backSegment() const { return toLineSegment(Spanner::backSegment()); }
    LineSegment* segmentAt(int n) { return toLineSegment(Spanner::segmentAt(n)); }
    const LineSegment* segmentAt(int n) const { return toLineSegment(Spanner::segmentAt(n)); }

    mu::engraving::PropertyValue getProperty(Pid id) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid id) const override;

    friend class LineSegment;
};
}     // namespace Ms
#endif
