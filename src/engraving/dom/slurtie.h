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

#pragma once

#include "spanner.h"

#include "draw/types/painterpath.h"

namespace mu::engraving {
//---------------------------------------------------------
//   SlurPos
//---------------------------------------------------------

struct SlurTiePos {
    PointF p1;               // start point of slur
    System* system1 = nullptr;          // start system of slur
    PointF p2;               // end point of slur
    System* system2 = nullptr;           // end system of slur
};

struct SlurOffsets {
    PointF o[4];
};

//---------------------------------------------------------
//   UP
//---------------------------------------------------------

struct UP {
    PointF p;              // layout position relative to pos()
    PointF off;            // user offset in point units

    PointF pos() const { return p + off; }
    bool operator!=(const UP& up) const { return p != up.p || off != up.off; }
};

//---------------------------------------------------------
//   CubicBezier
//    Helper class to optimize cubic Bezier curve points
//    calculation.
//---------------------------------------------------------

class CubicBezier
{
    PointF p1;
    PointF p2;
    PointF p3;
    PointF p4;

public:
    CubicBezier(PointF _p1, PointF _p2, PointF _p3, PointF _p4)
        : p1(_p1), p2(_p2), p3(_p3), p4(_p4) {}

    PointF pointAtPercent(double t) const
    {
        assert(t >= 0.0 && t <= 1.0);
        const double r = 1.0 - t;
        const PointF B123 = r * (r * p1 + t * p2) + t * (r * p2 + t * p3);
        const PointF B234 = r * (r * p2 + t * p3) + t * (r * p3 + t * p4);
        return r * B123 + t * B234;
    }
};

class SlurTie;

//---------------------------------------------------------
//   SlurTieSegment
//---------------------------------------------------------

class SlurTieSegment : public SpannerSegment
{
    OBJECT_ALLOCATOR(engraving, SlurTieSegment)

public:

    virtual void spatiumChanged(double, double) override;
    SlurTie* slurTie() const { return (SlurTie*)spanner(); }

    bool isEditAllowed(EditData&) const override;
    bool edit(EditData&) override;

    void startEditDrag(EditData& ed) override;
    void endEditDrag(EditData& ed) override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid id) const override;
    void reset() override;
    void undoChangeProperty(Pid id, const PropertyValue&, PropertyFlags ps) override;
    void move(const PointF& s) override;
    bool isEditable() const override { return true; }

    int subtype() const override;
    TranslatableString subtypeUserName() const override;

    void setSlurOffset(Grip i, const PointF& val) { m_ups[int(i)].off = val; }
    const UP& ups(Grip i) const { return m_ups[int(i)]; }
    UP& ups(Grip i) { return m_ups[int(i)]; }

    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return int(Grip::GRIPS); }
    Grip initialEditModeGrip() const override { return Grip::END; }
    Grip defaultGrip() const override { return Grip::DRAG; }
    std::vector<PointF> gripsPositions(const EditData& = EditData()) const override;

    virtual void drawEditMode(muse::draw::Painter* painter, EditData& editData, double currentViewScaling) override;

    virtual double endWidth() const = 0;
    virtual double midWidth() const = 0;
    virtual double dottedWidth() const = 0;

    struct LayoutData : public SpannerSegment::LayoutData
    {
        ld_field<muse::draw::PainterPath> path = "path";
        ld_field<double> midThickness = "midThickness";
    };
    DECLARE_LAYOUTDATA_METHODS(SlurTieSegment)

protected:
    SlurTieSegment(const ElementType& type, System*);
    SlurTieSegment(const SlurTieSegment&);

    virtual void changeAnchor(EditData&, EngravingItem*) = 0;
    std::vector<LineF> gripAnchorLines(Grip grip) const override;

    struct UP m_ups[int(Grip::GRIPS)];
};

//-------------------------------------------------------------------
//   @@ SlurTie
//   @P lineType       int  (0 - solid, 1 - dotted, 2 - dashed, 3 - wide dashed)
//   @P slurDirection  enum (Direction.AUTO, Direction.DOWN, Direction.UP)
//-------------------------------------------------------------------

class SlurTie : public Spanner
{
    OBJECT_ALLOCATOR(engraving, SlurTie)

public:
    SlurTie(const ElementType& type, EngravingItem* parent);
    SlurTie(const SlurTie&);
    ~SlurTie();

    bool up() const { return m_up; }
    void setUp(bool val) { m_up = val; }

    virtual void reset() override;

    DirectionV slurDirection() const { return m_slurDirection; }
    void setSlurDirection(DirectionV d) { m_slurDirection = d; }
    void undoSetSlurDirection(DirectionV d);

    virtual void layout2(const PointF, int, struct UP&) {}
    virtual bool contains(const PointF&) const { return false; }    // not selectable

    SlurStyleType styleType() const { return m_styleType; }
    void setStyleType(SlurStyleType type) { m_styleType = type; }

    int subtype() const override { return static_cast<int>(m_styleType) + 1; }
    TranslatableString subtypeUserName() const override;

    virtual SlurTieSegment* newSlurTieSegment(System* parent) = 0;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid id) const override;

    void fixupSegments(unsigned nsegs);

    virtual double scalingFactor() const = 0;

protected:

    bool m_up = true;                 // actual direction

    DirectionV m_slurDirection = DirectionV::AUTO;

private:

    SlurStyleType m_styleType = SlurStyleType::Undefined;
};
}
