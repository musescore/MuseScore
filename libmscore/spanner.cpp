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
#include "undo.h"
#include "staff.h"

namespace Ms {

//---------------------------------------------------------
//   SpannerSegment
//---------------------------------------------------------

SpannerSegment::SpannerSegment(Score* s)
   : Element(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE | ElementFlag::SEGMENT | ElementFlag::ON_STAFF);
      setSpannerSegmentType(SpannerSegmentType::SINGLE);
      _spanner = 0;
      }

SpannerSegment::SpannerSegment(const SpannerSegment& s)
   : Element(s)
      {
      _spanner = s._spanner;
      _spannerSegmentType = s._spannerSegmentType;
      }

//---------------------------------------------------------
//   system
//---------------------------------------------------------

System* SpannerSegment::system() const
      {
      return toSystem(parent());
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
            case P_ID::COLOR:
            case P_ID::VISIBLE:
                  return spanner()->getProperty(id);
            case P_ID::USER_OFF2:
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
            case P_ID::COLOR:
            case P_ID::VISIBLE:
                 return spanner()->setProperty(id, v);
            case P_ID::USER_OFF2:
                  _userOff2 = v.toPointF();
                  score()->setLayoutAll();
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
            case P_ID::COLOR:
            case P_ID::VISIBLE:
                  return spanner()->propertyDefault(id);
            case P_ID::USER_OFF2:
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
      undoChangeProperty(P_ID::USER_OFF2, QPointF());
      Element::reset();
      spanner()->reset();
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void SpannerSegment::setSelected(bool f)
      {
      for (SpannerSegment* ss : _spanner->spannerSegments())
            ss->Element::setSelected(f);
      _spanner->setSelected(f);
      }

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void SpannerSegment::setVisible(bool f)
      {
      if (_spanner) {
            for (SpannerSegment* ss : _spanner->spannerSegments())
                  ss->Element::setVisible(f);
            _spanner->setVisible(f);
            }
      else
            Element::setVisible(f);
      }

//---------------------------------------------------------
//   setColor
//---------------------------------------------------------

void SpannerSegment::setColor(const QColor& col)
      {
      if (_spanner) {
            for (SpannerSegment* ss : _spanner->spannerSegments())
                  ss->_color = col;
            _spanner->_color = col;
            }
      else
            _color = col;
      }

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

Element* SpannerSegment::nextSegmentElement()
      {
      return spanner()->nextSegmentElement();
      }

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

Element* SpannerSegment::prevSegmentElement()
      {
      return spanner()->prevSegmentElement();
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString SpannerSegment::accessibleInfo() const
      {
      return spanner()->accessibleInfo();
      }

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void SpannerSegment::styleChanged()
      {
      _spanner->styleChanged();
      }

//---------------------------------------------------------
//   triggerLayout
//---------------------------------------------------------

void SpannerSegment::triggerLayout() const
      {
      _spanner->triggerLayout();
      }

//---------------------------------------------------------
//   Spanner
//---------------------------------------------------------

Spanner::Spanner(Score* s)
   : Element(s)
      {
      }

Spanner::Spanner(const Spanner& s)
   : Element(s)
      {
      _anchor       = s._anchor;
      _startElement = s._startElement;
      _endElement   = s._endElement;
      _tick         = s._tick;
      _ticks        = s._ticks;
      _track2       = s._track2;
      }

Spanner::~Spanner()
      {
      qDeleteAll(spannerSegments());
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Spanner::add(Element* e)
      {
      SpannerSegment* ls = toSpannerSegment(e);
      ls->setSpanner(this);
      ls->setSelected(selected());
      ls->setTrack(track());
      segments.append(ls);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Spanner::remove(Element* e)
      {
      SpannerSegment* ss = toSpannerSegment(e);
      if (ss->system())
            ss->system()->remove(ss);
      segments.removeOne(ss);
      }

//---------------------------------------------------------
//   removeUnmanaged
//
//    Remove the Spanner and its segments from objects which may know about them
//
//    This method and the following are used for spanners which are contained within compound elements
//    which manage their parts themselves without using the standard management supplied by Score;
//    Example can be the LyricsLine within a Lyrics element or the FiguredBassLine within a FiguredBass
//    (not implemented yet).
//---------------------------------------------------------

void Spanner::removeUnmanaged()
      {
      for (SpannerSegment* ss : spannerSegments())
            if (ss->system()) {
//                  ss->system()->remove(ss);
                  ss->setSystem(nullptr);
                  }
      score()->removeUnmanagedSpanner(this);
      }

//---------------------------------------------------------
//   undoInserTimeUnmanaged
//---------------------------------------------------------

void Spanner::undoInsertTimeUnmanaged(int fromTick, int len)
      {
      int   newTick1    = tick();
      int   newTick2    = tick2();

      // check spanner start and end point
      if (len > 0) {                // adding time
            if (tick() > fromTick)        // start after insertion point: shift start to right
                  newTick1 += len;
            if (tick2() > fromTick)       // end after insertion point: shift end to right
                  newTick2 += len;
            }
      if (len < 0) {                // removing time
            int toTick = fromTick - len;
            if (tick() > fromTick) {      // start after beginning of removed time
                  if (tick() < toTick) {  // start within removed time: bring start at removing point
                        if (parent()) {
                              parent()->remove(this);
                              return;
                              }
                        else
                              newTick1 = fromTick;
                        }
                  else                    // start after removed time: shift start to left
                        newTick1 += len;
                  }
            if (tick2() > fromTick) {     // end after start of removed time
                  if (tick2() < toTick)   // end within removed time: bring end at removing point
                        newTick2 = fromTick;
                  else                    // end after removed time: shift end to left
                        newTick2 += len;
                  }
            }

      // update properties as required
      if (newTick2 <= newTick1) {               // if no longer any span: remove it
            if (parent())
                  parent()->remove(this);
            }
      else {                                    // if either TICKS or TICK did change, update property
            if (newTick2-newTick1 != tick2()- tick())
                  setProperty(P_ID::SPANNER_TICKS, newTick2-newTick1);
            if (newTick1 != tick())
                  setProperty(P_ID::SPANNER_TICK, newTick1);
            }
      }

//---------------------------------------------------------
//   scanElements
//    used in palettes
//---------------------------------------------------------

void Spanner::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      Q_UNUSED(all);
      for (SpannerSegment* seg : segments)
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
//   getProperty
//---------------------------------------------------------

QVariant Spanner::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::SPANNER_TICK:
                  return tick();
            case P_ID::SPANNER_TICKS:
                  return ticks();
            case P_ID::SPANNER_TRACK2:
                  return track2();
            case P_ID::ANCHOR:
                  return int(anchor());
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
      switch (propertyId) {
            case P_ID::SPANNER_TICK:
                  setTick(v.toInt());
                  setStartElement(0);     // invalidate
                  setEndElement(0);       //
                  if (score() && score()->spannerMap().removeSpanner(this))
                        score()->addSpanner(this);
                  break;
            case P_ID::SPANNER_TICKS:
                  setTicks(v.toInt());
                  setEndElement(0);       // invalidate
                  break;
            case P_ID::TRACK:
                  setTrack(v.toInt());
                  setStartElement(0);     // invalidate
                  break;
            case P_ID::SPANNER_TRACK2:
                  setTrack2(v.toInt());
                  setEndElement(0);       // invalidate
                  break;
            case P_ID::ANCHOR:
                  setAnchor(Anchor(v.toInt()));
                  break;
            default:
                  return Element::setProperty(propertyId, v);
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Spanner::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::ANCHOR:
                  return int(Anchor::SEGMENT);
            default:
                  break;
            }
      return Element::propertyDefault(propertyId);
      }

//---------------------------------------------------------
//   computeStartElement
//---------------------------------------------------------

void Spanner::computeStartElement()
      {
      switch (_anchor) {
            case Anchor::SEGMENT: {
                  Segment* seg = score()->tick2segmentMM(tick(), false, SegmentType::ChordRest);
                  int strack = (track() / VOICES) * VOICES;
                  int etrack = strack + VOICES;
                  _startElement = 0;
                  if (seg) {
                        for (int t = strack; t < etrack; ++t) {
                              if (seg->element(t)) {
                                    _startElement = seg->element(t);
                                    break;
                                    }
                              }
                        }
                  }
                  break;

            case Anchor::MEASURE:
                  _startElement = score()->tick2measure(tick());
                  break;

            case Anchor::CHORD:
            case Anchor::NOTE:
                  return;
            }
      }

//---------------------------------------------------------
//   computeEndElement
//---------------------------------------------------------

void Spanner::computeEndElement()
      {
      switch (_anchor) {
            case Anchor::SEGMENT: {
                  if (track2() == -1)
                        setTrack2(track());
                  if (ticks() == 0 && isTextLine() && parent())   // special case palette
                        setTicks(score()->lastSegment()->tick() - _tick);
                  // find last cr on this staff that ends before tick2

                  _endElement = score()->findCRinStaff(tick2(), track2() / VOICES);
                  if (!_endElement) {
                        qDebug("%s no end element for tick %d", name(), tick2());
                        return;
                        }
                  if (!endCR()->measure()->isMMRest()) {
                        ChordRest* cr = endCR();
                        int nticks = cr->tick() + cr->actualTicks() - _tick;
                        // allow fudge factor for tuplets
                        // TODO: replace with fraction-based calculation
                        int fudge = cr->tuplet() ? 5 : 0;
                        if (qAbs(_ticks - nticks) > fudge) {
                              qDebug("%s ticks changed, %d -> %d", name(), _ticks, nticks);
                              setTicks(nticks);
                              if (type() == ElementType::OTTAVA)
                                    staff()->updateOttava();
                              }
                        }
                  }
                  break;

            case Anchor::MEASURE:
                  _endElement = score()->tick2measure(tick2() - 1);
                  if (!_endElement) {
                        qDebug("Spanner::computeEndElement(), measure not found for tick %d\n", tick2()-1);
                        _endElement = score()->lastMeasure();
                        }
                  break;

            case Anchor::CHORD:
            case Anchor::NOTE:
                  break;
            }
      }

//---------------------------------------------------------
//   startElementFromSpanner
//
//    Given a Spanner and an end element, determines a start element suitable for the end
//    element of a new Spanner, so that it is 'parallel' to the old one.
//    Can be used while cloning a linked Spanner, to update the cloned spanner start and end elements
//    (Spanner(const Spanner&) copies start and end elements from the original to the copy).
//    NOTES:      Only spanners with Anchor::NOTE are currently supported.
//                Going back from end to start ensures the 'other' anchor of this is already set up
//                      (for instance, while cloning staves)
//---------------------------------------------------------

Note* Spanner::startElementFromSpanner(Spanner* sp, Element* newEnd)
      {
      if (sp->anchor() != Anchor::NOTE)
            return nullptr;

      Note*  oldStart   = toNote(sp->startElement());
      Note*  oldEnd     = toNote(sp->endElement());
      if (oldStart == nullptr || oldEnd == nullptr)
            return nullptr;
      Note*  newStart   = nullptr;
      Score* score      = newEnd->score();
      // determine the track where to expect the 'parallel' start element
      int   newTrack    = (newEnd->track() - oldEnd->track()) + oldStart->track();
      // look in notes linked to oldStart for a note with the
      // same score as new score and appropriate track
      for (ScoreElement* newEl : oldStart->linkList())
            if (toNote(newEl)->score() == score && toNote(newEl)->track() == newTrack) {
                  newStart = toNote(newEl);
                  break;
            }
      return newStart;
      }

//---------------------------------------------------------
//   endElementFromSpanner
//
//    Given a Spanner and a start element, determines an end element suitable for the start
//    element of a new Spanner, so that it is 'parallel' to the old one.
//    Can be used while cloning a linked Spanner, to update the cloned spanner start and end elements
//    (Spanner(const Spanner&) copies start and end elements from the original to the copy).
//    NOTES:      Only spanners with Anchor::NOTE are currently supported.
//---------------------------------------------------------

Note* Spanner::endElementFromSpanner(Spanner* sp, Element* newStart)
      {
      if (sp->anchor() != Anchor::NOTE)
            return nullptr;

      Note*  oldStart   = toNote(sp->startElement());
      Note*  oldEnd     = toNote(sp->endElement());
      if (oldStart == nullptr || oldEnd == nullptr)
            return nullptr;
      Note*  newEnd     = nullptr;
      Score* score      = newStart->score();
      // determine the track where to expect the 'parallel' start element
      int   newTrack    = newStart->track() + (oldEnd->track() - oldStart->track());
      // look in notes linked to oldEnd for a note with the
      // same score as new score and appropriate track
      for (ScoreElement* newEl : oldEnd->linkList())
            if (toNote(newEl)->score() == score && toNote(newEl)->track() == newTrack) {
                  newEnd = toNote(newEl);
                  break;
            }
      return newEnd;
      }

//---------------------------------------------------------
//   setNoteSpan
//
//    Sets up all the variables congruent with given start and end note anchors.
//---------------------------------------------------------

void  Spanner::setNoteSpan(Note* startNote, Note* endNote)
      {
      if (_anchor != Anchor::NOTE)
            return;

      setScore(startNote->score());
      setParent(startNote);
      setStartElement(startNote);
      setEndElement(endNote);
      setTick(startNote->chord()->tick());
      setTick2(endNote->chord()->tick());
      setTrack(startNote->track());
      setTrack2(endNote->track());
      }

//---------------------------------------------------------
//   startChord
//---------------------------------------------------------

Chord* Spanner::startChord()
      {
      Q_ASSERT(_anchor == Anchor::CHORD);
      if (!_startElement)
            _startElement = score()->findCR(tick(), track());
      return toChord(_startElement);
      }

//---------------------------------------------------------
//   endChord
//---------------------------------------------------------

Chord* Spanner::endChord()
      {
      Q_ASSERT(_anchor == Anchor::CHORD);

      if (!_endElement && type() == ElementType::SLUR) {
            Segment* s = score()->tick2segmentMM(tick2(), false, SegmentType::ChordRest);
            _endElement = s ? toChordRest(s->element(track2())) : nullptr;
            if (!_endElement->isChord())
                  _endElement = nullptr;
            }
      return toChord(_endElement);
      }

//---------------------------------------------------------
//   startCR
//---------------------------------------------------------

ChordRest* Spanner::startCR()
      {
      Q_ASSERT(_anchor == Anchor::SEGMENT || _anchor == Anchor::CHORD);
      if (!_startElement || _startElement->score() != score())
            _startElement = score()->findCR(tick(), track());
      return toChordRest(_startElement);
      }

//---------------------------------------------------------
//   endCR
//---------------------------------------------------------

ChordRest* Spanner::endCR()
      {
      Q_ASSERT(_anchor == Anchor::SEGMENT || _anchor == Anchor::CHORD);
      if ((!_endElement || _endElement->score() != score())) {
            Segment* s = score()->tick2segmentMM(tick2(), false, SegmentType::ChordRest);
            _endElement = s ? toChordRest(s->element(track2())) : 0;
            }
      return toChordRest(_endElement);
      }

//---------------------------------------------------------
//   startSegment
//---------------------------------------------------------

Segment* Spanner::startSegment() const
      {
      Q_ASSERT(score() != NULL);
      return score()->tick2rightSegment(tick());
      }

//---------------------------------------------------------
//   endSegment
//---------------------------------------------------------

Segment* Spanner::endSegment() const
      {
      return score()->tick2leftSegment(tick2());
      }

//---------------------------------------------------------
//   startMeasure
//---------------------------------------------------------

Measure* Spanner::startMeasure() const
      {
      return toMeasure(_startElement);
      }

//---------------------------------------------------------
//   endMeasure
//---------------------------------------------------------

Measure* Spanner::endMeasure() const
      {
      return toMeasure(_endElement);
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Spanner::setSelected(bool f)
      {
      for (SpannerSegment* ss : spannerSegments())
            ss->Element::setSelected(f);
      Element::setSelected(f);
      }

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void Spanner::setVisible(bool f)
      {
      for (SpannerSegment* ss : spannerSegments())
            ss->Element::setVisible(f);
      Element::setVisible(f);
      }

//---------------------------------------------------------
//   setColor
//---------------------------------------------------------

void Spanner::setColor(const QColor& col)
      {
      for (SpannerSegment* ss : spannerSegments())
            ss->setColor(col);
      _color = col;
      }

//---------------------------------------------------------
//   setStartElement
//---------------------------------------------------------

void Spanner::setStartElement(Element* e)
      {
#ifndef NDEBUG
      if (_anchor == Anchor::NOTE)
            Q_ASSERT(!e || e->type() == ElementType::NOTE);
#endif
      _startElement = e;
      }

//---------------------------------------------------------
//   setEndElement
//---------------------------------------------------------

void Spanner::setEndElement(Element* e)
      {
#ifndef NDEBUG
      if (_anchor == Anchor::NOTE)
            Q_ASSERT(!e || e->type() == ElementType::NOTE);
#endif
      _endElement = e;
      }

//---------------------------------------------------------
//   nextSpanner
//---------------------------------------------------------

Spanner* Spanner::nextSpanner(Element* e, int activeStaff)
      {
    std::multimap<int, Spanner*> mmap = score()->spanner();
          auto range = mmap.equal_range(tick());
          if (range.first != range.second) { // range not empty
                for (auto i = range.first; i != range.second; ++i) {
                      if (i->second == e) {
                            while (i != range.second) {
                                  ++i;
                                  if (i == range.second)
                                        return nullptr;
                                  Spanner* s =  i->second;
                                  Element* st = s->startElement();
                                  if (!st)
                                        continue;
                                  if (s->startSegment() == toSpanner(e)->startSegment() &&
                                      st->staffIdx() == activeStaff)
                                        return s;
                                  //else
                                        //return nullptr;
                                  }
                            break;
                           /* else {
                                  break;
                                  }*/
                            }
                      }
                 }
          return nullptr;
      }

//---------------------------------------------------------
//   prevSpanner
//---------------------------------------------------------

Spanner* Spanner::prevSpanner(Element* e, int activeStaff)
      {
      std::multimap<int, Spanner*> mmap = score()->spanner();
      auto range = mmap.equal_range(tick());
      if (range.first != range.second) { // range not empty
            for (auto i = range.first; i != range.second; ++i) {
                  if (i->second == e) {
                        if (i == range.first)
                              return nullptr;
                        while (i != range.first) {
                              --i;
                              Spanner* s =  i->second;
                              if (s->startSegment() == toSpanner(e)->startSegment() &&
                                  s->startElement()->staffIdx() == activeStaff)
                                    return s;
                              }
                        break;
                        }
                  }
            }
      return nullptr;
      }

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

Element* Spanner::nextSegmentElement()
      {
      Segment* s = startSegment();
      if (s)
            return s->firstElement(staffIdx());
      return score()->firstElement();
      }

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

Element* Spanner::prevSegmentElement()
      {
      Segment* s = endSegment();
      if (s)
            return s->lastElement(staffIdx());
      return score()->lastElement();
      }

//---------------------------------------------------------
//   setTick
//---------------------------------------------------------

void Spanner::setTick(int v)
      {
      _tick = v;
      if (score())
            score()->spannerMap().setDirty();
      }

//---------------------------------------------------------
//   setTick2
//---------------------------------------------------------

void Spanner::setTick2(int v)
      {
      setTicks(v - _tick);
      }

//---------------------------------------------------------
//   setTicks
//---------------------------------------------------------

void Spanner::setTicks(int v)
      {
      _ticks = v;
      if (score())
            score()->spannerMap().setDirty();
      }

//---------------------------------------------------------
//   triggerLayout
//---------------------------------------------------------

void Spanner::triggerLayout() const
      {
      score()->setLayout(_tick);
      score()->setLayout(_tick + _ticks);
      }

//---------------------------------------------------------
//   layoutSystem
//---------------------------------------------------------

SpannerSegment* Spanner::layoutSystem(System*)
      {
      qDebug(" %s", name());
      return 0;
      }

}

