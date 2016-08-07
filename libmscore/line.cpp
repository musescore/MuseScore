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
      _p2       = s._p2;
      _userOff2 = s._userOff2;
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
            setUserOff2(e.readPoint() * spatium());
            if (!userOff2().isNull())
                  setAutoplace(false);
            }
      else if (tag == "pos") {
            qreal _spatium = score()->spatium();
            setUserOff(QPointF());
            setReadPos(e.readPoint() * _spatium);
            if (e.pasteMode())      // x position will be wrong
                  setReadPos(QPointF());
            setAutoplace(false);
            }
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
//   updateGrips
//---------------------------------------------------------

void LineSegment::updateGrips(Grip* defaultGrip, QVector<QRectF>& grip) const
      {
      *defaultGrip = Grip::END;
      QPointF pp(pagePos());
      grip[int(Grip::START)].translate(pp);
      grip[int(Grip::END)].translate(pos2() + pp);
      grip[int(Grip::MIDDLE)].translate(pos2() * .5 + pp);
      }

//---------------------------------------------------------
//   setGrip
//---------------------------------------------------------

void LineSegment::setGrip(Grip grip, const QPointF& p)
      {
      QPointF pt(p * spatium());

      switch (grip) {
            case Grip::START: {
                  QPointF delta(pt - userOff());
                  setUserOff(pt);
                  setUserOff2(userOff2() - delta);
                  }
                  break;
            case Grip::END:
                  setUserOff2(pt);
                  break;
            case Grip::MIDDLE:
                  setUserOff(pt);
                  break;
            case Grip::APERTURE:
            default:
                  break;
            }
      layout();   // needed?
      }

//---------------------------------------------------------
//   getGrip
//---------------------------------------------------------

QPointF LineSegment::getGrip(Grip grip) const
      {
      QPointF p;
      switch (grip) {
            case Grip::START:
                  p = userOff();
                  break;
            case Grip::END:
                  p = userOff2();
                  break;
            case Grip::MIDDLE:
                  p = userOff();
                  break;
            case Grip::APERTURE:
            default:
                  break;
            }
      p /= spatium();
      return p;
      }

//---------------------------------------------------------
//   gripAnchor
//    return page coordinates
//---------------------------------------------------------

