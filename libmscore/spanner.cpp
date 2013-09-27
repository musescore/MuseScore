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
#include "chord.h"
#include "segment.h"
#include "measure.h"

namespace Ms {

int Spanner::editTick;
int Spanner::editTick2;
int Spanner::editTrack2;
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
//   reset
//---------------------------------------------------------

void SpannerSegment::reset()
      {
      score()->undoChangeProperty(this, P_USER_OFF2, QPointF());
      Element::reset();
      spanner()->reset();
      }

//---------------------------------------------------------
//   Spanner
//---------------------------------------------------------

Spanner::Spanner(Score* s)
   : Element(s)
      {
      _anchor       = ANCHOR_SEGMENT;
      _startElement = 0;
      _endElement   = 0;
      _tick         = -1;
      _tick2        = -1;
      _id           = 0;
      }

Spanner::Spanner(const Spanner& s)
   : Element(s)
      {
      _anchor       = s._anchor;
      _startElement = s._startElement;
      _endElement   = s._endElement;
      _tick         = s._tick;
      _tick2        = s._tick2;
      _id           = 0;
      }

Spanner::~Spanner()
      {
      foreach (SpannerSegment* ss, spannerSegments())
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
      editTick  = _tick;
      editTick2 = _tick2;
      editTrack2 = _track2;

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
      if (editTick != tick() || editTick2 != tick2() || editTrack2 != track2()) {
            score()->undoPropertyChanged(this, P_SPANNER_TICK, editTick);
            score()->undoPropertyChanged(this, P_SPANNER_TICK2, editTick2);
            score()->undoPropertyChanged(this, P_SPANNER_TRACK2, editTrack2);
            score()->rebuildBspTree();
            }

      if (spannerSegments().size() != userOffsets2.size()) {
            qDebug("SLine::endEdit(): segment size changed");
            return;
            }
#if 0 //HACK
      for (int i = 0; i < userOffsets2.size(); ++i) {
            SpannerSegment* ss = segments[i];
            score()->undoPropertyChanged(ss, P_USER_OFF, userOffsets[i]);
            score()->undoPropertyChanged(ss, P_USER_OFF2, userOffsets2[i]);
            }
#endif
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
//   getProperty
//---------------------------------------------------------

QVariant Spanner::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_SPANNER_TICK:
                  return tick();
            case P_SPANNER_TICK2:
                  return tick2();
            case P_SPANNER_TRACK2:
                  return track2();
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
            case P_SPANNER_TICK2:
                  setTick2(v.toInt());
                  break;
            case P_SPANNER_TRACK2:
                  setTrack2(v.toInt());
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

//---------------------------------------------------------
//   findCR
//---------------------------------------------------------

ChordRest* Score::findCR(int tick, int track) const
      {
      Segment* s = tick2segment(tick, false, Segment::SegChordRest);
      if (s)
            return static_cast<ChordRest*>(s->element(track));
      return nullptr;
      }

//---------------------------------------------------------
//   computeStartElement
//---------------------------------------------------------

void Spanner::computeStartElement()
      {
      switch (_anchor) {
            case ANCHOR_SEGMENT:
                  _startElement = score()->findCR(tick(), track());
                  break;

            case ANCHOR_MEASURE:
                  _startElement = score()->tick2measure(tick());
                  break;

            case ANCHOR_CHORD:
            case ANCHOR_NOTE:
                  return;
            }
      }

//---------------------------------------------------------
//   computeEndElement
//---------------------------------------------------------

void Spanner::computeEndElement()
      {
      switch (_anchor) {
            case ANCHOR_SEGMENT:
                  _endElement = score()->findCR(tick2(), track2());
                  break;

            case ANCHOR_MEASURE:
                  _endElement = score()->tick2measure(tick2() - 1);
                  if (!_endElement)
                        _endElement = score()->lastMeasure();
                  break;

            case ANCHOR_CHORD:
            case ANCHOR_NOTE:
                  break;
            }
      }

//---------------------------------------------------------
//   setStartChord
//---------------------------------------------------------

void Spanner::setStartChord(Chord* c)
      {
      _anchor = ANCHOR_CHORD;
      _startElement = c;
      }

//---------------------------------------------------------
//   startChord
//---------------------------------------------------------

Chord* Spanner::startChord() const
      {
      Q_ASSERT(_anchor == ANCHOR_CHORD);
      return static_cast<Chord*>(_startElement);
      }

//---------------------------------------------------------
//   setEndChord
//---------------------------------------------------------

void Spanner::setEndChord(Chord* c)
      {
      _endElement = c;
      }

//---------------------------------------------------------
//   endChord
//---------------------------------------------------------

Chord* Spanner::endChord() const
      {
      Q_ASSERT(_anchor == ANCHOR_CHORD);
      return static_cast<Chord*>(_endElement);
      }

//---------------------------------------------------------
//   startCR
//---------------------------------------------------------

ChordRest* Spanner::startCR() const
      {
      Q_ASSERT(_anchor == ANCHOR_SEGMENT || _anchor == ANCHOR_CHORD);
      return static_cast<ChordRest*>(_startElement);
      }

//---------------------------------------------------------
//   endCR
//---------------------------------------------------------

ChordRest* Spanner::endCR() const
      {
      Q_ASSERT(_anchor == ANCHOR_SEGMENT || _anchor == ANCHOR_CHORD);
      return static_cast<ChordRest*>(_endElement);
      }

//---------------------------------------------------------
//   startSegment
//---------------------------------------------------------

Segment* Spanner::startSegment() const
      {
      return score()->tick2rightSegment(tick());
      }

//---------------------------------------------------------
//   endSegment
//---------------------------------------------------------

Segment* Spanner::endSegment() const
      {
      return score()->tick2leftSegment(tick2());
      }
}

