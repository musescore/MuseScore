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

void RangeAnnotationSegment::layoutSegment(const QPointF& p1, const QPointF& p2)
      {
      QRectF rr = QRectF(p1.x(), p1.y(), p2.x()-p1.x(), p2.y()-p1.y());
      setbbox(rr);
      if ((staffIdx() > 0) && score()->mscVersion() < 206 && !readPos().isNull()) {
            QPointF staffOffset;
            if (system() && track() >= 0)
                  staffOffset = QPointF(0.0, system()->staff(staffIdx())->y());
            setReadPos(readPos() + staffOffset);
            }
      adjustReadPos();
      }


//---------------------------------------------------------
//   layoutSystem
//---------------------------------------------------------

RangeAnnotationSegment* RangeAnnotation::layoutSystem(System* system)
      {
      int stick = system->firstMeasure()->tick();
      int etick = system->lastMeasure()->endTick();
      Element* crp = parent();
      qDebug() << crp->tick();
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

      SpannerSegmentType sst;
      if (tick() >= stick) {
            //
            // this is the first call to layoutSystem,
            // processing the first line segment
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
                  rangeSegment->layoutSegment(rPos.p1, rPos.p2);
                  break;
            case SpannerSegmentType::BEGIN:
                  rangeSegment->layoutSegment(rPos.p1, QPointF(system->bbox().width(), rPos.p1.y()));
                  break;
            case SpannerSegmentType::MIDDLE: {
                  qreal x1 = firstNoteRestSegmentX(system);
                  qreal x2 = system->bbox().width();
                  qreal y  = staffIdx() > system->staves()->size() ? system->y() : system->staff(staffIdx())->y();
                  rangeSegment->layoutSegment(QPointF(x1, y), QPointF(x2, y));
                  }
                  break;
            case SpannerSegmentType::END:
                  rangeSegment->layoutSegment(QPointF(firstNoteRestSegmentX(system), rPos.p2.y()), rPos.p2);
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
      pen.setColor(MScore::selectColor[2]);
      pen.setWidthF(2.0 / painter->matrix().m11());
      pen.setStyle(Qt::SolidLine);
      painter->setPen(pen);
      painter->setOpacity(0.4);
      painter->setBackgroundMode(Qt::OpaqueMode);
      painter->fillRect(bbox(), Qt::yellow );
      }

//---------------------------------------------------------
//   rangePos
//    Anchor::NOTE: return anchor note position in system coordinates
//    Other:        return (x position (relative to what?), 0)
//---------------------------------------------------------

void RangeAnnotation::rangePos(RangePos* rp)
      {
      qreal x = 0.0;
      ChordRest* scr = startCR();
      ChordRest* ecr = endCR();
      rp->system1 = scr->measure()->system();
      rp->system2 = ecr->measure()->system();
      if (rp->system1 == 0 || rp->system2 == 0)
            return;
      rp->p1 = scr->pagePos() - rp->system1->pagePos();
      rp->p2 = ecr->pagePos() - rp->system2->pagePos();
      }

}

