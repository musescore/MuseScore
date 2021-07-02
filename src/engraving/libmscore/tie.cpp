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

#include <cmath>

#include "measure.h"
#include "score.h"
#include "system.h"
#include "undo.h"
#include "chord.h"
#include "tie.h"

#include "draw/transform.h"
#include "draw/pen.h"
#include "draw/brush.h"

using namespace mu;

namespace Ms {
Note* Tie::editStartNote;
Note* Tie::editEndNote;

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TieSegment::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    using namespace mu::draw;
    // hide tie toward the second chord of a cross-measure value
    if (tie()->endNote() && tie()->endNote()->chord()->crossMeasure() == CrossMeasure::SECOND) {
        return;
    }

    Pen pen(curColor());
    qreal mag = staff() ? staff()->staffMag(tie()->tick()) : 1.0;

    //Replace generic Qt dash patterns with improved equivalents to show true dots (keep in sync with slur.cpp)
    std::vector<double> dotted     = { 0.01, 1.99 };   // tighter than Qt PenStyle::DotLine equivalent - woud be { 0.01, 2.99 }
    std::vector<double> dashed     = { 3.00, 3.00 };   // Compensating for caps. Qt default PenStyle::DashLine is { 4.0, 2.0 }
    std::vector<double> wideDashed = { 5.00, 6.00 };

