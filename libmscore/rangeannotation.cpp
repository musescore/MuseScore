//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================


#include "rangeannotation.h"
#include "textannotation.h"
#include "measure.h"
#include "sym.h"
#include "chord.h"
#include "note.h"
#include "score.h"
#include "staff.h"
#include "part.h"
#include "segment.h"
#include "property.h"
#include "element.h"
#include "score.h"
#include "stafftext.h"
#include "system.h"
#include "xml.h"
#include "text.h"
#include <QRectF>
#include <QPainter>

namespace Ms {

//---------------------------------------------------------
//   RangeAnnotation
//---------------------------------------------------------

RangeAnnotation::RangeAnnotation(Score* s)
  : Spanner(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE | ElementFlag::HAS_TAG);
      _score         = s;
      }
//---------------------------------------------------------
//   RangeAnnotationSegment
//---------------------------------------------------------

RangeAnnotationSegment::RangeAnnotationSegment(Score* score)
   : SpannerSegment(score)
      {
      setFlag(ElementFlag::ON_STAFF, true);
      }

//---------------------------------------------------------
//   firstNoteRestSegmentX
//    in System() coordinates
//    returns the position just after the last non-chordrest segment
//---------------------------------------------------------

qreal RangeAnnotation::firstNoteRestSegmentX(System* system)
      {
      for (const MeasureBase* mb : system->measures()) {
            if (mb->isMeasure()) {
                  const Measure* measure = static_cast<const Measure*>(mb);
                  for (const Segment* seg = measure->first(); seg; seg = seg->next()) {
                        if (seg->isChordRestType()) {
                              // first CR found; back up to previous segment
                              seg = seg->prev();
                              if (seg) {
                                    // find maximum width
                                    qreal width = 0.0;
                                    int n = score()->nstaves();
                                    for (int i = 0; i < n; ++i) {
                                          if (!system->staff(i)->show())
                                                continue;
                                          Element* e = seg->element(i * VOICES);
                                          if (e)
                                                width = qMax(width, e->width());
                                          }
                                    return seg->measure()->pos().x() + seg->pos().x() + width;
                                    }
                              else
                                    return 0.0;
                              }
                        }
                  }
            }
      qDebug("firstNoteRestSegmentX: did not find segment");
      return 0.0;
      }

//---------------------------------------------------------
//   layout
//    p1, p2  are in System coordinates
//---------------------------------------------------------

void RangeAnnotationSegment::layoutSegment(const QPointF& p1, const QPointF& p2, RangeAnnotation* range)
      {
      setPos(p1);
      qreal left = 0.0 - range->getProperty(P_ID::LEFT_MARGIN).toDouble();
      qreal top = 0.0 - range->getProperty(P_ID::TOP_MARGIN).toDouble();
      qreal width =  p2.x() - p1.x() + range->getProperty(P_ID::LEFT_MARGIN).toDouble() + range->getProperty(P_ID::RIGHT_MARGIN).toDouble();
      qreal height = p2.y() - p1.y() + range->getProperty(P_ID::TOP_MARGIN).toDouble() + range->getProperty(P_ID::BOTTOM_MARGIN).toDouble();
      QRectF rr = QRectF(left, top , width, height);
      setbbox(rr);
     }


//---------------------------------------------------------
//   layoutSystem
//---------------------------------------------------------

