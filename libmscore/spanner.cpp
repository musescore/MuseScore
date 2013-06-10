//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "score.h"
#include "spanner.h"
#include "system.h"
#include "chordrest.h"
#include "segment.h"
#include "measure.h"

namespace Ms {

int Spanner::editTick;
int Spanner::editTickLen;
QList<QPointF> Spanner::userOffsets2;
QList<QPointF> Spanner::userOffsets;

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
            if (s)
                  s->add(this);
            else
                  setParent(0);
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant SpannerSegment::getProperty(P_ID id) const
      {
      switch (id) {
            case P_COLOR:
            case P_VISIBLE:
                  return spanner()->getProperty(id);
            case P_USER_OFF2:
                  return _userOff2;

            default:
                  return Element::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool SpannerSegment::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_COLOR:
            case P_VISIBLE:
                 return spanner()->setProperty(id, v);
            case P_USER_OFF2:
                  _userOff2 = v.toPointF();
                  score()->setLayoutAll(true);
                  break;
            default:
                  return Element::setProperty(id, v);
            }
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant SpannerSegment::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_COLOR:
            case P_VISIBLE:
                  return spanner()->propertyDefault(id);
            case P_USER_OFF2:
                  return QVariant();
            default:
                  return Element::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   Spanner
//---------------------------------------------------------

Spanner::Spanner(Score* s)
   : Element(s)
      {
      _anchor       = ANCHOR_SEGMENT;
      _tick         = -1;
      _tickLen      = -1;
      _id           = 0;
      }

Spanner::Spanner(const Spanner& s)
   : Element(s)
      {
      _anchor  = s._anchor;
      _tick    = s._tick;
      _tickLen = s._tickLen;
      _id      = 0;
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
      foreach (SpannerSegment* seg, segments)
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
      editTick    = _tick;
      editTickLen = _tickLen;
      userOffsets.clear();
      userOffsets2.clear();
      foreach (SpannerSegment* ss, spannerSegments()) {
            userOffsets.push_back(ss->userOff());
            userOffsets2.push_back(ss->userOff2());
            }
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Spanner::endEdit()
      {
      score()->undoPropertyChanged(this, P_SPANNER_TICK, editTick);
      score()->undoPropertyChanged(this, P_SPANNER_TICKLEN, editTickLen);

      if (spannerSegments().size() != userOffsets2.size()) {
            qDebug("SLine::endEdit(): segment size changed");
            return;
            }
      for (int i = 0; i < userOffsets2.size(); ++i) {
            SpannerSegment* ss = segments[i];
            score()->undoPropertyChanged(ss, P_USER_OFF, userOffsets[i]);
            score()->undoPropertyChanged(ss, P_USER_OFF2, userOffsets2[i]);
            }
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
      if (_tick != originalSpanner->_tick || _tickLen != originalSpanner->_tickLen)
            return true;
      if (spannerSegments().size() != originalSpanner->spannerSegments().size())
            return true;
      for (int i = 0; i < segments.size(); ++i) {
            if (segments[i]->isEdited(originalSpanner->segments[i]))
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Spanner::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_SPANNER_TICK:
                  return tick();
            case P_SPANNER_TICKLEN:
                  return tickLen();
            default:
                  break;
            }
      return Element::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Spanner::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case P_SPANNER_TICK:
                  setTick(v.toInt());
                  break;
            case P_SPANNER_TICKLEN:
                  setTickLen(v.toInt());
                  break;
            default:
                  if (!Element::setProperty(propertyId, v))
                        return false;
                  break;
            }
      score()->setLayoutAll(true);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Spanner::propertyDefault(P_ID propertyId) const
      {
      switch(propertyId) {
            default:
                  break;
            }
      return Element::propertyDefault(propertyId);
      }

Segment* Spanner::startSegment() const { return score()->tick2segment(tick()); }
Segment* Spanner::endSegment() const   { return score()->tick2segment(tick2()); }

//---------------------------------------------------------
//   findCR
//---------------------------------------------------------

ChordRest* Score::findCR(int tick, int track) const
      {
      Segment* s = tick2segment(tick);
      if (s)
            return static_cast<ChordRest*>(s->element(track));
      return nullptr;
      }

//---------------------------------------------------------
//   startElement
//---------------------------------------------------------

ChordRest* Spanner::startCR() const
      {
      return score()->findCR(tick(), track());
      }

//---------------------------------------------------------
//   endElement
//---------------------------------------------------------

ChordRest* Spanner::endCR() const
      {
      return score()->findCR(tick2(), track());
      }

//---------------------------------------------------------
//   startElement
//---------------------------------------------------------

Element* Spanner::startElement() const
      {
      switch (_anchor) {
            case ANCHOR_SEGMENT:
                  return score()->tick2segment(tick());
            case ANCHOR_MEASURE:
                  {
                  ChordRest* cr = startCR();
                  if (cr)
                        return cr->measure();
                  return nullptr;
                  }
            case ANCHOR_CHORD:
                  return startCR();
            case ANCHOR_NOTE:
                  break;
            }
      return nullptr;
      }

//---------------------------------------------------------
//   endElement
//---------------------------------------------------------

Element* Spanner::endElement() const
      {
      switch (_anchor) {
            case ANCHOR_SEGMENT:
                  return score()->tick2segment(tick2());
            case ANCHOR_MEASURE:
                  {
                  ChordRest* cr = endCR();
                  if (cr)
                        return cr->measure();
                  return nullptr;
                  }
            case ANCHOR_CHORD:
                  return endCR();
            case ANCHOR_NOTE:
                  break;
            }
      return nullptr;
      }
}

