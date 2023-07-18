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
#include "slurtie.h"

#include "draw/types/pen.h"

#include "chord.h"
#include "mscoreview.h"
#include "note.h"
#include "page.h"
#include "score.h"
#include "system.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   SlurTieSegment
//---------------------------------------------------------

SlurTieSegment::SlurTieSegment(const ElementType& type, System* parent)
    : SpannerSegment(type, parent)
{
    setFlag(ElementFlag::ON_STAFF, true);
}

SlurTieSegment::SlurTieSegment(const SlurTieSegment& b)
    : SpannerSegment(b)
{
    for (int i = 0; i < int(Grip::GRIPS); ++i) {
        m_ups[i]   = b.m_ups[i];
        m_ups[i].p = PointF();
    }
    m_path = b.m_path;
}

//---------------------------------------------------------
//   gripAnchorLines
//---------------------------------------------------------

std::vector<LineF> SlurTieSegment::gripAnchorLines(Grip grip) const
{
    std::vector<LineF> result;

    if (!system() || (grip != Grip::START && grip != Grip::END)) {
        return result;
    }

    PointF sp(system()->pagePos());
    PointF pp(pagePos());
    PointF p1(ups(Grip::START).p + pp);
    PointF p2(ups(Grip::END).p + pp);

    PointF anchorPosition;
    int gripIndex = static_cast<int>(grip);

    switch (spannerSegmentType()) {
    case SpannerSegmentType::SINGLE:
        anchorPosition = (grip == Grip::START ? p1 : p2);
        break;

    case SpannerSegmentType::BEGIN:
        anchorPosition = (grip == Grip::START ? p1 : system()->abbox().topRight());
        break;

    case SpannerSegmentType::MIDDLE:
        anchorPosition = (grip == Grip::START ? sp : system()->abbox().topRight());
        break;

    case SpannerSegmentType::END:
        anchorPosition = (grip == Grip::START ? sp : p2);
        break;
    }

    const Page* p = system()->page();
    const PointF pageOffset = p ? p->pos() : PointF();
    result.push_back(LineF(anchorPosition, gripsPositions().at(gripIndex)).translated(pageOffset));

    return result;
}

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void SlurTieSegment::move(const PointF& s)
{
    EngravingItem::move(s);
    for (int k = 0; k < int(Grip::GRIPS); ++k) {
        m_ups[k].p += s;
    }
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void SlurTieSegment::spatiumChanged(double oldValue, double newValue)
{
    EngravingItem::spatiumChanged(oldValue, newValue);
    double diff = newValue / oldValue;
    for (UP& u : m_ups) {
        u.off *= diff;
    }
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> SlurTieSegment::gripsPositions(const EditData&) const
{
    const int ngrips = gripsCount();
    std::vector<PointF> grips(ngrips);

    const PointF p(pagePos());
    for (int i = 0; i < ngrips; ++i) {
        grips[i] = m_ups[i].p + m_ups[i].off + p;
    }

    return grips;
}

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void SlurTieSegment::startEditDrag(EditData& ed)
{
    ElementEditDataPtr eed = ed.getData(this);
    IF_ASSERT_FAILED(eed) {
        return;
    }
    for (auto i : { Pid::SLUR_UOFF1, Pid::SLUR_UOFF2, Pid::SLUR_UOFF3, Pid::SLUR_UOFF4, Pid::OFFSET }) {
        eed->pushProperty(i);
    }
}

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

void SlurTieSegment::endEditDrag(EditData& ed)
{
    EngravingItem::endEditDrag(ed);
    triggerLayout();
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void SlurTieSegment::editDrag(EditData& ed)
{
    Grip g     = ed.curGrip;
    ups(g).off += ed.delta;

    PointF delta;

    switch (g) {
    case Grip::START:
    case Grip::END:
        //
        // move anchor for slurs/ties
        //
        if ((g == Grip::START && isSingleBeginType()) || (g == Grip::END && isSingleEndType())) {
            Spanner* spanner = slurTie();
            KeyboardModifiers km = ed.modifiers;
            EngravingItem* e = ed.view()->elementNear(ed.pos);
            if (e && e->isNote()) {
                Note* note = toNote(e);
                Fraction tick = note->chord()->tick();
                if ((g == Grip::END && tick > slurTie()->tick()) || (g == Grip::START && tick < slurTie()->tick2())) {
                    if (km != (ShiftModifier | ControlModifier)) {
                        Chord* c = note->chord();
                        ed.view()->setDropTarget(note);
                        if (c->part() == spanner->part() && c != spanner->endCR()) {
                            changeAnchor(ed, c);
                        }
                    }
                }
            } else {
                ed.view()->setDropTarget(0);
            }
        }
        break;
    case Grip::BEZIER1:
        break;
    case Grip::BEZIER2:
        break;
    case Grip::SHOULDER:
        ups(g).off = PointF();
        delta = ed.delta;
        break;
    case Grip::DRAG:
        ups(g).off = PointF();
        setOffset(offset() + ed.delta);
        break;
    case Grip::NO_GRIP:
    case Grip::GRIPS:
        break;
    }
    computeBezier(delta);
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue SlurTieSegment::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SLUR_STYLE_TYPE:
    case Pid::SLUR_DIRECTION:
        return slurTie()->getProperty(propertyId);
    case Pid::SLUR_UOFF1:
        return ups(Grip::START).off;
    case Pid::SLUR_UOFF2:
        return ups(Grip::BEZIER1).off;
    case Pid::SLUR_UOFF3:
        return ups(Grip::BEZIER2).off;
    case Pid::SLUR_UOFF4:
        return ups(Grip::END).off;
    default:
        return SpannerSegment::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool SlurTieSegment::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::SLUR_STYLE_TYPE:
    case Pid::SLUR_DIRECTION:
        return slurTie()->setProperty(propertyId, v);
    case Pid::SLUR_UOFF1:
        ups(Grip::START).off = v.value<PointF>();
        break;
    case Pid::SLUR_UOFF2:
        ups(Grip::BEZIER1).off = v.value<PointF>();
        break;
    case Pid::SLUR_UOFF3:
        ups(Grip::BEZIER2).off = v.value<PointF>();
        break;
    case Pid::SLUR_UOFF4:
        ups(Grip::END).off = v.value<PointF>();
        break;
    default:
        return SpannerSegment::setProperty(propertyId, v);
    }
    triggerLayoutAll();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue SlurTieSegment::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::SLUR_STYLE_TYPE:
    case Pid::SLUR_DIRECTION:
        return slurTie()->propertyDefault(id);
    case Pid::SLUR_UOFF1:
    case Pid::SLUR_UOFF2:
    case Pid::SLUR_UOFF3:
    case Pid::SLUR_UOFF4:
        return PointF();
    default:
        return SpannerSegment::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void SlurTieSegment::reset()
{
    EngravingItem::reset();
    undoResetProperty(Pid::SLUR_UOFF1);
    undoResetProperty(Pid::SLUR_UOFF2);
    undoResetProperty(Pid::SLUR_UOFF3);
    undoResetProperty(Pid::SLUR_UOFF4);
    slurTie()->reset();
}

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void SlurTieSegment::undoChangeProperty(Pid pid, const PropertyValue& val, PropertyFlags ps)
{
    if (pid == Pid::AUTOPLACE && (val.toBool() == true && !autoplace())) {
        // Switching autoplacement on. Save user-defined
        // placement properties to undo stack.
        undoPushProperty(Pid::SLUR_UOFF1);
        undoPushProperty(Pid::SLUR_UOFF2);
        undoPushProperty(Pid::SLUR_UOFF3);
        undoPushProperty(Pid::SLUR_UOFF4);
        // other will be saved in base classes.
    }
    SpannerSegment::undoChangeProperty(pid, val, ps);
}

//---------------------------------------------------------
//   drawEditMode
//---------------------------------------------------------

void SlurTieSegment::drawEditMode(mu::draw::Painter* p, EditData& ed, double /*currentViewScaling*/)
{
    using namespace mu::draw;
    PolygonF polygon(7);
    polygon[0] = PointF(ed.grip[int(Grip::START)].center());
    polygon[1] = PointF(ed.grip[int(Grip::BEZIER1)].center());
    polygon[2] = PointF(ed.grip[int(Grip::SHOULDER)].center());
    polygon[3] = PointF(ed.grip[int(Grip::BEZIER2)].center());
    polygon[4] = PointF(ed.grip[int(Grip::END)].center());
    polygon[5] = PointF(ed.grip[int(Grip::DRAG)].center());
    polygon[6] = PointF(ed.grip[int(Grip::START)].center());
    p->setPen(Pen(engravingConfiguration()->formattingMarksColor(), 0.0));
    p->drawPolyline(polygon);

    p->setPen(Pen(engravingConfiguration()->defaultColor(), 0.0));
    for (int i = 0; i < ed.grips; ++i) {
        // Can't use ternary operator, because we want different overloads of `setBrush`
        if (Grip(i) == ed.curGrip) {
            p->setBrush(engravingConfiguration()->formattingMarksColor());
        } else {
            p->setBrush(BrushStyle::NoBrush);
        }
        p->drawRect(ed.grip[i]);
    }
}

//---------------------------------------------------------
//   SlurTie
//---------------------------------------------------------

SlurTie::SlurTie(const ElementType& type, EngravingItem* parent)
    : Spanner(type, parent)
{
    m_slurDirection = DirectionV::AUTO;
    m_up            = true;
    m_styleType     = SlurStyleType::Solid;
}

SlurTie::SlurTie(const SlurTie& t)
    : Spanner(t)
{
    m_up            = t.m_up;
    m_slurDirection = t.m_slurDirection;
    m_styleType     = t.m_styleType;
}

//---------------------------------------------------------
//   SlurTie
//---------------------------------------------------------

SlurTie::~SlurTie()
{
}

//---------------------------------------------------------
//   undoSetSlurDirection
//---------------------------------------------------------

void SlurTie::undoSetSlurDirection(DirectionV d)
{
    undoChangeProperty(Pid::SLUR_DIRECTION, PropertyValue::fromValue<DirectionV>(d));
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue SlurTie::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SLUR_STYLE_TYPE:
        return PropertyValue::fromValue<SlurStyleType>(styleType());
    case Pid::SLUR_DIRECTION:
        return PropertyValue::fromValue<DirectionV>(slurDirection());
    default:
        return Spanner::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool SlurTie::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::SLUR_STYLE_TYPE:
        setStyleType(v.value<SlurStyleType>());
        break;
    case Pid::SLUR_DIRECTION:
        setSlurDirection(v.value<DirectionV>());
        break;
    default:
        return Spanner::setProperty(propertyId, v);
    }
    triggerLayoutAll();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue SlurTie::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::SLUR_STYLE_TYPE:
        return 0;
    case Pid::SLUR_DIRECTION:
        return PropertyValue::fromValue<DirectionV>(DirectionV::AUTO);
    default:
        return Spanner::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   fixupSegments
//---------------------------------------------------------

void SlurTie::fixupSegments(unsigned nsegs)
{
    Spanner::fixupSegments(nsegs, [this](System* parent) { return newSlurTieSegment(parent); });
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void SlurTie::reset()
{
    EngravingItem::reset();
    undoResetProperty(Pid::SLUR_DIRECTION);
    undoResetProperty(Pid::SLUR_STYLE_TYPE);
}
}
