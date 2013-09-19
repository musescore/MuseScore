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
#include "textline.h"
#include "segment.h"
#include "measure.h"
#include "score.h"
#include "xml.h"
#include "system.h"
#include "utils.h"
#include "barline.h"
#include "chord.h"

namespace Ms {


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

bool LineSegment::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());
      if (tag == "subtype")
            setSpannerSegmentType(SpannerSegmentType(e.readInt()));
      else if (tag == "off1")       // obsolete
            setUserOff(e.readPoint() * spatium());
      else if (tag == "off2")
            setUserOff2(e.readPoint() * spatium());
      else if (tag == "pos") {
            if (score()->mscVersion() > 114) {
                  qreal _spatium = spatium();
                  setUserOff(QPointF());
                  setReadPos(e.readPoint() * _spatium);
                  }
            else
                  e.readNext();
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

void LineSegment::updateGrips(int* grips, QRectF* grip) const
      {
      *grips = 3;
      QPointF pp(pagePos());
      grip[GRIP_LINE_START].translate(pp);
      grip[GRIP_LINE_END].translate(pos2() + pp);
      grip[GRIP_LINE_MIDDLE].translate(pos2() * .5 + pp);
      }

//---------------------------------------------------------
//   setGrip
//---------------------------------------------------------

void LineSegment::setGrip(int grip, const QPointF& p)
      {
      QPointF pt(p * spatium());

      switch (grip) {
            case GRIP_LINE_START: {
                  QPointF delta(pt - userOff());
                  setUserOff(pt);
                  setUserOff2(userOff2() - delta);
                  }
                  break;
            case GRIP_LINE_END:
                  setUserOff2(pt);
                  break;
            case GRIP_LINE_MIDDLE:
                  setUserOff(pt);
                  break;
            }
      layout();   // needed?
      }

//---------------------------------------------------------
//   getGrip
//---------------------------------------------------------

QPointF LineSegment::getGrip(int grip) const
      {
      QPointF p;
      switch(grip) {
            case GRIP_LINE_START:
                  p = userOff();
                  break;
            case GRIP_LINE_END:
                  p = userOff2();
                  break;
            case GRIP_LINE_MIDDLE:
                  p = userOff();
                  break;
            }
      p /= spatium();
      return p;
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
//    return page coordinates
//---------------------------------------------------------

QPointF LineSegment::gripAnchor(int grip) const
      {
      if (spannerSegmentType() == SEGMENT_MIDDLE) {
            qreal y = system()->staffYpage(staffIdx());
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
                  QPointF p(line()->linePos(grip, &s));
                  if (s)
                        p += s->pos();
                  return p;
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
         && ((spannerSegmentType() == SEGMENT_SINGLE)
              || (spannerSegmentType() == SEGMENT_BEGIN && curGrip == GRIP_LINE_START)
              || (spannerSegmentType() == SEGMENT_END && curGrip == GRIP_LINE_END))))
            return false;

      LineSegment* ls = 0;
      SLine* l        = line();
      bool bspDirty   = false;
      SpannerSegmentType st = spannerSegmentType();
      int track   = l->track();

      if (l->anchor() == Spanner::ANCHOR_SEGMENT) {
            Segment* s1 = spanner()->startSegment();
            Segment* s2 = spanner()->endSegment();
            if (!s1 && !s2) {
                  qDebug("LineSegment::edit: no start/end segment");
                  return true;
                  }
            if (key == Qt::Key_Left) {
                  if (curGrip == GRIP_LINE_START)
                        s1 = prevSeg1(s1, track);
                  else if (curGrip == GRIP_LINE_END || curGrip == GRIP_LINE_MIDDLE)
                        s2 = prevSeg1(s2, track);
                  }
            else if (key == Qt::Key_Right) {
                  if (curGrip == GRIP_LINE_START)
                        s1 = nextSeg1(s1, track);
                  else if (curGrip == GRIP_LINE_END || curGrip == GRIP_LINE_MIDDLE) {
                        if ((s2->system()->firstMeasure() == s2->measure())
                           && (s2->tick() == s2->measure()->tick()))
                              bspDirty = true;
                        s2 = nextSeg1(s2, track);
                        }
                  }
            if (s1 == 0 || s2 == 0 || s1->tick() > s2->tick())
                  return true;
            if (s1->tick() != spanner()->tick())
                  spanner()->setTick(s1->tick());
            if (s2->tick() != spanner()->tick2())
                  spanner()->setTick2(s2->tick());
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
                  else if (curGrip == GRIP_LINE_END || curGrip == GRIP_LINE_MIDDLE) {
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
                  else if (curGrip == GRIP_LINE_END || curGrip == GRIP_LINE_MIDDLE) {
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
                  l->setTick(m1->tick());
                  m1->add(l);
                  }
            else if (l->endElement() != m2) {
                  if (removeSegment) {
                        bspDirty = true;
                        if (key == Qt::Key_Left)
                              ls = l->takeLastSegment();
                        }
                  l->setTick2(m2->endTick());
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

      switch (ed.curGrip) {
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
      if ((line()->anchor() == Spanner::ANCHOR_NOTE)
         && (ed.curGrip == GRIP_LINE_START || ed.curGrip == GRIP_LINE_END)) {
            //
            // if we touch a different note, change anchor
            //
            Element* e = ed.view->elementNear(ed.pos);
            if (e && e->type() == NOTE) {
                  SLine* l = line();
                  if (ed.curGrip == GRIP_LINE_END && e != line()->endElement()) {
                        qDebug("LineSegment: move end anchor");
                        Note* noteOld = static_cast<Note*>(l->endElement());
                        Note* noteNew = static_cast<Note*>(e);

                        noteOld->removeSpannerBack(l);
                        noteNew->addSpannerBack(l);
                        l->setEndElement(noteNew);

                        _userOff2 += noteOld->canvasPos() - noteNew->canvasPos();
                        }
                  else if (ed.curGrip == GRIP_LINE_START && e != l->startElement()) {
                        qDebug("LineSegment: move start anchor (not impl.)");
                        }
                  }
            }
      line()->layout();
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
//   getProperty
//---------------------------------------------------------

QVariant LineSegment::getProperty(P_ID id) const
      {
      switch (id) {
            case P_DIAGONAL:
            case P_LINE_COLOR:
            case P_LINE_WIDTH:
            case P_LINE_STYLE:
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
            case P_DIAGONAL:
            case P_LINE_COLOR:
            case P_LINE_WIDTH:
            case P_LINE_STYLE:
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
      return line()->propertyDefault(id);
      }

//---------------------------------------------------------
//   SLine
//---------------------------------------------------------

SLine::SLine(Score* s)
   : Spanner(s)
      {
      _diagonal  = false;
      _lineColor = MScore::defaultColor;
      _lineWidth = Spatium(0.15);
      _lineStyle = Qt::SolidLine;
      setTrack(0);
      }

SLine::SLine(const SLine& s)
   : Spanner(s)
      {
      _diagonal  = s._diagonal;
      _lineWidth = s._lineWidth;
      _lineColor = s._lineColor;
      _lineStyle = s._lineStyle;
      }

//---------------------------------------------------------
//   linePos
//    return System() coordinates
//---------------------------------------------------------

QPointF SLine::linePos(int grip, System** sys)
      {
      qreal _spatium = spatium();
      qreal x = 0.0;
      switch (anchor()) {
            case Spanner::ANCHOR_SEGMENT:
                  {
                  Measure* m;
                  int t;
                  if (grip == GRIP_LINE_START) {
                        t = tick();
                        m = score()->tick2measure(t);
                        x = m->tick2pos(t);
                        }
                  else {
                        t = tick2();
                        m = score()->tick2measure(t);
                        x = m->tick2pos(t) + 2.0 * _spatium;
                        }
                  *sys = m->system();
                  }
                  break;

            case Spanner::ANCHOR_MEASURE:
                  {
                  // anchor() == ANCHOR_MEASURE
                  Measure* m;
                  if (grip == GRIP_LINE_START) {
                        Q_ASSERT(startElement()->type() == MEASURE);
                        m = static_cast<Measure*>(startElement());
                        x = m->pos().x();
                        }
                  else {
                        Q_ASSERT(endElement()->type() == MEASURE);
                        m = static_cast<Measure*>(endElement());
                        x = m->pos().x() + m->bbox().right();

#if 0 // MM
                        if (type() == VOLTA) {
                              if (score()->styleB(ST_createMultiMeasureRests)) {
                                    //find the actual measure where the volta should stop
                                    Measure* sm = static_cast<Measure*>(startElement());
                                    bool foundMeasure = false;
                                    while(sm != m) {
                                          Measure* mm = sm;
                                          int nn = mm->multiMeasure() - 1;
                                          if (nn > 0) {
                                                // skip to last rest measure of multi measure rest
                                                for (int k = 0; k < nn; ++k) {
                                                      mm = mm->nextMeasure();
                                                      if (mm == m) {
                                                            m = sm;
                                                            foundMeasure = true;
                                                            break;
                                                            }
                                                      }
                                                }
                                          if (foundMeasure)
                                                break;
                                          sm = sm->nextMeasure();
                                          }
                                    x = m->pos().x() + m->bbox().right();
                                    }
                              Segment* seg = m->last();
                              if (seg->segmentType() == Segment::SegEndBarLine) {
                                    Element* e = seg->element(0);
                                    if (e && e->type() == BAR_LINE) {
                                          if (static_cast<BarLine*>(e)->barLineType() == START_REPEAT)
                                                x -= e->width() - _spatium * .5;
                                          else
                                                x -= _spatium * .5;
                                          }
                                    }
                              }
#endif
                        }

                  Q_ASSERT(m->system());
                  *sys = m->system();
                  }
                  break;

            case Spanner::ANCHOR_NOTE:
                  {
                  System* s = static_cast<Note*>(startElement())->chord()->segment()->system();
                  *sys = s;
                  if (grip == GRIP_LINE_START)
                        return startElement()->pagePos() - s->pagePos();
                  else
                        return endElement()->pagePos() - s->pagePos();
                  }

            case Spanner::ANCHOR_CHORD:
                  qFatal("Sline::linePos(): anchor not implemented\n");
                  break;
            }
      qreal y = (*sys)->staves()->isEmpty() ? 0.0 : (*sys)->staffYpage(staffIdx());
      y -= (*sys)->pos().y();
//      x += (*sys)->pos().x();
      return QPointF(x, y);
      }

//---------------------------------------------------------
//   layout
//    compute segments from tick1 tick2
//---------------------------------------------------------

void SLine::layout()
      {
      if (score() == gscore || tick() == -1) {
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

      computeStartElement();
      computeEndElement();

      System* s1;
      System* s2;
      QPointF p1(linePos(GRIP_LINE_START, &s1));
      QPointF p2(linePos(GRIP_LINE_END,   &s2));

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
//                  qDebug("SLine: segments %d needed %d, remove %d", segCount, segmentsNeeded, n);
                  for (int i = 0; i < n; ++i) {
                        if (spannerSegments().isEmpty()) {
                              qDebug("SLine::layout(): no segment %d, %d expected", i, n);
                              break;
                              }
                        else {
                              /*LineSegment* seg =*/ takeLastSegment();
//                              delete seg;
                              }
                        }
                  }
            }

      int segIdx = 0;
      for (int i = sysIdx1; i <= sysIdx2; ++i) {
            System* system = systems->at(i);
            if (system->isVbox())
                  continue;
            LineSegment* seg = segmentAt(segIdx++);
            seg->setTrack(track());       // DEBUG
            seg->setSystem(system);

            Measure* m = system->firstMeasure();
            Segment* mseg = m->first(Segment::SegChordRest);

            if (sysIdx1 == sysIdx2) {
                  // single segment
                  seg->setSpannerSegmentType(SEGMENT_SINGLE);
                  qreal len = p2.x() - p1.x();
                  if (anchor() == ANCHOR_SEGMENT)
                        len = qMax(3 * spatium(), len);
                  seg->setPos(p1);
                  seg->setPos2(QPointF(len, p2.y() - p1.y()));
                  }
            else if (i == sysIdx1) {
                  // start segment
                  seg->setSpannerSegmentType(SEGMENT_BEGIN);
                  seg->setPos(p1);
                  qreal x2 = system->bbox().right();
                  seg->setPos2(QPointF(x2 - p1.x(), 0.0));
                  }
            else if (i > 0 && i != sysIdx2) {
                  // middle segment
                  seg->setSpannerSegmentType(SEGMENT_MIDDLE);
                  qreal x1 = (mseg ? mseg->pos().x() : 0) + m->pos().x();
                  qreal x2 = system->bbox().right();
                  seg->setPos(QPointF(x1, p1.y()));
                  seg->setPos2(QPointF(x2 - x1, 0.0));
                  }
            else if (i == sysIdx2) {
                  // end segment
                  qreal x1 = (mseg ? mseg->pos().x() : 0) + m->pos().x();
                  qreal len = p2.x() - x1;
                  if (anchor() == ANCHOR_SEGMENT)
                        len = qMax(3 * spatium(), len);
                  seg->setSpannerSegmentType(SEGMENT_END);
                  seg->setPos(QPointF(x1, p1.y()));
                  seg->setPos2(QPointF(len, 0.0));    // p2 is relative to p1
                  }
            seg->layout();
            }
      }

//---------------------------------------------------------
//   writeProperties
//    write properties different from prototype
//---------------------------------------------------------

void SLine::writeProperties(Xml& xml) const
      {
      Element::writeProperties(xml);
      if (_diagonal)
            xml.tag("diagonal", _diagonal);
      if (propertyStyle(P_LINE_WIDTH) != PropertyStyle::STYLED)
            xml.tag("lineWidth", lineWidth().val());
      if (propertyStyle(P_LINE_STYLE) == PropertyStyle::UNSTYLED || (lineStyle() != Qt::SolidLine))
            if (propertyStyle(P_LINE_STYLE) != PropertyStyle::STYLED)
                  xml.tag("lineStyle", int(lineStyle()));
      if (propertyStyle(P_LINE_COLOR) == PropertyStyle::UNSTYLED || (lineColor() != MScore::defaultColor))
            xml.tag("lineColor", lineColor());

      if (anchor() != Spanner::ANCHOR_SEGMENT)
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
            xml.tag("subtype", seg->spannerSegmentType());
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

      if (tag == "tick2")                 // obsolete
            setTick2(e.readInt());
      else if (tag == "tick")             // obsolete
            setTick(e.readInt());
      else if (tag == "Segment") {
            LineSegment* ls = createLineSegment();
            ls->read(e);
            add(ls);
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

const QRectF& SLine::bbox() const
      {
      if (spannerSegments().isEmpty())
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
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id()));
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
      setId(e.intAttribute("id", -1));

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
            case P_DIAGONAL:
                  return _diagonal;
            case P_LINE_COLOR:
                  return _lineColor;
            case P_LINE_WIDTH:
                  return _lineWidth.val();
            case P_LINE_STYLE:
                  return QVariant(int(_lineStyle));
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
            case P_DIAGONAL:
                  _diagonal = v.toBool();
                  break;
            case P_LINE_COLOR:
                  _lineColor = v.value<QColor>();
                  break;
            case P_LINE_WIDTH:
                  _lineWidth = Spatium(v.toDouble());
                  break;
            case P_LINE_STYLE:
                  _lineStyle = Qt::PenStyle(v.toInt());
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
            case P_DIAGONAL:
                  return false;
            case P_LINE_COLOR:
                  return MScore::defaultColor;
            case P_LINE_WIDTH:
                  return 0.15;
            case P_LINE_STYLE:
                  return int(Qt::SolidLine);
            default:
                  return Spanner::propertyDefault(id);
            }
      }

}

