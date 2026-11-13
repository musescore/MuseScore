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

#include "boxline.h"

#include <algorithm>

#include "editing/elementeditdata.h"

namespace mu::engraving {
//---------------------------------------------------------
//   BoxLineSegment
//---------------------------------------------------------

BoxLineSegment::BoxLineSegment(BoxLine* sp)
    : LineSegment(ElementType::BOXLINE_SEGMENT, sp, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{}

std::vector<PointF> BoxLineSegment::gripsPositions(const EditData&) const
{
    const RectF& r = ldata()->rect;
    const bool above = boxLine()->placeAbove();
    const double nearY = above ? r.bottom() : r.top(); // near = edge nearest the staff
    const double farY = above ? r.top() : r.bottom();
    const PointF pp = pagePos();

    // in page coordinates (+ pp)
    std::vector<PointF> grips(gripsCount());
    grips[int(Grip::START)] = PointF(r.left(), nearY) + pp;
    grips[int(Grip::END)] = PointF(r.right(), nearY) + pp;
    grips[int(Grip::MIDDLE)] = r.center() + pp;
    grips[int(HEIGHT)] = PointF(r.center().x(), farY) + pp;
    return grips;
}

EngravingObject* BoxLineSegment::propertyDelegate(Pid pid) const
{
    if (pid == Pid::BOXLINE_HEIGHT) {
        return spanner();
    }
    return LineSegment::propertyDelegate(pid);
}

bool BoxLineSegment::edit(EditData& ed)
{
    if (ed.curGrip == HEIGHT) {
        return false;
    }
    return LineSegment::edit(ed);
}

std::vector<LineF> BoxLineSegment::gripAnchorLines(Grip grip) const
{
    if (grip == HEIGHT) {
        return {};
    }
    return LineSegment::gripAnchorLines(grip);
}

void BoxLineSegment::startDragGrip(EditData& ed)
{
    LineSegment::startDragGrip(ed);
    if (ed.curGrip == HEIGHT) {
        if (ElementEditDataPtr eed = ed.getData(this)) {
            eed->pushProperty(Pid::BOXLINE_HEIGHT);
        }
    }
}

void BoxLineSegment::dragGrip(EditData& ed)
{
    if (ed.curGrip != HEIGHT) {
        // BoxLine is never diagonal, so START/END only resize horizontally
        LineSegment::dragGrip(ed);
        return;
    }

    BoxLine* box = boxLine();
    const double dy = ed.evtDelta.y() / absoluteFromSpatium(Spatium(1.0));
    const double growth = box->placeAbove() ? -dy : dy; // Mouse movement away from the staff
    box->setBoxHeight(box->boxHeight() + Spatium(growth));
    triggerLayout();
}

//---------------------------------------------------------
//   BoxLine
//---------------------------------------------------------

BoxLine::BoxLine(DummyParentOr<EngravingItem> parent)
    : SLine(ElementType::BOXLINE, parent, ElementFlag::PLACE_ABOVE)
{}

BoxLine::BoxLine(const BoxLine& other)
    : SLine(other)
{
    m_height = other.m_height;
}

LineSegment* BoxLine::createLineSegment()
{
    BoxLineSegment* seg = new BoxLineSegment(this);
    seg->setTrack(track());
    return seg;
}

PropertyValue BoxLine::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::BOXLINE_HEIGHT:
        return boxHeight();
    default:
        break;
    }
    return SLine::getProperty(propertyId);
}

bool BoxLine::setProperty(Pid propertyId, const PropertyValue& val)
{
    switch (propertyId) {
    case Pid::BOXLINE_HEIGHT:
        setBoxHeight(val.value<Spatium>());
        break;
    case Pid::DIAGONAL:
        // BoxLineSegment::dragGrip relies on LineSegment::dragGrip resizing
        // horizontally only, which holds only if diagonal() is false.
        return false;
    default:
        return SLine::setProperty(propertyId, val);
    }
    triggerLayout();
    return true;
}

PropertyValue BoxLine::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::BOXLINE_HEIGHT:
        return 2.0_sp;
    case Pid::PLACEMENT:
        return PlacementV::ABOVE;
    default:
        return SLine::propertyDefault(propertyId);
    }
}

PointF BoxLine::defaultPos() const
{
    const double gap = 1.0 * spatium();
    return PointF(0.0, placeAbove() ? -gap : gap);
}

void BoxLine::reset()
{
    SLine::reset();
    undoResetProperty(Pid::BOXLINE_HEIGHT);
}

void BoxLine::setBoxHeight(const Spatium& v)
{
    m_height = Spatium(std::clamp(v.val(), MIN_HEIGHT.val(), MAX_HEIGHT.val()));
}
}
