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
using namespace muse::draw;

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
    if (SlurTieSegment::isEditAllowed(ed)) {
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

    return muse::contains(navigationKeys, ed.key);
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

    if (SlurTieSegment::edit(ed)) {
        return true;
    }

    Slur* sl = slur();

    ChordRest* cr = nullptr;
    ChordRest* e;
    ChordRest* e1;
    const bool start = ed.curGrip == Grip::START;
    if (start) {
        e  = sl->startCR();
        e1 = sl->endCR();
    } else {
        e  = sl->endCR();
        e1 = sl->startCR();
    }

    const bool altMod = ed.modifiers & AltModifier;
    const bool shiftMod = ed.modifiers & ShiftModifier;
    const bool extendToBarLine = shiftMod && altMod;
    const bool isPartialSlur = sl->isIncoming() || sl->isOutgoing();

    ChordRestNavigateOptions options;
    options.disableOverRepeats = true;

    if (ed.key == Key_Left) {
        if (extendToBarLine) {
            const Measure* measure = e->measure();
            if (start) {
                cr = measure->firstChordRest(e->track());
                if (!cr->hasPrecedingJumpItem()) {
                    return false;
                }
                sl->undoSetIncoming(true);
            } else if (e->hasFollowingJumpItem()) {
                sl->undoSetOutgoing(false);
            }
        } else {
            if (start && sl->isIncoming()) {
                sl->undoSetIncoming(false);
                cr = prevChordRest(e, options);
            } else if (!start && sl->isOutgoing()) {
                sl->undoSetOutgoing(false);
            } else {
                cr = prevChordRest(e, options);
            }
        }
    } else if (ed.key == Key_Right) {
        if (extendToBarLine) {
            const Measure* measure = e->measure();
            if (start && e->hasPrecedingJumpItem()) {
                sl->undoSetIncoming(false);
            } else if (!start) {
                cr = measure->lastChordRest(e->track());
                if (!cr->hasFollowingJumpItem()) {
                    return false;
                }
                sl->undoSetOutgoing(true);
            }
        } else {
            if (start && sl->isIncoming()) {
                sl->undoSetIncoming(false);
            } else if (!start && sl->isOutgoing()) {
                sl->undoSetOutgoing(false);
                cr = nextChordRest(e, options);
            } else {
                cr = nextChordRest(e, options);
            }
        }
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
    if (cr && (cr != e1 || isPartialSlur)) {
        if (cr->staff() != e->staff() && (cr->staffType()->isTabStaff() || e->staffType()->isTabStaff())) {
            return false; // Cross-staff slurs don't make sense for TAB staves
        }
        if (cr->staff()->isLinked(e->staff())) {
            return false; // Don't allow slur to cross into staff that's linked to this
        }
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
        score()->undoStack()->pushWithoutPerforming(new ChangeStartEndSpanner(sp, sp->startElement(), sp->endElement()));
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
            if (se && ee) {
                score()->undo(new ChangeStartEndSpanner(sp, se, ee));
                renderer()->layoutItem(sp);
            }
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
    Grip g = ed.curGrip;
    if (g == Grip::NO_GRIP) {
        return;
    }

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
    triggerLayout();
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

double SlurSegment::endWidth() const
{
    return style().styleMM(Sid::slurEndWidth);
}

double SlurSegment::midWidth() const
{
    return style().styleMM(Sid::slurMidWidth);
}

double SlurSegment::dottedWidth() const
{
    return style().styleMM(Sid::slurDottedWidth);
}

Slur::Slur(const Slur& s)
    : SlurTie(s)
{
    _connectedElement = s._connectedElement;
    _partialSpannerDirection = s._partialSpannerDirection;
}

//---------------------------------------------------------
//   Slur
//---------------------------------------------------------

Slur::Slur(EngravingItem* parent)
    : SlurTie(ElementType::SLUR, parent)
{
    setAnchor(Anchor::CHORD);
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

void Slur::undoSetIncoming(bool incoming)
{
    if (incoming == isIncoming()) {
        return;
    }

    undoChangeProperty(Pid::PARTIAL_SPANNER_DIRECTION, calcIncomingDirection(incoming), PropertyFlags::UNSTYLED);
}

void Slur::undoSetOutgoing(bool outgoing)
{
    if (outgoing == isOutgoing()) {
        return;
    }

    undoChangeProperty(Pid::PARTIAL_SPANNER_DIRECTION, calcOutgoingDirection(outgoing), PropertyFlags::UNSTYLED);
}

void Slur::setIncoming(bool incoming)
{
    if (incoming == isIncoming()) {
        return;
    }

    _partialSpannerDirection = calcIncomingDirection(incoming);
}

void Slur::setOutgoing(bool outgoing)
{
    if (outgoing == isOutgoing()) {
        return;
    }

    _partialSpannerDirection = calcOutgoingDirection(outgoing);
}

PartialSpannerDirection Slur::calcIncomingDirection(bool incoming)
{
    PartialSpannerDirection dir = PartialSpannerDirection::INCOMING;
    if (incoming) {
        SlurSegment* firstSeg = nsegments() > 0 ? frontSegment() : nullptr;
        if (firstSeg) {
            firstSeg->setSlurOffset(Grip::START, PointF(0, 0));
        }
        dir = _partialSpannerDirection
              == PartialSpannerDirection::OUTGOING ? PartialSpannerDirection::BOTH : PartialSpannerDirection::INCOMING;
    } else {
        dir = _partialSpannerDirection == PartialSpannerDirection::BOTH ? PartialSpannerDirection::OUTGOING : PartialSpannerDirection::NONE;
    }
    return dir;
}

PartialSpannerDirection Slur::calcOutgoingDirection(bool outgoing)
{
    PartialSpannerDirection dir = PartialSpannerDirection::OUTGOING;
    if (outgoing) {
        SlurSegment* lastSeg = nsegments() > 0 ? backSegment() : nullptr;
        if (lastSeg) {
            lastSeg->setSlurOffset(Grip::END, PointF(0, 0));
        }
        dir = _partialSpannerDirection
              == PartialSpannerDirection::INCOMING ? PartialSpannerDirection::BOTH : PartialSpannerDirection::OUTGOING;
    } else {
        dir = _partialSpannerDirection == PartialSpannerDirection::BOTH ? PartialSpannerDirection::INCOMING : PartialSpannerDirection::NONE;
    }
    return dir;
}

bool Slur::isIncoming() const
{
    return _partialSpannerDirection == PartialSpannerDirection::BOTH || _partialSpannerDirection == PartialSpannerDirection::INCOMING;
}

bool Slur::isOutgoing() const
{
    return _partialSpannerDirection == PartialSpannerDirection::BOTH || _partialSpannerDirection == PartialSpannerDirection::OUTGOING;
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

PropertyValue Slur::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::PARTIAL_SPANNER_DIRECTION:
        return partialSpannerDirection();
    default:
        return SlurTie::getProperty(propertyId);
    }
}

PropertyValue Slur::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::PARTIAL_SPANNER_DIRECTION:
        return PartialSpannerDirection::NONE;
    default:
        return SlurTie::propertyDefault(id);
    }
}

bool Slur::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::PARTIAL_SPANNER_DIRECTION:
        setPartialSpannerDirection(v.value<PartialSpannerDirection>());
        break;
    default:
        return SlurTie::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}
}
