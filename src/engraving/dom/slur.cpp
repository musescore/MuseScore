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
#include "slur.h"

#include <cmath>

#include "draw/types/transform.h"

#include "arpeggio.h"
#include "beam.h"
#include "chord.h"
#include "measure.h"
#include "mscoreview.h"
#include "navigate.h"
#include "part.h"
#include "score.h"
#include "stem.h"
#include "system.h"
#include "tie.h"
#include "undo.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::draw;

struct SlurCollision
{
    bool left = false;
    bool mid = false;
    bool right = false;

    void reset()
    {
        left = false;
        mid = false;
        right = false;
    }
};

namespace mu::engraving {
SlurSegment::SlurSegment(System* parent)
    : SlurTieSegment(ElementType::SLUR_SEGMENT, parent)
{
}

SlurSegment::SlurSegment(const SlurSegment& ss)
    : SlurTieSegment(ss)
{
}

//---------------------------------------------------------
//   searchCR
//---------------------------------------------------------

static ChordRest* searchCR(Segment* segment, track_idx_t startTrack, track_idx_t endTrack)
{
    // for (Segment* s = segment; s; s = s->next1MM(SegmentType::ChordRest)) {
    for (Segment* s = segment; s; s = s->next(SegmentType::ChordRest)) {       // restrict search to measure
        if (startTrack > endTrack) {
            for (int t = static_cast<int>(startTrack) - 1; t >= static_cast<int>(endTrack); --t) {
                if (s->element(t)) {
                    return toChordRest(s->element(t));
                }
            }
        } else {
            for (track_idx_t t = startTrack; t < endTrack; ++t) {
                if (s->element(t)) {
                    return toChordRest(s->element(t));
                }
            }
        }
    }
    return 0;
}

bool SlurSegment::isEditAllowed(EditData& ed) const
{
    if (ed.key == Key_Home && !ed.modifiers) {
        return true;
    }

    const bool moveStart = ed.curGrip == Grip::START;
    const bool moveEnd = ed.curGrip == Grip::END || ed.curGrip == Grip::DRAG;

    if (!((ed.modifiers & ShiftModifier) && (isSingleType()
                                             || (isBeginType() && moveStart) || (isEndType() && moveEnd)))) {
        return false;
    }

    static const std::set<int> navigationKeys {
        Key_Left,
        Key_Up,
        Key_Down,
        Key_Right
    };

    return mu::contains(navigationKeys, ed.key);
}

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool SlurSegment::edit(EditData& ed)
{
    if (!isEditAllowed(ed)) {
        return false;
    }

    Slur* sl = slur();

    if (ed.key == Key_Home && !ed.modifiers) {
        if (ed.hasCurrentGrip()) {
            ups(ed.curGrip).off = PointF();
            renderer()->layoutItem(sl);
            triggerLayout();
        }
        return true;
    }

    ChordRest* cr = 0;
    ChordRest* e;
    ChordRest* e1;
    if (ed.curGrip == Grip::START) {
        e  = sl->startCR();
        e1 = sl->endCR();
    } else {
        e  = sl->endCR();
        e1 = sl->startCR();
    }

    if (ed.key == Key_Left) {
        cr = prevChordRest(e);
    } else if (ed.key == Key_Right) {
        cr = nextChordRest(e);
    } else if (ed.key == Key_Up) {
        Part* part     = e->part();
        track_idx_t startTrack = part->startTrack();
        track_idx_t endTrack   = e->track();
        cr = searchCR(e->segment(), endTrack, startTrack);
    } else if (ed.key == Key_Down) {
        track_idx_t startTrack = e->track() + 1;
        Part* part     = e->part();
        track_idx_t endTrack   = part->endTrack();
        cr = searchCR(e->segment(), startTrack, endTrack);
    } else {
        return false;
    }
    if (cr && cr != e1) {
        changeAnchor(ed, cr);
    }
    return true;
}

//---------------------------------------------------------
//   changeAnchor
//---------------------------------------------------------

void SlurSegment::changeAnchor(EditData& ed, EngravingItem* element)
{
    ChordRest* cr = element->isChordRest() ? toChordRest(element) : nullptr;
    ChordRest* scr = spanner()->startCR();
    ChordRest* ecr = spanner()->endCR();
    if (!cr || !scr || !ecr) {
        return;
    }

    // save current start/end elements
    for (EngravingObject* e : spanner()->linkList()) {
        Spanner* sp = toSpanner(e);
        score()->undoStack()->push1(new ChangeStartEndSpanner(sp, sp->startElement(), sp->endElement()));
    }

    if (ed.curGrip == Grip::START) {
        spanner()->undoChangeProperty(Pid::SPANNER_TICK, cr->tick());
        Fraction ticks = ecr->tick() - cr->tick();
        spanner()->undoChangeProperty(Pid::SPANNER_TICKS, ticks);
        int diff = static_cast<int>(cr->track() - spanner()->track());
        for (auto e : spanner()->linkList()) {
            Spanner* s = toSpanner(e);
            s->undoChangeProperty(Pid::TRACK, s->track() + diff);
        }
        scr = cr;
    } else {
        Fraction ticks = cr->tick() - scr->tick();
        spanner()->undoChangeProperty(Pid::SPANNER_TICKS, ticks);
        int diff = static_cast<int>(cr->track() - spanner()->track());
        for (auto e : spanner()->linkList()) {
            Spanner* s = toSpanner(e);
            s->undoChangeProperty(Pid::SPANNER_TRACK2, s->track() + diff);
        }
        ecr = cr;
    }

    // update start/end elements (which could be grace notes)
    for (EngravingObject* lsp : spanner()->linkList()) {
        Spanner* sp = static_cast<Spanner*>(lsp);
        if (sp == spanner()) {
            score()->undo(new ChangeSpannerElements(sp, scr, ecr));
        } else {
            EngravingItem* se = 0;
            EngravingItem* ee = 0;
            if (scr) {
                std::list<EngravingObject*> sel = scr->linkList();
                for (EngravingObject* lcr : sel) {
                    EngravingItem* le = toEngravingItem(lcr);
                    if (le->score() == sp->score() && le->track() == sp->track()) {
                        se = le;
                        break;
                    }
                }
            }
            if (ecr) {
                std::list<EngravingObject*> sel = ecr->linkList();
                for (EngravingObject* lcr : sel) {
                    EngravingItem* le = toEngravingItem(lcr);
                    if (le->score() == sp->score() && le->track() == sp->track2()) {
                        ee = le;
                        break;
                    }
                }
            }
            score()->undo(new ChangeStartEndSpanner(sp, se, ee));
            renderer()->layoutItem(sp);
        }
    }

    const size_t segments  = spanner()->spannerSegments().size();
    ups(ed.curGrip).off = PointF();
    renderer()->layoutItem(spanner());
    if (spanner()->spannerSegments().size() != segments) {
        const std::vector<SpannerSegment*>& ss = spanner()->spannerSegments();
        const bool moveEnd = ed.curGrip == Grip::END || ed.curGrip == Grip::DRAG;
        SlurSegment* newSegment = toSlurSegment(moveEnd ? ss.back() : ss.front());
        ed.view()->changeEditElement(newSegment);
        triggerLayout();
    }
}

void SlurSegment::editDrag(EditData& ed)
{
    SlurTieSegment::editDrag(ed);
    System* startSys = slur()->startCR()->measure()->system();
    System* endSys = slur()->endCR()->measure()->system();
    if (startSys && endSys && startSys == endSys) {
        renderer()->layoutItem(slur());
    }
}

//---------------------------------------------------------
//   adjustEndpoints
//    move endpoints so as not to collide with staff lines
//---------------------------------------------------------
void SlurSegment::adjustEndpoints()
{
    double lw = (staffType() ? staffType()->lineDistance().val() : 1.0) * spatium();
    const double staffLineMargin = 0.175 + (0.5 * style().styleS(Sid::staffLineWidth).val() * (spatium() / lw));
    PointF p1 = ups(Grip::START).p;
    PointF p2 = ups(Grip::END).p;

    double y1sp = p1.y() / lw;
    double y2sp = p2.y() / lw;

    // point 1
    int lines = staff()->lines(tick());
    auto adjustPoint = [staffLineMargin](bool up, double ysp) {
        double y1offset = ysp - floor(ysp);
        double adjust = 0;
        if (up) {
            if (y1offset < staffLineMargin) {
                // endpoint too close to the line above
                adjust = -(y1offset + staffLineMargin);
            } else if (y1offset > 1 - staffLineMargin) {
                // endpoint too close to the line below
                adjust = -(y1offset - (1 - staffLineMargin));
            }
        } else {
            if (y1offset < staffLineMargin) {
                // endpoint too close to the line above
                adjust = staffLineMargin - y1offset;
            }
            if (y1offset > 1 - staffLineMargin) {
                // endpoint too close to the line below
                adjust = (1 - y1offset) + staffLineMargin;
            }
        }
        return adjust;
    };
    if (y1sp > -staffLineMargin && y1sp < (lines - 1) + staffLineMargin) {
        ups(Grip::START).p.ry() += adjustPoint(slur()->up(), y1sp) * lw;
    }
    if (y2sp > -staffLineMargin && y2sp < (lines - 1) + staffLineMargin) {
        ups(Grip::END).p.ry() += adjustPoint(slur()->up(), y2sp) * lw;
    }
}

Shape SlurSegment::getSegmentShape(Segment* seg, ChordRest* startCR, ChordRest* endCR)
{
    staff_idx_t startStaffIdx = startCR->staffIdx();
    staff_idx_t endStaffIdx = endCR->staffIdx();
    Shape segShape = seg->staffShape(startStaffIdx).translated(seg->pos() + seg->measure()->pos());

    // If cross-staff, also add the shape of second staff
    if (slur()->isCrossStaff() && seg != startCR->segment()) {
        endStaffIdx = (endCR->staffIdx() != startStaffIdx) ? endCR->staffIdx() : endCR->vStaffIdx();
        SysStaff* startStaff = system()->staves().at(startStaffIdx);
        SysStaff* endStaff = system()->staves().at(endStaffIdx);
        double dist = endStaff->y() - startStaff->y();
        Shape secondStaffShape = seg->staffShape(endStaffIdx).translated(seg->pos() + seg->measure()->pos()); // translate horizontally
        secondStaffShape.translate(PointF(0.0, dist)); // translate vertically
        segShape.add(secondStaffShape);
    }

    for (track_idx_t track = staff2track(startStaffIdx); track < staff2track(endStaffIdx, VOICES); ++track) {
        EngravingItem* e = seg->elementAt(track);
        if (!e || !e->isChordRest()) {
            continue;
        }
        // Gets tie and 2 note tremolo shapes
        if (e->isChord()) {
            Chord* chord = toChord(e);
            if (chord->tremolo() && chord->tremolo()->twoNotes()) {
                segShape.add(chord->tremolo()->shape());
            }
            for (Note* note : toChord(e)->notes()) {
                Tie* tieFor = note->tieFor();
                Tie* tieBack = note->tieBack();
                if (tieFor && tieFor->up() == slur()->up() && !tieFor->segmentsEmpty()) {
                    TieSegment* tieSegment = tieFor->frontSegment();
                    if (tieSegment->isSingleBeginType()) {
                        segShape.add(tieSegment->shape());
                    }
                }
                if (tieBack && tieBack->up() == slur()->up() && !tieBack->segmentsEmpty()) {
                    TieSegment* tieSegment = tieBack->backSegment();
                    if (tieSegment->isEndType()) {
                        segShape.add(tieSegment->shape());
                    }
                }
            }
        }
    }

    // Remove items that the slur shouldn't try to avoid
    segShape.remove_if([&](ShapeElement& shapeEl) {
        if (!shapeEl.item() || !shapeEl.item()->parentItem()) {
            return true;
        }
        const EngravingItem* item = shapeEl.item();
        const EngravingItem* parent = item->parentItem();
        // Don't remove arpeggio starting on a different voice and ending on the same voice as endCR when slur is on the outside
        if (item->isArpeggio() && (endCR->track() == toArpeggio(item)->endTrack()) && endCR->tick() == item->tick()
            && (!slur()->up() && toArpeggio(item)->span() > 1)) {
            return false;
        }

        // Its own startCR or items belonging to it, lyrics, fingering, ledger lines, articulation on endCR
        if (item == startCR || parent == startCR || item->isTextBase() || item->isLedgerLine()
            || (item->isArticulationFamily() && parent == endCR) || item->isBend() || item->isStretchedBend()) {
            return true;
        }
        // Items that are on the start segment but in a different voice
        if ((item->tick() == startCR->tick() && item->track() != startCR->track())
            || (item->tick() == endCR->tick() && item->track() != endCR->track())) {
            return true;
        }
        // Edge-case: multiple voices and slur is on the inside
        if (item->vStaffIdx() == startCR->staffIdx()
            && ((!slur()->up() && item->track() > startCR->track()) // slur-down: ignore lower voices
                || (slur()->up() && item->track() < startCR->track()))) { // slur-up: ignore higher voices
            return true;
        }
        // Remove arpeggios spanning more than 1 voice starting on endCR's voice when the slur is on the inside
        if (item->isArpeggio() && (endCR->track() != item->track() || (!slur()->up() && toArpeggio(item)->span() > 1))) {
            return true;
        }
        return false;
    });

    return segShape;
}

void SlurSegment::avoidCollisions(PointF& pp1, PointF& p2, PointF& p3, PointF& p4, Transform& toSystemCoordinates, double& slurAngle)
{
    TRACEFUNC;
    ChordRest* startCR = slur()->startCR();
    ChordRest* endCR = slur()->endCR();

    if (!startCR || !endCR) {
        return;
    }

    // Determine start and end segments for collision checks
    Segment* startSeg = nullptr;
    if (isSingleBeginType()) {
        if (startCR->isChord() && toChord(startCR)->isGraceAfter()) {
            // if this is a grace-note-after, the shape is stored the *appended* segment
            Chord* parent = toChord(startCR->parentItem());
            if (parent) {
                startSeg = parent->graceNotesAfter().appendedSegment();
            }
        } else {
            startSeg = startCR->segment(); // first of the slur
        }
    } else {
        startSeg = system()->firstMeasure()->findFirstR(SegmentType::ChordRest, Fraction(0, 0)); // first of the system
    }
    Segment* endSeg = nullptr;
    if (isSingleEndType()) {
        if (endCR->isChord() && toChord(endCR)->isGraceAfter()) {
            // if this is a grace-note-after, the shape is stored the *appended* segment
            Chord* parent = toChord(endCR->parentItem());
            if (parent) {
                endSeg = parent->graceNotesAfter().appendedSegment();
            }
        } else {
            endSeg = endCR->segment(); // last of the slur
        }
    } else {
        endSeg = system()->lastMeasure()->last(); // last of the system
    }
    if (!startSeg || !endSeg) {
        return;
    }

    // Collect all the segments shapes spanned by this slur segment in a single vector
    std::vector<Shape> segShapes;
    for (Segment* seg = startSeg; seg && seg->tick() <= endSeg->tick(); seg = seg->next1enabled()) {
        if (seg->isType(SegmentType::BarLineType) || seg->isBreathType()) {
            continue;
        }
        segShapes.push_back(getSegmentShape(seg, startCR, endCR));
    }
    if (segShapes.empty()) {
        return;
    }

    // Collision clearance at the center of the slur
    double slurLength = abs(p2.x() / spatium());
    double clearance;
    if (slurLength < 4) {
        clearance = 0.15 * spatium();
    } else if (slurLength < 8) {
        clearance = 0.4 * spatium();
    } else if (slurLength < 12) {
        clearance = 0.6 * spatium();
    } else {
        clearance = 0.75 * spatium();
    }
    // balance: determines how much endpoint adjustment VS shape adjustment we will do.
    // 0 = end point is fixed, only the shape can be adjusted,
    // 1 = shape is fixed, only end the point can be adjusted.
    // left and right side of the slur may have different balance depending on context:
    double leftBalance, rightBalance;
    if (isSingleBeginType() && !slur()->stemFloated().left) {
        if (startCR->isChord() && toChord(startCR)->stem() && startCR->up() == slur()->up()) {
            leftBalance = 0.1;
        } else {
            leftBalance = 0.4;
        }
    } else {
        leftBalance = 0.9;
    }
    if (isSingleEndType() && !slur()->stemFloated().right) {
        if (endCR->isChord() && toChord(endCR)->stem() && endCR->up() == slur()->up()) {
            rightBalance = 0.1;
        } else {
            rightBalance = 0.4;
        }
    } else {
        rightBalance = 0.9;
    }

    static constexpr unsigned maxIter = 30;     // Max iterations allowed
    const double vertClearance = slur()->up() ? clearance : -clearance;
    // Optimize the slur shape and position in quarter-space steps
    double step = slur()->up() ? -0.25 * spatium() : 0.25 * spatium();
    // ...but allow long slurs to user coarser steps
    static constexpr double longSlurLimit = 16.0; // in spaces
    if (slurLength > longSlurLimit) {
        step *= slurLength / longSlurLimit;
        step = std::min(step, 1.5 * spatium());
    }
    // Divide slur in several rectangles to localize collisions
    const unsigned npoints = 20;
    std::vector<RectF> slurRects;
    slurRects.reserve(npoints);
    // Define separate collision areas (left-mid-center)
    SlurCollision collision;

    // CHECK FOR COLLISIONS
    unsigned iter = 0;
    do {
        collision.reset();
        // Update tranform because pp1 may change
        toSystemCoordinates.reset();
        toSystemCoordinates.translate(pp1.x(), pp1.y());
        toSystemCoordinates.rotateRadians(slurAngle);
        // Create rectangles
        slurRects.clear();
        CubicBezier clearanceBezier(PointF(0, 0), p3 + PointF(0.0, vertClearance), p4 + PointF(0.0, vertClearance), p2);
        for (unsigned i = 0; i < npoints - 1; i++) {
            PointF clearancePoint1 = clearanceBezier.pointAtPercent(double(i) / double(npoints));
            PointF clearancePoint2 = clearanceBezier.pointAtPercent(double(i + 1) / double(npoints));
            clearancePoint1 = toSystemCoordinates.map(clearancePoint1);
            clearancePoint2 = toSystemCoordinates.map(clearancePoint2);
            slurRects.push_back(RectF(clearancePoint1, clearancePoint2));
        }
        // Check collisions
        for (Shape& segShape : segShapes) {
            for (unsigned i=0; i < slurRects.size(); i++) {
                bool leftSection = i < slurRects.size() / 3;
                bool midSection = i >= slurRects.size() / 3 && i < 2 * slurRects.size() / 3;
                bool rightSection = i >= 2 * slurRects.size() / 3;
                if ((leftSection && collision.left)
                    || (midSection && collision.mid)
                    || (rightSection && collision.right)) {     // If a collision is already found in this section, no need to check again
                    continue;
                }
                bool intersection = slur()->up() ? !Shape(slurRects[i]).clearsVertically(segShape)
                                    : !segShape.clearsVertically(slurRects[i]);
                if (intersection) {
                    if (leftSection) {
                        collision.left = true;
                    }
                    if (midSection) {
                        collision.mid = true;
                    }
                    if (rightSection) {
                        collision.right = true;
                    }
                }
            }
        }
        // In the even iterations, adjust the shape
        if (iter % 2 == 0) {
            double shapeLeftStep = (1 - leftBalance) * step;
            double shapeRightStep = (1 - rightBalance) * step;
            if (collision.left) {
                // Move left Bezier point up(/down) and outwards
                p3 += PointF(-abs(shapeLeftStep), shapeLeftStep);
                // and a bit also the right point to compensate asymmetry
                p4 += PointF(abs(shapeLeftStep), shapeLeftStep) / 2.0;
            }
            if (collision.mid) {     // Move both Bezier points up(/down)
                p3 += PointF(0.0, (shapeLeftStep + shapeRightStep) / 2);
                p4 += PointF(0.0, (shapeLeftStep + shapeRightStep) / 2);
            }
            if (collision.right) {
                // Move right Bezier point up(/down) and outwards
                p4 += PointF(abs(shapeRightStep), shapeRightStep);
                // and a bit also the left point to compensate asymmetry
                p3 += PointF(-abs(shapeRightStep), shapeRightStep) / 2.0;
            }
        } else if (!isEndPointsEdited()) {
            // In the odd iterations, adjust the end points
            // Slurs steeper than 45° are gently compensated
            static constexpr double steepLimit = M_PI / 4;
            if (collision.left || collision.mid || slurAngle < -steepLimit) {
                double endPointLeftStep = leftBalance * step;
                // Lift the left end point, i.e. tilt the slur around p2
                double stepX = sin(slurAngle) * endPointLeftStep;
                double stepY = cos(slurAngle) * endPointLeftStep;
                PointF pp1delta = PointF(stepX, stepY);
                pp1 += PointF(0.0, endPointLeftStep);
                p3 += pp1delta * (p2.x() - p3.x()) / p2.x();
                p4 += pp1delta * (p2.x() - p4.x()) / p2.x();
                // All points are expressed with respect to pp1, so we need
                // to subtract pp1delta to avoid the whole slur moving up
                p2 -= pp1delta;
                p3 -= pp1delta;
                p4 -= pp1delta;
            }
            if (collision.right || collision.mid || slurAngle > steepLimit) {
                double endPointRightStep = rightBalance * step;
                // Lift the right end point, i.e. tilt the slur around p1
                double stepX = sin(slurAngle) * endPointRightStep;
                double stepY = cos(slurAngle) * endPointRightStep;
                PointF p2delta = PointF(stepX, stepY);
                p2 += p2delta;
                p3 += p2delta * p3.x() / p2.x();
                p4 += p2delta * p4.x() / p2.x();
            }
        }
        // Enforce non-ugliness rules
        // 1) Slur cannot be taller than it is wide
        const double maxRelativeHeight = abs(p2.x());
        p3 = slur()->up() ? PointF(p3.x(), std::max(p3.y(), -maxRelativeHeight)) : PointF(p3.x(), std::min(p3.y(), maxRelativeHeight));
        p4 = slur()->up() ? PointF(p4.x(), std::max(p4.y(), -maxRelativeHeight)) : PointF(p4.x(), std::min(p4.y(), maxRelativeHeight));
        // 2) Tangent rule: p3 and p4 cannot be further left than p1 nor further right than p2
        PointF p3SysCoord = toSystemCoordinates.map(p3);
        PointF p4SysCoord = toSystemCoordinates.map(p4);
        PointF p2SysCoord = toSystemCoordinates.map(p2);
        p3SysCoord = PointF(std::max(pp1.x(), p3SysCoord.x()), p3SysCoord.y());
        p3SysCoord = PointF(std::min(p2SysCoord.x(), p3SysCoord.x()), p3SysCoord.y());
        p4SysCoord = PointF(std::max(pp1.x(), p4SysCoord.x()), p4SysCoord.y());
        p4SysCoord = PointF(std::min(p2SysCoord.x(), p4SysCoord.x()), p4SysCoord.y());
        p3 = toSystemCoordinates.inverted().map(p3SysCoord);
        p4 = toSystemCoordinates.inverted().map(p4SysCoord);

        ++iter;
    } while ((collision.left || collision.mid || collision.right) && iter < maxIter);
}

//---------------------------------------------------------
//   computeBezier
//    compute the shape of the slur segment, optimize it
//    for avoiding collisions, and set grip points
//---------------------------------------------------------

void SlurSegment::computeBezier(mu::PointF p6offset)
{
    /* ************************************************
     * LEGEND: pp1 = start point
     *         pp2 = end point
     *         p2 = end point (in slur coordinates)
     *         p3 = first bezier point (in slur coord.)
     *         p4 = second bezier point (in slur coord.)
     *         p5 = whole slur drag point (in slur coord.)
     *         p6 = shoulder drag point (in slur coord.)
     * REMEMBER! ups().pos() = ups().p + ups().off
     * ***********************************************/
    // Avoid bad staff line intersections
    if (autoplace()) {
        adjustEndpoints();
    }
    // If end point adjustment is locked, restore the endpoints to
    // where they were before
    if (isEndPointsEdited()) {
        ups(Grip::START).off += m_endPointOff1;
        ups(Grip::END).off += m_endPointOff2;
    }
    // Get start and end points (have been calculated before)
    PointF pp1 = ups(Grip::START).p + ups(Grip::START).off;
    PointF pp2 = ups(Grip::END).p + ups(Grip::END).off;
    // Keep track of the original value before it gets changed
    PointF oldp1 = pp1;
    PointF oldp2 = pp2;
    // Check slur integrity
    if (pp2 == pp1) {
        Measure* m1 = slur()->startCR()->segment()->measure();
        Measure* m2 = slur()->endCR()->segment()->measure();
        LOGD("zero slur at tick %d(%d) track %zu in measure %d-%d  tick %d ticks %d",
             m1->tick().ticks(), tick().ticks(), track(), m1->no(), m2->no(), slur()->tick().ticks(), slur()->ticks().ticks());
        slur()->setBroken(true);
        return;
    }

    // Set up coordinate transforms
    // CAUTION: transform operations are applies in reverse order to how
    // they are added to the transformation.
    double slurAngle = atan((pp2.y() - pp1.y()) / (pp2.x() - pp1.x()));
    Transform rotate;
    rotate.rotateRadians(-slurAngle);
    Transform toSlurCoordinates;
    toSlurCoordinates.rotateRadians(-slurAngle);
    toSlurCoordinates.translate(-pp1.x(), -pp1.y());
    Transform toSystemCoordinates = toSlurCoordinates.inverted();
    // Transform p2 and shoulder offset
    PointF p2 = toSlurCoordinates.map(pp2);
    p6offset = rotate.map(p6offset);

    // COMPUTE DEFAULT SLUR SHAPE
    // Compute default shoulder height and width
    double _spatium  = spatium();
    double shoulderW; // expressed as fraction of slur-length
    double shoulderH;
    double d = p2.x() / _spatium;

    if (d < 0) {
        //! NOTE A negative d means that end point is before the start point.
        //! This only exists as a temporary state when horizontal spacing hasn't yet been computed,
        //! and it makes no sense for any of the following calculations
        return;
    }

    if (d < 2) {
        shoulderW = 0.60;
    } else if (d < 10) {
        shoulderW = 0.5;
    } else if (d < 18) {
        shoulderW = 0.6;
    } else {
        shoulderW = 0.7;
    }
    shoulderH = sqrt(d / 4) * _spatium;

    static constexpr double shoulderReduction = 0.75;
    if (slur()->isOverBeams()) {
        shoulderH *= shoulderReduction;
    }
    shoulderH -= p6offset.y();
    if (!slur()->up()) {
        shoulderH = -shoulderH;
    }

    double c    = p2.x();
    double c1   = (c - c * shoulderW) * .5 + p6offset.x();
    double c2   = c1 + c * shoulderW + p6offset.x();
    PointF p3(c1, -shoulderH);
    PointF p4(c2, -shoulderH);
    // Set Bezier points default position
    ups(Grip::BEZIER1).p  = toSystemCoordinates.map(p3);
    ups(Grip::BEZIER2).p  = toSystemCoordinates.map(p4);
    // Add offsets
    p3 += p6offset + rotate.map(ups(Grip::BEZIER1).off);
    p4 += p6offset + rotate.map(ups(Grip::BEZIER2).off);
    ups(Grip::BEZIER1).off += rotate.inverted().map(p6offset);
    ups(Grip::BEZIER2).off += rotate.inverted().map(p6offset);

    // ADAPT SLUR SHAPE AND ENDPOINT POSITION
    // to clear collisions with underlying items
    if (autoplace()) {
        avoidCollisions(pp1, p2, p3, p4, toSystemCoordinates, slurAngle);
    }

    // Re-check end points for bad staff line collisions
    ups(Grip::START).p = pp1 - ups(Grip::START).off;
    ups(Grip::END).p = toSystemCoordinates.map(p2) - ups(Grip::END).off;
    adjustEndpoints();
    PointF newpp1 = ups(Grip::START).p + ups(Grip::START).off;
    PointF difference = rotate.map(newpp1 - pp1);
    pp1 = newpp1;
    pp2 = ups(Grip::END).p + ups(Grip::END).off;
    p3 -= difference;
    p4 -= difference;
    // Keep track of how much the end points position has changed
    if (!isEndPointsEdited()) {
        m_endPointOff1 = pp1 - oldp1;
        m_endPointOff2 = pp2 - oldp2;
    } else {
        m_endPointOff1 = PointF(0.0, 0.0);
        m_endPointOff2 = PointF(0.0, 0.0);
    }
    // Recompute the transformation because pp1 and pp1 may have changed
    toSlurCoordinates.reset();
    toSlurCoordinates.rotateRadians(-slurAngle);
    toSlurCoordinates.translate(-pp1.x(), -pp1.y());
    toSystemCoordinates = toSlurCoordinates.inverted();
    p2 = toSlurCoordinates.map(pp2);

    // Calculate p5 and p6
    PointF p5 = 0.5 * p2; // mid-point between pp1 and p2
    PointF p6 = 0.5 * (p3 + p4); // mid-point between p3 and p4

    // Update all slur points after collision avoidance
    ups(Grip::BEZIER1).p  = toSystemCoordinates.map(p3) - ups(Grip::BEZIER1).off;
    ups(Grip::BEZIER2).p  = toSystemCoordinates.map(p4) - ups(Grip::BEZIER2).off;
    ups(Grip::DRAG).p     = toSystemCoordinates.map(p5);
    ups(Grip::SHOULDER).p = toSystemCoordinates.map(p6);

    // Set slur thickness
    double w = style().styleMM(Sid::SlurMidWidth) - style().styleMM(Sid::SlurEndWidth);
    if (staff()) {
        w *= staff()->staffMag(slur()->tick());
    }
    if ((c2 - c1) <= _spatium) {
        w *= .5;
    }
    PointF thick(0.0, w);

    // Set path
    m_path = PainterPath();
    m_path.moveTo(PointF());
    m_path.cubicTo(p3 - thick, p4 - thick, p2);
    if (slur()->styleType() == SlurStyleType::Solid) {
        m_path.cubicTo(p4 + thick, p3 + thick, PointF());
    }
    thick = PointF(0.0, 3.0 * w);
    m_shapePath = PainterPath();
    m_shapePath.moveTo(PointF());
    m_shapePath.cubicTo(p3 - thick, p4 - thick, p2);
    m_shapePath.cubicTo(p4 + thick, p3 + thick, PointF());

    m_path = toSystemCoordinates.map(m_path);
    m_shapePath = toSystemCoordinates.map(m_shapePath);

    // Create shape for the skyline
    Shape shape;
    PointF start = pp1;
    int nbShapes  = 32;
    double minH    = abs(2 * w);
    const CubicBezier b(ups(Grip::START).pos(), ups(Grip::BEZIER1).pos(), ups(Grip::BEZIER2).pos(), ups(Grip::END).pos());
    for (int i = 1; i <= nbShapes; i++) {
        const PointF point = b.pointAtPercent(i / float(nbShapes));
        RectF re     = RectF(start, point).normalized();
        if (re.height() < minH) {
            double d1 = (minH - re.height()) * .5;
            re.adjust(0.0, -d1, 0.0, d1);
        }
        shape.add(re, this);
        start = point;
    }

    mutldata()->setShape(shape);
}

//---------------------------------------------------------
//   isEdited
//---------------------------------------------------------

bool SlurSegment::isEdited() const
{
    for (int i = 0; i < int(Grip::GRIPS); ++i) {
        if (!m_ups[i].off.isNull()) {
            return true;
        }
    }
    return false;
}

bool SlurSegment::isEndPointsEdited() const
{
    return !(m_ups[int(Grip::START)].off.isNull() && m_ups[int(Grip::END)].off.isNull());
}

Slur::Slur(const Slur& s)
    : SlurTie(s)
{
    m_sourceStemArrangement = s.m_sourceStemArrangement;
}

//---------------------------------------------------------
//   slurPos
//    Calculate position of start- and endpoint of slur
//    relative to System() position.
//---------------------------------------------------------

void Slur::slurPosChord(SlurTiePos* sp)
{
    Chord* stChord;
    Chord* enChord;
    if (startChord()->isGraceAfter()) {      // grace notes after, coming in reverse order
        stChord = endChord();
        enChord = startChord();
        m_up = false;
    } else {
        stChord = startChord();
        enChord = endChord();
    }
    Note* _startNote = stChord->downNote();
    Note* _endNote   = enChord->downNote();
    double hw         = _startNote->bboxRightPos();
    double __up       = m_up ? -1.0 : 1.0;
    double _spatium = spatium();

    Measure* measure = endChord()->measure();
    sp->system1 = measure->system();
    if (!sp->system1) {               // DEBUG
        LOGD("no system1");
        return;
    }
    assert(sp->system1);
    sp->system2 = sp->system1;
    PointF pp(sp->system1->pagePos());

    double xo;
    double yo;

    //------p1
    if (m_up) {
        xo = _startNote->x() + hw * 1.12;
        yo = _startNote->pos().y() + hw * .3 * __up;
    } else {
        xo = _startNote->x() + hw * 0.4;
        yo = _startNote->pos().y() + _spatium * .75 * __up;
    }
    sp->p1 = stChord->pagePos() - pp + PointF(xo, yo);

    //------p2
    if ((enChord->notes().size() > 1) || (enChord->stem() && !enChord->up() && !m_up)) {
        xo = _endNote->x() - hw * 0.12;
        yo = _endNote->pos().y() + hw * .3 * __up;
    } else {
        xo = _endNote->x() + hw * 0.15;
        yo = _endNote->pos().y() + _spatium * .75 * __up;
    }
    sp->p2 = enChord->pagePos() - pp + PointF(xo, yo);
}

//---------------------------------------------------------
//   Slur
//---------------------------------------------------------

Slur::Slur(EngravingItem* parent)
    : SlurTie(ElementType::SLUR, parent)
{
    setAnchor(Anchor::CHORD);
}

//---------------------------------------------------------
//   calcStemArrangement
//---------------------------------------------------------

int Slur::calcStemArrangement(EngravingItem* start, EngravingItem* end)
{
    return (start && start->isChord() && toChord(start)->stem() && toChord(start)->stem()->up() ? 2 : 0)
           + (end && end->isChord() && toChord(end)->stem() && toChord(end)->stem()->up() ? 4 : 0);
}

//---------------------------------------------------------
//   directionMixture
//---------------------------------------------------------

bool Slur::isDirectionMixture(Chord* c1, Chord* c2)
{
    UNUSED(c1);
    UNUSED(c2);
    UNREACHABLE;
    return false;
}

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Slur::setTrack(track_idx_t n)
{
    EngravingItem::setTrack(n);
    for (SpannerSegment* ss : spannerSegments()) {
        ss->setTrack(n);
    }
}

bool Slur::isCrossStaff()
{
    return startCR() && endCR()
           && (startCR()->staffMove() != 0 || endCR()->staffMove() != 0
               || startCR()->vStaffIdx() != endCR()->vStaffIdx());
}

//---------------------------------------------------------
//   stemSideForBeam
//    determines if the anchor point is exempted from the stem inset
//    due to beams or tremolos.
//---------------------------------------------------------

bool Slur::stemSideForBeam(bool start)
{
    ChordRest* cr = start ? startCR() : endCR();
    Chord* c = toChord(cr);
    bool adjustForBeam = cr && cr->beam() && cr->up() == up();
    if (start) {
        adjustForBeam = adjustForBeam && cr->beam()->elements().back() != cr;
    } else {
        adjustForBeam = adjustForBeam && cr->beam()->elements().front() != cr;
    }
    if (adjustForBeam) {
        return true;
    }

    bool adjustForTrem = false;
    Tremolo* trem = c ? c->tremolo() : nullptr;
    adjustForTrem = trem && trem->twoNotes() && trem->up() == up();
    if (start) {
        adjustForTrem = adjustForTrem && trem->chord2() != c;
    } else {
        adjustForTrem = adjustForTrem && trem->chord1() != c;
    }
    return adjustForTrem;
}

//---------------------------------------------------------
//   isOverBeams
//    returns true if all the chords spanned by the slur are
//    beamed, and all beams are on the same side of the slur
//---------------------------------------------------------
bool Slur::isOverBeams()
{
    if (!startCR() || !endCR()) {
        return false;
    }
    if (startCR()->track() != endCR()->track()
        || startCR()->tick() >= endCR()->tick()) {
        return false;
    }
    size_t track = startCR()->track();
    Segment* seg = startCR()->segment();
    while (seg && seg->tick() <= endCR()->tick()) {
        if (!seg->isChordRestType()
            || !seg->elist().at(track)
            || !seg->elist().at(track)->isChordRest()) {
            return false;
        }
        ChordRest* cr = toChordRest(seg->elist().at(track));
        bool hasBeam = cr->beam() && cr->up() == up();
        bool hasTrem = false;
        if (cr->isChord()) {
            Chord* c = toChord(cr);
            hasTrem = c->tremolo() && c->tremolo()->twoNotes() && c->up() == up();
        }
        if (!(hasBeam || hasTrem)) {
            return false;
        }
        if ((!seg->next() || seg->next()->isEndBarLineType()) && seg->measure()->nextMeasure()) {
            seg = seg->measure()->nextMeasure()->first();
        } else {
            seg = seg->next();
        }
    }
    return true;
}
}
