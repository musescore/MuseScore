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

#include "beam.h"

#include <cmath>
#include <algorithm>

#include "containers.h"
#include "realfn.h"

#include "actionicon.h"
#include "chord.h"
#include "groups.h"
#include "measure.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "system.h"

#include "tremolotwochord.h"
#include "tuplet.h"

#include "log.h"

using namespace mu;
using namespace muse;
using namespace mu::engraving;

namespace mu::engraving {
static const ElementStyle beamStyle {
    { Sid::beamNoSlope,                        Pid::BEAM_NO_SLOPE },
};

//---------------------------------------------------------
//   Beam
//---------------------------------------------------------

Beam::Beam(System* parent)
    : BeamBase(ElementType::BEAM, parent)
{
    initElementStyle(&beamStyle);
    resetProperty(Pid::BEAM_CROSS_STAFF_MOVE);
}

//---------------------------------------------------------
//   Beam
//---------------------------------------------------------

Beam::Beam(const Beam& b)
    : BeamBase(b)
{
    m_elements     = b.m_elements;
    m_id           = b.m_id;
    m_growLeft           = b.m_growLeft;
    m_growRight           = b.m_growRight;
    m_beamDist        = b.m_beamDist;
    for (const BeamFragment* f : b.m_fragments) {
        m_fragments.push_back(new BeamFragment(*f));
    }
    m_minCRMove          = b.m_minCRMove;
    m_maxCRMove          = b.m_maxCRMove;
    m_isGrace          = b.m_isGrace;
    m_cross            = b.m_cross;
    m_fullCross        = b.m_fullCross;
    m_slope            = b.m_slope;
}

//---------------------------------------------------------
//   Beam
//---------------------------------------------------------

Beam::~Beam()
{
    //
    // delete all references from chords
    //
    for (ChordRest* cr : m_elements) {
        cr->setBeam(nullptr);
    }

    clearBeamSegments();

    muse::DeleteAll(m_fragments);
    m_fragments.clear();
}

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

PointF Beam::pagePos() const
{
    System* s = system();
    if (s == 0) {
        return pos();
    }
    double yp = y() + s->staff(staffIdx())->y() + s->y();
    return PointF(pageX(), yp);
}

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

PointF Beam::canvasPos() const
{
    PointF p(pagePos());
    if (system() && system()->explicitParent()) {
        p += system()->parentItem()->pos();
    }
    return p;
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Beam::add(EngravingItem* e)
{
    if (e->isChordRest()) {
        addChordRest(toChordRest(e));
        e->added();
    }
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Beam::remove(EngravingItem* e)
{
    if (e->isChordRest()) {
        removeChordRest(toChordRest(e));
        e->removed();
    }
}

//---------------------------------------------------------
//   addChordRest
//---------------------------------------------------------

void Beam::addChordRest(ChordRest* a)
{
    a->setBeam(this);
    if (!muse::contains(m_elements, a)) {
        //
        // insert element in same order as it appears
        // in the score
        //
        if (a->segment() && !m_elements.empty()) {
            for (size_t i = 0; i < m_elements.size(); ++i) {
                Segment* s = m_elements[i]->segment();
                if ((s->tick() > a->segment()->tick())
                    || ((s->tick() == a->segment()->tick()) && (a->segment()->next(SegmentType::ChordRest) == s))
                    ) {
                    m_elements.insert(m_elements.begin() + i, a);
                    return;
                }
            }
        }
        m_elements.push_back(a);
    }
}

//---------------------------------------------------------
//   removeChordRest
//---------------------------------------------------------

void Beam::removeChordRest(ChordRest* a)
{
    if (!muse::remove(m_elements, a)) {
        LOGD("Beam::remove(): cannot find ChordRest");
    }
    a->setBeam(0);
}

const Chord* Beam::findChordWithCustomStemDirection() const
{
    for (const ChordRest* rest: elements()) {
        if (!rest->isChord()) {
            continue;
        }

        const Chord* chord = toChord(rest);
        if (chord->stemDirection() != DirectionV::AUTO) {
            return chord;
        }
    }

    return nullptr;
}

const BeamSegment* Beam::topLevelSegmentForElement(const ChordRest* element) const
{
    const std::vector<BeamSegment*>& segments = beamSegments();
    size_t segmentsSize = segments.size();

    IF_ASSERT_FAILED(segmentsSize > 0) {
        return nullptr;
    }

    const BeamSegment* curSegment = segments[0];
    if (segmentsSize == 1) {
        return curSegment;
    }

    for (const BeamSegment* segment : segments) {
        if (segment->level <= curSegment->level) {
            continue;
        }
        Fraction elementTick = element->tick();
        if (segment->startTick <= elementTick && segment->endTick >= elementTick) {
            curSegment = segment;
        }
    }

    return curSegment;
}

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void Beam::move(const PointF& offset)
{
    EngravingItem::move(offset);
    for (BeamSegment* bs : m_beamSegments) {
        bs->line.translate(offset);
    }
}

void Beam::calcBeamBreaks(const ChordRest* cr, const ChordRest* prevCr, int level, bool& isBroken16, bool& isBroken32) const
{
    BeamMode beamMode = cr->beamMode();
    if (cr->isRest() && (beamMode == BeamMode::MID || beamMode == BeamMode::BEGIN16 || beamMode == BeamMode::BEGIN32)) {
        // when a rest has beamMode MID we can just ignore it entirely and allow any beams to continue through
        switch (beamMode) {
        case BeamMode::MID:
            isBroken16 = isBroken32 = false;
            break;
        case BeamMode::BEGIN16:
            isBroken16 = level > 0;
            isBroken32 = false;
            break;
        case BeamMode::BEGIN32:
            isBroken16 = false;
            isBroken32 = level > 1;
            break;
        default:
            // should be unreachable
            assert(false);
        }
        return;
    }
    // get default beam mode -- based on time signature preferences
    const Groups& group = cr->staff()->group(cr->measure()->tick());
    BeamMode defaultBeamMode = group.endBeam(cr, prevCr);

    bool isManuallyBroken16 = level >= 1 && beamMode == BeamMode::BEGIN16;
    bool isManuallyBroken32 = level >= 2 && beamMode == BeamMode::BEGIN32;
    bool isDefaultBroken16 = beamMode == BeamMode::AUTO && level >= 1 && defaultBeamMode == BeamMode::BEGIN16;
    bool isDefaultBroken32 = beamMode == BeamMode::AUTO && level >= 2 && defaultBeamMode == BeamMode::BEGIN32;

    isBroken16 = isManuallyBroken16 || isDefaultBroken16;
    isBroken32 = isManuallyBroken32 || isDefaultBroken32;

    // deal with beam-embedded triplets by breaking beams as if they are their underlying durations
    // note that we use max(hooks, 1) here because otherwise we'd end up breaking the main (level 0) beam for
    // tuplets that take up non-beamed amounts of space (eg. 16th note quintuplets)
    if (level > 0 && prevCr && cr->beamMode() == BeamMode::AUTO) {
        if (cr->tuplet() && cr->tuplet() != prevCr->tuplet()) {
            // this cr starts a tuplet
            int beams = std::max(TDuration(cr->tuplet()->ticks()).hooks(), 1);
            if (beams <= level) {
                isBroken16 = level == 1;
                isBroken32 = level >= 2;
            }
        } else if (prevCr->tuplet() && prevCr->tuplet() != cr->tuplet()) {
            // this is a non-tuplet cr that is first after a tuplet
            int beams = std::max(TDuration(prevCr->tuplet()->ticks()).hooks(), 1);
            if (beams <= level) {
                isBroken16 = level == 1;
                isBroken32 = level >= 2;
            }
        }
    }
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Beam::spatiumChanged(double oldValue, double newValue)
{
    int idx = directionIdx();
    if (userModified()) {
        double diff = newValue / oldValue;
        for (BeamFragment* f : m_fragments) {
            f->py1[idx] = f->py1[idx] * diff;
            f->py2[idx] = f->py2[idx] * diff;
        }
    }
}

//---------------------------------------------------------
//   BeamEditData
//---------------------------------------------------------

class BeamEditData : public ElementEditData
{
    OBJECT_ALLOCATOR(engraving, BeamEditData)
public:
    int editFragment;
    virtual EditDataType type() override { return EditDataType::BeamEditData; }
};

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Beam::editDrag(EditData& ed)
{
    int idx = directionIdx();
    double dy = ed.delta.y();
    BeamEditData* bed = static_cast<BeamEditData*>(ed.getData(this).get());
    BeamFragment* f = m_fragments[bed->editFragment];
    double y1 = f->py1[idx];
    double y2 = f->py2[idx];

    if (ed.curGrip == Grip::MIDDLE || noSlope()) {
        y1 += dy;
        y2 += dy;
    } else if (ed.curGrip == Grip::START) {
        y1 += dy;
    } else if (ed.curGrip == Grip::END) {
        y2 += dy;
    }

    double _spatium = spatium();
    // Because of the logic in Beam::setProperty(),
    // changing Pid::BEAM_POS only has an effect if Pid::USER_MODIFIED is true.
    undoChangeProperty(Pid::USER_MODIFIED, true);
    undoChangeProperty(Pid::BEAM_POS, PairF(y1 / _spatium, y2 / _spatium));
    undoChangeProperty(Pid::GENERATED, false);

    triggerLayout();
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> Beam::gripsPositions(const EditData& ed) const
{
    int idx = directionIdx();
    BeamEditData* bed = static_cast<BeamEditData*>(ed.getData(this).get());
    BeamFragment* f = m_fragments[bed->editFragment];

    ChordRest* c1 = nullptr;
    ChordRest* c2 = nullptr;
    size_t n = m_elements.size();

    if (n == 0) {
        return std::vector<PointF>();
    }

    for (size_t i = 0; i < n; ++i) {
        if (m_elements[i]->isChordRest()) {
            c1 = toChordRest(m_elements[i]);
            break;
        }
    }
    if (!c1) { // no chord/rest found, no need to check again below
        return {}; // just ignore the requested operation
    }
    for (int i = static_cast<int>(n) - 1; i >= 0; --i) {
        if (m_elements[i]->isChordRest()) {
            c2 = toChordRest(m_elements[i]);
            break;
        }
    }
    if (!c2) { // no chord/rest found, no need to check again below
        return {}; // just ignore the requested operation
    }

    int y = pagePos().y();
    double beamStartX = m_startAnchor.x() + (system() ? system()->x() : 0);
    double beamEndX = m_endAnchor.x() + (system() ? system()->x() : 0);
    double middleX = (beamStartX + beamEndX) / 2;
    double middleY = (f->py1[idx] + y + f->py2[idx] + y) / 2;

    return {
        PointF(beamStartX, f->py1[idx] + y),
        PointF(beamEndX, f->py2[idx] + y),
        PointF(middleX, middleY)
    };
}

//---------------------------------------------------------
//   setBeamDirection
//---------------------------------------------------------

void Beam::setDirection(DirectionV d)
{
    if (direction() == d || m_cross) {
        return;
    }

    doSetDirection(d);

    if (d != DirectionV::AUTO) {
        setUp(d == DirectionV::UP);
    }

    for (ChordRest* e : elements()) {
        if (e->isChord()) {
            toChord(e)->undoChangeProperty(Pid::STEM_DIRECTION, d);
        }
    }
}

void Beam::setAsFeathered(const bool slower)
{
    if (slower) {
        undoChangeProperty(Pid::GROW_LEFT, 1.0);
        undoChangeProperty(Pid::GROW_RIGHT, 0.0);
    } else {
        undoChangeProperty(Pid::GROW_LEFT, 0.0);
        undoChangeProperty(Pid::GROW_RIGHT, 1.0);
    }
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Beam::reset()
{
    if (!RealIsEqual(growLeft(), 1.0)) {
        undoChangeProperty(Pid::GROW_LEFT, 1.0);
    }
    if (!RealIsEqual(growRight(), 1.0)) {
        undoChangeProperty(Pid::GROW_RIGHT, 1.0);
    }
    if (userModified()) {
        undoChangeProperty(Pid::BEAM_POS, PropertyValue::fromValue(beamPos()));
        undoChangeProperty(Pid::USER_MODIFIED, false);
    }
    undoChangeProperty(Pid::STEM_DIRECTION, DirectionV::AUTO);
    undoResetProperty(Pid::BEAM_NO_SLOPE);
    undoResetProperty(Pid::BEAM_CROSS_STAFF_MOVE);
    undoChangeProperty(Pid::GENERATED, true);
}

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Beam::startEdit(EditData& ed)
{
    initBeamEditData(ed);
}

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Beam::endEdit(EditData& ed)
{
    EngravingItem::endEdit(ed);
}

//---------------------------------------------------------
//   triggerLayout
//---------------------------------------------------------

void Beam::triggerLayout() const
{
    if (!m_elements.empty()) {
        m_elements.front()->triggerLayout();
        m_elements.back()->triggerLayout();
    }
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Beam::acceptDrop(EditData& data) const
{
    EngravingItem* e = data.dropElement;

    if (e->isActionIcon()) {
        ActionIconType type = toActionIcon(e)->actionType();
        return type == ActionIconType::BEAM_FEATHERED_DECELERATE
               || type == ActionIconType::BEAM_FEATHERED_ACCELERATE;
    }

    return false;
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* Beam::drop(EditData& data)
{
    if (!data.dropElement->isActionIcon()) {
        return nullptr;
    }

    ActionIcon* e = toActionIcon(data.dropElement);

    if (e->actionType() == ActionIconType::BEAM_FEATHERED_DECELERATE) {
        setAsFeathered(true /*slower*/);
    } else if (e->actionType() == ActionIconType::BEAM_FEATHERED_ACCELERATE) {
        setAsFeathered(false /*slower*/);
    }

    return nullptr;
}

//---------------------------------------------------------
//   beamPos
//---------------------------------------------------------

PairF Beam::beamPos() const
{
    if (m_fragments.empty()) {
        return PairF(0.0, 0.0);
    }
    BeamFragment* f = m_fragments.back();
    int idx = directionIdx();
    double _spatium = spatium();
    return PairF(f->py1[idx] / _spatium, f->py2[idx] / _spatium);
}

//---------------------------------------------------------
//   setBeamPos
//---------------------------------------------------------

void Beam::setBeamPos(const PairF& bp)
{
    if (m_fragments.empty()) {
        m_fragments.push_back(new BeamFragment);
    }
    BeamFragment* f = m_fragments.back();
    int idx = directionIdx();
    setUserModified(true);
    setGenerated(false);

    double _spatium = spatium();
    if (noSlope()) {
        f->py1[idx] = f->py2[idx] = (bp.first + bp.second) * 0.5 * _spatium;
    } else {
        f->py1[idx] = bp.first * _spatium;
        f->py2[idx] = bp.second * _spatium;
    }
}

void Beam::setNoSlope(bool b)
{
    m_noSlope = b;

    // Make flat if usermodified
    if (m_noSlope) {
        int idx = directionIdx();
        if (userModified()) {
            BeamFragment* f = m_fragments.back();
            f->py1[idx] = f->py2[idx] = (f->py1[idx] + f->py2[idx]) * 0.5;
        }
    }
}

void Beam::computeAndSetSlope()
{
    double xDiff = m_endAnchor.x() - m_startAnchor.x();
    double yDiff = m_endAnchor.y() - m_startAnchor.y();
    if (std::abs(xDiff) < 0.5 * spatium()) {
        // Temporary safeguard: a beam this short is invalid, and exists only as a temporary state,
        // so don't try to compute the slope as it will be wrong. Needs a better solution in future.
        setSlope(0.0);
    } else {
        setSlope(yDiff / xDiff);
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Beam::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::GROW_LEFT:      return growLeft();
    case Pid::GROW_RIGHT:     return growRight();
    case Pid::BEAM_POS:       return PropertyValue::fromValue(beamPos());
    case Pid::BEAM_NO_SLOPE:  return noSlope();
    case Pid::POSITION_LINKED_TO_MASTER:
    case Pid::APPEARANCE_LINKED_TO_MASTER:
        for (ChordRest* chordRest : elements()) {
            bool linked = chordRest->getProperty(propertyId).toBool();
            if (!linked) {
                return false;
            }
        }
        return true;
    default:
        return BeamBase::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Beam::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::GROW_LEFT:
        setGrowLeft(v.toDouble());
        break;
    case Pid::GROW_RIGHT:
        setGrowRight(v.toDouble());
        break;
    case Pid::BEAM_POS:
        if (userModified()) {
            setBeamPos(v.value<PairF>());
        }
        break;
    case Pid::BEAM_NO_SLOPE:
        setNoSlope(v.toBool());
        break;
    case Pid::POSITION_LINKED_TO_MASTER:
    case Pid::APPEARANCE_LINKED_TO_MASTER:
        if (v.toBool() == true) {
            for (ChordRest* chordRest : elements()) {
                // when re-linking, re-link all the chords
                chordRest->setProperty(propertyId, v);
            }
            resetProperty(Pid::STEM_DIRECTION);
            break;
        }
    // fall through
    default:
        if (!BeamBase::setProperty(propertyId, v)) {
            return false;
        }
        break;
    }
    triggerLayout();
    setGenerated(false);
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Beam::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::GROW_LEFT:      return 1.0;
    case Pid::GROW_RIGHT:     return 1.0;
    case Pid::BEAM_POS:       return PropertyValue::fromValue(beamPos());
    default:                  return BeamBase::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   addSkyline
//    add beam shape to skyline
//---------------------------------------------------------

void Beam::addSkyline(Skyline& sk)
{
    if (m_beamSegments.empty() || !addToSkyline()) {
        return;
    }

    // Only add the outer segment, no need to add the inner one
    sk.add(m_beamSegments.front()->shape());
}

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

Fraction Beam::tick() const
{
    return m_elements.empty() ? Fraction(0, 1) : m_elements.front()->segment()->tick();
}

//---------------------------------------------------------
//   rtick
//---------------------------------------------------------

Fraction Beam::rtick() const
{
    return m_elements.empty() ? Fraction(0, 1) : m_elements.front()->segment()->rtick();
}

//---------------------------------------------------------
//   ticks
//    calculate the ticks of all chords and rests connected by the beam
//---------------------------------------------------------

Fraction Beam::ticks() const
{
    Fraction ticks = Fraction(0, 1);
    for (ChordRest* cr : m_elements) {
        ticks += cr->actualTicks();
    }
    return ticks;
}

//---------------------------------------------------------
//   actionIconTypeForBeamMode
//---------------------------------------------------------

ActionIconType Beam::actionIconTypeForBeamMode(BeamMode mode)
{
    switch (mode) {
    case BeamMode::AUTO:
        return ActionIconType::BEAM_AUTO;
    case BeamMode::NONE:
        return ActionIconType::BEAM_NONE;
    case BeamMode::BEGIN:
        return ActionIconType::BEAM_BREAK_LEFT;
    case BeamMode::BEGIN16:
        return ActionIconType::BEAM_BREAK_INNER_8TH;
    case BeamMode::BEGIN32:
        return ActionIconType::BEAM_BREAK_INNER_16TH;
    case BeamMode::MID:
        return ActionIconType::BEAM_JOIN;
    default:
        break;
    }
    return ActionIconType::UNDEFINED;
}

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

RectF Beam::drag(EditData& ed)
{
    int idx = directionIdx();
    double dy = ed.pos.y() - ed.lastPos.y();
    BeamEditData* bed = static_cast<BeamEditData*>(ed.getData(this).get());
    BeamFragment* f = m_fragments[bed->editFragment];

    double y1 = f->py1[idx];
    double y2 = f->py2[idx];

    y1 += dy;
    y2 += dy;

    double _spatium = spatium();
    // Because of the logic in Beam::setProperty(),
    // changing Pid::BEAM_POS only has an effect if Pid::USER_MODIFIED is true.
    undoChangeProperty(Pid::USER_MODIFIED, true);
    undoChangeProperty(Pid::BEAM_POS, PairF(y1 / _spatium, y2 / _spatium));
    undoChangeProperty(Pid::GENERATED, false);

    triggerLayout();

    return canvasBoundingRect();
}

//---------------------------------------------------------
//   isMovable
//---------------------------------------------------------
bool Beam::isMovable() const
{
    return true;
}

//---------------------------------------------------------
//   initBeamEditData
//---------------------------------------------------------
void Beam::initBeamEditData(EditData& ed)
{
    std::shared_ptr<BeamEditData> bed = std::make_shared<BeamEditData>();
    bed->e    = this;
    bed->editFragment = 0;
    ed.addData(bed);

    PointF pt(ed.normalizedStartMove - pagePos());
    double ydiff = 100000000.0;
    int idx = directionIdx();
    int i = 0;
    for (BeamFragment* f : m_fragments) {
        double d = std::fabs(f->py1[idx] - pt.y());
        if (d < ydiff) {
            ydiff = d;
            bed->editFragment = i;
        }
        ++i;
    }
}

//---------------------------------------------------------
//   startDrag
//---------------------------------------------------------
void Beam::startDrag(EditData& editData)
{
    initBeamEditData(editData);
}
}

//---------------------------------------------------------
//   containsChord
//---------------------------------------------------------
bool Beam::hasAllRests()
{
    for (ChordRest* cr : m_elements) {
        if (cr && cr->isChord()) {
            return false;
        }
    }
    return true;
}

void Beam::clearBeamSegments()
{
    for (ChordRest* chordRest : m_elements) {
        chordRest->setBeamlet(nullptr);
    }

    BeamBase::clearBeamSegments();
}

//-------------------------------------------------------
// BEAM SEGMENT CLASS
//-------------------------------------------------------

BeamSegment::BeamSegment(EngravingItem* b)
    : parentElement(b)
{
    DO_ASSERT(parentElement->isType(ElementType::BEAM) || parentElement->isType(ElementType::TREMOLO_TWOCHORD));
}

Shape BeamSegment::shape() const
{
    Shape shape;
    PointF startPoint = line.p1();
    PointF endPoint = line.p2();
    double _beamWidth = parentElement->isBeam()
                        ? item_cast<const Beam*>(parentElement)->m_beamWidth
                        : item_cast<const TremoloTwoChord*>(parentElement)->ldata()->beamWidth;
    // This is the case of right-beamlets
    if (startPoint.x() > endPoint.x()) {
        std::swap(startPoint, endPoint);
    }
    double beamHorizontalLength = endPoint.x() - startPoint.x();
    // If beam is horizontal, one rectangle is enough
    if (muse::RealIsEqual(startPoint.y(), endPoint.y())) {
        RectF rect(startPoint.x(), startPoint.y(), beamHorizontalLength, _beamWidth / 2);
        rect.adjust(0.0, -_beamWidth / 2, 0.0, 0.0);
        shape.add(rect, parentElement);
        return shape;
    }
    // If not, break the beam shape into multiple rectangles
    double beamHeightDiff = endPoint.y() - startPoint.y();
    int subBoxesCount = floor(beamHorizontalLength / parentElement->spatium());
    subBoxesCount = std::max(subBoxesCount, 1); // at least one rectangle, of course (avoid division by zero)
    double horizontalStep = beamHorizontalLength / subBoxesCount;
    double verticalStep = beamHeightDiff / subBoxesCount;
    std::vector<PointF> pointsOnBeamLine;
    pointsOnBeamLine.push_back(startPoint);
    for (int i = 0; i < subBoxesCount - 1; ++i) {
        PointF nextPoint = pointsOnBeamLine.back() + PointF(horizontalStep, verticalStep);
        pointsOnBeamLine.push_back(nextPoint);
    }
    for (PointF point : pointsOnBeamLine) {
        RectF rect(point.x(), point.y(), horizontalStep, _beamWidth / 2);
        rect.adjust(0.0, -_beamWidth / 2, 0.0, 0.0);
        shape.add(rect, parentElement);
    }
    return shape;
}
