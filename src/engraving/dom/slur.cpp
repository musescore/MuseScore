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
#include "tremolotwochord.h"
#include "undo.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::draw;

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
            Slur* slr = slur();
            KeyboardModifiers km = ed.modifiers;
            EngravingItem* e = ed.view()->elementNear(ed.pos);
            if (e && e->isNote()) {
                Note* note = toNote(e);
                Fraction tick = note->chord()->tick();
                if ((g == Grip::END && tick > slr->tick()) || (g == Grip::START && tick < slr->tick2())) {
                    if (km != (ShiftModifier | ControlModifier)) {
                        Chord* c = note->chord();
                        ed.view()->setDropTarget(note);
                        if (c->part() == slr->part() && c != slr->endCR()) {
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
    renderer()->computeBezier(this, delta);

    System* startSys = slur()->startCR()->measure()->system();
    System* endSys = slur()->endCR()->measure()->system();
    if (startSys && endSys && startSys == endSys) {
        renderer()->layoutItem(slur());
    }
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
    m_connectedElement = s.m_connectedElement;
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

double Slur::scalingFactor() const
{
    Chord* startC = startElement() && startElement()->isChord() ? toChord(startElement()) : nullptr;
    Chord* endC = endElement() && endElement()->isChord() ? toChord(endElement()) : nullptr;

    if (!startC || !endC) {
        return 1.0;
    }

    if ((startC->isGraceBefore() && startC->parent() == endC)
        || (endC->isGraceAfter() && endC->parent() == startC)) {
        return style().styleD(Sid::graceNoteMag);
    }

    if (startC->isGrace()) {
        startC = toChord(startC->parent());
    }
    if (endC->isGrace()) {
        endC = toChord(endC->parent());
    }

    return 0.5 * (startC->intrinsicMag() + endC->intrinsicMag());
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
    TremoloTwoChord* trem = c ? c->tremoloTwoChord() : nullptr;
    adjustForTrem = trem && trem->up() == up();
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
            hasTrem = c->tremoloTwoChord() && c->up() == up();
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