    switch (slurTie()->lineType()) {
    case 0:
        painter->setBrush(Brush(pen.color()));
        pen.setCapStyle(PenCapStyle::RoundCap);
        pen.setJoinStyle(PenJoinStyle::RoundJoin);
        pen.setWidthF(score()->styleP(Sid::SlurEndWidth) * mag);
        break;
    case 1:
        painter->setBrush(BrushStyle::NoBrush);
        pen.setCapStyle(PenCapStyle::RoundCap);           // True dots
        pen.setDashPattern(dotted);
        pen.setWidthF(score()->styleP(Sid::SlurDottedWidth) * mag);
        break;
    case 2:
        painter->setBrush(BrushStyle::NoBrush);
        pen.setDashPattern(dashed);
        pen.setWidthF(score()->styleP(Sid::SlurDottedWidth) * mag);
        break;
    case 3:
        painter->setBrush(BrushStyle::NoBrush);
        pen.setDashPattern(wideDashed);
        pen.setWidthF(score()->styleP(Sid::SlurDottedWidth) * mag);
        break;
    }
    painter->setPen(pen);
    painter->drawPath(path);
}

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool TieSegment::edit(EditData& ed)
{
    SlurTie* sl = tie();

    if (ed.key == Qt::Key_X) {
        sl->setSlurDirection(sl->up() ? Direction::DOWN : Direction::UP);
        sl->layout();
        return true;
    }
    if (ed.key == Qt::Key_Home) {
        ups(ed.curGrip).off = PointF();
        sl->layout();
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   changeAnchor
//---------------------------------------------------------

void TieSegment::changeAnchor(EditData& ed, Element* element)
{
    if (ed.curGrip == Grip::START) {
        spanner()->setStartElement(element);
        Note* note = toNote(element);
        if (note->chord()->tick() <= tie()->endNote()->chord()->tick()) {
            tie()->startNote()->setTieFor(0);
            tie()->setStartNote(note);
            note->setTieFor(tie());
        }
    } else {
        spanner()->setEndElement(element);
        Note* note = toNote(element);
        // do not allow backward ties
        if (note->chord()->tick() >= tie()->startNote()->chord()->tick()) {
            tie()->endNote()->setTieBack(0);
            tie()->setEndNote(note);
            note->setTieBack(tie());
        }
    }

    const size_t segments  = spanner()->spannerSegments().size();
    ups(ed.curGrip).off = PointF();
    spanner()->layout();
    if (spanner()->spannerSegments().size() != segments) {
        const std::vector<SpannerSegment*>& ss = spanner()->spannerSegments();

        TieSegment* newSegment = toTieSegment(ed.curGrip == Grip::END ? ss.back() : ss.front());
        score()->endCmd();
        score()->startCmd();
        ed.view()->startEdit(newSegment, ed.curGrip);
        triggerLayoutAll();
    }
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void TieSegment::editDrag(EditData& ed)
{
    Grip g = ed.curGrip;
    ups(g).off += ed.delta;

    if (g == Grip::START || g == Grip::END) {
        computeBezier();
        //
        // move anchor for slurs/ties
        //
        if ((g == Grip::START && isSingleBeginType()) || (g == Grip::END && isSingleEndType())) {
            Spanner* spanner = tie();
            Element* e = ed.view()->elementNear(ed.pos);
            Note* note = (e && e->isNote()) ? toNote(e) : nullptr;
            if (note && ((g == Grip::END && note->tick() > tie()->tick()) || (g == Grip::START && note->tick() < tie()->tick2()))) {
                if (g == Grip::END) {
                    Tie* tie = toTie(spanner);
                    if (tie->startNote()->pitch() == note->pitch()
                        && tie->startNote()->chord()->tick() < note->chord()->tick()) {
                        ed.view()->setDropTarget(note);
                        if (note != tie->endNote()) {
                            changeAnchor(ed, note);
                            return;
                        }
                    }
                }
            } else {
                ed.view()->setDropTarget(0);
            }
        }
    } else if (g == Grip::BEZIER1 || g == Grip::BEZIER2) {
        computeBezier();
    } else if (g == Grip::SHOULDER) {
        ups(g).off = PointF();
        computeBezier(ed.delta);
    } else if (g == Grip::DRAG) {
        ups(Grip::DRAG).off = PointF();
        roffset() += ed.delta;
    }

    // if this SlurSegment was automatically adjusted to avoid collision
    // lock this edit by resetting SlurSegment to default position
    // and incorporating previous adjustment into user offset
    PointF offset = getAutoAdjust();
    if (!offset.isNull()) {
        setAutoAdjust(0.0, 0.0);
        roffset() += offset;
    }
}

//---------------------------------------------------------
//   computeBezier
//    compute help points of slur bezier segment
//---------------------------------------------------------

void TieSegment::computeBezier(PointF p6o)
{
    qreal _spatium  = spatium();
    qreal shoulderW;                // height as fraction of slur-length
    qreal shoulderH;

    //
    // pp1      start of slur
    // pp2      end of slur
    // pp3      bezier 1
    // pp4      bezier 2
    // pp5      drag
    // pp6      shoulder
    //
    PointF pp1 = ups(Grip::START).p + ups(Grip::START).off;
    PointF pp2 = ups(Grip::END).p + ups(Grip::END).off;

    PointF p2 = pp2 - pp1;         // normalize to zero
    if (p2.x() == 0.0) {
        qDebug("zero tie");
        return;
    }

    qreal sinb = atan(p2.y() / p2.x());
    Transform t;
    t.rotateRadians(-sinb);
    p2  = t.map(p2);
    p6o = t.map(p6o);

    double smallH = 0.38;
    qreal d   = p2.x() / _spatium;
    shoulderH = d * 0.4 * smallH;
    shoulderH = qBound(0.4, shoulderH, 1.3);
    shoulderH *= _spatium;
    shoulderW = .6;

    shoulderH -= p6o.y();

    if (!tie()->up()) {
        shoulderH = -shoulderH;
    }

    qreal c    = p2.x();
    qreal c1   = (c - c * shoulderW) * .5 + p6o.x();
    qreal c2   = c1 + c * shoulderW + p6o.x();

    PointF p5 = PointF(c * .5, 0.0);

    PointF p3(c1, -shoulderH);
    PointF p4(c2, -shoulderH);

    qreal w = score()->styleP(Sid::SlurMidWidth) - score()->styleP(Sid::SlurEndWidth);
    if (staff()) {
        w *= staff()->staffMag(tie()->tick());
    }
    PointF th(0.0, w);      // thickness of slur

    PointF p3o = p6o + t.map(ups(Grip::BEZIER1).off);
    PointF p4o = p6o + t.map(ups(Grip::BEZIER2).off);

    if (!p6o.isNull()) {
        PointF p6i = t.inverted().map(p6o);
        ups(Grip::BEZIER1).off += p6i;
        ups(Grip::BEZIER2).off += p6i;
    }

    //-----------------------------------calculate p6
    PointF pp3  = p3 + p3o;
    PointF pp4  = p4 + p4o;
    PointF ppp4 = pp4 - pp3;

    qreal r2 = atan(ppp4.y() / ppp4.x());
    t.reset();
    t.rotateRadians(-r2);
    PointF p6  = PointF(t.map(ppp4).x() * .5, 0.0);

    t.rotateRadians(2 * r2);
    p6 = t.map(p6) + pp3 - p6o;
    //-----------------------------------

    path = PainterPath();
    path.moveTo(PointF());
    path.cubicTo(p3 + p3o - th, p4 + p4o - th, p2);
    if (tie()->lineType() == 0) {
        path.cubicTo(p4 + p4o + th, p3 + p3o + th, PointF());
    }

    th = PointF(0.0, 3.0 * w);
    shapePath = PainterPath();
    shapePath.moveTo(PointF());
    shapePath.cubicTo(p3 + p3o - th, p4 + p4o - th, p2);
    shapePath.cubicTo(p4 + p4o + th, p3 + p3o + th, PointF());

    // translate back
    double y = pp1.y();
    const double offsetFactor = 0.2;
    if (staff()->isTabStaff(slurTie()->tick())) {
        y += (_spatium * (slurTie()->up() ? -offsetFactor : offsetFactor));
    }
    t.reset();
    t.translate(pp1.x(), y);
    t.rotateRadians(sinb);
    path                  = t.map(path);
    shapePath             = t.map(shapePath);
    ups(Grip::BEZIER1).p  = t.map(p3);
    ups(Grip::BEZIER2).p  = t.map(p4);
    ups(Grip::END).p      = t.map(p2) - ups(Grip::END).off;
    ups(Grip::DRAG).p     = t.map(p5);
    ups(Grip::SHOULDER).p = t.map(p6);

//      PointF staffOffset;
//      if (system() && track() >= 0)
//            staffOffset = PointF(0.0, -system()->staff(staffIdx())->y());

//      path.translate(staffOffset);
//      shapePath.translate(staffOffset);

    _shape.clear();
    PointF start;
    start = t.map(start);

    qreal minH = qAbs(3.0 * w);
    int nbShapes = 15;
    const CubicBezier b(pp1, ups(Grip::BEZIER1).pos(), ups(Grip::BEZIER2).pos(), ups(Grip::END).pos());
    for (int i = 1; i <= nbShapes; i++) {
        const PointF point = b.pointAtPercent(i / float(nbShapes));
        RectF re = RectF(start, point).normalized();
        if (re.height() < minH) {
            d = (minH - re.height()) * .5;
            re.adjust(0.0, -d, 0.0, d);
        }
        _shape.add(re);
        start = point;
    }
}

//---------------------------------------------------------
//   layout
//    p1, p2  are in System coordinates
//---------------------------------------------------------

void TieSegment::layoutSegment(const PointF& p1, const PointF& p2)
{
    setPos(PointF());
    ups(Grip::START).p = p1;
    ups(Grip::END).p   = p2;

    //Adjust Y pos to staff type offset before other calculations
    if (staffType()) {
        rypos() += staffType()->yoffset().val() * spatium();
    }

    computeBezier();

    RectF bbox = path.boundingRect();

    // adjust position to avoid staff line if necessary
    Staff* st          = staff();
    bool reverseAdjust = false;

    if (slurTie()->isTie() && st && !st->isTabStaff(slurTie()->tick())) {
        // multinote chords with ties need special handling
        // otherwise, adjusted tie might crowd an unadjusted tie unnecessarily
        Tie* t    = toTie(slurTie());
        Note* sn  = t->startNote();
        t->setTick(t->startNote()->tick());
        Chord* sc = sn ? sn->chord() : 0;

        // normally, the adjustment moves ties according to their direction (eg, up if tie is up)
        // but we will reverse this for notes within chords when appropriate
        // for two-note chords, it looks better to have notes on spaces tied outside the lines

        if (sc) {
            size_t notes = sc->notes().size();
            bool onLine = !(sn->line() & 1);
            if ((onLine && notes > 1) || (!onLine && notes > 2)) {
                reverseAdjust = true;
            }
        }
    }
    qreal sp          = spatium();
    qreal minDistance = 0.5;
    autoAdjustOffset  = PointF();
    if (bbox.height() < minDistance * 2 * sp && st && !st->isTabStaff(slurTie()->tick())) {
        // slur/tie is fairly flat
        bool up       = slurTie()->up();
        qreal ld      = st->lineDistance(tick()) * sp;
        qreal topY    = bbox.top() / ld;
        qreal bottomY = bbox.bottom() / ld;
        int lineY     = up ? qRound(topY) : qRound(bottomY);
        if (lineY >= 0 && lineY < st->lines(tick()) * st->lineDistance(tick())) {
            // on staff
            if (qAbs(topY - lineY) < minDistance && qAbs(bottomY - lineY) < minDistance) {
                // too close to line
                if (!isNudged() && !isEdited()) {
                    // user has not nudged or edited
                    qreal offY;
                    if (up != reverseAdjust) {              // exclusive or
                        offY = (lineY - minDistance) - topY;
                    } else {
                        offY = (lineY + minDistance) - bottomY;
                    }
                    setAutoAdjust(0.0, offY * sp);
                }
            }
        }
    }
    setbbox(path.boundingRect());
}

//---------------------------------------------------------
//   setAutoAdjust
//---------------------------------------------------------

void TieSegment::setAutoAdjust(const PointF& offset)
{
    PointF diff = offset - autoAdjustOffset;
    if (!diff.isNull()) {
        path.translate(diff);
        shapePath.translate(diff);
        _shape.translate(diff);
        for (int i = 0; i < int(Grip::GRIPS); ++i) {
            _ups[i].p += diff;
        }
        autoAdjustOffset = offset;
    }
}

//---------------------------------------------------------
//   isEdited
//---------------------------------------------------------

bool TieSegment::isEdited() const
{
    for (int i = 0; i < int(Grip::GRIPS); ++i) {
        if (!_ups[i].off.isNull()) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   slurPos
//    Calculate position of start- and endpoint of slur
//    relative to System() position.
//---------------------------------------------------------

void Tie::slurPos(SlurPos* sp)
{
    bool useTablature = staff() && staff()->isTabStaff(tick());
    const StaffType* stt = useTablature ? staff()->staffType(tick()) : 0;
    qreal _spatium    = spatium();
    qreal hw          = startNote()->tabHeadWidth(stt);     // if stt == 0, defaults to headWidth()
    qreal __up        = _up ? -1.0 : 1.0;
    // y offset for ties inside chord margins (typically multi-note chords): lined up with note top or bottom margin
    //    or outside (typically single-note chord): overlaps note and is above/below it
    // Outside: Tab: uses font size and may be asymmetric placed above/below line (frets ON or ABOVE line)
    //          Std: assumes notehead is 1 sp high, 1/2 sp above and 1/2 below line; add 1/4 sp to it
    // Inside:  Tab: 1/2 of Outside offset
    //          Std: use a fixed percentage of note width
    qreal yOffOutside = useTablature
                        ? (_up ? stt->fretBoxY() : stt->fretBoxY() + stt->fretBoxH()) * magS()
                        : 0.75 * _spatium * __up;
    qreal yOffInside  = useTablature ? yOffOutside * 0.5 : hw * .3 * __up;

    Chord* sc   = startNote()->chord();
    sp->system1 = sc->measure()->system();
    if (!sp->system1) {
        Measure* m = sc->measure();
        qDebug("No system: measure is %d has %d count %d", m->isMMRest(), m->hasMMRest(), m->mmRestCount());
    }

    qreal xo;
    qreal yo;
    bool shortStart = false;

    // determine attachment points
    // similar code is used in Chord::layoutPitched()
    // to allocate extra space to enforce minTieLength
    // so keep these in sync

    sp->p1    = sc->pos() + sc->segment()->pos() + sc->measure()->pos();

    //------p1
    if ((sc->notes().size() > 1) || (sc->stem() && (sc->up() == _up))) {
        xo = startNote()->x() + hw * 1.12;
        yo = startNote()->pos().y() + yOffInside;
        shortStart = true;
    } else {
        xo = startNote()->x() + hw * 0.65;
        yo = startNote()->pos().y() + yOffOutside;
    }
    sp->p1 += PointF(xo, yo);

    //------p2
    if (endNote() == 0) {
        sp->p2 = sp->p1 + PointF(_spatium * 3, 0.0);
        sp->system2 = sp->system1;
        return;
    }
    Chord* ec = endNote()->chord();
    sp->p2    = ec->pos() + ec->segment()->pos() + ec->measure()->pos();
    sp->system2 = ec->measure()->system();

    // force tie to be horizontal except for cross-staff or if there is a difference of line (tpc, clef, tpc)
    bool horizontal = startNote()->line() == endNote()->line() && sc->vStaffIdx() == ec->vStaffIdx();

    hw = endNote()->tabHeadWidth(stt);
    if ((ec->notes().size() > 1) || (ec->stem() && !ec->up() && !_up)) {
        xo = endNote()->x() - hw * 0.12;
        if (!horizontal) {
            yo = endNote()->pos().y() + yOffInside;
        }
    } else if (shortStart) {
        xo = endNote()->x() + hw * 0.15;
        if (!horizontal) {
            yo = endNote()->pos().y() + yOffOutside;
        }
    } else {
        xo = endNote()->x() + hw * 0.35;
        if (!horizontal) {
            yo = endNote()->pos().y() + yOffOutside;
        }
    }
    sp->p2 += PointF(xo, yo);

    // adjust for cross-staff
    if (sc->vStaffIdx() != vStaffIdx() && sp->system1) {
        qreal diff = sp->system1->staff(sc->vStaffIdx())->y() - sp->system1->staff(vStaffIdx())->y();
        sp->p1.ry() += diff;
    }
    if (ec->vStaffIdx() != vStaffIdx() && sp->system2) {
        qreal diff = sp->system2->staff(ec->vStaffIdx())->y() - sp->system2->staff(vStaffIdx())->y();
        sp->p2.ry() += diff;
    }
}

//---------------------------------------------------------
//   Tie
//---------------------------------------------------------

Tie::Tie(Score* s)
    : SlurTie(s)
{
    setAnchor(Anchor::NOTE);
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Tie::write(XmlWriter& xml) const
{
    xml.stag(this);
    SlurTie::writeProperties(xml);
    xml.etag();
}

//---------------------------------------------------------
//   calculateDirection
//---------------------------------------------------------

static int compareNotesPos(const Note* n1, const Note* n2)
{
    if (n1->line() != n2->line()) {
        return n2->line() - n1->line();
    } else if (n1->string() != n2->string()) {
        return n2->string() - n1->string();
    } else {
        return n1->pitch() - n2->pitch();
    }
}

void Tie::calculateDirection()
{
    Chord* c1   = startNote()->chord();
    Chord* c2   = endNote()->chord();
    Measure* m1 = c1->measure();
    Measure* m2 = c2->measure();

    if (_slurDirection == Direction::AUTO) {
        std::vector<Note*> notes = c1->notes();
        size_t n = notes.size();
        if (m1->hasVoices(c1->staffIdx(), c1->tick(), c1->actualTicks()) || m2->hasVoices(c2->staffIdx(), c2->tick(), c2->actualTicks())) {
            // in polyphonic passage, ties go on the stem side
            _up = c1->up();
        } else if (n == 1) {
            //
            // single note
            //
            if (c1->up() != c2->up()) {
                // if stem direction is mixed, always up
                _up = true;
            } else {
                _up = !c1->up();
            }
        } else {
            //
            // chords
            //
            int tiesCount = 0;
            Note* tieNote = startNote();
            Note* tieAbove = nullptr;
            Note* tieBelow = nullptr;

            for (size_t i = 0; i < n; ++i) {
                if (notes[i]->tieFor()) {
                    tiesCount++;
                    int noteDiff = compareNotesPos(notes[i], tieNote);

                    if (noteDiff > 0) {
                        if (!tieAbove) {
                            tieAbove = notes[i];
                        } else if (compareNotesPos(notes[i], tieAbove) < 0) {
                            tieAbove = notes[i];
                        }
                    } else if (noteDiff < 0) {
                        if (!tieBelow) {
                            tieBelow = notes[i];
                        } else if (compareNotesPos(notes[i], tieBelow) > 0) {
                            tieBelow = notes[i];
                        }
                    }
                }
            }
            if (!tieBelow) {
                // bottom tie is up if it is the only tie and not the bottom note of the chord
                _up = tiesCount == 1 && tieNote != c1->downNote();
            } else if (!tieAbove) {
                // top tie always up
                _up = true;
            } else {
                bool tabStaff = onTabStaff();
                int tieLine = tabStaff ? tieNote->string() : tieNote->line();
                int belowLine = tabStaff ? tieBelow->string() : tieBelow->line();
                int aboveLine = tabStaff ? tieAbove->string() : tieAbove->line();

                if (tieLine <= (tabStaff ? 2 : 4)) {
                    _up = ((belowLine - tieLine) <= 1) || ((tieLine - aboveLine) > 1);
                } else {
                    _up = ((belowLine - tieLine) <= 1) && ((tieLine - aboveLine) > 1);
                }
            }
        }
    } else {
        _up = _slurDirection == Direction::UP ? true : false;
    }
}

//---------------------------------------------------------
//   layoutFor
//    layout the first SpannerSegment of a slur
//---------------------------------------------------------

TieSegment* Tie::layoutFor(System* system)
{
    // do not layout ties in tablature if not showing back-tied fret marks
    StaffType* st = staff()->staffType(startNote() ? startNote()->tick() : Fraction(0, 1));
    if (st && st->isTabStaff() && !st->showBackTied()) {
        if (!segmentsEmpty()) {
            eraseSpannerSegments();
        }
        return nullptr;
    }
    //
    //    show short bow
    //
    if (startNote() == 0 || endNote() == 0) {
        if (startNote() == 0) {
            qDebug("no start note");
            return 0;
        }
        Chord* c1 = startNote()->chord();
        if (_slurDirection == Direction::AUTO) {
            if (c1->measure()->hasVoices(c1->staffIdx(), c1->tick(), c1->actualTicks())) {
                // in polyphonic passage, ties go on the stem side
                _up = c1->up();
            } else {
                _up = !c1->up();
            }
        } else {
            _up = _slurDirection == Direction::UP ? true : false;
        }
        fixupSegments(1);
        TieSegment* segment = segmentAt(0);
        segment->setSpannerSegmentType(SpannerSegmentType::SINGLE);
        segment->setSystem(startNote()->chord()->segment()->measure()->system());
        SlurPos sPos;
        slurPos(&sPos);
        segment->layoutSegment(sPos.p1, sPos.p2);
        return segment;
    }
    calculateDirection();

    SlurPos sPos;
    slurPos(&sPos);

    setPos(0, 0);

    int n;
    if (sPos.system1 != sPos.system2) {
        n = 2;
        sPos.p2 = PointF(system->width(), sPos.p1.y());
    } else {
        n = 1;
    }

    fixupSegments(n);
    TieSegment* segment = segmentAt(0);
    segment->setSystem(system);   // Needed to populate System.spannerSegments
    segment->layoutSegment(sPos.p1, sPos.p2);
    segment->setSpannerSegmentType(sPos.system1 != sPos.system2 ? SpannerSegmentType::BEGIN : SpannerSegmentType::SINGLE);
    return segment;
}

//---------------------------------------------------------
//   layoutBack
//    layout the second SpannerSegment of a split slur
//---------------------------------------------------------

TieSegment* Tie::layoutBack(System* system)
{
    // do not layout ties in tablature if not showing back-tied fret marks
    StaffType* st = staff()->staffType(startNote() ? startNote()->tick() : Fraction(0, 1));
    if (st->isTabStaff() && !st->showBackTied()) {
        if (!segmentsEmpty()) {
            eraseSpannerSegments();
        }
        return nullptr;
    }

    SlurPos sPos;
    slurPos(&sPos);

    fixupSegments(2);
    TieSegment* segment = segmentAt(1);
    segment->setSystem(system);

    qreal x = system->firstNoteRestSegmentX(true);

    segment->layoutSegment(PointF(x, sPos.p2.y()), sPos.p2);
    segment->setSpannerSegmentType(SpannerSegmentType::END);
    return segment;
}

#if 0
//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Tie::startEdit(EditData& ed)
{
    printf("tie start edit %p %p\n", editStartNote, editEndNote);
    editStartNote = startNote();
    editEndNote   = endNote();
    SlurTie::startEdit(ed);
}

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Tie::endEdit(EditData& ed)
{
//printf("tie::endEdit\n");
//      if (editStartNote != startNote() || editEndNote != endNote()) {
//            score()->undoStack()->push1(new ChangeSpannerElements(this, editStartNote, editEndNote));
//            }
    SlurTie::endEdit(ed);
}

#endif

//---------------------------------------------------------
//   setStartNote
//---------------------------------------------------------

void Tie::setStartNote(Note* note)
{
    setStartElement(note);
    setParent(note);
}

//---------------------------------------------------------
//   startNote
//---------------------------------------------------------

Note* Tie::startNote() const
{
    Q_ASSERT(!startElement() || startElement()->type() == ElementType::NOTE);
    return toNote(startElement());
}

//---------------------------------------------------------
//   endNote
//---------------------------------------------------------

Note* Tie::endNote() const
{
    return toNote(endElement());
}
}
