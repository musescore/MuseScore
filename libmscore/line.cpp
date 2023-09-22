//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "line.h"

#include "barline.h"
#include "chord.h"
#include "lyrics.h"
#include "measure.h"
#include "note.h"
#include "part.h"
#include "page.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "sym.h"
#include "system.h"
#include "textline.h"
#include "utils.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   LineSegment
//---------------------------------------------------------

LineSegment::LineSegment(const LineSegment& s)
   : SpannerSegment(s)
      {
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool LineSegment::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());
      if (tag == "subtype")
            setSpannerSegmentType(SpannerSegmentType(e.readInt()));
      else if (tag == "off2") {
            setUserOff2(e.readPoint() * score()->spatium());
            }
/*      else if (tag == "pos") {
            setOffset(QPointF());
            e.readNext();
            }
      */
      else if (!SpannerSegment::readProperties(e)) {
            e.unknown();
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void LineSegment::read(XmlReader& e)
      {
      while (e.readNextStartElement())
            readProperties(e);
      }

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<QPointF> LineSegment::gripsPositions(const EditData&) const
      {
      std::vector<QPointF> grips(gripsCount());
      QPointF pp(pagePos());
      grips[int(Grip::START)] = pp;
      grips[int(Grip::END)] = pos2() + pp;
      grips[int(Grip::MIDDLE)] = pos2() * .5 + pp;
      return grips;
      }

//---------------------------------------------------------
//   leftAnchorPosition
//---------------------------------------------------------

QPointF LineSegment::leftAnchorPosition(const qreal& systemPositionY) const
      {

      if (isMiddleType() || isEndType())
            return QPointF(system()->firstMeasure()->abbox().left(), systemPositionY);

      QPointF result;

      System* s;
      result = line()->linePos(Grip::START, &s);
      result.ry() += systemPositionY - system()->pos().y();

      if (s)
            result += s->pos();  // to page coordinates

      return result;
      }

//---------------------------------------------------------
//   rightAnchorPosition
//---------------------------------------------------------

QPointF LineSegment::rightAnchorPosition(const qreal& systemPositionY) const
    {

    if (isMiddleType() || isBeginType())
          return QPointF(system()->lastNoteRestSegmentX(true), systemPositionY);

    QPointF result;

    System* s;
    result = line()->linePos(Grip::END, &s);
    result.ry() += systemPositionY - system()->pos().y();

    if (s)
          result += s->pos();  // to page coordinates

    return result;
    }

//---------------------------------------------------------
//   gripAnchorLines
//    return page coordinates
//---------------------------------------------------------

QVector<QLineF> LineSegment::gripAnchorLines(Grip grip) const
      {
      QVector<QLineF> result;

      // Middle or aperture grip have no anchor
      if (!system() || grip == Grip::APERTURE)
            return result;

      // note-anchored spanners are relative to the system
      qreal y;
      if (spanner()->anchor() == Spanner::Anchor::NOTE)
            y = system()->pos().y();
      else {
            const int stIdx = staffIdx();
            y = system()->staffYpage(stIdx);
            if (line()->placement() == Placement::BELOW)
                  y += system()->staff(stIdx)->bbox().height();
            // adjust Y to staffType offset
            if (staffType())
                  y += staffType()->yoffset().val() * spatium();
            }

      const Page* p = system()->page();
      const QPointF pageOffset = p ? p->pos() : QPointF();

      switch (grip) {
      case Grip::START:
            result << QLineF(leftAnchorPosition(y), gripsPositions().at(static_cast<int>(Grip::START))).translated(pageOffset);
            break;
      case Grip::END:
            result << QLineF(rightAnchorPosition(y), gripsPositions().at(static_cast<int>(Grip::END))).translated(pageOffset);
            break;
      case Grip::MIDDLE:
            result << QLineF(leftAnchorPosition(y), gripsPositions().at(static_cast<int>(Grip::START))).translated(pageOffset);
            result << QLineF(rightAnchorPosition(y), gripsPositions().at(static_cast<int>(Grip::END))).translated(pageOffset);
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
      ElementEditData* eed = ed.getData(this);
      if (eed)
            eed->pushProperty(Pid::OFFSET2);
      }

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void LineSegment::startEditDrag(EditData& ed)
      {
      ElementEditData* eed = ed.getData(this);
      eed->pushProperty(Pid::OFFSET);
      eed->pushProperty(Pid::OFFSET2);
      eed->pushProperty(Pid::AUTOPLACE);
      if (ed.modifiers & Qt::AltModifier)
            setAutoplace(false);
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool LineSegment::edit(EditData& ed)
      {
      const bool moveStart = ed.curGrip == Grip::START;
      const bool moveEnd = ed.curGrip == Grip::END || ed.curGrip == Grip::MIDDLE;

      if (!((ed.modifiers & Qt::ShiftModifier)
         && ((isSingleBeginType() && moveStart) || (isSingleEndType() && moveEnd))
         ))
            return false;

      LineSegment* ls       = 0;
      SpannerSegmentType st = spannerSegmentType(); // may change later
      SLine* l              = line();
      int track             = l->track();
      int track2            = l->track2();    // assumed to be same as track

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
                        qDebug("LineSegment::edit: no start/end segment");
                        return true;
                        }
                  if (ed.key == Qt::Key_Left) {
                        if (moveStart)
                              s1 = prevSeg1(s1, track);
                        else if (moveEnd)
                              s2 = prevSeg1(s2, track2);
                        }
                  else if (ed.key == Qt::Key_Right) {
                        if (moveStart)
                              s1 = nextSeg1(s1, track);
                        else if (moveEnd) {
                              Segment* ns2 = nextSeg1(s2, track2);
                              if (ns2)
                                    s2 = ns2;
                              else
                                    s2 = score()->lastSegment();
                              }
                        }
                  if (s1 == 0 || s2 == 0 || s1->tick() >= s2->tick())
                        return true;
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
                        qDebug("LineSegment::edit: no start/end note");
                        return true;            // accept the event without doing anything
                        }

                  switch (ed.key) {
                        case Qt::Key_Left:
                              if (moveStart)
                                    note1 = prevChordNote(note1);
                              else if (moveEnd)
                                    note2 = prevChordNote(note2);
                              break;
                        case Qt::Key_Right:
                              if (moveStart)
                                    note1 = nextChordNote(note1);
                              else if (moveEnd)
                                    note2 = nextChordNote(note2);
                              break;
                        case Qt::Key_Up:
                              if (moveStart)
                                    note1 = toNote(score()->upAlt(note1));
                              else if (moveEnd)
                                    note2 = toNote(score()->upAlt(note2));
                              break;
                        case Qt::Key_Down:
                              if (moveStart)
                                    note1 = toNote(score()->downAlt(note1));
                              else if (moveEnd)
                                    note2 = toNote(score()->downAlt(note2));
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
                     != note2->chord()->staff()->part()->instrument(note2->chord()->tick()) )
                        return true;
                  if (note1 != oldNote1 || note2 != oldNote2)
                        score()->undoChangeSpannerElements(spanner(), note1, note2);
                  }
                  break;
            case Spanner::Anchor::MEASURE:
            case Spanner::Anchor::CHORD:
                  {
                  Measure* m1 = l->startMeasure();
                  Measure* m2 = l->endMeasure();

                  if (ed.key == Qt::Key_Left) {
                        if (moveStart) {
                              if (m1->prevMeasure())
                                    m1 = m1->prevMeasure();
                              }
                        else if (moveEnd) {
                              Measure* m = m2->prevMeasure();
                              if (m)
                                    m2 = m;
                              }
                        }
                  else if (ed.key == Qt::Key_Right) {
                        if (moveStart) {
                              if (m1->nextMeasure())
                                    m1 = m1->nextMeasure();
                              }
                        else if (moveEnd) {
                              if (m2->nextMeasure())
                                    m2 = m2->nextMeasure();
                              }
                        }
                  if (m1->tick() > m2->tick())
                        return true;
                  if (l->startElement() != m1) {
                        spanner()->undoChangeProperty(Pid::SPANNER_TICK,  m1->tick());
                        spanner()->undoChangeProperty(Pid::SPANNER_TICKS, m2->endTick() - m1->tick());
                        }
                  else if (l->endElement() != m2) {
                        spanner()->undoChangeProperty(Pid::SPANNER_TICKS, m2->endTick() - m1->tick());
                        }
                  }
            }
      triggerLayout();
      l->layout();            // recompute segment list, segment type may change

      LineSegment* nls = 0;
      if (st == SpannerSegmentType::SINGLE) {
            if (moveStart)
                  nls = l->frontSegment();
            else if (moveEnd)
                  nls = l->backSegment();
            }
      else if (st == SpannerSegmentType::BEGIN)
            nls = l->frontSegment();
      else if (st == SpannerSegmentType::END)
            nls = l->backSegment();
      else
            qDebug("spannerSegmentType %d", int(spannerSegmentType()));

      if (nls && (nls != this))
            ed.view->changeEditElement(nls);
      if (ls)
            score()->undoRemoveElement(ls);

      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   findSegmentForGrip
