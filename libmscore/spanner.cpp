//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "spanner.h"
#include "system.h"
#include "chordrest.h"
#include "segment.h"

//---------------------------------------------------------
//   SpannerSegment
//---------------------------------------------------------

SpannerSegment::SpannerSegment(Score* s)
   : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE | ELEMENT_SEGMENT);
      setSpannerSegmentType(SEGMENT_SINGLE);
      _spanner = 0;
      }

SpannerSegment::SpannerSegment(const SpannerSegment& s)
   : Element(s)
      {
      _spanner = s._spanner;
      _spannerSegmentType = s._spannerSegmentType;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void SpannerSegment::startEdit(MuseScoreView*s , const QPointF& p)
      {
      spanner()->startEdit(s, p);
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void SpannerSegment::endEdit()
      {
      spanner()->endEdit();
      }

//---------------------------------------------------------
//   setSystem
//---------------------------------------------------------

void SpannerSegment::setSystem(System* s)
      {
      if (system() != s) {
            if (system())
                  system()->remove(this);
            }
      s->add(this);
      }

//---------------------------------------------------------
//   Spanner
//---------------------------------------------------------

Spanner::Spanner(Score* s)
   : Element(s)
      {
      _next         = 0;
      _startElement = 0;
      _endElement   = 0;
      _anchor       = ANCHOR_SEGMENT;
      _id           = 0;
      _tick1        = -1;
      _tick2        = -1;
      oStartElement = 0;
      oEndElement   = 0;
      }

Spanner::Spanner(const Spanner& s)
   : Element(s)
      {
      _next         = 0;
      _startElement = s._startElement;
      _endElement   = s._endElement;
      _anchor       = s._anchor;
      foreach(SpannerSegment* ss, s.segments) {
            SpannerSegment* nss = ss->clone();
            add(nss);
            }
      }

Spanner::~Spanner()
      {
      foreach(SpannerSegment* ss, spannerSegments())
            delete ss;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Spanner::add(Element* e)
      {
      SpannerSegment* ls = static_cast<SpannerSegment*>(e);
      ls->setSpanner(this);
      segments.append(ls);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Spanner::remove(Element* e)
      {
      SpannerSegment* ss = static_cast<SpannerSegment*>(e);
      if (ss->system())
            ss->system()->remove(ss);
      segments.removeOne(ss);
      }

//---------------------------------------------------------
//   scanElements
//    used for palettes
//---------------------------------------------------------

void Spanner::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      Q_UNUSED(all)
      foreach(SpannerSegment* seg, segments)
            seg->scanElements(data, func, true);
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Spanner::setScore(Score* s)
      {
      Element::setScore(s);
      foreach(SpannerSegment* seg, segments)
            seg->setScore(s);
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Spanner::startEdit(MuseScoreView*, const QPointF&)
      {
      oStartElement = _startElement;
      oEndElement   = _endElement;
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Spanner::setSelected(bool f)
      {
      foreach(SpannerSegment* ss, segments)
            ss->setSelected(f);
      Element::setSelected(f);
      }

//---------------------------------------------------------
//   isEdited
//    compare edited spanner with origSpanner
//---------------------------------------------------------

bool Spanner::isEdited(Spanner* originalSpanner) const
      {
      if (startElement() != originalSpanner->startElement()
         || endElement() != originalSpanner->endElement()) {
            return true;
            }
      if (spannerSegments().size() != originalSpanner->spannerSegments().size())
            return true;
      for (int i = 0; i < segments.size(); ++i) {
            if (segments[i]->isEdited(originalSpanner->segments[i]))
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   startTick
//---------------------------------------------------------

int Spanner::startTick() const
      {
      if (startElement()->isChordRest())
            return static_cast<ChordRest*>(startElement())->tick();
      if (startElement()->type() == SEGMENT)
            return static_cast<Segment*>(startElement())->tick();
      qDebug("Spanner:: unknown spanner start %s\n", startElement()->name());
      return 0;
      }

//---------------------------------------------------------
//   endTick
//---------------------------------------------------------

int Spanner::endTick() const
      {
      if (endElement()->isChordRest())
            return static_cast<ChordRest*>(endElement())->tick();
      if (endElement()->type() == SEGMENT)
            return static_cast<Segment*>(endElement())->tick();
      qDebug("Spanner:: unknown spanner end %s\n", endElement()->name());
      return 0;
      }

//---------------------------------------------------------
//   removeSpannerBack
//---------------------------------------------------------

bool Spanner::removeSpannerBack()
      {
      if (endElement()->isChordRest())
            return static_cast<ChordRest*>(endElement())->removeSpannerBack(this);
      if (endElement()->type() == SEGMENT)
            return static_cast<Segment*>(endElement())->removeSpannerBack(this);
      qDebug("Spanner:: unknown spanner end %s\n", endElement()->name());
      return false;
      }

//---------------------------------------------------------
//   addSpannerBack
//---------------------------------------------------------

void Spanner::addSpannerBack()
      {
      if (endElement()->isChordRest())
            static_cast<ChordRest*>(endElement())->addSpannerBack(this);
      else if (endElement()->type() == SEGMENT)
            static_cast<Segment*>(endElement())->addSpannerBack(this);
      else
            qDebug("Spanner:: unknown spanner end %s\n", endElement()->name());
      }