QPointF LineSegment::gripAnchor(Grip grip) const
      {
      // Middle or aperture grip have no anchor
      if (grip == Grip::MIDDLE || grip == Grip::APERTURE)
            return QPointF(0, 0);
      // note-anchored spanners are relative to the system
      qreal y = spanner()->anchor() == Spanner::Anchor::NOTE ?
                  system()->pos().y() : system()->staffYpage(staffIdx());
      if (spannerSegmentType() == SpannerSegmentType::MIDDLE) {
            qreal x;
            switch (grip) {
                  case Grip::START:
                        x = system()->firstMeasure()->abbox().left();
                        break;
                  case Grip::END:
                        x = system()->lastMeasure()->abbox().right();
                        break;
                  default:
                        x = 0; // No Anchor
                        y = 0;
                        break;
                  }
            return QPointF(x, y);
            }
      else {
            if ((grip == Grip::END && spannerSegmentType() == SpannerSegmentType::BEGIN)
               || (grip == Grip::START && spannerSegmentType() == SpannerSegmentType::END)
               )
                  return QPointF(0, 0);
            else {
                  System* s;
                  QPointF p(line()->linePos(grip, &s));
                  p.ry() += y - system()->pos().y();
                  if (s)
                        p += s->pos();    // to page coordinates
                  return p;
                  }
            }
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool LineSegment::edit(MuseScoreView* sv, Grip curGrip, int key, Qt::KeyboardModifiers modifiers, const QString&)
      {
      if (!((modifiers & Qt::ShiftModifier)
         && ((spannerSegmentType() == SpannerSegmentType::SINGLE)
              || (spannerSegmentType() == SpannerSegmentType::BEGIN && curGrip == Grip::START)
              || (spannerSegmentType() == SpannerSegmentType::END && curGrip == Grip::END))))
            return false;

      LineSegment* ls = 0;
      SLine* l        = line();
      SpannerSegmentType st = spannerSegmentType();
      int track   = l->track();
      int track2  = l->track2();    // assumed to be same as track

      switch(l->anchor()) {
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
                  if (key == Qt::Key_Left) {
                        if (curGrip == Grip::START)
                              s1 = prevSeg1(s1, track);
                        else if (curGrip == Grip::END || curGrip == Grip::MIDDLE)
                              s2 = prevSeg1(s2, track2);
                        }
                  else if (key == Qt::Key_Right) {
                        if (curGrip == Grip::START)
                              s1 = nextSeg1(s1, track);
                        else if (curGrip == Grip::END || curGrip == Grip::MIDDLE) {
                              Segment* ns2 = nextSeg1(s2, track2);
                              if (ns2)
                                    s2 = ns2;
                              else
                                    s2 = score()->lastSegment();
                              }
                        }
                  if (s1 == 0 || s2 == 0 || s1->tick() >= s2->tick())
                        return true;
                  if (s1->tick() != spanner()->tick())
                        spanner()->setTick(s1->tick());
                  if (s2->tick() != spanner()->tick2())
                        spanner()->setTick2(s2->tick());
                  }
                  break;
            case Spanner::Anchor::NOTE:
                  {
                  Note* note1       = static_cast<Note*>(l->startElement());
                  Note* note2       = static_cast<Note*>(l->endElement());
                  Note* oldNote1    = note1;
                  Note* oldNote2    = note2;
                  if (!note1 && !note2) {
                        qDebug("LineSegment::edit: no start/end note");
                        return true;            // accept the event without doing anything
                        }

                  switch(key) {
                        case Qt::Key_Left:
                              if (curGrip == Grip::START)
                                    note1 = prevChordNote(note1);
                              else if (curGrip == Grip::END || curGrip == Grip::MIDDLE)
                                    note2 = prevChordNote(note2);
                              break;
                        case Qt::Key_Right:
                              if (curGrip == Grip::START)
                                    note1 = nextChordNote(note1);
                              else if (curGrip == Grip::END || curGrip == Grip::MIDDLE)
                                    note2 = nextChordNote(note2);
                              break;
                        case Qt::Key_Up:
                              if (curGrip == Grip::START)
                                    note1 = static_cast<Note*>(score()->upAlt(note1));
                              else if (curGrip == Grip::END || curGrip == Grip::MIDDLE)
                                    note2 = static_cast<Note*>(score()->upAlt(note2));
                              break;
                        case Qt::Key_Down:
                              if (curGrip == Grip::START)
                                    note1 = static_cast<Note*>(score()->downAlt(note1));
                              else if (curGrip == Grip::END || curGrip == Grip::MIDDLE)
                                    note2 = static_cast<Note*>(score()->downAlt(note2));
                              break;
                        default:
                              return true;
                  }

                  // check prevChordNote() and nextchordNote() didn't return null
                  // OR Score::upAlt() and Score::downAlt() didn't return non-Note (notably rests)
                  // OR spanner duration is > 0
                  // OR note1 and note2 didn't end up in different instruments
                  // if this is the case, accepts the event and return without doing nothing
                  if (note1 == 0 || note2 == 0
                              || note1->type() != Element::Type::NOTE || note2->type() != Element::Type::NOTE
                              || note1->chord()->tick() >= note2->chord()->tick()
                              || note1->chord()->staff()->part()->instrument(note1->chord()->tick())
                                    != note2->chord()->staff()->part()->instrument(note2->chord()->tick()) )
                        return true;
                  if (note1 != oldNote1 || note2 != oldNote2) {
                        spanner()->setNoteSpan(note1, note2);          // set new spanner span
                        }
                  }
                  break;
            default:
                  {
                  Measure* m1 = l->startMeasure();
                  Measure* m2 = l->endMeasure();

                  if (key == Qt::Key_Left) {
                        if (curGrip == Grip::START) {
                              if (m1->prevMeasure())
                                    m1 = m1->prevMeasure();
                              }
                        else if (curGrip == Grip::END || curGrip == Grip::MIDDLE) {
                              Measure* m = m2->prevMeasure();
                              if (m)
                                    m2 = m;
                              }
                        }
                  else if (key == Qt::Key_Right) {
                        if (curGrip == Grip::START) {
                              if (m1->nextMeasure())
                                    m1 = m1->nextMeasure();
                              }
                        else if (curGrip == Grip::END || curGrip == Grip::MIDDLE) {
                              if (m2->nextMeasure())
                                    m2 = m2->nextMeasure();
                              }
                        }
                  if (m1->tick() > m2->tick())
                        return true;
                  if (l->startElement() != m1) {
                        l->setTick(m1->tick());
                        l->setTicks(m2->endTick() - m1->tick());
                        }
                  else if (l->endElement() != m2) {
                        l->setTicks(m2->endTick() - m1->tick());
                        }
                  }
      }

      score()->doLayout();     // needed to compute multi measure rests

//      l->layout();

      LineSegment* nls = 0;
      if (st == SpannerSegmentType::SINGLE) {
            if (curGrip == Grip::START)
                  nls = l->frontSegment();
            else if (curGrip == Grip::END)
                  nls = l->backSegment();
            }
      else if (st == SpannerSegmentType::BEGIN)
            nls = l->frontSegment();
      else if (st == SpannerSegmentType::END)
            nls = l->backSegment();

      if (nls && (nls != this))
            sv->changeEditElement(nls);
      if (ls)
            score()->undoRemoveElement(ls);

      score()->setLayoutAll();
      return true;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void LineSegment::editDrag(const EditData& ed)
      {
      // Only for resizing according to the diagonal properties
      QPointF deltaResize(ed.delta.x(), line()->diagonal() ? ed.delta.y() : 0.0);

      // Only for moving, no y limitaion
      QPointF deltaMove(ed.delta.x(), ed.delta.y());

      switch (ed.curGrip) {
            case Grip::START: // Resize the begin of element (left grip)
                  setUserOff(userOff() + deltaResize);
                  _userOff2 -= deltaResize;
                  break;
            case Grip::END: // Resize the end of element (rigth grip)
                  _userOff2 += deltaResize;
                  break;
            case Grip::MIDDLE: // Move the element (middle grip)
                  setUserOff(userOff() + deltaMove);
                  break;
            default:
                  break;
            }
      if ((line()->anchor() == Spanner::Anchor::NOTE)
         && (ed.curGrip == Grip::START || ed.curGrip == Grip::END)) {
            //
            // if we touch a different note, change anchor
            //
            Element* e = ed.view->elementNear(ed.pos);
            if (e && e->type() == Element::Type::NOTE) {
                  SLine* l = line();
                  if (ed.curGrip == Grip::END && e != line()->endElement()) {
                        qDebug("LineSegment: move end anchor");
                        Note* noteOld = static_cast<Note*>(l->endElement());
                        Note* noteNew = static_cast<Note*>(e);

                        noteOld->removeSpannerBack(l);
                        noteNew->addSpannerBack(l);
                        l->setEndElement(noteNew);

                        _userOff2 += noteOld->canvasPos() - noteNew->canvasPos();
                        }
                  else if (ed.curGrip == Grip::START && e != l->startElement()) {
                        qDebug("LineSegment: move start anchor (not impl.)");
                        }
                  }
            }
//      line()->layout();
//      layout();
      triggerLayout();
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void LineSegment::spatiumChanged(qreal ov, qreal nv)
      {
      Element::spatiumChanged(ov, nv);
      _userOff2 *= nv / ov;
      }

//---------------------------------------------------------
//   localSpatiumChanged
//---------------------------------------------------------

void LineSegment::localSpatiumChanged(qreal ov, qreal nv)
      {
      Element::localSpatiumChanged(ov, nv);
      _userOff2 *= nv / ov;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant LineSegment::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::DIAGONAL:
            case P_ID::LINE_COLOR:
            case P_ID::LINE_WIDTH:
            case P_ID::LINE_STYLE:
            case P_ID::DASH_LINE_LEN:
            case P_ID::DASH_GAP_LEN:
                  return line()->getProperty(id);
            default:
                  return SpannerSegment::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool LineSegment::setProperty(P_ID id, const QVariant& val)
      {
      switch (id) {
            case P_ID::DIAGONAL:
            case P_ID::LINE_COLOR:
            case P_ID::LINE_WIDTH:
            case P_ID::LINE_STYLE:
            case P_ID::DASH_LINE_LEN:
            case P_ID::DASH_GAP_LEN:
                  return line()->setProperty(id, val);
            default:
                  return SpannerSegment::setProperty(id, val);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant LineSegment::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::DIAGONAL:
            case P_ID::LINE_COLOR:
            case P_ID::LINE_WIDTH:
            case P_ID::LINE_STYLE:
            case P_ID::DASH_LINE_LEN:
            case P_ID::DASH_GAP_LEN:
                  return line()->propertyDefault(id);
            default:
                  return SpannerSegment::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF LineSegment::dragAnchor() const
      {
      if (spannerSegmentType() != SpannerSegmentType::SINGLE && spannerSegmentType() != SpannerSegmentType::BEGIN)
            return QLineF();
      System* s;
      QPointF p = line()->linePos(Grip::START, &s);
      p += QPointF(s->canvasPos().x(), s->staffCanvasYpage(line()->staffIdx()));

      return QLineF(p, canvasPos());
      }

//---------------------------------------------------------
//   SLine
//---------------------------------------------------------

SLine::SLine(Score* s)
   : Spanner(s)
      {
      setTrack(0);
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
//    Anchor::NOTE: return anchor note position in system coordinates
//    Other:        return (x position (relative to what?), 0)
//---------------------------------------------------------

QPointF SLine::linePos(Grip grip, System** sys) const
      {
      qreal x = 0.0;
      qreal sp = staff()->spatium();
      switch (anchor()) {
            case Spanner::Anchor::SEGMENT:
                  {
                  ChordRest* cr;
                  if (grip == Grip::START) {
                        cr = static_cast<ChordRest*>(startElement());
                        if (cr && type() == Element::Type::OTTAVA) {
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
                        cr = static_cast<ChordRest*>(endElement());
                        if (type() == Element::Type::OTTAVA) {
                              if (cr && cr->durationType() == TDuration::DurationType::V_MEASURE) {
                                    x = cr->x() + cr->width() + sp;
                                    }
                              else if (cr) {
                                    // lay out just past right edge of all notes for this segment on this staff

                                    Segment* s = cr->segment();
                                    qreal width = s->staffShape(staffIdx()).right();
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
                                          qreal nextNoteDistance = ns->x() - s->x() + lineWidth().val() * sp;
                                          if (x > nextNoteDistance)
                                                x = qMax(width, nextNoteDistance);
                                          }
                                    }
                              }
                        else if (type() == Element::Type::LYRICSLINE && static_cast<Lyrics*>(parent())->ticks() > 0) {
                              // melisma line
                              // it is possible CR won't be in correct track
                              // prefer element in current track if available
                              if (!cr)
                                    qDebug("no end for lyricsline segment - start %d, ticks %d", tick(), ticks());
                              else if (cr->track() != track()) {
                                    Element* e = cr->segment()->element(track());
                                    if (e)
                                          cr = static_cast<ChordRest*>(e);
                                    }
                              // layout to right edge of CR
                              if (cr) {
                                    qreal maxRight = 0.0;
                                    if (cr->type() == Element::Type::CHORD) {
                                          // chord bbox() is unreliable, look at notes
                                          // this also allows us to more easily ignore ledger lines
                                          for (Note* n : static_cast<Chord*>(cr)->notes())
                                                maxRight = qMax(maxRight, cr->x() + n->x() + n->headWidth());
                                          }
                                    else {
                                          // rest - won't normally happen
                                          maxRight = cr->x() + cr->width();
                                          }
                                    x = maxRight; // cr->width()
                                    }
                             }
                        else if (type() == Element::Type::HAIRPIN || type() == Element::Type::TRILL
                                    || type() == Element::Type::TEXTLINE || type() == Element::Type::LYRICSLINE) {
                              // (for LYRICSLINE, this is hyphen; melisma line is handled above)
                              // lay out to just before next chordrest on this staff, or barline
                              // tick2 actually tells us the right chordrest to look for
                              if (cr && endElement()->parent() && endElement()->parent()->type() == Element::Type::SEGMENT) {
                                    qreal x2 = cr->x() /* TODO + cr->space().rw() */;
                                    Segment* currentSeg = static_cast<Segment*>(endElement()->parent());
                                    Segment* seg = score()->tick2segmentMM(tick2(), false, Segment::Type::ChordRest);
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
                                          seg = currentSeg->next(Segment::Type::EndBarLine);
                                          if (!seg)
                                                seg = currentSeg->measure()->last();
                                          // allow lyrics hyphen to extend to barline
                                          // other lines stop 1sp short
                                          qreal gap = (type() == Element::Type::LYRICSLINE) ? 0.0 : sp;
                                          x2 = qMax(x2, seg->x() - gap);
                                          }
                                    x = x2 - endElement()->parent()->x();
                                    }
                              }
                        }

                  int t = grip == Grip::START ? tick() : tick2();
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
                        // start after clef/key
                        qreal offset = 0.0;
                        Segment* s = m->first(Segment::Type::ChordRest);
                        if (s) {
                              s = s->prev();
                              if (s) {
                                    offset = s->x();
                                    Element* e = s->element(staffIdx() * VOICES);
                                    if (e)
                                          offset += e->width();
                                    }
                              }
                        x = m->pos().x() + offset;
                        if (score()->styleB(StyleIdx::createMultiMeasureRests) && m->hasMMRest()) {
                              x = m->mmRest()->pos().x();
                              }
                        }
                  else {
                        qreal _spatium = spatium();

                        if (score()->styleB(StyleIdx::createMultiMeasureRests)) {
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
                        while (seg && seg->segmentType() != Segment::Type::EndBarLine)
                              seg = seg->prev();
                        qreal mwidth = seg ? seg->x() : m->bbox().right();
                        x = m->pos().x() + mwidth;
                        // align to barline
                        if (seg && seg->segmentType() == Segment::Type::EndBarLine) {
                              Element* e = seg->element(0);
                              if (e && e->type() == Element::Type::BAR_LINE) {
                                    BarLineType blt = static_cast<BarLine*>(e)->barLineType();
                                    switch (blt) {
                                          case BarLineType::END_REPEAT:
                                          case BarLineType::END_START_REPEAT:
                                                // skip dots
                                                x += symWidth(SymId::repeatDot);
                                                x += score()->styleS(StyleIdx::endBarDistance).val() * _spatium;
                                                // fall through
                                          case BarLineType::DOUBLE:
                                                // center on leftmost (thinner) barline
                                                x += score()->styleS(StyleIdx::doubleBarWidth).val() * _spatium * 0.5;
                                                break;
                                          case BarLineType::START_REPEAT:
                                                // center on leftmost (thicker) barline
                                                x += score()->styleS(StyleIdx::endBarWidth).val() * _spatium * 0.5;
                                                break;
                                          default:
                                                // center on barline
                                                x += score()->styleS(StyleIdx::barWidth).val() * _spatium * 0.5;
                                                break;
                                          }
                                    }
                              }
                        }
                  if (score()->styleB(StyleIdx::createMultiMeasureRests))
                        m = m->mmRest1();
                  Q_ASSERT(m->system());
                  *sys = m->system();
                  }
                  break;

            case Spanner::Anchor::NOTE:
                  {
                  Element* e = grip == Grip::START ? startElement() : endElement();
                  if (!e)
                        return QPointF();
                  System* s = static_cast<Note*>(e)->chord()->segment()->system();
                  *sys = s;
                  // return the position of the anchor note relative to the system
//                  QPointF     elemPagePos = e->pagePos();                   // DEBUG
//                  QPointF     systPagePos = s->pagePos();
//                  qreal       staffYPage  = s->staffYpage(e->staffIdx());
                  return e->pagePos() - s->pagePos();
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
      int stick = system->firstMeasure()->tick();
      int etick = system->lastMeasure()->endTick();

      LineSegment* lineSegm = 0;
      for (SpannerSegment* ss : segments) {
            if (!ss->system()) {
                  lineSegm = static_cast<LineSegment*>(ss);
                  break;
                  }
            }
      if (!lineSegm) {
            lineSegm = createLineSegment();
            add(lineSegm);
            }
      lineSegm->setSystem(system);
      lineSegm->setSpanner(this);

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
                  qreal x2 = system->bbox().right();
                  lineSegm->setPos2(QPointF(x2 - p1.x(), 0.0));
                  }
                  break;
            case SpannerSegmentType::MIDDLE: {
                  Measure* firstMeasure = system->firstMeasure();
                  Segment* firstCRSeg   = firstMeasure->first(Segment::Type::ChordRest);
                  qreal x1              = (firstCRSeg ? firstCRSeg->pos().x() : 0) + firstMeasure->pos().x();
                  qreal x2              = system->bbox().right();
                  System* s;
                  QPointF p1 = linePos(Grip::START, &s);
                  lineSegm->setPos(QPointF(x1, p1.y()));
                  lineSegm->setPos2(QPointF(x2 - x1, 0.0));
                  }
                  break;
            case SpannerSegmentType::END: {
                  qreal offset = 0.0;
                  System* s;
                  QPointF p2 = linePos(Grip::END,   &s);
                  Measure* firstMeas  = system->firstMeasure();
                  Segment* firstCRSeg = firstMeas->first(Segment::Type::ChordRest);
                  if (anchor() == Anchor::SEGMENT || anchor() == Anchor::MEASURE) {
                        // start line just after previous element (eg, key signature)
                        firstCRSeg = firstCRSeg->prev();
                        Element* e = firstCRSeg ? firstCRSeg->element(staffIdx() * VOICES) : nullptr;
                        if (e)
                              offset = e->width();
                        }
                  qreal x1  = (firstCRSeg ? firstCRSeg->pos().x() : 0) + firstMeas->pos().x() + offset;
                  qreal len = p2.x() - x1;
                  lineSegm->setPos(QPointF(p2.x() - len, p2.y()));
                  lineSegm->setPos2(QPointF(len, 0.0));
                  }
                  break;
            }
      lineSegm->layout();
      QList<SpannerSegment*> sl;
      for (SpannerSegment* ss : segments) {
            if (ss->system())
                  sl.push_back(ss);
            else {
                  qDebug("delete spanner segment %s", ss->name());
                  delete ss;
                  }
            }
      segments.swap(sl);
      return lineSegm;
      }

//---------------------------------------------------------
//   layout
//    compute segments from tick1 tick2
//    (obsolete)
//---------------------------------------------------------

void SLine::layout()
      {
      if (score() == gscore || tick() == -1 || tick2() == 1) {
            //
            // when used in a palette or while dragging from palette,
            // SLine has no parent and
            // tick and tick2 has no meaning so no layout is
            // possible and needed
            //
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

      for (int i = sysIdx1; i < sysIdx2+1;  ++i) {
            if (systems.at(i)->vbox())
                  continue;
            ++segmentsNeeded;
            }

      int segCount = spannerSegments().size();

      if (segmentsNeeded != segCount) {
            if (segmentsNeeded > segCount) {
                  int n = segmentsNeeded - segCount;
                  for (int i = 0; i < n; ++i) {
                        LineSegment* lineSegm = createLineSegment();
                        add(lineSegm);
                        // set user offset to previous segment's offset
                        if (segCount > 0)
                              lineSegm->setUserOff(QPointF(0, segmentAt(segCount+i-1)->userOff().y()));
                        else
                              lineSegm->setUserOff(QPointF(0, userOff().y()));
                        }
                  }
            else {
                  int n = segCount - segmentsNeeded;
//                  qDebug("SLine: segments %d needed %d, remove %d", segCount, segmentsNeeded, n);
                  for (int i = 0; i < n; ++i) {
                        if (spannerSegments().empty()) {
                              qDebug("SLine::layout(): no segment %d, %d expected", i, n);
                              break;
                              }
                        else {
                              /*LineSegment* lineSegm =*/ takeLastSegment();
//                              delete lineSegm;
                              }
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

            Measure* firstMeas = system->firstMeasure();
            Segment* firstCRSeg = firstMeas->first(Segment::Type::ChordRest);

            if (sysIdx1 == sysIdx2) {
                  // single segment
                  lineSegm->setSpannerSegmentType(SpannerSegmentType::SINGLE);
                  qreal len = p2.x() - p1.x();
                  // enforcing a minimum length would be possible but inadvisable
                  // the line length calculations are tuned well enough that this should not be needed
                  //if (anchor() == Anchor::SEGMENT && type() != Element::Type::PEDAL)
                  //      len = qMax(1.0 * spatium(), len);
                  lineSegm->setPos(p1);
                  lineSegm->setPos2(QPointF(len, p2.y() - p1.y()));
                  }
            else if (i == sysIdx1) {
                  // start segment
                  lineSegm->setSpannerSegmentType(SpannerSegmentType::BEGIN);
                  lineSegm->setPos(p1);
                  qreal x2 = system->bbox().right();
                  lineSegm->setPos2(QPointF(x2 - p1.x(), 0.0));
                  }
            else if (i > 0 && i != sysIdx2) {
                  // middle segment
                  lineSegm->setSpannerSegmentType(SpannerSegmentType::MIDDLE);
                  qreal x1 = (firstCRSeg ? firstCRSeg->pos().x() : 0) + firstMeas->pos().x();
                  qreal x2 = system->bbox().right();
                  lineSegm->setPos(QPointF(x1, p1.y()));
                  lineSegm->setPos2(QPointF(x2 - x1, 0.0));
                  }
            else if (i == sysIdx2) {
                  // end segment
                  qreal offset = 0.0;
                  qreal minLen = 0.0;
                  if (anchor() == Anchor::SEGMENT || anchor() == Anchor::MEASURE) {
                        // start line just after previous element (eg, key signature)
                        firstCRSeg = firstCRSeg->prev();
                        Element* e = firstCRSeg ? firstCRSeg->element(staffIdx() * VOICES) : nullptr;
                        if (e)
                              offset = e->width();
                        // enforcing a minimum length would be possible but inadvisable
                        // the line length calculations are tuned well enough that this should not be needed
                        //if (type() != Element::Type::PEDAL)
                        //      minLen = 1.0 * spatium();
                        }
//                  qreal firstCRSegX = firstCRSeg ? firstCRSeg->pos().x() : 0;       // DEBUG
//                  qreal firstMeasX  = firstMeas  ? firstMeas->pos().x()  : 0;
                  qreal x1 = (firstCRSeg ? firstCRSeg->pos().x() : 0) + firstMeas->pos().x() + offset;
                  qreal len = qMax(minLen, p2.x() - x1);
                  lineSegm->setSpannerSegmentType(SpannerSegmentType::END);
                  lineSegm->setPos(QPointF(p2.x() - len, p2.y()));
                  lineSegm->setPos2(QPointF(len, 0.0));
                  }
            lineSegm->layout();
            }
      adjustReadPos();
      }

//---------------------------------------------------------
//   writeProperties
//    write properties different from prototype
//---------------------------------------------------------

void SLine::writeProperties(Xml& xml) const
      {
      if (!endElement())
            xml.tag("ticks", ticks());
      Spanner::writeProperties(xml);
      if (_diagonal)
            xml.tag("diagonal", _diagonal);
      if (propertyStyle(P_ID::LINE_WIDTH) != PropertyStyle::STYLED)
            xml.tag("lineWidth", lineWidth().val());
      if (propertyStyle(P_ID::LINE_STYLE) == PropertyStyle::UNSTYLED || (lineStyle() != Qt::SolidLine))
            if (propertyStyle(P_ID::LINE_STYLE) != PropertyStyle::STYLED)
                  xml.tag("lineStyle", int(lineStyle()));
      if (propertyStyle(P_ID::LINE_COLOR) == PropertyStyle::UNSTYLED || (lineColor() != MScore::defaultColor))
            xml.tag("lineColor", lineColor());

      writeProperty(xml, P_ID::ANCHOR);
      writeProperty(xml, P_ID::DASH_LINE_LEN);
      writeProperty(xml, P_ID::DASH_GAP_LEN);
      if (score() == gscore) {
            // when used as icon
            if (!spannerSegments().empty()) {
                  LineSegment* s = frontSegment();
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
            if (!seg->autoplace() || !seg->visible()) {
                  modified = true;
                  break;
                  }
            }
      if (!modified)
            return;

      //
      // write user modified layout
      //
      qreal _spatium = spatium();
      for (const SpannerSegment* seg : spannerSegments()) {
            xml.stag("Segment");
            xml.tag("subtype", int(seg->spannerSegmentType()));
            xml.tag("off2", seg->userOff2() / _spatium);
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
            if (tick() == -1) // not necessarily set (for first note of score?) #30151
                  setTick(e.tick());
            setTick2(e.readInt());
            }
      else if (tag == "tick")             // obsolete
            setTick(e.readInt());
      else if (tag == "ticks")
            setTicks(e.readInt());
      else if (tag == "Segment") {
            LineSegment* ls = createLineSegment();
            ls->setTrack(track()); // needed in read to get the right staff mag
            ls->read(e);
            add(ls);
            // in v1.x "visible" is a property of the segment only;
            // we must ensure that it propagates also to the parent element.
            // That's why the visibility is set after adding the segment
            // to the corresponding spanner
            if (score()->mscVersion() <= 114)
                  ls->setVisible(ls->visible());
            else
                  ls->setVisible(visible());
            }
      else if (tag == "length")
            setLen(e.readDouble());
      else if (tag == "diagonal")
            setDiagonal(e.readInt());
      else if (tag == "anchor")
            setAnchor(Anchor(e.readInt()));
      else if (tag == "lineWidth")
            _lineWidth = Spatium(e.readDouble());
      else if (tag == "lineStyle")
            _lineStyle = Qt::PenStyle(e.readInt());
      else if (tag == "dashLineLength")
            _dashLineLen = e.readDouble();
      else if (tag == "dashGapLength")
            _dashGapLen = e.readDouble();
      else if (tag == "lineColor")
            _lineColor = e.readColor();
      else if (Element::readProperties(e))
            ;
      else
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

void SLine::write(Xml& xml) const
      {
      int id = xml.spannerId(this);
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id));
      SLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void SLine::read(XmlReader& e)
      {
      foreach(SpannerSegment* seg, spannerSegments())
            delete seg;
      spannerSegments().clear();
      e.addSpanner(e.intAttribute("id", -1), this);

      while (e.readNextStartElement()) {
            if (!SLine::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant SLine::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::DIAGONAL:
                  return _diagonal;
            case P_ID::LINE_COLOR:
                  return _lineColor;
            case P_ID::LINE_WIDTH:
                  return _lineWidth;
            case P_ID::LINE_STYLE:
                  return QVariant(int(_lineStyle));
            case P_ID::DASH_LINE_LEN:
                  return dashLineLen();
            case P_ID::DASH_GAP_LEN:
                  return dashGapLen();
            default:
                  return Spanner::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool SLine::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_ID::DIAGONAL:
                  _diagonal = v.toBool();
                  break;
            case P_ID::LINE_COLOR:
                  _lineColor = v.value<QColor>();
                  break;
            case P_ID::LINE_WIDTH:
                  _lineWidth = v.value<Spatium>();
                  break;
            case P_ID::LINE_STYLE:
                  _lineStyle = Qt::PenStyle(v.toInt());
                  break;
            case P_ID::DASH_LINE_LEN:
                  setDashLineLen(v.toDouble());
                  break;
            case P_ID::DASH_GAP_LEN:
                  setDashGapLen(v.toDouble());
                  break;
            default:
                  return Spanner::setProperty(id, v);
            }
       return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant SLine::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::DIAGONAL:
                  return false;
            case P_ID::LINE_COLOR:
                  return MScore::defaultColor;
            case P_ID::LINE_WIDTH:
                  return Spatium(0.15);
            case P_ID::LINE_STYLE:
                  return int(Qt::SolidLine);
            case P_ID::DASH_LINE_LEN:
            case P_ID::DASH_GAP_LEN:
                  return 5.0;
            default:
                  return Spanner::propertyDefault(id);
            }
      }

}