//---------------------------------------------------------

Segment* LineSegment::findSegmentForGrip(Grip grip, QPointF pos) const
      {
      if (grip != Grip::START && grip != Grip::END)
            return nullptr;

      SLine* l = line();
      const bool left = (grip == Grip::START);

      Segment* const oldSeg = left ? l->startSegment() : score()->tick2leftSegmentMM(l->tick2() - Fraction::eps());
      const int oldStaffIndex = left ? staffIdx() : track2staff(l->effectiveTrack2());

      const qreal spacingFactor = left ? 0.5 : 1.0; // defines the point where canvas is divided between segments, systems etc.

      System* sys = oldSeg->system();
      const QList<System*> foundSystems = score()->searchSystem(pos, sys, spacingFactor);

      if (!foundSystems.empty() && !foundSystems.contains(sys) && foundSystems[0]->staves()->size())
            sys = foundSystems[0];

      // Restrict searching segment to the correct staff
      pos.setY(sys->staffCanvasYpage(oldStaffIndex));

      Segment* seg = nullptr; // don't prefer any segment while searching line position
      int staffIndex = oldStaffIndex;
      score()->dragPosition(pos, &staffIndex, &seg, spacingFactor);

      return seg;
      }

//---------------------------------------------------------
//   deltaRebaseLeft
///   Helper function for anchors rebasing when dragging.
//---------------------------------------------------------

QPointF LineSegment::deltaRebaseLeft(const Segment* oldSeg, const Segment* newSeg)
      {
      if (oldSeg == newSeg)
            return QPointF();
      return oldSeg->canvasPos() - newSeg->canvasPos();
      }

//---------------------------------------------------------
//   deltaRebaseRight
///   Helper function for anchors rebasing when dragging.
//---------------------------------------------------------

