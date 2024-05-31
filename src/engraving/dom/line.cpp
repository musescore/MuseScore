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

#include "line.h"

#include <vector>

#include "containers.h"

#include "anchors.h"
#include "barline.h"
#include "chord.h"
#include "lyrics.h"
#include "measure.h"
#include "mscoreview.h"
#include "note.h"
#include "page.h"
#include "part.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "system.h"
#include "utils.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
LineSegment::LineSegment(const ElementType& type, Spanner* sp, System* parent, ElementFlags f)
    : SpannerSegment(type, sp, parent, f)
{
}

LineSegment::LineSegment(const ElementType& type, System* parent, ElementFlags f)
    : SpannerSegment(type, parent, f)
{
}

//---------------------------------------------------------
//   LineSegment
//---------------------------------------------------------

LineSegment::LineSegment(const LineSegment& s)
    : SpannerSegment(s)
{
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> LineSegment::gripsPositions(const EditData&) const
{
    std::vector<PointF> grips(gripsCount());
    PointF pp(pagePos());
    grips[int(Grip::START)] = pp;
    grips[int(Grip::END)] = pos2() + pp;
    grips[int(Grip::MIDDLE)] = pos2() * .5 + pp;
    return grips;
}

//---------------------------------------------------------
//   leftAnchorPosition
//---------------------------------------------------------

PointF LineSegment::leftAnchorPosition(const double& systemPositionY) const
{
    if (isMiddleType() || isEndType()) {
        return PointF(system()->firstMeasure()->abbox().left(), systemPositionY);
    }

    PointF result;

    Segment* startSeg = line()->startSegment();
    if (!startSeg) {
        return result;
    }

    System* s = startSeg->measure()->system();
    result = startSeg->pos() + startSeg->measure()->pos();
    result.ry() += systemPositionY - system()->pos().y();

    if (s) {
        result += s->pos();      // to page coordinates
    }
    return result;
}

//---------------------------------------------------------
//   rightAnchorPosition
//---------------------------------------------------------

PointF LineSegment::rightAnchorPosition(const double& systemPositionY) const
{
    const System* sys = system();
    IF_ASSERT_FAILED(sys) {
        return PointF();
    }

    bool endsAtSystemEnd = isSingleEndType() && line()->tick2() == sys->endTick();
    if (isMiddleType() || isBeginType() || endsAtSystemEnd) {
        return PointF(sys->endingXForOpenEndedLines() + sys->x(), systemPositionY);
    }

    PointF result;

    Segment* endSeg = line()->endSegment();
    if (!endSeg) {
        return result;
    }

    System* s = endSeg->measure()->system();
    result = endSeg->pos() + endSeg->measure()->pos();
    result.ry() += systemPositionY - system()->pos().y();

    if (s) {
        result += s->pos();    // to page coordinates
    }
    return result;
}

//---------------------------------------------------------
//   gripAnchorLines
//    return page coordinates
//---------------------------------------------------------

std::vector<LineF> LineSegment::gripAnchorLines(Grip grip) const
{
    std::vector<LineF> result;

    // Middle or aperture grip have no anchor
    if (!system() || grip == Grip::APERTURE) {
        return result;
    }

    // note-anchored spanners are relative to the system
    double y;
    if (spanner()->anchor() == Spanner::Anchor::NOTE) {
        y = system()->pos().y();
    } else {
        const staff_idx_t stIdx = staffIdx();
        y = system()->staffYpage(stIdx);
        if (line()->placement() == PlacementV::BELOW) {
            y += system()->staff(stIdx)->bbox().height();
        }
        // adjust Y to staffType offset
        y += staffOffsetY();
    }

    const Page* p = system()->page();
    const PointF pageOffset = p ? p->pos() : PointF();
    switch (grip) {
    case Grip::START:
        result.push_back(LineF(leftAnchorPosition(y), gripsPositions().at(static_cast<int>(Grip::START))).translated(pageOffset));
        break;
    case Grip::END:
        result.push_back(LineF(rightAnchorPosition(y), gripsPositions().at(static_cast<int>(Grip::END))).translated(pageOffset));
        break;
    case Grip::MIDDLE:
        result.push_back(LineF(leftAnchorPosition(y), gripsPositions().at(static_cast<int>(Grip::START))).translated(pageOffset));
        result.push_back(LineF(rightAnchorPosition(y), gripsPositions().at(static_cast<int>(Grip::END))).translated(pageOffset));
        break;
    default:
        break;
    }

    return result;
}

//---------------------------------------------------------
//   startDrag
//---------------------------------------------------------

void LineSegment::startDrag(EditData& ed)
{
    SpannerSegment::startDrag(ed);
    ElementEditDataPtr eed = ed.getData(this);
    if (!eed) {
        return;
    }
    eed->pushProperty(Pid::OFFSET2);
}

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void LineSegment::startEditDrag(EditData& ed)
{
    ElementEditDataPtr eed = ed.getData(this);
    if (!eed) {
        return;
    }
    eed->pushProperty(Pid::OFFSET);
    eed->pushProperty(Pid::OFFSET2);
    eed->pushProperty(Pid::AUTOPLACE);
    if (ed.modifiers & AltModifier) {
        setAutoplace(false);
    }

    updateAnchors(ed);
}

bool LineSegment::isEditAllowed(EditData& ed) const
{
    const bool moveStart = ed.curGrip == Grip::START;
    const bool moveEnd = ed.curGrip == Grip::END || ed.curGrip == Grip::MIDDLE;

    if (!((ed.modifiers & ShiftModifier) && ((isSingleBeginType() && moveStart)
                                             || (isSingleEndType() && moveEnd)))) {
        return false;
    }

    return true;
}

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool LineSegment::edit(EditData& ed)
{
    const bool moveStart = ed.curGrip == Grip::START;
    const bool moveEnd = ed.curGrip == Grip::END || ed.curGrip == Grip::MIDDLE;

    LineSegment* ls       = 0;
    SpannerSegmentType st = spannerSegmentType();   // may change later
    SLine* l              = line();
    track_idx_t track = l->track();
    track_idx_t track2 = l->track2();      // assumed to be same as track

    bool allowTimeAnchor = line()->allowTimeAnchor();

    if (ed.key == KeyboardKey::Key_Shift) {
        if (ed.isKeyRelease) {
            score()->hideAnchors();
        } else {
            EditTimeTickAnchors::updateAnchors(this, moveStart ? spanner()->tick() : spanner()->tick2(), moveStart ? track : track2);
        }
        triggerLayout();
        return true;
    }

    if (!isEditAllowed(ed)) {
        return false;
    }

    switch (l->anchor()) {
    case Spanner::Anchor::SEGMENT:
    {
        Segment* s1 = spanner()->startSegment();
        Segment* s2 = spanner()->endSegment();
        // check for line going to end of score
        if (spanner()->tick2() >= score()->lastSegment()->tick()) {
            // endSegment calculated above will be the last chord/rest of score
            // but that is not correct - it should be an imaginary note *after* the end of the score
            // best we can do is set s2 to lastSegment (probably the end barline)
            s2 = score()->lastSegment();
        }
        if (!s1 && !s2) {
            LOGD("LineSegment::edit: no start/end segment");
            return true;
        }
        EditTimeTickAnchors::updateAnchors(this, moveStart ? spanner()->tick() : spanner()->tick2(), moveStart ? track : track2);
        if (ed.key == Key_Left) {
            if (moveStart) {
                s1 = allowTimeAnchor ? s1->prev1ChordRestOrTimeTick() : s1->prev1WithElemsOnStaff(track2staff(track));
            } else if (moveEnd) {
                s2 = allowTimeAnchor ? s2->prev1ChordRestOrTimeTick() : s2->prev1WithElemsOnStaff(track2staff(track2));
            }
        } else if (ed.key == Key_Right) {
            if (moveStart) {
                s1 = allowTimeAnchor ? s1->next1ChordRestOrTimeTick() : s1->next1WithElemsOnStaff(track2staff(track));
            } else if (moveEnd) {
                Segment* ns2 = allowTimeAnchor ? s2->next1ChordRestOrTimeTick() : s2->next1WithElemsOnStaff(track2staff(track2));
                if (ns2) {
                    s2 = ns2;
                } else {
                    s2 = score()->lastSegment();
                }
            }
        }
        if (s1 == 0 || s2 == 0 || s1->tick() >= s2->tick()) {
            return true;
        }
        spanner()->undoChangeProperty(Pid::SPANNER_TICK, s1->tick());
        spanner()->undoChangeProperty(Pid::SPANNER_TICKS, s2->tick() - s1->tick());
    }
    break;
    case Spanner::Anchor::NOTE:
    {
        Note* note1       = toNote(l->startElement());
        Note* note2       = toNote(l->endElement());
        Note* oldNote1    = note1;
        Note* oldNote2    = note2;
        if (!note1 && !note2) {
            LOGD("LineSegment::edit: no start/end note");
            return true;                        // accept the event without doing anything
        }

        switch (ed.key) {
        case Key_Left:
            if (moveStart) {
                note1 = prevChordNote(note1);
            } else if (moveEnd) {
                note2 = prevChordNote(note2);
            }
            break;
        case Key_Right:
            if (moveStart) {
                note1 = nextChordNote(note1);
            } else if (moveEnd) {
                note2 = nextChordNote(note2);
            }
            break;
        case Key_Up:
            if (moveStart) {
                note1 = toNote(score()->upAlt(note1));
            } else if (moveEnd) {
                note2 = toNote(score()->upAlt(note2));
            }
            break;
        case Key_Down:
            if (moveStart) {
                note1 = toNote(score()->downAlt(note1));
            } else if (moveEnd) {
                note2 = toNote(score()->downAlt(note2));
            }
            break;
        default:
            return true;
        }

        // check prevChordNote() and nextchordNote() didn't return null
        // OR Score::upAlt() and Score::downAlt() didn't return non-Note (notably rests)
        // OR spanner duration is > 0
        // OR note1 and note2 didn't end up in different instruments
        // if this is the case, accepts the event and return without doing nothing
        if (!note1 || !note2
            || !note1->isNote() || !note2->isNote()
            || note1->chord()->tick() >= note2->chord()->tick()
            || note1->chord()->staff()->part()->instrument(note1->chord()->tick())
            != note2->chord()->staff()->part()->instrument(note2->chord()->tick())) {
            return true;
        }
        if (note1 != oldNote1 || note2 != oldNote2) {
            score()->undoChangeSpannerElements(spanner(), note1, note2);
        }
    }
    break;
    case Spanner::Anchor::MEASURE:
    case Spanner::Anchor::CHORD:
    {
        Measure* m1 = l->startMeasure();
        Measure* m2 = l->endMeasure();

        if (ed.key == Key_Left) {
            if (moveStart) {
                if (m1->prevMeasure()) {
                    m1 = m1->prevMeasure();
                }
            } else if (moveEnd) {
                Measure* m = m2->prevMeasure();
                if (m) {
                    m2 = m;
                }
            }
        } else if (ed.key == Key_Right) {
            if (moveStart) {
                if (m1->nextMeasure()) {
                    m1 = m1->nextMeasure();
                }
            } else if (moveEnd) {
                if (m2->nextMeasure()) {
                    m2 = m2->nextMeasure();
                }
            }
        }
        if (m1->tick() > m2->tick()) {
            return true;
        }
        if (l->startElement() != m1) {
            spanner()->undoChangeProperty(Pid::SPANNER_TICK,  m1->tick());
            spanner()->undoChangeProperty(Pid::SPANNER_TICKS, m2->endTick() - m1->tick());
        } else if (l->endElement() != m2) {
            spanner()->undoChangeProperty(Pid::SPANNER_TICKS, m2->endTick() - m1->tick());
        }
    }
    }
    triggerLayout();
    // recompute segment list, segment type may change
    renderer()->layoutItem(l);

    LineSegment* nls = 0;
    if (st == SpannerSegmentType::SINGLE) {
        if (moveStart) {
            nls = l->frontSegment();
        } else if (moveEnd) {
            nls = l->backSegment();
        }
    } else if (st == SpannerSegmentType::BEGIN) {
        nls = l->frontSegment();
    } else if (st == SpannerSegmentType::END) {
        nls = l->backSegment();
    } else {
        LOGD("spannerSegmentType %d", int(spannerSegmentType()));
    }

    if (nls && (nls != this)) {
        ed.view()->changeEditElement(nls);
    }
    if (ls) {
        score()->undoRemoveElement(ls);
    }

    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   findSegmentForGrip
//---------------------------------------------------------

Segment* LineSegment::findSegmentForGrip(Grip grip, PointF pos) const
{
    if (grip != Grip::START && grip != Grip::END) {
        return nullptr;
    }

    SLine* l = line();
    const bool left = (grip == Grip::START);

    const staff_idx_t oldStaffIndex = left ? staffIdx() : track2staff(l->effectiveTrack2());

    const double spacingFactor = 0.5;   // defines the point where canvas is divided between segments, systems etc.

    System* sys = system();
    const std::vector<System*> foundSystems = score()->searchSystem(pos, sys, spacingFactor);

    if (!foundSystems.empty() && !muse::contains(foundSystems, sys) && foundSystems[0]->staves().size()) {
        sys = foundSystems[0];
    }

    // Restrict searching segment to the correct staff
    pos.setY(sys->staffCanvasYpage(oldStaffIndex));

    Segment* seg = nullptr;   // don't prefer any segment while searching line position
    staff_idx_t staffIndex = oldStaffIndex;
    score()->dragPosition(pos, &staffIndex, &seg, spacingFactor, allowTimeAnchor());

    return seg;
}

//---------------------------------------------------------
//   deltaRebaseLeft
///   Helper function for anchors rebasing when dragging.
//---------------------------------------------------------

PointF LineSegment::deltaRebaseLeft(const Segment* oldSeg, const Segment* newSeg)
{
    if (oldSeg == newSeg) {
        return PointF();
    }
    return oldSeg->canvasPos() - newSeg->canvasPos();
}

//---------------------------------------------------------
//   deltaRebaseRight
///   Helper function for anchors rebasing when dragging.
//---------------------------------------------------------

PointF LineSegment::deltaRebaseRight(const Segment* oldSeg, const Segment* newSeg)
{
    if (oldSeg == newSeg) {
        return PointF();
    }

    const PointF oldBase = oldSeg->canvasPos();
    const PointF newBase = newSeg->canvasPos();
    return oldBase - newBase;
}

//---------------------------------------------------------
//   lastSegmentEndTick
///   Helper function for anchors rebasing when dragging.
//---------------------------------------------------------

Fraction LineSegment::lastSegmentEndTick(const Segment* seg, const Spanner* s)
{
    return seg->tick() + seg->ticksInStaff(track2staff(s->effectiveTrack2()));
}

//---------------------------------------------------------
//   rebaseAnchor
///   If system has changed, returns the new line segment
///   (may appear to be reused \c this segment), otherwise
///   returns nullptr.
//---------------------------------------------------------

LineSegment* LineSegment::rebaseAnchor(Grip grip, Segment* newSeg)
{
    switch (grip) {
    case Grip::START:
        if (!isSingleBeginType()) {
            return nullptr;
        }
        break;
    case Grip::END:
        if (!isSingleEndType()) {
            return nullptr;
        }
        break;
    default:
        return nullptr;
    }

    SLine* l = line();
    const bool left = (grip == Grip::START);
    Segment* const oldSeg = left ? l->startSegment() : l->endSegment();
    System* const oldSystem = system();

    if (!newSeg || oldSeg == newSeg) {
        return nullptr;
    }

    Fraction newSegTick = newSeg->tick();
    Fraction startTick = left ? newSegTick : l->tick();
    Fraction endTick = left ? l->tick2() : (newSegTick == startTick ? newSegTick + newSeg->ticks() : newSegTick);

    if (endTick <= startTick) {
        if (left) {
            endTick = lastSegmentEndTick(newSeg, l);
        } else {
            startTick = newSeg->tick();
        }
    }

    bool anchorChanged = false;

    System* sys = system();
    PointF oldPos = line()->linePos(grip, &sys);

    if (l->tick() != startTick) {
        l->undoChangeProperty(Pid::SPANNER_TICK, startTick);
        anchorChanged = true;
    }

    const Fraction ticksLength = endTick - startTick;
    if (ticksLength != l->ticks()) {
        l->undoChangeProperty(Pid::SPANNER_TICKS, ticksLength);
        anchorChanged = true;
    }

    if (newSeg->system() != oldSystem) {
        renderer()->layoutItem(l);
        return left ? l->frontSegment() : l->backSegment();
    } else if (anchorChanged) {
        PointF newPos = line()->linePos(grip, &sys);
        PointF delta = oldPos - newPos;
        if (left) {
            setOffset(offset() + delta);
            m_offset2 -= delta;
            setOffsetChanged(true);
        } else {
            m_offset2 += delta;
        }
    }

    return nullptr;
}

//---------------------------------------------------------
//   rebaseAnchors
//---------------------------------------------------------

void LineSegment::rebaseAnchors(EditData& ed, Grip grip)
{
    if (line()->anchor() != Spanner::Anchor::SEGMENT) {
        return;
    }

    if (isTrillSegment()) {
        EngravingItem* startElement = spanner()->startElement();
        if (startElement && startElement->isChord() && toChord(startElement)->staffMove() != 0) {
            // This trill is on a cross-staff chord. Don't try to rebase its anchors when dragging.
            return;
        }
    }

    // don't change anchors on keyboard adjustment or if Ctrl is pressed
    // (Ctrl+Left/Right is handled elsewhere!)
    if (ed.key == Key_Left || ed.key == Key_Right || ed.modifiers & ControlModifier) {
        return;
    }

    switch (grip) {
    case Grip::START:
    case Grip::END: {
        const bool left = (grip == Grip::START);

        if (left && !isSingleBeginType()) {
            return;
        }
        if (!left && !isSingleEndType()) {
            return;
        }

        // Find an appropriate segment to bind from actual mouse
        // position. This allows changing systems while dragging
        // while not setting line position to something
        // inappropriate.
        Segment* seg = findSegmentForGrip(grip, ed.pos);
        LineSegment* newLineSegment = rebaseAnchor(grip, seg);

        if (newLineSegment) {
            // If new line segment is not the same as this one,
            // switch to dragging that new segment.
            if (newLineSegment != this) {
                // Reset offset for the old line segment
                if (left) {
                    m_offset2.rx() -= offset().x();
                    setOffset(PointF());
                } else {
                    setUserOff2(PointF());
                }

                // Switch to dragging the new line segment
                ed.view()->changeEditElement(newLineSegment);
            }

            // Set offset for the new line segment for grip to appear under the mouse cursor
            System* sys;
            const PointF lp = line()->linePos(grip, &sys);
            const double xoff = sys->mapFromCanvas(ed.pos).x() - lp.x();

            if (left) {
                newLineSegment->rxoffset() = xoff;
                newLineSegment->setUserOff2(PointF(-xoff, 0.0));
            } else {
                newLineSegment->setUserXoffset2(xoff);
            }
        }
    }
    break;
    case Grip::MIDDLE: {
        // The middle grip is used for vertical movement 99% of the time, so don't try to rebase anchors.
        return;
    }
    default:
        break;
    }
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void LineSegment::editDrag(EditData& ed)
{
    // Only for resizing according to the diagonal properties
    const PointF deltaResize(ed.evtDelta.x(), line()->diagonal() ? ed.evtDelta.y() : 0.0);

    switch (ed.curGrip) {
    case Grip::START:         // Resize the begin of element (left grip)
        setOffset(offset() + deltaResize);
        m_offset2 -= deltaResize;

        if (isStyled(Pid::OFFSET)) {
            setPropertyFlags(Pid::OFFSET, PropertyFlags::UNSTYLED);
        }

        rebaseAnchors(ed, ed.curGrip);
        break;
    case Grip::END:         // Resize the end of element (right grip)
        m_offset2 += deltaResize;
        rebaseAnchors(ed, ed.curGrip);
        break;
    case Grip::MIDDLE: {         // Move the element (middle grip)
        // Only for moving, no y limitation
        const PointF deltaMove(ed.evtDelta);
        setOffset(offset() + deltaMove);
        setOffsetChanged(true);
        if (isStyled(Pid::OFFSET)) {
            setPropertyFlags(Pid::OFFSET, PropertyFlags::UNSTYLED);
        }
        rebaseAnchors(ed, ed.curGrip);
    }
    break;
    default:
        break;
    }
    if (line()->anchor() == Spanner::Anchor::NOTE && ed.isStartEndGrip()) {
        //
        // if we touch a different note, change anchor
        //
        EngravingItem* e = ed.view()->elementNear(ed.pos);
        if (e && e->isNote()) {
            SLine* l = line();
            if (ed.curGrip == Grip::END && e != line()->endElement()) {
                LOGD("LineSegment: move end anchor");
                Note* noteOld = toNote(l->endElement());
                Note* noteNew = toNote(e);
                Note* sNote   = toNote(l->startElement());
                // do not change anchor if new note is before start note
                if (sNote && sNote->chord() && noteNew->chord() && sNote->chord()->tick() < noteNew->chord()->tick()) {
                    score()->undoChangeSpannerElements(l, sNote, noteNew);

                    m_offset2 += noteOld->canvasPos() - noteNew->canvasPos();
                }
            } else if (ed.curGrip == Grip::START && e != l->startElement()) {
                LOGD("LineSegment: move start anchor (not impl.)");
            }
        }
    }

    updateAnchors(ed);

    triggerLayout();
}

void LineSegment::updateAnchors(EditData& ed) const
{
    if (ed.curGrip != Grip::START && ed.curGrip != Grip::END) {
        return;
    }
    EditTimeTickAnchors::updateAnchors(this, ed.curGrip == Grip::START ? line()->tick() : line()->tick2(),
                                       ed.curGrip == Grip::START ? line()->track() : line()->track2());
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void LineSegment::spatiumChanged(double ov, double nv)
{
    EngravingItem::spatiumChanged(ov, nv);
    double scale = nv / ov;
    line()->setLineWidth(line()->lineWidth() * scale);
    m_offset2 *= scale;
}

//---------------------------------------------------------
//   localSpatiumChanged
//---------------------------------------------------------

void LineSegment::localSpatiumChanged(double ov, double nv)
{
    EngravingItem::localSpatiumChanged(ov, nv);
    m_offset2 *= nv / ov;
}

//---------------------------------------------------------
//   propertyDelegate
//---------------------------------------------------------

EngravingItem* LineSegment::propertyDelegate(Pid pid)
{
    if (pid == Pid::DIAGONAL
        || pid == Pid::COLOR
        || pid == Pid::LINE_WIDTH
        || pid == Pid::LINE_STYLE
        || pid == Pid::DASH_LINE_LEN
        || pid == Pid::DASH_GAP_LEN) {
        return spanner();
    }
    return SpannerSegment::propertyDelegate(pid);
}

//---------------------------------------------------------
//   dragAnchorLines
//---------------------------------------------------------

std::vector<LineF> LineSegment::dragAnchorLines() const
{
    return gripAnchorLines(Grip::MIDDLE);
}

RectF LineSegment::drag(EditData& ed)
{
    setOffset(offset() + ed.evtDelta);
    setOffsetChanged(true);

    if (isStyled(Pid::OFFSET)) {
        setPropertyFlags(Pid::OFFSET, PropertyFlags::UNSTYLED);
    }

    rebaseAnchors(ed, Grip::MIDDLE);

    return canvasBoundingRect();
}

//---------------------------------------------------------
//   SLine
//---------------------------------------------------------

SLine::SLine(const ElementType& type, EngravingItem* parent, ElementFlags f)
    : Spanner(type, parent, f)
{
    setTrack(0);
    m_lineColor = configuration()->defaultColor();
    m_lineWidth = 0.15 * spatium();
}

SLine::SLine(const SLine& s)
    : Spanner(s)
{
    m_diagonal    = s.m_diagonal;
    m_lineWidth   = s.m_lineWidth;
    m_lineColor   = s.m_lineColor;
    m_lineStyle   = s.m_lineStyle;
    m_dashLineLen = s.m_dashLineLen;
    m_dashGapLen  = s.m_dashGapLen;
}

PointF SLine::linePos(Grip grip, System** system) const
{
    bool start = grip == Grip::START;
    bool mmRest = style().styleB(Sid::createMultiMeasureRests);

    switch (anchor()) {
    case Spanner::Anchor::NOTE:
    {
        EngravingItem* item = start ? startElement() : endElement();
        Note* note = item && item->isNote() ? toNote(item) : nullptr;
        if (!note) {
            return PointF();
        }
        *system = note->chord()->segment()->measure()->system();
        if (!(*system)) {
            return PointF();
        }
        return note->pagePos() - (*system)->pagePos();
    }
    case Spanner::Anchor::CHORD:
    case Spanner::Anchor::SEGMENT:
    {
        Segment* segment = start ? startSegment() : endSegment();
        if (!segment) {
            return PointF();
        }
        if (anchor() == Spanner::Anchor::CHORD && !segment->isChordRestType()) {
            return PointF();
        }
        if (!start) {
            Fraction curTick = segment->tick();
            while (true) {
                Segment* prevSeg = mmRest ? segment->prev1MM() : segment->prev1();
                if (prevSeg && prevSeg->tick() == curTick) {
                    segment = prevSeg;
                } else {
                    break;
                }
            }
        }
        *system = segment->measure()->system();
        double x = segment->x() + segment->measure()->x() - (start ? 0.0 : spatium());
        return PointF(x, 0.0);
    }
    case Spanner::Anchor::MEASURE:
    {
        Measure* measure = score()->tick2measureMM(start ? tick() : tick2());
        if (!measure) {
            return PointF();
        }
        *system = measure->system();
        double x = measure->x();
        return PointF(x, 0.0);
    }
    default:
        return PointF();
    }
}

//---------------------------------------------------------
//   setLen
//    used to create an element suitable for palette
//---------------------------------------------------------

void SLine::setLen(double l)
{
    if (spannerSegments().empty()) {
        add(createLineSegment(score()->dummy()->system()));
    }
    LineSegment* s = frontSegment();
    s->setPos(PointF());
    s->setPos2(PointF(l, 0));
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue SLine::getProperty(Pid id) const
{
    switch (id) {
    case Pid::DIAGONAL:
        return m_diagonal;
    case Pid::COLOR:
        return PropertyValue::fromValue(m_lineColor);
    case Pid::LINE_WIDTH:
        return m_lineWidth;
    case Pid::LINE_STYLE:
        return m_lineStyle;
    case Pid::DASH_LINE_LEN:
        return dashLineLen();
    case Pid::DASH_GAP_LEN:
        return dashGapLen();
    default:
        return Spanner::getProperty(id);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool SLine::setProperty(Pid id, const PropertyValue& v)
{
    switch (id) {
    case Pid::DIAGONAL:
        m_diagonal = v.toBool();
        break;
    case Pid::COLOR:
        m_lineColor = v.value<Color>();
        break;
    case Pid::LINE_WIDTH:
        if (v.type() == P_TYPE::MILLIMETRE) {
            m_lineWidth = v.value<Millimetre>();
        } else if (v.type() == P_TYPE::SPATIUM) {
            m_lineWidth = v.value<Spatium>().toMM(spatium());
        }
        break;
    case Pid::LINE_STYLE:
        m_lineStyle = v.value<LineType>();
        break;
    case Pid::DASH_LINE_LEN:
        setDashLineLen(v.toDouble());
        break;
    case Pid::DASH_GAP_LEN:
        setDashGapLen(v.toDouble());
        break;
    default:
        return Spanner::setProperty(id, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue SLine::propertyDefault(Pid pid) const
{
    switch (pid) {
    case Pid::DIAGONAL:
        return false;
    case Pid::COLOR:
        return PropertyValue::fromValue(configuration()->defaultColor());
    case Pid::LINE_WIDTH:
        if (propertyFlags(pid) != PropertyFlags::NOSTYLE) {
            return Spanner::propertyDefault(pid);
        }
        return Millimetre(0.15 * spatium());
    case Pid::LINE_STYLE:
        if (propertyFlags(pid) != PropertyFlags::NOSTYLE) {
            return Spanner::propertyDefault(pid);
        }
        return LineType::SOLID;
    case Pid::DASH_LINE_LEN:
    case Pid::DASH_GAP_LEN:
        if (propertyFlags(pid) != PropertyFlags::NOSTYLE) {
            return Spanner::propertyDefault(pid);
        }
        return 5.0;
    default:
        return Spanner::propertyDefault(pid);
    }
}
}
