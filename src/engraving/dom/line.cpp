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
#include "dynamic.h"
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
        return PointF(system()->firstMeasure()->pageBoundingRect().left(), systemPositionY);
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

    const staff_idx_t stIdx = staffIdx();
    double y = system()->staffYpage(stIdx);
    if (line()->placement() == PlacementV::BELOW) {
        y += system()->staff(stIdx)->bbox().height();
    }
    // adjust Y to staffType offset
    y += staffOffsetY();

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
    return ed.modifiers & ShiftModifier;
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

    if (ed.key == KeyboardKey::Key_Shift) {
        if (ed.isKeyRelease) {
            score()->hideAnchors();
        } else {
            EditTimeTickAnchors::updateAnchors(this, moveStart ? track : track2);
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
        if (moveStart) {
            s1 = findNewAnchorSegment(ed, s1);
        } else {
            s2 = findNewAnchorSegment(ed, s2);
        }
        if (s1 == 0 || s2 == 0 || s1->tick() >= s2->tick()) {
            return true;
        }

        undoMoveStartEndAndSnappedItems(moveStart, moveEnd, s1, s2);

        EditTimeTickAnchors::updateAnchors(this, moveStart ? track : track2);
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

Segment* LineSegment::findNewAnchorSegment(const EditData& ed, const Segment* curSeg)
{
    if (!line()->allowTimeAnchor()) {
        if (ed.key == Key_Left) {
            return curSeg->prev1WithElemsOnStaff(staffIdx());
        }
        if (ed.key == Key_Right) {
            return curSeg->next1WithElemsOnStaff(staffIdx());
        }
    }

    if (ed.modifiers & ControlModifier) {
        if (ed.key == Key_Left) {
            Measure* measure = curSeg->rtick().isZero() ? curSeg->measure()->prevMeasure() : curSeg->measure();
            return measure ? measure->findFirstR(SegmentType::ChordRest, Fraction(0, 1)) : nullptr;
        }
        if (ed.key == Key_Right) {
            Measure* measure = curSeg->measure()->nextMeasure();
            return measure ? measure->findFirstR(SegmentType::ChordRest, Fraction(0, 1)) : nullptr;
        }
    }

    if (ed.key == Key_Left) {
        return curSeg->prev1ChordRestOrTimeTick();
    }
    if (ed.key == Key_Right) {
        return curSeg->next1ChordRestOrTimeTick();
    }

    return nullptr;
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

    if (!sys) {
        return nullptr;
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
    System* const oldLineSegSystem = system();
    System* const oldSpannerSystem = oldSeg->measure()->system();

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

    System* lineSegSys = system();
    PointF oldPos = line()->linePos(grip, &lineSegSys);

    if (l->tick() != startTick) {
        l->undoChangeProperty(Pid::SPANNER_TICK, startTick);
        anchorChanged = true;
    }

    const Fraction ticksLength = endTick - startTick;
    if (ticksLength != l->ticks()) {
        l->undoChangeProperty(Pid::SPANNER_TICKS, ticksLength);
        anchorChanged = true;
    }

    const Segment* newTickSeg = left ? l->startSegment() : l->endSegment();
    const System* newSpannerSystem = newTickSeg->measure()->system();

    if ((oldLineSegSystem && newSeg->system() != oldLineSegSystem) || oldSpannerSystem != newSpannerSystem) {
        renderer()->layoutItem(l);
        return l->segmentsEmpty() ? nullptr : left ? l->frontSegment() : l->backSegment();
    } else if (anchorChanged) {
        rebaseOffsetsOnAnchorChanged(grip, oldPos, oldLineSegSystem);
    }

    return nullptr;
}

void LineSegment::rebaseOffsetsOnAnchorChanged(Grip grip, const PointF& oldPos, System* sys)
{
    PointF newPos = line()->linePos(grip, &sys);
    PointF delta = oldPos - newPos;
    if (grip == Grip::START) {
        setOffset(offset() + delta);
        m_offset2 -= delta;
        setOffsetChanged(true);
    } else {
        m_offset2 += delta;
    }
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

    const Segment* startSeg = line()->startSegment();
    const Segment* endSeg = line()->endSegment();
    const double xDistanceFromStartSegment = startSeg && startSeg->system() == system()
                                             ? (startSeg->x() + startSeg->measure()->x()) - ldata()->pos().x() : 0.0;
    const double xDistanceFromEndSegment = endSeg && endSeg->system() == system()
                                           ? (endSeg->x() + endSeg->measure()->x()) - (ldata()->pos().x() + ipos2().x()) : 0.0;

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
        Segment* seg = findSegmentForGrip(grip, ed.pos
                                          + (left ? PointF(xDistanceFromStartSegment, 0.0) : PointF(xDistanceFromEndSegment, 0.0)));
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
        if (!isSingleType()) {
            return;
        }

        System* sys = system();
        if (!sys) {
            return;
        }

        SLine* l = line();

        // If dragging middle grip (or the entire hairpin), mouse position
        // does not directly correspond to any sensible position, so use
        // actual line coordinates instead. This method doesn't allow for
        // system changes, but that seems OK when dragging the entire line:
        // the line will just push away other systems according to autoplacement
        // rules if necessary.
        PointF cpos = canvasPos();
        cpos.setY(system()->staffCanvasYpage(l->staffIdx()));           // prevent cross-system move

        Segment* seg1 = findSegmentForGrip(Grip::START, cpos + PointF(xDistanceFromStartSegment, 0.0));
        Segment* seg2 = findSegmentForGrip(Grip::END, cpos + pos2() + PointF(xDistanceFromEndSegment, 0.0));

        if (!(seg1 && seg2 && seg1->system() == seg2->system() && seg1->system() == system())) {
            return;
        }

        PointF oldStartPos = line()->linePos(Grip::START, &sys);
        PointF oldEndPos = line()->linePos(Grip::END, &sys);

        undoMoveStartEndAndSnappedItems(true, true, seg1, seg2);

        rebaseOffsetsOnAnchorChanged(Grip::START, oldStartPos, sys);
        rebaseOffsetsOnAnchorChanged(Grip::END, oldEndPos, sys);
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
    if (line()->allowTimeAnchor()) {
        EditTimeTickAnchors::updateAnchors(this, ed.curGrip == Grip::START ? line()->track() : line()->track2());
    }
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void LineSegment::spatiumChanged(double ov, double nv)
{
    EngravingItem::spatiumChanged(ov, nv);
    double scale = nv / ov;
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

Spatium LineSegment::lineWidth() const
{
    if (!line()) {
        return Spatium(0.0);
    }

    return line()->lineWidth();
}

double LineSegment::absoluteFromSpatium(const Spatium& sp) const
{
    if (!line()) {
        return EngravingItem::absoluteFromSpatium(sp);
    }

    return line()->absoluteFromSpatium(sp);
}

void LineSegment::undoMoveStartEndAndSnappedItems(bool moveStart, bool moveEnd, Segment* s1, Segment* s2)
{
    SLine* thisLine = line();
    if (moveStart) {
        Fraction tickDiff = s1->tick() - thisLine->tick();
        if (EngravingItem* itemSnappedBefore = ldata()->itemSnappedBefore()) {
            if (itemSnappedBefore->isTextBase()) {
                // Let the TextBase manage the move
                toTextBase(itemSnappedBefore)->undoMoveSegment(s1, tickDiff);
            } else if (itemSnappedBefore->isLineSegment()) {
                toLineSegment(itemSnappedBefore)->line()->undoMoveEnd(tickDiff);
                thisLine->undoMoveStart(tickDiff);
            }
        } else {
            thisLine->undoMoveStart(tickDiff);
        }
    }
    if (moveEnd) {
        Fraction tickDiff = s2->tick() - thisLine->tick2();
        if (EngravingItem* itemSnappedAfter = thisLine->backSegment()->ldata()->itemSnappedAfter()) {
            if (itemSnappedAfter->isTextBase()) {
                // Let the TextBase manage the move
                toTextBase(itemSnappedAfter)->undoMoveSegment(s2, tickDiff);
            } else if (itemSnappedAfter->isLineSegment()) {
                toLineSegment(itemSnappedAfter)->line()->undoMoveStart(tickDiff);
                thisLine->undoMoveEnd(tickDiff);
            }
        } else {
            thisLine->undoMoveEnd(tickDiff);
        }
    }
}

//---------------------------------------------------------
//   SLine
//---------------------------------------------------------

SLine::SLine(const ElementType& type, EngravingItem* parent, ElementFlags f)
    : Spanner(type, parent, f)
{
    setTrack(0);
    m_lineColor = configuration()->defaultColor();
    m_lineWidth = Spatium(0.15);
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
            m_lineWidth = Spatium::fromMM(v.value<Millimetre>(), spatium());
        } else if (v.type() == P_TYPE::SPATIUM) {
            m_lineWidth = v.value<Spatium>();
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
        return Spatium(0.15);
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

void SLine::undoMoveStart(Fraction tickDiff)
{
    Fraction newTick = tick() + tickDiff;
    if (newTick >= Fraction(0, 1)) {
        undoChangeProperty(Pid::SPANNER_TICK, newTick);
    }
    Fraction newDuration = ticks() - tickDiff;
    if (newDuration > Fraction(0, 1)) {
        undoChangeProperty(Pid::SPANNER_TICKS, newDuration);
    }
}

void SLine::undoMoveEnd(Fraction tickDiff)
{
    Fraction newDuration = ticks() + tickDiff;
    if (newDuration > Fraction(0, 1)) {
        undoChangeProperty(Pid::SPANNER_TICKS, newDuration);
    } else {
        Fraction newTick = tick() + tickDiff;
        if (newTick >= Fraction(0, 1)) {
            undoChangeProperty(Pid::SPANNER_TICK, tick() + tickDiff);
        }
    }
}

Note* SLine::guessFinalNote(Note* startNote)
{
    Chord* chord = startNote->chord();
    if (chord->isGraceBefore()) {
        Chord* parentChord = toChord(chord->parent());
        GraceNotesGroup& gracesBefore = parentChord->graceNotesBefore();
        auto positionOfThis = std::find(gracesBefore.begin(), gracesBefore.end(), chord);
        if (positionOfThis != gracesBefore.end()) {
            auto nextPosition = ++positionOfThis;
            if (nextPosition != gracesBefore.end()) {
                return (*nextPosition)->upNote();
            }
        }
        return parentChord->upNote();
    } else if (chord->isGraceAfter()) {
        Chord* parentChord = toChord(chord->parent());
        GraceNotesGroup& gracesAfter = parentChord->graceNotesAfter();
        auto positionOfThis = std::find(gracesAfter.begin(), gracesAfter.end(), chord);
        if (positionOfThis != gracesAfter.end()) {
            auto nextPosition = ++positionOfThis;
            if (nextPosition != gracesAfter.end()) {
                return (*nextPosition)->upNote();
            }
        }
        chord = toChord(chord->parent());
    } else {
        std::vector<Chord*> graces = chord->graceNotesAfter();
        if (graces.size() > 0) {
            return graces.front()->upNote();
        }
    }

    if (!chord->explicitParent()->isSegment()) {
        return 0;
    }

    Segment* segm = chord->score()->tick2rightSegment(chord->tick() + chord->actualTicks());
    while (segm && !segm->isChordRestType()) {
        segm = segm->next1();
    }

    if (!segm) {
        return nullptr;
    }

    track_idx_t chordTrack = chord->track();
    Part* part = chord->part();

    Chord* target = nullptr;
    if (segm->element(chordTrack) && segm->element(chordTrack)->isChord()) {
        target = toChord(segm->element(chordTrack));
    } else {
        for (EngravingItem* currChord : segm->elist()) {
            if (currChord && currChord->isChord() && toChord(currChord)->part() == part) {
                target = toChord(currChord);
                break;
            }
        }
    }

    if (target && target->notes().size() > 0) {
        const std::vector<Chord*>& graces = target->graceNotesBefore();
        if (graces.size() > 0) {
            return graces.front()->upNote();
        }
        // normal case: try to return the note in the next chord that is in the
        // same position as the start note relative to the end chord
        size_t startNoteIdx = muse::indexOf(chord->notes(), startNote);
        size_t endNoteIdx = std::min(startNoteIdx, target->notes().size() - 1);
        return target->notes().at(endNoteIdx);
    }

    LOGD("no second note for note anchored line found");
    return nullptr;
}
}
