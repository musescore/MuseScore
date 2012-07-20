//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: line.cpp 5629 2012-05-15 12:38:33Z wschweer $
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "line.h"
#include "textline.h"
#include "segment.h"
#include "measure.h"
#include "score.h"
#include "xml.h"
#include "system.h"
#include "utils.h"
#include "barline.h"

enum { GRIP_LINE_START, GRIP_LINE_END, GRIP_LINE_MIDDLE };

//---------------------------------------------------------
//   LineSegment
//---------------------------------------------------------

LineSegment::LineSegment(Score* s)
   : SpannerSegment(s)
      {
      }

LineSegment::LineSegment(const LineSegment& s)
   : SpannerSegment(s)
      {
      _p2       = s._p2;
      _userOff2 = s._userOff2;
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool LineSegment::readProperties(const QDomElement& e)
      {
      const QString& tag(e.tagName());
      if (tag == "subtype")
            setSubtype(SpannerSegmentType(e.text().toInt()));
      else if (tag == "off1")       // obsolete
            setUserOff(readPoint(e) * spatium());
      else if (tag == "off2")
            setUserOff2(readPoint(e) * spatium());
      else if (tag == "pos") {
            if (score()->mscVersion() > 114) {
                  qreal _spatium = spatium();
                  setUserOff(QPointF());
                  setReadPos(readPoint(e) * _spatium);
                  }
            }
      else if (!Element::readProperties(e)) {
            domError(e);
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void LineSegment::read(const QDomElement& e)
      {
      for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement())
            readProperties(ee);
      }

//---------------------------------------------------------
//   isEdited
//---------------------------------------------------------

bool LineSegment::isEdited(SpannerSegment* ss) const
      {
      LineSegment* ls = static_cast<LineSegment*>(ss);
      if (pos() != ls->pos() || pos2() != ls->pos2())
            return true;
      return false;
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void LineSegment::updateGrips(int* grips, QRectF* grip) const
      {
      *grips = 3;
      QPointF pp(pagePos());
      QPointF pp1(pp);
      QPointF pp2(pos2() + pp);
      QPointF pp3(pos2() * .5 + pp);
      grip[2].translate(pp3);
      grip[1].translate(pp2);
      grip[0].translate(pp1);
      }

//---------------------------------------------------------
//   setGrip
//---------------------------------------------------------

void LineSegment::setGrip(int grip, const QPointF& p)
      {
      QPointF pt(p * spatium());

      if (grip == GRIP_LINE_START) {
            QPointF delta = pt - (pagePos() - gripAnchor(grip));
            setUserOff(userOff() + delta);
            _userOff2 -= delta;
            }
      else {
            setUserOff2(pt - pagePos() - _p2 + gripAnchor(grip));
            }
      layout();
      }

//---------------------------------------------------------
//   getGrip
//---------------------------------------------------------

QPointF LineSegment::getGrip(int grip) const
      {
      QPointF pt;
      switch(grip) {
            case GRIP_LINE_START:
                  pt = pagePos() - gripAnchor(grip);
                  break;
            case GRIP_LINE_END:
                  pt = _p2 + _userOff2 + pagePos() - gripAnchor(grip);
                  break;
            case GRIP_LINE_MIDDLE:
                  pt = (_p2 + _userOff2) * .5 + pagePos() - gripAnchor(grip);
                  break;
            }
      return pt / spatium();
      }

//---------------------------------------------------------
//   pagePos
//    return position in canvas coordinates
//---------------------------------------------------------

QPointF LineSegment::pagePos() const
      {
      QPointF pt(pos());
      if (parent())
            pt += parent()->pos();
      return pt;
      }

//---------------------------------------------------------
//   gripAnchor
//---------------------------------------------------------

QPointF LineSegment::gripAnchor(int grip) const
      {
      if (subtype() == SEGMENT_MIDDLE) {
            qreal y = system()->staffY(staffIdx());
            qreal x;
            switch(grip) {
                  case GRIP_LINE_START:
                        x = system()->firstMeasure()->abbox().left();
                        break;
                  case GRIP_LINE_END:
                        x = system()->lastMeasure()->abbox().right();
                        break;
                  default:
                  case GRIP_LINE_MIDDLE:
                        x = 0; // No Anchor
                        y = 0;
                        break;
                  }
            return QPointF(x, y);
            }
      else {
            if (grip == GRIP_LINE_MIDDLE) // center grip
                  return QPointF(0, 0);
            else {
                  System* s;
                  QPointF pt(line()->linePos(grip, &s));
                  return pt + s->pagePos();
                  }
            }
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool LineSegment::edit(MuseScoreView* sv, int curGrip, int key, Qt::KeyboardModifiers modifiers, const QString&)
      {
      if (!((modifiers & Qt::ShiftModifier)
         && ((subtype() == SEGMENT_SINGLE)
              || (subtype() == SEGMENT_BEGIN && curGrip == GRIP_LINE_START)
              || (subtype() == SEGMENT_END && curGrip == GRIP_LINE_END))))
            return false;

      LineSegment* ls = 0;
      SLine* l        = line();
      bool bspDirty   = false;
      SpannerSegmentType st = subtype();
      int track   = l->track();

      if (l->anchor() == ANCHOR_SEGMENT) {
            Segment* s1 = static_cast<Segment*>(l->startElement());
            Segment* s2 = static_cast<Segment*>(l->endElement());

            bool removeSegment = false;

            if (key == Qt::Key_Left) {
                  if (curGrip == GRIP_LINE_START) {
                        s1 = prevSeg1(s1, track);
                        }
                  else if (curGrip == GRIP_LINE_END) {
                        s2 = prevSeg1(s2, track);
                        if (s2
                           && (s2->system()->firstMeasure() == s2->measure())
                           && (s2->tick() == s2->measure()->tick())) {
                              removeSegment = true;
                              }
                        }
                  }
            else if (key == Qt::Key_Right) {
                  if (curGrip == GRIP_LINE_START)
                        s1 = nextSeg1(s1, track);
                  else if (curGrip == GRIP_LINE_END) {
                        if ((s2->system()->firstMeasure() == s2->measure())
                           && (s2->tick() == s2->measure()->tick()))
                              bspDirty = true;
                        s2 = nextSeg1(s2, track);
                        }
                  }
            if (s1 == 0 || s2 == 0 || s1->tick() >= s2->tick())
                  return true;

            if (l->startElement() != s1) {
                  if (s1->system() != (static_cast<Segment*>(l->startElement())->system())) {
                        bspDirty = true;
                        if (key == Qt::Key_Right)
                              ls = l->takeFirstSegment();
                        }
                  static_cast<Segment*>(l->startElement())->remove(l);
                  l->setStartElement(s1);
                  s1->add(l);
                  }
            else if (l->endElement() != s2) {
                  if (removeSegment) {
                        bspDirty = true;
                        if (key == Qt::Key_Left)
                              ls = l->takeLastSegment();
                        }

                  static_cast<Segment*>(l->endElement())->removeSpannerBack(l);
                  l->setEndElement(s2);
                  s2->addSpannerBack(l);
                  }
            }
      else {
            Measure* m1 = static_cast<Measure*>(l->startElement());
            Measure* m2 = static_cast<Measure*>(l->endElement());

            bool removeSegment = false;

            if (key == Qt::Key_Left) {
                  if (curGrip == GRIP_LINE_START) {
                        if (m1->prevMeasure())
                              m1 = m1->prevMeasure();
                        }
                  else if (curGrip == GRIP_LINE_END) {
                        if (m2 && (m2->system()->firstMeasure() == m2))
                              removeSegment = true;
                        Measure* m = m2->prevMeasure();
                        if (m)
                              m2 = m;
                        }
                  }
            else if (key == Qt::Key_Right) {
                  if (curGrip == GRIP_LINE_START) {
                        if (m1->nextMeasure())
                              m1 = m1->nextMeasure();
                        }
                  else if (curGrip == GRIP_LINE_END) {
                        if (m2->nextMeasure())
                              m2 = m2->nextMeasure();
                        if (m2->system()->firstMeasure() == m2)
                              bspDirty = true;
                        }
                  }
            if (m1->tick() > m2->tick())
                  return true;

            if (l->startElement() != m1) {
                  if (m1->system() != (static_cast<Measure*>(l->startElement())->system())) {
                        bspDirty = true;
                        if (key == Qt::Key_Right)
                              ls = l->takeFirstSegment();
                        }
                  l->startElement()->remove(l);
                  l->setStartElement(m1);
                  m1->add(l);
                  }
            else if (l->endElement() != m2) {
                  if (removeSegment) {
                        bspDirty = true;
                        if (key == Qt::Key_Left)
                              ls = l->takeLastSegment();
                        }
                  static_cast<Measure*>(l->endElement())->removeSpannerBack(l);
                  l->setEndElement(m2);
                  m2->addSpannerBack(l);
                  }
            }
      l->layout();

      LineSegment* nls = 0;
      if (st == SEGMENT_SINGLE) {
            if (curGrip == GRIP_LINE_START)
                  nls = l->frontSegment();
            else if (curGrip == GRIP_LINE_END)
                  nls = l->backSegment();
            }
      else if (st == SEGMENT_BEGIN)
            nls = l->frontSegment();
      else if (st == SEGMENT_END)
            nls = l->backSegment();

      if (nls && (nls != this))
            sv->changeEditElement(nls);
      if (bspDirty)
            _score->rebuildBspTree();
      if (ls)
            _score->undoRemoveElement(ls);
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

      switch(ed.curGrip) {
            case GRIP_LINE_START: // Resize the begin of element (left grip)
                  setUserOff(userOff() + deltaResize);
                  _userOff2 -= deltaResize;
                  break;
            case GRIP_LINE_END: // Resize the end of element (rigth grip)
                  _userOff2 += deltaResize;
                  break;
            case GRIP_LINE_MIDDLE: // Move the element (middle grip)
                  setUserOff(userOff() + deltaMove);
                  break;
            }
      layout();
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
//   toDefault
//    TODO: make undoable
//---------------------------------------------------------

void LineSegment::toDefault()
      {
      Element::toDefault();
      setUserOff2(QPointF());
      }

//---------------------------------------------------------
//   SLine
//---------------------------------------------------------

SLine::SLine(Score* s)
   : Spanner(s)
      {
      _diagonal = false;
      setTrack(0);
      }

SLine::SLine(const SLine& s)
   : Spanner(s)
      {
      _diagonal = s._diagonal;
      }

//---------------------------------------------------------
//   linePos
//    return System() coordinates
//---------------------------------------------------------

QPointF SLine::linePos(int grip, System** sys)
      {
      qreal _spatium = spatium();

      qreal x;

      if (anchor() == ANCHOR_SEGMENT) {
            Segment* seg = static_cast<Segment*>(grip == 0 ? startElement() : endElement());
            Measure* m   = seg->measure();
            *sys         = m->system();
            if (*sys == 0)
                  return QPointF(x, 0.0);
            x            = seg->pos().x() + m->pos().x();
            if (grip == GRIP_LINE_END) {
                  if (((*sys)->firstMeasure() == m) && (seg->tick() == m->tick())) {
                        m = m->prevMeasure();
                        if (m) {
                              *sys = m->system();
                              x = m->pos().x() + m->width();
                              }
                        }
                  }
            }
      else {
            // anchor() == ANCHOR_MEASURE
            Measure* m;
            if (grip == GRIP_LINE_START) {
                  m = static_cast<Measure*>(startElement());
                  x = m->pos().x();
                  }
            else {
                  m = static_cast<Measure*>(endElement());
                  x = m->pos().x() + m->bbox().right();
                  if (type() == VOLTA) {
                        Segment* seg = m->last();
                        if (seg->subtype() == SegEndBarLine) {
                              Element* e = seg->element(0);
                              if (e && e->type() == BAR_LINE) {
                                    if (static_cast<BarLine*>(e)->subtype() == START_REPEAT)
                                          x -= e->width() - _spatium * .5;
                                    else
                                          x -= _spatium * .5;
                                    }
                              }
                        }
                  }
            *sys = m->system();
            }
      //DEBUG:
      if ((*sys)->staves()->isEmpty())
            return QPointF(x, 0.0);

      qreal y = (*sys)->staff(staffIdx())->y();
      return QPointF(x, y);
      }

//---------------------------------------------------------
//   layout
//    compute segments from tick1 tick2
//---------------------------------------------------------

void SLine::layout()
      {
      if (parent() == 0) {
            //
            // when used in a palette, SLine has no parent and
            // tick and tick2 has no meaning so no layout is
            // possible and needed
            //
            if (!spannerSegments().isEmpty()) {
                  LineSegment* s = frontSegment();
                  s->layout();
                  setbbox(s->bbox());
                  }
            return;
            }
      if (startElement() == 0 || endElement() == 0) {
            qDebug("SLine::layout() failed: %s %s", parent()->name(), name());
            qDebug("   start %p   end %p", startElement(), endElement());
            return;
            }

      System* s1;
      System* s2;
      QPointF p1 = linePos(GRIP_LINE_START, &s1);
      QPointF p2 = linePos(GRIP_LINE_END,   &s2);

      QList<System*>* systems = score()->systems();
      int sysIdx1 = systems->indexOf(s1);
      int sysIdx2 = systems->indexOf(s2);
      int segmentsNeeded = 0;
      for (int i = sysIdx1; i < sysIdx2+1;  ++i) {
            if (systems->at(i)->isVbox())
                  continue;
            ++segmentsNeeded;
            }
      int segCount = spannerSegments().size();

      if (segmentsNeeded != segCount) {
            if (segmentsNeeded > segCount) {
                  int n = segmentsNeeded - segCount;
                  for (int i = 0; i < n; ++i) {
                        LineSegment* ls = createLineSegment();
                        add(ls);
                        // set user offset to previous segment's offset
                        if (segCount > 0)
                              ls->setUserOff(QPointF(0, segmentAt(segCount+i-1)->userOff().y()));
                        }
                  }
            else {
                  int n = segCount - segmentsNeeded;
                  qDebug("SLine: segments %d needed %d, remove %d", segCount, segmentsNeeded, n);
                  for (int i = 0; i < n; ++i) {
                        if (spannerSegments().isEmpty()) {
                              qDebug("SLine::layout(): no segment %d, %d expected", i, n);
                              break;
                              }
                        else {
                              // LineSegment* seg = takeLastSegment();
                              // TODO delete seg;
                              }
                        }
                  }
            }

      int segIdx = 0;
      int si  = staffIdx();
      for (int i = sysIdx1; i <= sysIdx2; ++i) {
            System* system = systems->at(i);
            if (system->isVbox())
                  continue;
            LineSegment* seg = segmentAt(segIdx++);
            seg->setSystem(system);

            Measure* m = system->firstMeasure();
            Segment* mseg = m->first(SegChordRest);
            qreal x1 = (mseg ? mseg->pos().x() : 0) + m->pos().x();
            qreal x2 = system->bbox().right();
            qreal y  = system->staff(si)->y();

            if (sysIdx1 == sysIdx2) {
                  // single segment
                  seg->setSubtype(SEGMENT_SINGLE);
                  seg->setPos(p1);
                  seg->setPos2(QPointF(p2.x() - p1.x(), 0.0));
                  }
            else if (i == sysIdx1) {
                  // start segment
                  seg->setSubtype(SEGMENT_BEGIN);
                  seg->setPos(p1);
                  seg->setPos2(QPointF(x2 - p1.x(), 0.0));
                  }
            else if (i > 0 && i != sysIdx2) {
                  // middle segment
                  seg->setSubtype(SEGMENT_MIDDLE);
                  seg->setPos(QPointF(x1, y));
                  seg->setPos2(QPointF(x2 - x1, 0.0));
                  }
            else if (i == sysIdx2) {
                  // end segment
                  seg->setSubtype(SEGMENT_END);
                  seg->setPos(QPointF(x1, y));
                  seg->setPos2(QPointF(p2.x() - x1, 0.0));
                  }
            seg->layout();
            seg->rypos() += (_yoffset * spatium());
            seg->adjustReadPos();
            }
      }

//---------------------------------------------------------
//   writeProperties
//    write properties different from prototype
//---------------------------------------------------------

void SLine::writeProperties(Xml& xml, const SLine* proto) const
      {
      Element::writeProperties(xml);
      if (_diagonal && (proto == 0 || proto->diagonal() != _diagonal))
            xml.tag("diagonal", _diagonal);
      if (anchor() != ANCHOR_SEGMENT && (proto == 0 || proto->anchor() != anchor()))
            xml.tag("anchor", anchor());
      if (score() == gscore) {
            // when used as icon
            if (!spannerSegments().isEmpty()) {
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
      int n = spannerSegments().size();
      for (int i = 0; i < n; ++i) {
            const LineSegment* seg = segmentAt(i);
            if (!seg->userOff().isNull()
               || !seg->userOff2().isNull()
               || !seg->visible()) {
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
      for (int i = 0; i < n; ++i) {
            const LineSegment* seg = segmentAt(i);
            xml.stag("Segment");
            xml.tag("subtype", seg->subtype());
            xml.tag("off2", seg->userOff2() / _spatium);
            seg->Element::writeProperties(xml);
            xml.etag();
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool SLine::readProperties(const QDomElement& e)
      {
      if (Element::readProperties(e))
            return true;
      const QString& tag(e.tagName());
      const QString& val(e.text());
      int i = val.toInt();

      if (tag == "tick2")           // obsolete
            __setTick2(score()->fileDivision(i));
      else if (tag == "tick")       // obsolete
            __setTick1(score()->fileDivision(i));
      else if (tag == "Segment") {
            LineSegment* ls = createLineSegment();
            ls->read(e);
            add(ls);
            }
      else if (tag == "track")
            setTrack(i);
      else if (tag == "length")
            setLen(val.toDouble());
      else if (tag == "diagonal")
            setDiagonal(i);
      else if (tag == "anchor")
            setAnchor(Anchor(i));
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
      if (spannerSegments().isEmpty())
            add(createLineSegment());
      LineSegment* s = frontSegment();
      s->setPos(QPointF());
      s->setPos2(QPointF(l, 0));
      }

//---------------------------------------------------------
//   bbox
//    used by palette: only one segment
//---------------------------------------------------------

QRectF SLine::bbox() const
      {
      if (spannerSegments().isEmpty())
            return QRectF();
      else
            return segmentAt(0)->bbox();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void SLine::write(Xml& xml) const
      {
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id()));
      SLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void SLine::read(const QDomElement& de)
      {
      foreach(SpannerSegment* seg, spannerSegments())
            delete seg;
      spannerSegments().clear();
      setId(de.attribute("id", "-1").toInt());

      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!SLine::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

int SLine::tick() const
      {
      if (startElement()->type() != SEGMENT)
            return -1;
      return static_cast<Segment*>(startElement())->tick();
      }

//---------------------------------------------------------
//   tick2
//---------------------------------------------------------

int SLine::tick2() const
      {
      if (endElement()->type() != SEGMENT)
            return -1;
      return static_cast<Segment*>(endElement())->tick();
      }

