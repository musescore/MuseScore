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
      _startSegment  = 0;
      _endSegment    = 0;
      _staffStart    = 0;
      _staffEnd      = 0;
      }
//---------------------------------------------------------
//   RangeAnnotationSegment
//---------------------------------------------------------

RangeAnnotationSegment::RangeAnnotationSegment(const RangeAnnotationSegment& s)
   : SpannerSegment(s)
      {
      _p2       = s._p2;
      _userOff2 = s._userOff2;
      }


int RangeAnnotationSegment::tickStart() const
      {
      return _startSegment->tick();
      }

//---------------------------------------------------------
//   tickEnd
//---------------------------------------------------------

int RangeAnnotationSegment::tickEnd() const
      {
      return _endSegment->tick();
      }

//---------------------------------------------------------
//   setRange
//---------------------------------------------------------

void RangeAnnotationSegment::setRange(Segment* startSegment, Segment* endSegment, int staffStart, int staffEnd)
      {
      _startSegment  = startSegment;
      _endSegment    = endSegment;
      _staffStart    = staffStart;
      _staffEnd      = staffEnd;
      }

//---------------------------------------------------------
//   layout
//    p1, p2  are in System coordinates
//---------------------------------------------------------

void RangeAnnotationSegment::layoutSegment(const QPointF& p1, const QPointF& p2)
      {
      // calculate the co-ordinates of the particular segment which is being layouted
      // and store them as necessary
      }


//---------------------------------------------------------
//   layoutSystem
//---------------------------------------------------------

RangeAnnotationSegment* RangeAnnotation::layoutSystem(System* system)
      {
     // int stick = system->firstMeasure()->tick();
     // int etick = system->lastMeasure()->endTick();

      RangeAnnotationSegment* rangeSegment = 0;
      for (SpannerSegment* ss : segments) {
            if (!ss->system()) {
                  //rangeSegment = toRangeSegment(ss);
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
      // if else conditions go here, to
      // determine the type of the range annotation segment
      // to be BEGIN, MIDDLE, OR END depending upon the
      // relative positions of tick(), stick, etick and so on

      rangeSegment->setSpannerSegmentType(sst);

    //  RangePos rPos;
    //  rangePos(&rPos);

      switch (sst) {
            // call layoutSegment for the rangeannotation depending upon the segment type (begin, middle or end)
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
    //  QRectF rangeRect = QRectF(x1, y1, x2-x1, y2-y1);
      // create a rectangle from the four co-ordinates calculated in layout
      painter->setOpacity(0.4);
      painter->setBackgroundMode(Qt::OpaqueMode);
      //painter.fillRect(rangeRect, Qt::yellow );
      }
//void RangeAnnotation::rangePos(RangePos* sp)
//      {
      // calculate the start and end point of the range annotation
      // relative to the system position
//      }

int RangeAnnotation::tickStart() const
      {
      return _startSegment->tick();
      }

//---------------------------------------------------------
//   tickEnd
//---------------------------------------------------------

int RangeAnnotation::tickEnd() const
      {
      return _endSegment->tick();
      }

//---------------------------------------------------------
//   setRange
//---------------------------------------------------------

void RangeAnnotation::setRange(Segment* startSegment, Segment* endSegment, int staffStart, int staffEnd)
      {
      _startSegment  = startSegment;
      _endSegment    = endSegment;
      _staffStart    = staffStart;
      _staffEnd      = staffEnd;
      }
}

