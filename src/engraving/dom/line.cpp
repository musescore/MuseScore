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

#include "line.h"

#include <vector>

#include "containers.h"

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

    System* s;
    result = line()->linePos(Grip::START, &s);
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
    if (isMiddleType() || isBeginType()) {
        return PointF(system()->endingXForOpenEndedLines(), systemPositionY);
    }

    PointF result;

    System* s;
    result = line()->linePos(Grip::END, &s);
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
        if (staffType()) {
            y += staffType()->yoffset().val() * spatium();
        }
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
    if (!isEditAllowed(ed)) {
        return false;
    }

    const bool moveStart = ed.curGrip == Grip::START;
    const bool moveEnd = ed.curGrip == Grip::END || ed.curGrip == Grip::MIDDLE;

    LineSegment* ls       = 0;
    SpannerSegmentType st = spannerSegmentType();   // may change later
    SLine* l              = line();
    track_idx_t track = l->track();
    track_idx_t track2 = l->track2();      // assumed to be same as track

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
        if (ed.key == Key_Left) {
            if (moveStart) {
                s1 = prevSeg1(s1, track);
            } else if (moveEnd) {
                s2 = prevSeg1(s2, track2);
            }
        } else if (ed.key == Key_Right) {
            if (moveStart) {
                s1 = nextSeg1(s1, track);
            } else if (moveEnd) {
                Segment* ns2 = nextSeg1(s2, track2);
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

    const double spacingFactor = left ? 0.5 : 1.0;   // defines the point where canvas is divided between segments, systems etc.

    System* sys = system();
    const std::vector<System*> foundSystems = score()->searchSystem(pos, sys, spacingFactor);

    if (!foundSystems.empty() && !mu::contains(foundSystems, sys) && foundSystems[0]->staves().size()) {
        sys = foundSystems[0];
    }

    // Restrict searching segment to the correct staff
    pos.setY(sys->staffCanvasYpage(oldStaffIndex));

    Segment* seg = nullptr;   // don't prefer any segment while searching line position
    staff_idx_t staffIndex = oldStaffIndex;
    score()->dragPosition(pos, &staffIndex, &seg, spacingFactor);

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

PointF LineSegment::deltaRebaseRight(const Segment* oldSeg, const Segment* newSeg, staff_idx_t staffIndex)
{
    if (oldSeg == newSeg) {
        return PointF();
    }

    const PointF oldBase = oldSeg->canvasPos() + PointF(oldSeg->width(), 0);
    const PointF newBase = newSeg->canvasPos() + PointF(newSeg->widthInStaff(staffIndex), 0);
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
    Segment* const oldSeg = left ? l->startSegment() : score()->tick2leftSegmentMM(l->tick2() - Fraction::eps());
    System* const oldSystem = system();

    if (!newSeg || oldSeg == newSeg) {
        return nullptr;
    }

    Fraction startTick = left ? newSeg->tick() : l->tick();
    Fraction endTick = left ? l->tick2() : lastSegmentEndTick(newSeg, l);

    if (endTick <= startTick) {
        if (left) {
            endTick = lastSegmentEndTick(newSeg, l);
        } else {
            startTick = newSeg->tick();
        }
    }

    bool anchorChanged = false;

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
        const PointF delta = left ? deltaRebaseLeft(oldSeg, newSeg) : deltaRebaseRight(oldSeg, newSeg, track2staff(l->effectiveTrack2()));
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
        if (!isSingleType()) {
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

        Segment* seg1 = findSegmentForGrip(Grip::START, cpos);
        Segment* seg2 = findSegmentForGrip(Grip::END, cpos + pos2());

        if (!(seg1 && seg2 && seg1->system() == seg2->system() && seg1->system() == system())) {
            return;
        }

        rebaseAnchor(Grip::START, seg1);
        rebaseAnchor(Grip::END, seg2);
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
    triggerLayout();
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

//---------------------------------------------------------
//   linePos
///   - Anchor::NOTE:  return anchor note position in system
///                    coordinates
///   - Anchor::CHORD: not implemented
///   - Other:         return (x position in system coordinates, 0)
//---------------------------------------------------------

PointF SLine::linePos(Grip grip, System** sys) const
{
    double x = 0.0;
    double sp = staff() ? staff()->spatium(tick()) : 0;
    switch (anchor()) {
    case Spanner::Anchor::SEGMENT:
    {
        ChordRest* cr;
        if (grip == Grip::START) {
            cr = (startElement() && startElement()->isChordRest()) ? toChordRest(startElement()) : nullptr;
            if (cr && type() == ElementType::OTTAVA) {
                // some sources say to center the text over the notehead
                // others say to start the text just to left of notehead
                // some say to include accidental, others don't
                // our compromise - left align, but account for accidental
                if (cr->durationType() == DurationType::V_MEASURE && !cr->measure()->hasVoices(cr->staffIdx())) {
                    x = cr->x();                            // center for measure rests
                }
//TODO                              else if (cr->spaceLw > 0.0)
//                                    x = -cr->spaceLw;  // account for accidentals, etc
            }
        } else {
            cr = (endElement() && endElement()->isChordRest()) ? toChordRest(endElement()) : nullptr;
            if (isOttava()) {
                if (cr && cr->durationType() == DurationType::V_MEASURE) {
                    x = cr->x() + cr->width() + sp;
                } else if (cr) {
                    // lay out just past right edge of all notes for this segment on this staff

                    Segment* s = cr->segment();

                    track_idx_t startTrack = staffIdx() * VOICES;
                    track_idx_t endTrack   = startTrack + VOICES;
                    double width    = 0.0;

                    // don’t consider full measure rests, which are centered
                    // (TODO: what if there is only a full measure rest?)

                    for (track_idx_t track = startTrack; track < endTrack; ++track) {
                        ChordRest* cr1 = toChordRest(s->element(track));
                        if (!cr1) {
                            continue;
                        }
                        if (cr1->isChord()) {
                            for (Note* n : toChord(cr1)->notes()) {
                                width = std::max(width, n->shape().right() + n->pos().x() + cr1->pos().x());
                            }
                        } else if (cr1->isRest() && (cr1->actualDurationType() != DurationType::V_MEASURE)) {
                            width = std::max(width, cr1->ldata()->bbox().right() + cr1->pos().x());
                        }
                    }

                    x = width + sp;

                    // extend past chord/rest
                    // but don't overlap next chord/rest

                    bool crFound = false;
                    track_idx_t n = staffIdx() * VOICES;
                    Segment* ns = s->next();
                    while (ns) {
                        for (voice_idx_t i = 0; i < VOICES; ++i) {
                            if (ns->element(n + i)) {
                                crFound = true;
                                break;
                            }
                        }
                        if (crFound) {
                            break;
                        }
                        ns = ns->next();
                    }
                    if (crFound) {
                        double nextNoteDistance = ns->x() - s->x() + lineWidth();
                        if (x > nextNoteDistance) {
                            x = std::max(width, nextNoteDistance);
                        }
                    }
                }
            } else if (isLyricsLine() && explicitParent() && toLyrics(explicitParent())->ticks() > Fraction(0, 1)) {
                // melisma line
                // it is possible CR won't be in correct track
                // prefer element in current track if available
                if (!cr) {
                    LOGD("no end for lyricsline segment - start %d, ticks %d", tick().ticks(), ticks().ticks());
                } else if (cr->track() != track()) {
                    EngravingItem* e = cr->segment()->element(track());
                    if (e) {
                        cr = toChordRest(e);
                    }
                }

                // layout to right edge of CR
                // except if CR is start element, in which case use a nominal length
                if (cr && cr != toChordRest(startElement())) {
                    x = cr->rightEdge();
                } else {
                    x = spatium() - style().styleMM(Sid::minNoteDistance);
                }
            } else if (isHairpin() || isTrill() || isVibrato() || isTextLine() || isLyricsLine() || isGradualTempoChange()) {
                // (for LYRICSLINE, this is hyphen; melisma line is handled above)
                // lay out to just before next chordrest on this staff, or barline
                // tick2 actually tells us the right chordrest to look for
                if (cr && endElement()->explicitParent() && endElement()->explicitParent()->type() == ElementType::SEGMENT) {
                    double x2 = cr->x() /* TODO + cr->space().rw() */;
                    Segment* currentSeg = toSegment(endElement()->explicitParent());
                    Segment* seg = score()->tick2segmentMM(tick2(), false, SegmentType::ChordRest);
                    if (!seg) {
                        // no end segment found, use measure width
                        x2 = endElement()->parentItem()->parentItem()->width() - sp;
                    } else if (currentSeg->measure() == seg->measure()) {
                        // next chordrest found in same measure;
                        // end line 1sp to left
                        x2 = std::max(x2, seg->x() - sp);
                    } else {
                        // next chordrest is in next measure
                        // lay out to end (barline) of current measure instead
                        seg = currentSeg->next(SegmentType::EndBarLine);
                        if (!seg) {
                            seg = currentSeg->measure()->last();
                        }
                        // allow lyrics hyphen to extend to barline
                        // other lines stop 1sp short
                        double gap = (type() == ElementType::LYRICSLINE) ? 0.0 : sp;
                        double x3 = seg->enabled() ? seg->x() : seg->measure()->width();
                        x2 = std::max(x2, x3 - gap);
                    }
                    x = x2 - endElement()->parentItem()->x();
                }
            }
        }

        Measure* m = nullptr;
        if (cr) {
            m = cr->measure();
            x += cr->segment()->pos().x() + m->pos().x();
        } else {
            Segment* segment = (grip == Grip::START) ? startSegment() : endSegment();
            if (grip == Grip::END && segment && segment->rtick().ticks() == 0) {
                // The line should end on the left-most segment at this tick. If endSegment() is the first of
                // the measure, we need to look back for other segments (eg endBarLine) in the prev measure
                Segment* prevSegment = segment->prev1MM();
                while (prevSegment && prevSegment->tick() == segment->tick()) {
                    segment = prevSegment;
                    prevSegment = segment->prev1MM();
                }
            }
            if (segment) {
                m = segment->measure();
                if (m->isMMRest() && m->mmRest()) {
                    m = m->mmRest();
                }
                x += m->pos().x() + segment->pos().x() - (grip == Grip::START ? 0 : sp);
            }
        }

        if (m) {
            *sys = m->system();
        } else {
            *sys = 0;
        }
    }
    break;

    case Spanner::Anchor::MEASURE:
    {
        // anchor() == Anchor::MEASURE
        const Measure* m;
        if (grip == Grip::START) {
            m = startMeasure();
            // start after clef/keysig/timesig/barline
            double offset = 0.0;
            Segment* s = m->first(SegmentType::ChordRest);
            if (s) {
                s = s->prev();
                if (s && s->enabled()) {
                    offset = s->x();
                    EngravingItem* e = s->element(staffIdx() * VOICES);
                    if (e) {
                        offset += e->width();
                    }
                }
            }
            x = m->pos().x() + offset;
            if (style().styleB(Sid::createMultiMeasureRests) && m->hasMMRest()) {
                x = m->mmRest()->pos().x();
            }
        } else {
            double _spatium = spatium();

            if (style().styleB(Sid::createMultiMeasureRests)) {
                // find the actual measure where the volta should stop
                m = startMeasure();
                if (m->hasMMRest()) {
                    m = m->mmRest();
                }
                while (m->nextMeasureMM() && (m->endTick() < tick2())) {
                    m = m->nextMeasureMM();
                }
            } else {
                m = endMeasure();
            }

            // back up to barline (skip courtesy elements)
            Segment* seg = m->last();
            while (seg && seg->segmentType() != SegmentType::EndBarLine) {
                seg = seg->prev();
            }
            if (!seg || !seg->enabled()) {
                // no end bar line; look for BeginBarLine or StartRepeatBarLine of next measure
                Measure* nm = m->nextMeasure();
                if (nm->system() == m->system()) {
                    seg = nm->first(SegmentType::BeginBarLine | SegmentType::StartRepeatBarLine);
                }
            }
            double mwidth = seg && seg->measure() == m ? seg->x() : m->ldata()->bbox().right();
            x = m->pos().x() + mwidth;
            // align to barline
            if (seg && (seg->segmentType() & SegmentType::BarLineType)) {
                EngravingItem* e = seg->element(0);
                if (e && e->type() == ElementType::BAR_LINE) {
                    BarLineType blt = toBarLine(e)->barLineType();
                    switch (blt) {
                    case BarLineType::END_REPEAT:
                        // skip dots
                        x += symWidth(SymId::repeatDot);
                        x += style().styleS(Sid::endBarDistance).val() * _spatium;
                    // fall through
                    case BarLineType::DOUBLE:
                        // center on leftmost (thinner) barline
                        x += style().styleS(Sid::doubleBarWidth).val() * _spatium * 0.5;
                        break;
                    case BarLineType::START_REPEAT:
                        // center on leftmost (thicker) barline
                        x += style().styleS(Sid::endBarWidth).val() * _spatium * 0.5;
                        break;
                    default:
                        // center on barline
                        x += style().styleS(Sid::barWidth).val() * _spatium * 0.5;
                        break;
                    }
                }
            }
        }

        m = m->coveringMMRestOrThis();

        assert(m->system());
        *sys = m->system();
    }
    break;

    case Spanner::Anchor::NOTE: {
        EngravingItem* e = grip == Grip::START ? startElement() : endElement();
        if (!e) {
            return PointF();
        }
        Note* n = toNote(e);
        System* s = n->chord()->segment()->system();
        if (s == 0) {
            LOGD("no system: %s  start %s chord parent %s\n", typeName(), n->typeName(), n->chord()->explicitParent()->typeName());
            return PointF();
        }
        *sys = s;
        // return the position of the anchor note relative to the system
//                  PointF     elemPagePos = e->pagePos();                   // DEBUG
//                  PointF     systPagePos = s->pagePos();
//                  double       staffYPage  = s->staffYpage(e->staffIdx());
        PointF p = n->pagePos() - s->pagePos();
        if (!isGlissando()) {
            p.rx() += n->headWidth() * 0.5;
        }
        return p;
    }

    case Spanner::Anchor::CHORD:
        ASSERT_X("Sline::linePos(): anchor not implemented");
        break;
    }
    return PointF(x, 0.0);
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
        m_lineColor = v.value<mu::draw::Color>();
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
        return PropertyValue::fromValue(engravingConfiguration()->defaultColor());
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
