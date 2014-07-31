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

void LineSegment::updateGrips(int* grips, int* defaultGrip, QRectF* grip) const
      {
      *grips = 3;
      *defaultGrip = 2;
      QPointF pp(pagePos());
      grip[int(GripLine::START)].translate(pp);
      grip[int(GripLine::END)].translate(pos2() + pp);
      grip[int(GripLine::MIDDLE)].translate(pos2() * .5 + pp);
      }

//---------------------------------------------------------
//   setGrip
//---------------------------------------------------------

void LineSegment::setGrip(int grip, const QPointF& p)
      {
      QPointF pt(p * spatium());

      switch ((GripLine)grip) {
            case GripLine::START: {
                  QPointF delta(pt - userOff());
                  setUserOff(pt);
                  setUserOff2(userOff2() - delta);
                  }
                  break;
            case GripLine::END:
                  setUserOff2(pt);
                  break;
            case GripLine::MIDDLE:
                  setUserOff(pt);
                  break;
            default:
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
      switch((GripLine)grip) {
            case GripLine::START:
                  p = userOff();
                  break;
            case GripLine::END:
                  p = userOff2();
                  break;
            case GripLine::MIDDLE:
                  p = userOff();
                  break;
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

QPointF LineSegment::gripAnchor(int grip) const
      {
      qreal y = system()->staffYpage(staffIdx());
      if (spannerSegmentType() == SpannerSegmentType::MIDDLE) {
            qreal x;
            switch((GripLine)grip) {
                  case GripLine::START:
                        x = system()->firstMeasure()->abbox().left();
                        break;
                  case GripLine::END:
                        x = system()->lastMeasure()->abbox().right();
                        break;
                  default:
                  case GripLine::MIDDLE:
                  case GripLine::APERTURE:
                        x = 0; // No Anchor
                        y = 0;
                        break;
                  }
            return QPointF(x, y);
            }
      else {
            if (grip == int(GripLine::MIDDLE) || grip == int(GripLine::APERTURE)) // center grip or aperture grip
                  return QPointF(0, 0);
            else {
                  System* s;
                  QPointF p(line()->linePos((GripLine)grip, &s));
                  p.ry() += y - system()->pos().y();
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
         && ((spannerSegmentType() == SpannerSegmentType::SINGLE)
              || (spannerSegmentType() == SpannerSegmentType::BEGIN && curGrip == int(GripLine::START))
              || (spannerSegmentType() == SpannerSegmentType::END && curGrip == int(GripLine::END)))))
            return false;

      LineSegment* ls = 0;
      SLine* l        = line();
      bool bspDirty   = false;
      SpannerSegmentType st = spannerSegmentType();
      int track   = l->track();

      if (l->anchor() == Spanner::Anchor::SEGMENT) {
            Segment* s1 = spanner()->startSegment();
            Segment* s2 = spanner()->endSegment();
            if (!s1 && !s2) {
                  qDebug("LineSegment::edit: no start/end segment");
                  return true;
                  }
            if (key == Qt::Key_Left) {
                  if (curGrip == int(GripLine::START))
                        s1 = prevSeg1(s1, track);
                  else if (curGrip == int(GripLine::END) || curGrip == int(GripLine::MIDDLE))
                        s2 = prevSeg1(s2, track);
                  }
            else if (key == Qt::Key_Right) {
                  if (curGrip == int(GripLine::START))
                        s1 = nextSeg1(s1, track);
                  else if (curGrip == int(GripLine::END) || curGrip == int(GripLine::MIDDLE)) {
                        if ((s2->system()->firstMeasure() == s2->measure())
                           && (s2->tick() == s2->measure()->tick()))
                              bspDirty = true;
                        s2 = nextSeg1(s2, track);
                        }
                  }
            if (s1 == 0 || s2 == 0 || s1->tick() >= s2->tick())
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
                  if (curGrip == int(GripLine::START)) {
                        if (m1->prevMeasure())
                              m1 = m1->prevMeasure();
                        }
                  else if (curGrip == int(GripLine::END) || curGrip == int(GripLine::MIDDLE)) {
                        if (m2 && (m2->system()->firstMeasure() == m2))
                              removeSegment = true;
                        Measure* m = m2->prevMeasure();
                        if (m)
                              m2 = m;
                        }
                  }
            else if (key == Qt::Key_Right) {
                  if (curGrip == int(GripLine::START)) {
                        if (m1->nextMeasure())
                              m1 = m1->nextMeasure();
                        }
                  else if (curGrip == int(GripLine::END) || curGrip == int(GripLine::MIDDLE)) {
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
      if (st == SpannerSegmentType::SINGLE) {
            if (curGrip == int(GripLine::START))
                  nls = l->frontSegment();
            else if (curGrip == int(GripLine::END))
                  nls = l->backSegment();
            }
      else if (st == SpannerSegmentType::BEGIN)
            nls = l->frontSegment();
      else if (st == SpannerSegmentType::END)
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

      switch ((GripLine)ed.curGrip) {
            case GripLine::START: // Resize the begin of element (left grip)
                  setUserOff(userOff() + deltaResize);
                  _userOff2 -= deltaResize;
                  break;
            case GripLine::END: // Resize the end of element (rigth grip)
                  _userOff2 += deltaResize;
                  break;
            case GripLine::MIDDLE: // Move the element (middle grip)
                  setUserOff(userOff() + deltaMove);
                  break;
            default:
                  break;
            }
      if ((line()->anchor() == Spanner::Anchor::NOTE)
         && (ed.curGrip == int(GripLine::START) || ed.curGrip == int(GripLine::END))) {
            //
            // if we touch a different note, change anchor
            //
            Element* e = ed.view->elementNear(ed.pos);
            if (e && e->type() == Element::Type::NOTE) {
                  SLine* l = line();
                  if (ed.curGrip == int(GripLine::END) && e != line()->endElement()) {
                        qDebug("LineSegment: move end anchor");
                        Note* noteOld = static_cast<Note*>(l->endElement());
                        Note* noteNew = static_cast<Note*>(e);

                        noteOld->removeSpannerBack(l);
                        noteNew->addSpannerBack(l);
                        l->setEndElement(noteNew);

                        _userOff2 += noteOld->canvasPos() - noteNew->canvasPos();
                        }
                  else if (ed.curGrip == int(GripLine::START) && e != l->startElement()) {
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
            case P_ID::DIAGONAL:
            case P_ID::LINE_COLOR:
            case P_ID::LINE_WIDTH:
            case P_ID::LINE_STYLE:
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
//    return System/Staff coordinates
//---------------------------------------------------------

QPointF SLine::linePos(GripLine grip, System** sys)
      {
      qreal x = 0.0;
      switch (anchor()) {
            case Spanner::Anchor::SEGMENT:
                  {
                  ChordRest* cr;
                  if (grip == GripLine::START)
                        cr = static_cast<ChordRest*>(startElement());
                  else {
                        cr = static_cast<ChordRest*>(endElement());
                        if (cr)
                              x += cr->width();
                        }

                  int t = grip == GripLine::START ? tick() : tick2();
                  Measure* m = cr ? cr->measure() : score()->tick2measure(t);

                  if (m) {
                        x += cr ? cr->pos().x() + cr->segment()->pos().x() + m->pos().x() : m->tick2pos(t);
                        *sys = m->system();
                        }
                  else
                        *sys = 0;
                  }
                  break;

            case Spanner::Anchor::MEASURE:
                  {
                  // anchor() == Anchor::MEASURE
                  Measure* m;
                  if (grip == GripLine::START) {
                        Q_ASSERT(startElement()->type() == Element::Type::MEASURE);
                        m = static_cast<Measure*>(startElement());
                        x = m->pos().x();
                        if(score()->styleB(StyleIdx::createMultiMeasureRests) && m->hasMMRest()) {
                              x = m->mmRest()->pos().x();
                              }
                        }
                  else {
                        qreal _spatium = spatium();

                        Q_ASSERT(endElement()->type() == Element::Type::MEASURE);
                        m = static_cast<Measure*>(endElement());
                        x = m->pos().x() + m->bbox().right();

                        if (score()->styleB(StyleIdx::createMultiMeasureRests)) {
                              //find the actual measure where the volta should stop
                              Measure* sm = static_cast<Measure*>(startElement());
                              Measure* m = sm;
                              if (sm->hasMMRest())
                                    m = sm->mmRest();
                              while (m->endTick() < tick2()) {
                                    m = m->nextMeasureMM();
                              }
                              x = m->pos().x() + m->bbox().right();
                              }
                        Segment* seg = m->last();
                        if (seg->segmentType() == Segment::Type::EndBarLine) {
                              Element* e = seg->element(0);
                              if (e && e->type() == Element::Type::BAR_LINE) {
                                    if (static_cast<BarLine*>(e)->barLineType() == BarLineType::START_REPEAT)
                                          x -= e->width() - _spatium * .5;
                                    else
                                          x -= _spatium * .5;
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
                  System* s = static_cast<Note*>(startElement())->chord()->segment()->system();
                  *sys = s;
                  Element* e = grip == GripLine::START ? startElement() : endElement();
                  return e->pagePos() - QPointF(s->pagePos().x(), s->staffYpage(e->staffIdx()));
                  }

            case Spanner::Anchor::CHORD:
                  qFatal("Sline::linePos(): anchor not implemented");
                  break;
            }
      return QPointF(x, 0.0);
      }

//---------------------------------------------------------
//   layout
//    compute segments from tick1 tick2
//---------------------------------------------------------

void SLine::layout()
      {
      if (score() == gscore || tick() == -1 || tick2() == -1) {
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
      QPointF p1(linePos(GripLine::START, &s1));
      QPointF p2(linePos(GripLine::END,   &s2));

      QList<System*>* systems = score()->systems();
      int sysIdx1 = systems->indexOf(s1);
      int sysIdx2 = systems->indexOf(s2);
      int segmentsNeeded = 0;

      if (sysIdx1 == -1 || sysIdx2 == -1)
            return;

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
                        else
                              ls->setUserOff(QPointF(0, userOff().y()));
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
            Segment* mseg = m->first(Segment::Type::ChordRest);

            if (sysIdx1 == sysIdx2) {
                  // single segment
                  seg->setSpannerSegmentType(SpannerSegmentType::SINGLE);
                  qreal len = p2.x() - p1.x();
                  if (anchor() == Anchor::SEGMENT)
                        len = qMax(3 * spatium(), len);
                  seg->setPos(p1);
                  seg->setPos2(QPointF(len, p2.y() - p1.y()));
                  }
            else if (i == sysIdx1) {
                  // start segment
                  seg->setSpannerSegmentType(SpannerSegmentType::BEGIN);
                  seg->setPos(p1);
                  qreal x2 = system->bbox().right();
                  seg->setPos2(QPointF(x2 - p1.x(), 0.0));
                  }
            else if (i > 0 && i != sysIdx2) {
                  // middle segment
                  seg->setSpannerSegmentType(SpannerSegmentType::MIDDLE);
                  qreal x1 = (mseg ? mseg->pos().x() : 0) + m->pos().x();
                  qreal x2 = system->bbox().right();
                  seg->setPos(QPointF(x1, p1.y()));
                  seg->setPos2(QPointF(x2 - x1, 0.0));
                  }
            else if (i == sysIdx2) {
                  // end segment
                  qreal x1 = (mseg ? mseg->pos().x() : 0) + m->pos().x();
                  qreal len = p2.x() - x1;
                  if (anchor() == Anchor::SEGMENT)
                        len = qMax(3 * spatium(), len);
                  seg->setSpannerSegmentType(SpannerSegmentType::END);
                  seg->setPos(QPointF(x1, p1.y()));
                  seg->setPos2(QPointF(len, 0.0));    // p2 is relative to p1
                  }
            seg->layout();
            }

      adjustReadPos();
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
      if (propertyStyle(P_ID::LINE_WIDTH) != PropertyStyle::STYLED)
            xml.tag("lineWidth", lineWidth().val());
      if (propertyStyle(P_ID::LINE_STYLE) == PropertyStyle::UNSTYLED || (lineStyle() != Qt::SolidLine))
            if (propertyStyle(P_ID::LINE_STYLE) != PropertyStyle::STYLED)
                  xml.tag("lineStyle", int(lineStyle()));
      if (propertyStyle(P_ID::LINE_COLOR) == PropertyStyle::UNSTYLED || (lineColor() != MScore::defaultColor))
            xml.tag("lineColor", lineColor());

      writeProperty(xml, P_ID::ANCHOR);
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

      if (tag == "tick2")                 // obsolete
            setTick2(e.readInt());
      else if (tag == "tick")             // obsolete
            setTick(e.readInt());
      else if (tag == "Segment") {
            LineSegment* ls = createLineSegment();
            ls->read(e);
            ls->setVisible(visible());
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
                  return _lineWidth.val();
            case P_ID::LINE_STYLE:
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
            case P_ID::DIAGONAL:
                  _diagonal = v.toBool();
                  break;
            case P_ID::LINE_COLOR:
                  _lineColor = v.value<QColor>();
                  break;
            case P_ID::LINE_WIDTH:
                  _lineWidth = Spatium(v.toDouble());
                  break;
            case P_ID::LINE_STYLE:
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
            case P_ID::DIAGONAL:
                  return false;
            case P_ID::LINE_COLOR:
                  return MScore::defaultColor;
            case P_ID::LINE_WIDTH:
                  return 0.15;
            case P_ID::LINE_STYLE:
                  return int(Qt::SolidLine);
            default:
                  return Spanner::propertyDefault(id);
            }
      }

}