QPointF LineSegment::deltaRebaseRight(const Segment* oldSeg, const Segment* newSeg, int staffIndex)
      {
      if (oldSeg == newSeg)
            return QPointF();

      const QPointF oldBase = oldSeg->canvasPos() + QPointF(oldSeg->width(), 0);
      const QPointF newBase = newSeg->canvasPos() + QPointF(newSeg->widthInStaff(staffIndex), 0);
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
                  if (!isSingleBeginType())
                        return nullptr;
                  break;
            case Grip::END:
                  if (!isSingleEndType())
                        return nullptr;
                  break;
            default:
                  return nullptr;
            }

      SLine* l = line();
      const bool left = (grip == Grip::START);
      Segment* const oldSeg = left ? l->startSegment() : score()->tick2leftSegmentMM(l->tick2() - Fraction::eps());
      System* const oldSystem = system();

      if (!newSeg || oldSeg == newSeg)
            return nullptr;

      Fraction startTick = left ? newSeg->tick() : l->tick();
      Fraction endTick = left ? l->tick2() : lastSegmentEndTick(newSeg, l);

      if (endTick <= startTick) {
            if (left)
                  endTick = lastSegmentEndTick(newSeg, l);
            else
                  startTick = newSeg->tick();
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
            l->layout();
            return left ? l->frontSegment() : l->backSegment();
            }
      else if (anchorChanged) {
            const QPointF delta = left ? deltaRebaseLeft(oldSeg, newSeg) : deltaRebaseRight(oldSeg, newSeg, track2staff(l->effectiveTrack2()));
            if (left) {
                  setOffset(offset() + delta);
                  _offset2 -= delta;
                  setOffsetChanged(true);
                  }
            else {
                  _offset2 += delta;
                  }
            }

      return nullptr;
      }

//---------------------------------------------------------
//   rebaseAnchors
//---------------------------------------------------------