RangeAnnotationSegment* RangeAnnotation::layoutSystem(System* system)
      {
      int stick = system->firstMeasure()->tick();
      int etick = system->lastMeasure()->endTick();

      RangeAnnotationSegment* rangeSegment = 0;
      for (SpannerSegment* ss : segments) {
            if (!ss->system()) {
                  rangeSegment = static_cast<RangeAnnotationSegment*>(ss);
                  break;
                  }
            }
      if (!rangeSegment) {
            rangeSegment = new RangeAnnotationSegment(score());
            add(rangeSegment);
            }
      rangeSegment->setSystem(system);
      rangeSegment->setSpanner(this);
      rangeSegment->setColor(color());
      SpannerSegmentType sst;
      computeStartElement();
      computeEndElement();
      if (tick() >= stick) {
            //
            // this is the first call to layoutSystem,
            // processing the first annotation segment
            //
            if (track2() == -1)
                  setTrack2(track());
            if (startCR() == 0 || startCR()->measure() == 0) {
                  qDebug("null start anchor");
                  return rangeSegment;
                  }
            if (endCR() == 0) {     // sanity check
                  setEndElement(startCR());
                  setTick2(tick());
                  }
            sst = tick2() <= etick ? SpannerSegmentType::SINGLE : SpannerSegmentType::BEGIN;
            }
      else if (tick() < stick && tick2() > etick)
            sst = SpannerSegmentType::MIDDLE;
      else
            sst = SpannerSegmentType::END;

      rangeSegment->setSpannerSegmentType(sst);

      RangePos rPos;
      rangePos(&rPos);

      switch (sst) {
            case SpannerSegmentType::SINGLE:
                  rangeSegment->layoutSegment(rPos.p1, rPos.p2, this);
                  break;
            case SpannerSegmentType::BEGIN:
                  rangeSegment->layoutSegment(rPos.p1, QPointF(system->bbox().width(), rPos.p2.y()), this);
                  break;
            case SpannerSegmentType::MIDDLE: {
                  qreal x1 = firstNoteRestSegmentX(system);
                  qreal x2 = system->bbox().width();
                  rangeSegment->layoutSegment(QPointF(x1, 0), QPointF(x2, rPos.p2.y()), this);
                  }
                  break;
            case SpannerSegmentType::END:
                  rangeSegment->layoutSegment(QPointF(firstNoteRestSegmentX(system), 0), rPos.p2, this);
                  break;
            }

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
      return rangeSegment;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void RangeAnnotationSegment::draw(QPainter* painter) const
      {
      painter->setBrush(Qt::NoBrush);
      QPen pen;
      if (selected())
            pen.setColor(Qt::lightGray);
      else
            pen.setColor(MScore::selectColor[2]);
      pen.setWidthF(2.0 / painter->matrix().m11());
      pen.setStyle(Qt::SolidLine);
      painter->setPen(pen);
      painter->setOpacity(0.4);
      painter->setBackgroundMode(Qt::OpaqueMode);
      if (selected())
            painter->fillRect(bbox(), Qt::lightGray);
      else
            painter->fillRect(bbox(), color());
      painter->setOpacity(1.0);     // set opacity back to normal
  //  Uncomment the following for adding border to the annotation
  //  painter->drawRect(bbox());
      }

//---------------------------------------------------------
//   rangePos
//---------------------------------------------------------

void RangeAnnotation::rangePos(RangePos* rp)
      {
      Segment* ss =  score()->tick2segment(tick());
      Segment* es =  score()->tick2segment(tick2());

      if (!ss || !es) {
            qDebug() << "Error : Null segment";
            return;
            }

      // Reset end segment of spanner segment to end segment of previous measure if end tick of a segment is same as start tick of the next measure
      if (es->rtick() == 0)
            es = es->measure()->prevMeasure()->last();

      if (!ss->measure()->system()) {
            // segment is in a measure that has not been laid out yet
            // this can happen in mmrests
            // first chordrest segment of mmrest instead
            const Measure* mmr = ss->measure()->mmRest1();
            if (mmr && mmr->system())
                  ss = mmr->first(Segment::Type::ChordRest);
            else
                  return;                 // still no system?
            if (!ss)
                  return;                 // no chordrest segment?
            }

      System* system1 = ss->measure()->system();
      System* system2 = es->measure()->system();
      int staffStart = _staffStart;
      int staffEnd = _staffEnd;
      rp->system1 = system1;
      rp->system2 = system2;

      if (rp->system1 == 0 || rp->system2 == 0)
            return;

      rp->p1 = ss->pagePos() - rp->system1->pagePos();
      rp->p2 = es->pagePos() - rp->system2->pagePos();
      SysStaff* ss1   = system1->staff(staffStart);

      // Calculate the last visible staff of the selection
      int lastStaff = 0;
      for (int i = staffEnd - 1; i >= 0; --i) {
            if (score()->staff(i)->show()) {
                  lastStaff = i;
                  break;
                  }
            }

      SysStaff* ss2 = system2->staff(lastStaff);
      if (!ss1 || !ss2) {
            return;
            }
      rp->p2.setY(rp->p2.y() + ss2->bbox().y() - ss1->bbox().y() + ss2->bbox().height());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void RangeAnnotation::write(Xml& xml) const
      {
      if (!xml.canWrite(this))
            return;
      int id = xml.spannerId(this);
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id));
      xml.tag("startTick", int(tick()));
      xml.tag("endTick", int(tick2()));
      xml.tag("staffStart", int(staffStart()));
      xml.tag("staffEnd", int(staffEnd()));
      xml.tag("startTrack", int(track()));
      xml.tag("endTrack", int(track2()));
      xml.tag("color", curColor());
      RangeAnnotation::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void RangeAnnotation::read(XmlReader& e)
      {
      foreach(SpannerSegment* seg, spannerSegments())
            delete seg;
      spannerSegments().clear();

      int id = e.intAttribute("id", -1);
      e.addSpanner(id, this);
      setParent(0);
        while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "startTick") {
                  int stick = e.readInt();
                  Segment* ss = score()->tick2segment(e.readInt());
                  setTick(stick);
                  setStartSegment(ss);
                  }
            else if (tag == "endTick") {
                  int etick = e.readInt();
                  Segment* es = score()->tick2segment(e.readInt());
                  setTick2(etick);
                  setEndSegment(es);
                  }
            else if (tag == "startTrack")
                  setTrack(e.readInt());
            else if (tag == "endTrack")
                  setTrack2(e.readInt());
            else if (tag == "staffStart")
                  setStaffStart(e.readInt());
            else if (tag == "staffEnd")
                  setStaffEnd(e.readInt());
            else if (tag == "color")
                  setColor(e.readColor());
            else if (!RangeAnnotation::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void RangeAnnotation::writeProperties(Xml& xml) const
      {
      writeProperty(xml, P_ID::LINE_WIDTH);
      writeProperty(xml, P_ID::LEFT_MARGIN);
      writeProperty(xml, P_ID::RIGHT_MARGIN);
      writeProperty(xml, P_ID::TOP_MARGIN);
      writeProperty(xml, P_ID::BOTTOM_MARGIN);

      Element::writeProperties(xml);
      }
//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool RangeAnnotation::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "leftMargin")
            _leftMargin = e.readDouble();
      else if (tag == "rightMargin")
            _rightMargin = e.readDouble();
      else if (tag == "topMargin")
            _topMargin = e.readDouble();
      else if (tag == "bottomMargin")
            _bottomMargin = e.readDouble();
      else if (Spanner::readProperties(e))
            ;
      else
            return false;
      return true;
      }
//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant RangeAnnotation::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::LINE_WIDTH:
                  return _borderWidth;
            case P_ID::LEFT_MARGIN:
                  return _leftMargin;
            case P_ID::RIGHT_MARGIN:
                  return _rightMargin;
            case P_ID::TOP_MARGIN:
                  return _topMargin;
            case P_ID::BOTTOM_MARGIN:
                  return _bottomMargin;
            default:
                  return Spanner::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool RangeAnnotation::setProperty(P_ID propertyId, const QVariant& v)
      {
      score()->addRefresh(canvasBoundingRect());
      switch(propertyId) {
            case P_ID::LINE_WIDTH:
                  _borderWidth = v.value<Spatium>();
                  break;
            case P_ID::LEFT_MARGIN:
                  _leftMargin = v.toDouble();
                  break;
            case P_ID::RIGHT_MARGIN:
                  _rightMargin = v.toDouble();
                  break;
            case P_ID::TOP_MARGIN:
                  _topMargin = v.toDouble();
                  break;
            case P_ID::BOTTOM_MARGIN:
                  _bottomMargin = v.toDouble();
                  break;
            default:
                  return Spanner::setProperty(propertyId, v);
            }
      score()->setLayoutAll();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant RangeAnnotation::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_ID::LINE_WIDTH:
                  return Spatium(0.0);
            case P_ID::LEFT_MARGIN:
                  return 3.0;
            case P_ID::RIGHT_MARGIN:
                  return -3.0;
            case P_ID::TOP_MARGIN:
            case P_ID::BOTTOM_MARGIN:
                  return 1.0;
            default:
                  return Spanner::propertyDefault(id);
            }
      }
}