void LineSegment::rebaseAnchors(EditData& ed, Grip grip)
      {
      if (line()->anchor() != Spanner::Anchor::SEGMENT)
            return;
      // don't change anchors on keyboard adjustment or if Ctrl is pressed
      // (Ctrl+Left/Right is handled elsewhere!)
      if (ed.key == Qt::Key_Left || ed.key == Qt::Key_Right || ed.modifiers & Qt::ControlModifier)
            return;

      switch (grip) {
            case Grip::START:
            case Grip::END: {
                  const bool left = (grip == Grip::START);

                  if (left && !isSingleBeginType())
                        return;
                  if (!left && !isSingleEndType())
                        return;

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
                                    _offset2.rx() -= offset().x();
                                    setOffset(QPointF());
                                    }
                              else {
                                    setUserOff2(QPointF());
                                    }

                              // Switch to dragging the new line segment
                              ed.view->changeEditElement(newLineSegment);
                              }

                        // Set offset for the new line segment for grip to appear under the mouse cursor
                        System* sys;
                        const QPointF lp = line()->linePos(grip, &sys);
                        const qreal xoff = sys->mapFromCanvas(ed.pos).x() - lp.x();

                        if (left) {
                              newLineSegment->rxoffset() = xoff;
                              newLineSegment->setUserOff2(QPointF(-xoff, 0.0));
                              }
                        else {
                              newLineSegment->setUserXoffset2(xoff);
                              }
                        }
                  }
                  break;
            case Grip::MIDDLE: {
                  if (!isSingleType())
                        return;

                  SLine* l = line();

                  // If dragging middle grip (or the entire hairpin), mouse position
                  // does not directly correspond to any sensible position, so use
                  // actual line coordinates instead. This method doesn't allow for
                  // system changes, but that seems OK when dragging the entire line:
                  // the line will just push away other systems according to autoplacement
                  // rules if necessary.
                  QPointF cpos = canvasPos();
                  cpos.setY(l->startSegment()->system()->staffCanvasYpage(l->staffIdx())); // prevent cross-system move

                  Segment* seg1 = findSegmentForGrip(Grip::START, cpos);
                  Segment* seg2 = findSegmentForGrip(Grip::END, cpos + pos2());

                  if (!(seg1 && seg2 && seg1->system() == seg2->system() && seg1->system() == system()))
                        return;

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
      const QPointF deltaResize(ed.evtDelta.x(), line()->diagonal() ? ed.evtDelta.y() : 0.0);

      switch (ed.curGrip) {
            case Grip::START: // Resize the begin of element (left grip)
                  setOffset(offset() + deltaResize);
                  _offset2 -= deltaResize;

                  if (isStyled(Pid::OFFSET))
                        setPropertyFlags(Pid::OFFSET, PropertyFlags::UNSTYLED);

                  rebaseAnchors(ed, ed.curGrip);
                  break;
            case Grip::END: // Resize the end of element (right grip)
                  _offset2 += deltaResize;
                  rebaseAnchors(ed, ed.curGrip);
                  break;
            case Grip::MIDDLE: { // Move the element (middle grip)
                  // Only for moving, no y limitation
                  const QPointF deltaMove(ed.evtDelta);
                  setOffset(offset() + deltaMove);
                  setOffsetChanged(true);
                  if (isStyled(Pid::OFFSET))
                        setPropertyFlags(Pid::OFFSET, PropertyFlags::UNSTYLED);
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
            Element* e = ed.view->elementNear(ed.pos);
            if (e && e->isNote()) {
                  SLine* l = line();
                  if (ed.curGrip == Grip::END && e != line()->endElement()) {
                        qDebug("LineSegment: move end anchor");
                        Note* noteOld = toNote(l->endElement());
                        Note* noteNew = toNote(e);
                        Note* sNote   = toNote(l->startElement());
                        // do not change anchor if new note is before start note
                        if (sNote && sNote->chord() && noteNew->chord() && sNote->chord()->tick() < noteNew->chord()->tick()) {
                              score()->undoChangeSpannerElements(l, sNote, noteNew);

                              _offset2 += noteOld->canvasPos() - noteNew->canvasPos();
                              }
                        }
                  else if (ed.curGrip == Grip::START && e != l->startElement())
                        qDebug("LineSegment: move start anchor (not impl.)");
                  }
            }
      triggerLayout();
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void LineSegment::spatiumChanged(qreal ov, qreal nv)
      {
      Element::spatiumChanged(ov, nv);
      qreal scale = nv / ov;
      line()->setLineWidth(line()->lineWidth() * scale);
      _offset2 *= scale;
      }

//---------------------------------------------------------
//   localSpatiumChanged
//---------------------------------------------------------

void LineSegment::localSpatiumChanged(qreal ov, qreal nv)
      {
      Element::localSpatiumChanged(ov, nv);
      _offset2 *= nv / ov;
      }

//---------------------------------------------------------
//   propertyDelegate
//---------------------------------------------------------

Element* LineSegment::propertyDelegate(Pid pid)
      {
      if (pid == Pid::DIAGONAL
         || pid == Pid::COLOR
         || pid ==   Pid::LINE_WIDTH
         || pid ==   Pid::LINE_STYLE
         || pid ==   Pid::DASH_LINE_LEN
         || pid ==   Pid::DASH_GAP_LEN)
            return spanner();
      return SpannerSegment::propertyDelegate(pid);
      }

//---------------------------------------------------------
//   dragAnchorLines
//---------------------------------------------------------

QVector<QLineF> LineSegment::dragAnchorLines() const
      {
      return gripAnchorLines(Grip::MIDDLE);
      }

QRectF LineSegment::drag(EditData& ed)
      {
      setOffset(offset() + ed.evtDelta);
      setOffsetChanged(true);

      if (isStyled(Pid::OFFSET))
            setPropertyFlags(Pid::OFFSET, PropertyFlags::UNSTYLED);

      rebaseAnchors(ed, Grip::MIDDLE);

      return canvasBoundingRect();
      }

//---------------------------------------------------------
//   SLine
//---------------------------------------------------------

SLine::SLine(Score* s, ElementFlags f)
   : Spanner(s, f)
      {
      setTrack(0);
      _lineWidth = 0.15 * spatium();
      }

SLine::SLine(const SLine& s)
   : Spanner(s)
      {
      _diagonal    = s._diagonal;
      _lineWidth   = s._lineWidth;
      _lineColor   = s._lineColor;
      _lineStyle   = s._lineStyle;
      _dashLineLen = s._dashLineLen;
      _dashGapLen  = s._dashGapLen;
      }

//---------------------------------------------------------
//   linePos
///   - Anchor::NOTE:  return anchor note position in system
///                    coordinates
///   - Anchor::CHORD: not implemented
///   - Other:         return (x position in system coordinates, 0)
//---------------------------------------------------------

QPointF SLine::linePos(Grip grip, System** sys) const
      {
      qreal x = 0.0;
      qreal sp = staff() ? staff()->spatium(tick()) : 0.0;
      switch (anchor()) {
            case Spanner::Anchor::SEGMENT:
                  {
                  ChordRest* cr;
                  if (grip == Grip::START) {
                        cr = toChordRest(startElement());
                        if (cr && type() == ElementType::OTTAVA) {
                              // some sources say to center the text over the notehead
                              // others say to start the text just to left of notehead
                              // some say to include accidental, others don't
                              // our compromise - left align, but account for accidental
                              if (cr->durationType() == TDuration::DurationType::V_MEASURE && !cr->measure()->hasVoices(cr->staffIdx()))
                                    x = cr->x();            // center for measure rests
//TODO                              else if (cr->spaceLw > 0.0)
//                                    x = -cr->spaceLw;  // account for accidentals, etc
                              }
                        }
                  else {
                        cr = toChordRest(endElement());
                        if (isOttava()) {
                              if (cr && cr->durationType() == TDuration::DurationType::V_MEASURE) {
                                    x = cr->x() + cr->width() + sp;
                                    }
                              else if (cr) {
                                    // lay out just past right edge of all notes for this segment on this staff

                                    Segment* s = cr->segment();

                                    int startTrack = staffIdx() * VOICES;
                                    int endTrack   = startTrack + VOICES;
                                    qreal width    = 0.0;

                                    // don’t consider full measure rests, which are centered
                                    // (TODO: what if there is only a full measure rest?)

                                    for (int track = startTrack; track < endTrack; ++track) {
                                          ChordRest* cr1 = toChordRest(s->element(track));
                                          if (!cr1)
                                                continue;
                                          if (cr1->isChord()) {
                                                for (Note* n : toChord(cr1)->notes())
                                                      width = qMax(width, n->shape().right() + n->pos().x() + cr1->pos().x());
                                                }
                                          else if (cr1->isRest() && (cr1->actualDurationType() != TDuration::DurationType::V_MEASURE))
                                                width = qMax(width, cr1->bbox().right() + cr1->pos().x());
                                          }

                                    x = width + sp;

                                    // extend past chord/rest
                                    // but don't overlap next chord/rest

                                    bool crFound = false;
                                    int n = staffIdx() * VOICES;
                                    Segment* ns = s->next();
                                    while (ns) {
                                          for (int i = 0; i < VOICES; ++i) {
                                                if (ns->element(n + i)) {
                                                      crFound = true;
                                                      break;
                                                      }
                                                }
                                          if (crFound)
                                                break;
                                          ns = ns->next();
                                          }
                                    if (crFound) {
                                          qreal nextNoteDistance = ns->x() - s->x() + lineWidth();
                                          if (x > nextNoteDistance)
                                                x = qMax(width, nextNoteDistance);
                                          }
                                    }
                              }
                        else if (isLyricsLine() && toLyrics(parent())->ticks() > Fraction(0,1)) {
                              // melisma line
                              // it is possible CR won't be in correct track
                              // prefer element in current track if available
                              if (!cr)
                                    qDebug("no end for lyricsline segment - start %d, ticks %d", tick().ticks(), ticks().ticks());
                              else if (cr->track() != track()) {
                                    Element* e = cr->segment()->element(track());
                                    if (e)
                                          cr = toChordRest(e);
                                    }

                              // layout to right edge of CR
                              // except if CR is start element, in which case use a nominal length
                              if (cr && cr != toChordRest(startElement()))
                                    x = cr->rightEdge();
                              else
                                    x = spatium() - score()->styleP(Sid::minNoteDistance);
                             }
                        else if (isHairpin() || isTrill() || isVibrato() || isTextLine() || isLyricsLine()) {
                              // (for LYRICSLINE, this is hyphen; melisma line is handled above)
                              // lay out to just before next chordrest on this staff, or barline
                              // tick2 actually tells us the right chordrest to look for
                              if (cr && endElement()->parent() && endElement()->parent()->type() == ElementType::SEGMENT) {
                                    qreal x2 = cr->x() /* TODO + cr->space().rw() */;
                                    Segment* currentSeg = toSegment(endElement()->parent());
                                    Segment* seg = score()->tick2segmentMM(tick2(), false, SegmentType::ChordRest);
                                    if (!seg) {
                                          // no end segment found, use measure width
                                          x2 = endElement()->parent()->parent()->width() - sp;
                                          }
                                    else if (currentSeg->measure() == seg->measure()) {
                                          // next chordrest found in same measure;
                                          // end line 1sp to left
                                          x2 = qMax(x2, seg->x() - sp);
                                          }
                                    else {
                                          // next chordrest is in next measure
                                          // lay out to end (barline) of current measure instead
                                          seg = currentSeg->next(SegmentType::EndBarLine);
                                          if (!seg)
                                                seg = currentSeg->measure()->last();
                                          // allow lyrics hyphen to extend to barline
                                          // other lines stop 1sp short
                                          qreal gap = (type() == ElementType::LYRICSLINE) ? 0.0 : sp;
                                          qreal x3 = seg->enabled() ? seg->x() : seg->measure()->width();
                                          x2 = qMax(x2, x3 - gap);
                                          }
                                    x = x2 - endElement()->parent()->x();
                                    }
                              }
                        }

                  Fraction t = grip == Grip::START ? tick() : tick2();
                  Measure* m = cr ? cr->measure() : score()->tick2measure(t);

                  if (m) {
                        x += cr ? cr->segment()->pos().x() + m->pos().x() : m->tick2pos(t);
                        *sys = m->system();
                        }
                  else
                        *sys = 0;
                  }
                  break;

            case Spanner::Anchor::MEASURE:
                  {
                  // anchor() == Anchor::MEASURE
                  const Measure* m;
                  if (grip == Grip::START) {
                        m = startMeasure();
                        // start after clef/keysig/timesig/barline
                        qreal offset = 0.0;
                        Segment* s = m->first(SegmentType::ChordRest);
                        if (s) {
                              s = s->prev();
                              if (s && s->enabled()) {
                                    offset = s->x();
                                    Element* e = s->element(staffIdx() * VOICES);
                                    if (e)
                                          offset += e->width();
                                    }
                              }
                        x = m->pos().x() + offset;
                        if (score()->styleB(Sid::createMultiMeasureRests) && m->hasMMRest()) {
                              x = m->mmRest()->pos().x();
                              }
                        }
                  else {
                        qreal _spatium = spatium();

                        if (score()->styleB(Sid::createMultiMeasureRests)) {
                              // find the actual measure where the volta should stop
                              m = startMeasure();
                              if (m->hasMMRest())
                                    m = m->mmRest();
                              while (m->nextMeasureMM() && (m->endTick() < tick2()))
                                    m = m->nextMeasureMM();
                              }
                        else {
                              m = endMeasure();
                              }

                        // back up to barline (skip courtesy elements)
                        Segment* seg = m->last();
                        while (seg && seg->segmentType() != SegmentType::EndBarLine)
                              seg = seg->prev();
                        if (!seg || !seg->enabled()) {
                              // no end bar line; look for BeginBarLine or StartRepeatBarLine of next measure
                              Measure* nm = m->nextMeasure();
                              if (nm->system() == m->system())
                                    seg = nm->first(SegmentType::BeginBarLine|SegmentType::StartRepeatBarLine);
                              }
                        qreal mwidth = seg && seg->measure() == m ? seg->x() : m->bbox().right();
                        x = m->pos().x() + mwidth;
                        // align to barline
                        if (seg && (seg->segmentType() & SegmentType::BarLineType)) {
                              Element* e = seg->element(0);
                              if (e && e->type() == ElementType::BAR_LINE) {
                                    BarLineType blt = toBarLine(e)->barLineType();
                                    switch (blt) {
                                          case BarLineType::END_REPEAT:
                                                // skip dots
                                                x += symWidth(SymId::repeatDot);
                                                x += score()->styleS(Sid::endBarDistance).val() * _spatium;
                                                // fall through
                                          case BarLineType::DOUBLE:
                                                // center on leftmost (thinner) barline
                                                x += score()->styleS(Sid::doubleBarWidth).val() * _spatium * 0.5;
                                                break;
                                          case BarLineType::START_REPEAT:
                                                // center on leftmost (thicker) barline
                                                x += score()->styleS(Sid::endBarWidth).val() * _spatium * 0.5;
                                                break;
                                          default:
                                                // center on barline
                                                x += score()->styleS(Sid::barWidth).val() * _spatium * 0.5;
                                                break;
                                          }
                                    }
                              }
                        }

                  m = m->coveringMMRestOrThis();

                  Q_ASSERT(m->system());
                  *sys = m->system();
                  }
                  break;

            case Spanner::Anchor::NOTE: {
                  Element* e = grip == Grip::START ? startElement() : endElement();
                  if (!e)
                        return QPointF();
                  Note* n = toNote(e);
                  System* s = n->chord()->segment()->system();
                  if (s == 0) {
                        qDebug("no system: %s  start %s chord parent %s\n", name(), n->name(), n->chord()->parent()->name());
                        *sys = s;
                        return QPointF();
                        }
                  *sys = s;
                  // return the position of the anchor note relative to the system
//                  QPointF     elemPagePos = e->pagePos();                   // DEBUG
//                  QPointF     systPagePos = s->pagePos();
//                  qreal       staffYPage  = s->staffYpage(e->staffIdx());
                  QPointF p = n->pagePos() - s->pagePos();
                  if (!isGlissando())
                        p.rx() += n->headWidth() * 0.5;
                  return p;
                  }

            case Spanner::Anchor::CHORD:
                  qFatal("Sline::linePos(): anchor not implemented");
                  break;
            }
      return QPointF(x, 0.0);
      }

//---------------------------------------------------------
//   layoutSystem
//    layout spannersegment for system
//---------------------------------------------------------

SpannerSegment* SLine::layoutSystem(System* system)
      {
      Fraction stick = system->firstMeasure()->tick();
      Fraction etick = system->lastMeasure()->endTick();

      LineSegment* lineSegm = toLineSegment(getNextLayoutSystemSegment(system, [this]() { return createLineSegment(); }));

      SpannerSegmentType sst;
      if (tick() >= stick) {
            //
            // this is the first call to layoutSystem,
            // processing the first line segment
            //
            computeStartElement();
            computeEndElement();
            sst = tick2() <= etick ? SpannerSegmentType::SINGLE : SpannerSegmentType::BEGIN;
            }
      else if (tick() < stick && tick2() > etick) {
            sst = SpannerSegmentType::MIDDLE;
            }
      else {
            //
            // this is the last call to layoutSystem
            // processing the last line segment
            //
            sst = SpannerSegmentType::END;
            }
      lineSegm->setSpannerSegmentType(sst);

      switch (sst) {
            case SpannerSegmentType::SINGLE: {
                  System* s;
                  QPointF p1 = linePos(Grip::START, &s);
                  QPointF p2 = linePos(Grip::END,   &s);
                  qreal len = p2.x() - p1.x();
                  lineSegm->setPos(p1);
                  lineSegm->setPos2(QPointF(len, p2.y() - p1.y()));
                  }
                  break;
            case SpannerSegmentType::BEGIN: {
                  System* s;
                  QPointF p1 = linePos(Grip::START, &s);
                  lineSegm->setPos(p1);
                  qreal x2 = system->lastNoteRestSegmentX(true);
                  lineSegm->setPos2(QPointF(x2 - p1.x(), 0.0));
                  }
                  break;
            case SpannerSegmentType::MIDDLE: {
                  qreal x1 = system->firstNoteRestSegmentX(true);
                  qreal x2 = system->lastNoteRestSegmentX(true);
                  System* s;
                  QPointF p1 = linePos(Grip::START, &s);
                  lineSegm->setPos(QPointF(x1, p1.y()));
                  lineSegm->setPos2(QPointF(x2 - x1, 0.0));
                  }
                  break;
            case SpannerSegmentType::END: {
                  System* s;
                  QPointF p2 = linePos(Grip::END,   &s);
                  qreal x1 = system->firstNoteRestSegmentX(true);
                  qreal len = p2.x() - x1;
                  lineSegm->setPos(QPointF(p2.x() - len, p2.y()));
                  lineSegm->setPos2(QPointF(len, 0.0));
                  }
                  break;
            }
      lineSegm->layout();
      return lineSegm;
      }

//---------------------------------------------------------
//   layout
//    compute segments from tick1 tick2
//    (used for palette, edit mode, and layout of note lines and glissandi)
//---------------------------------------------------------

void SLine::layout()
      {
      if (score() == gscore || (tick() == Fraction(-1,1)) || (tick2() == Fraction::fromTicks(1))) {
            //
            // when used in a palette or while dragging from palette,
            // SLine has no parent and
            // tick and tick2 has no meaning so no layout is
            // possible and needed
            //
            setLen(gscore->spatium() * 7);
            if (!spannerSegments().empty()) {
                  LineSegment* lineSegm = frontSegment();
                  lineSegm->layout();
                  setbbox(lineSegm->bbox());
                  }
            return;
            }

      computeStartElement();
      computeEndElement();

      System* s1;
      System* s2;
      QPointF p1(linePos(Grip::START, &s1));
      QPointF p2(linePos(Grip::END,   &s2));

      const QList<System*>& systems = score()->systems();
      int sysIdx1 = systems.indexOf(s1);
      int sysIdx2 = systems.indexOf(s2);
      int segmentsNeeded = 0;

      if (sysIdx1 == -1 || sysIdx2 == -1)
            return;

      for (int i = sysIdx1; i <= sysIdx2;  ++i) {
            if (systems.at(i)->vbox())
                  continue;
            ++segmentsNeeded;
            }

      int segCount = int(spannerSegments().size());

      if (segmentsNeeded != segCount) {
            fixupSegments(segmentsNeeded, [this]() { return createLineSegment(); });
            if (segmentsNeeded > segCount) {
                  for (int i = segCount; i < segmentsNeeded; ++i) {
                        LineSegment* lineSegm = segmentAt(i);
                        // set user offset to previous segment's offset
                        if (segCount > 0)
                              lineSegm->setOffset(QPointF(0, segmentAt(i-1)->offset().y()));
                        else
                              lineSegm->setOffset(QPointF(0, offset().y()));
                        }
                  }
            }

      int segIdx = 0;
      for (int i = sysIdx1; i <= sysIdx2; ++i) {
            System* system = systems.at(i);
            if (system->vbox())
                  continue;
            LineSegment* lineSegm = segmentAt(segIdx++);
            lineSegm->setTrack(track());       // DEBUG
            lineSegm->setSystem(system);

            if (sysIdx1 == sysIdx2) {
                  // single segment
                  lineSegm->setSpannerSegmentType(SpannerSegmentType::SINGLE);
                  qreal len = p2.x() - p1.x();
                  // enforcing a minimum length would be possible but inadvisable
                  // the line length calculations are tuned well enough that this should not be needed
                  //if (anchor() == Anchor::SEGMENT && type() != ElementType::PEDAL)
                  //      len = qMax(1.0 * spatium(), len);
                  lineSegm->setPos(p1);
                  lineSegm->setPos2(QPointF(len, p2.y() - p1.y()));
                  }
            else if (i == sysIdx1) {
                  // start segment
                  lineSegm->setSpannerSegmentType(SpannerSegmentType::BEGIN);
                  lineSegm->setPos(p1);
                  qreal x2 = system->lastNoteRestSegmentX(true);
                  lineSegm->setPos2(QPointF(x2 - p1.x(), 0.0));
                  }
            else if (i > 0 && i != sysIdx2) {
                  // middle segment
                  lineSegm->setSpannerSegmentType(SpannerSegmentType::MIDDLE);
                  qreal x1 = system->firstNoteRestSegmentX(true);
                  qreal x2 = system->lastNoteRestSegmentX(true);
                  lineSegm->setPos(QPointF(x1, p1.y()));
                  lineSegm->setPos2(QPointF(x2 - x1, 0.0));
                  }
            else if (i == sysIdx2) {
                  // end segment
                  qreal minLen = 0.0;
                  qreal x1 = system->firstNoteRestSegmentX(true);
                  qreal len = qMax(minLen, p2.x() - x1);
                  lineSegm->setSpannerSegmentType(SpannerSegmentType::END);
                  lineSegm->setPos(QPointF(p2.x() - len, p2.y()));
                  lineSegm->setPos2(QPointF(len, 0.0));
                  }
            lineSegm->layout();
            }
      }

//---------------------------------------------------------
//   writeProperties
//    write properties different from prototype
//---------------------------------------------------------

void SLine::writeProperties(XmlWriter& xml) const
      {
      if (!endElement()) {
            ((Spanner*)this)->computeEndElement();                // HACK
            if (!endElement())
                  xml.tag("ticks", ticks());
            }
      Spanner::writeProperties(xml);
      if (_diagonal)
            xml.tag("diagonal", _diagonal);
      writeProperty(xml, Pid::LINE_WIDTH);
      writeProperty(xml, Pid::LINE_STYLE);
      writeProperty(xml, Pid::COLOR);
      writeProperty(xml, Pid::ANCHOR);
      writeProperty(xml, Pid::DASH_LINE_LEN);
      writeProperty(xml, Pid::DASH_GAP_LEN);
      if (score() == gscore) {
            // when used as icon
            if (!spannerSegments().empty()) {
                  const LineSegment* s = frontSegment();
                  xml.tag("length", s->pos2().x());
                  }
            else
                  xml.tag("length", spatium() * 4);
            return;
            }
      //
      // check if user has modified the default layout
      //
      bool modified = false;
      for (const SpannerSegment* seg : spannerSegments()) {
            if (!seg->autoplace() || !seg->visible() ||
               (seg->propertyFlags(Pid::MIN_DISTANCE) == PropertyFlags::UNSTYLED || seg->getProperty(Pid::MIN_DISTANCE) != seg->propertyDefault(Pid::MIN_DISTANCE)) ||
               (!seg->isStyled(Pid::OFFSET) && (!seg->offset().isNull() || !seg->userOff2().isNull()))) {
                  modified = true;
                  break;
                  }
            }
      if (!modified)
            return;

      //
      // write user modified layout and other segment properties
      //
      qreal _spatium = score()->spatium();
      for (const SpannerSegment* seg : spannerSegments()) {
            xml.stag("Segment", seg);
            xml.tag("subtype", int(seg->spannerSegmentType()));
            // TODO:
            // NOSTYLE offset written in Element::writeProperties,
            // so we probably don't need to duplicate it here
            // see https://musescore.org/en/node/286848
            //if (seg->propertyFlags(Pid::OFFSET) & PropertyFlags::UNSTYLED)
            xml.tag("offset", seg->offset() / _spatium);
            xml.tag("off2", seg->userOff2() / _spatium);
            seg->writeProperty(xml, Pid::MIN_DISTANCE);
            seg->Element::writeProperties(xml);
            xml.etag();
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool SLine::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "tick2") {                // obsolete
            if (tick() == Fraction(-1,1)) // not necessarily set (for first note of score?) #30151
                  setTick(e.tick());
            setTick2(Fraction::fromTicks(e.readInt()));
            }
      else if (tag == "tick")             // obsolete
            setTick(Fraction::fromTicks(e.readInt()));
      else if (tag == "ticks")
            setTicks(Fraction::fromTicks(e.readInt()));
      else if (tag == "Segment") {
            LineSegment* ls = createLineSegment();
            ls->setTrack(track()); // needed in read to get the right staff mag
            ls->read(e);
            add(ls);
            ls->setVisible(visible());
            }
      else if (tag == "length")
            setLen(e.readDouble());
      else if (tag == "diagonal")
            setDiagonal(e.readInt());
      else if (tag == "anchor")
            setAnchor(Anchor(e.readInt()));
      else if (tag == "lineWidth")
            _lineWidth = e.readDouble() * spatium();
      else if (tag == "lineStyle")
            if (score()->mscVersion() > MSCVERSION) { // 4.x compat
                  QString lineStyle = e.readElementText();
                  if (lineStyle == "dotted")
                        _lineStyle = Qt::DotLine;
                  else if (lineStyle == "dashed")
                        _lineStyle = Qt::DashLine;
                  else
                        _lineStyle = Qt::SolidLine;
                  }
            else
                  _lineStyle = Qt::PenStyle(e.readInt());
      else if (tag == "dashLineLength")
            _dashLineLen = e.readDouble();
      else if (tag == "dashGapLength")
            _dashGapLen = e.readDouble();
      else if (tag == "lineColor")
            _lineColor = e.readColor();
      else if (tag == "color")
            _lineColor = e.readColor();
      else if (!Spanner::readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   setLen
//    used to create an element suitable for palette
//---------------------------------------------------------

void SLine::setLen(qreal l)
      {
      if (spannerSegments().empty())
            add(createLineSegment());
      LineSegment* s = frontSegment();
      s->setPos(QPointF());
      s->setPos2(QPointF(l, 0));
      }

//---------------------------------------------------------
//   bbox
//    used by palette: only one segment
//---------------------------------------------------------

const QRectF& SLine::bbox() const
      {
      if (spannerSegments().empty())
            setbbox(QRectF());
      else
            setbbox(segmentAt(0)->bbox());
      return Element::bbox();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void SLine::write(XmlWriter& xml) const
      {
      xml.stag(this);
      SLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void SLine::read(XmlReader& e)
      {
      eraseSpannerSegments();

      if (score()->mscVersion() < 301)
            e.addSpanner(e.intAttribute("id", -1), this);

      while (e.readNextStartElement()) {
            if (!SLine::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant SLine::getProperty(Pid id) const
      {
      switch (id) {
            case Pid::DIAGONAL:
                  return _diagonal;
            case Pid::COLOR:
                  return _lineColor;
            case Pid::LINE_WIDTH:
                  return _lineWidth;
            case Pid::LINE_STYLE:
                  return QVariant(int(_lineStyle));
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

bool SLine::setProperty(Pid id, const QVariant& v)
      {
      switch (id) {
            case Pid::DIAGONAL:
                  _diagonal = v.toBool();
                  break;
            case Pid::COLOR:
                  _lineColor = v.value<QColor>();
                  break;
            case Pid::LINE_WIDTH:
                  _lineWidth = v.toReal();
                  break;
            case Pid::LINE_STYLE:
                  _lineStyle = Qt::PenStyle(v.toInt());
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

QVariant SLine::propertyDefault(Pid pid) const
      {
      switch (pid) {
            case Pid::DIAGONAL:
                  return false;
            case Pid::COLOR:
                  return MScore::defaultColor;
            case Pid::LINE_WIDTH:
                  if (propertyFlags(pid) != PropertyFlags::NOSTYLE)
                        return Spanner::propertyDefault(pid);
                  return 0.15 * spatium();
            case Pid::LINE_STYLE:
                  if (propertyFlags(pid) != PropertyFlags::NOSTYLE)
                        return Spanner::propertyDefault(pid);
                  return int(Qt::SolidLine);
            case Pid::DASH_LINE_LEN:
            case Pid::DASH_GAP_LEN:
                  if (propertyFlags(pid) != PropertyFlags::NOSTYLE)
                        return Spanner::propertyDefault(pid);
                  return 5.0;
            default:
                  return Spanner::propertyDefault(pid);
            }
      }

}

