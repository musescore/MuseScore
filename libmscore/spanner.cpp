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

#include "connector.h"
#include "score.h"
#include "spanner.h"
#include "system.h"
#include "chordrest.h"
#include "chord.h"
#include "segment.h"
#include "measure.h"
#include "undo.h"
#include "staff.h"
#include "musescoreCore.h"

namespace Ms {

//-----------------------------------------------------------------------------
//   @@ SpannerWriter
///   Helper class for writing Spanners
//-----------------------------------------------------------------------------

class SpannerWriter : public ConnectorInfoWriter {
   protected:
      const char* tagName() const override { return "Spanner"; }
   public:
      SpannerWriter(XmlWriter& xml, const Element* current, const Spanner* spanner, int track, Fraction frac, bool start);

      static void fillSpannerPosition(Location& l, const MeasureBase* endpoint, const Fraction& tick, bool clipboardmode);
      };

//---------------------------------------------------------
//   SpannerSegment
//---------------------------------------------------------

SpannerSegment::SpannerSegment(Spanner* sp, Score* s, ElementFlags f)
   : Element(s, f)
      {
      _spanner = sp;
      setSpannerSegmentType(SpannerSegmentType::SINGLE);
      }

SpannerSegment::SpannerSegment(Score* s, ElementFlags f)
   : Element(s, f)
      {
      setSpannerSegmentType(SpannerSegmentType::SINGLE);
      _spanner = 0;
      }

SpannerSegment::SpannerSegment(const SpannerSegment& s)
   : Element(s)
      {
      _spanner            = s._spanner;
      _spannerSegmentType = s._spannerSegmentType;
      _p2                 = s._p2;
      _offset2            = s._offset2;
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
//   propertyDelegate
//---------------------------------------------------------

Element* SpannerSegment::propertyDelegate(Pid pid)
      {
      if (pid == Pid::COLOR || pid == Pid::VISIBLE || pid == Pid::PLACEMENT)
            return spanner();
      return 0;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant SpannerSegment::getProperty(Pid pid) const
      {
      if (Element* e = const_cast<SpannerSegment*>(this)->propertyDelegate(pid))
            return e->getProperty(pid);
      switch (pid) {
            case Pid::OFFSET2:
                  return _offset2;
            default:
                  return Element::getProperty(pid);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool SpannerSegment::setProperty(Pid pid, const QVariant& v)
      {
      if (Element* e = propertyDelegate(pid))
            return e->setProperty(pid, v);
      switch (pid) {
            case Pid::OFFSET2:
                  _offset2 = v.toPointF();
                  score()->setLayoutAll();
                  break;
            default:
                  return Element::setProperty(pid, v);
            }
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant SpannerSegment::propertyDefault(Pid pid) const
      {
      if (Element* e = const_cast<SpannerSegment*>(this)->propertyDelegate(pid))
            return e->propertyDefault(pid);
      switch (pid) {
            case Pid::OFFSET2:
                  return QVariant();
            default:
                  return Element::propertyDefault(pid);
            }
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid SpannerSegment::getPropertyStyle(Pid pid) const
      {
      if (Element* e = const_cast<SpannerSegment*>(this)->propertyDelegate(pid))
            return e->getPropertyStyle(pid);
      return Element::getPropertyStyle(pid);
      }

//---------------------------------------------------------
//   propertyFlags
//---------------------------------------------------------

PropertyFlags SpannerSegment::propertyFlags(Pid pid) const
      {
      if (Element* e = const_cast<SpannerSegment*>(this)->propertyDelegate(pid))
            return e->propertyFlags(pid);
      return Element::propertyFlags(pid);
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void SpannerSegment::resetProperty(Pid pid)
      {
      if (Element* e = propertyDelegate(pid))
            return e->resetProperty(pid);
      return Element::resetProperty(pid);
      }

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void SpannerSegment::styleChanged()
      {
      spanner()->styleChanged();
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void SpannerSegment::reset()
      {
      undoChangeProperty(Pid::OFFSET2, QPointF());
      Element::reset();
      spanner()->reset();
      }

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void SpannerSegment::undoChangeProperty(Pid pid, const QVariant& val, PropertyFlags ps)
      {
      if (pid == Pid::AUTOPLACE && (val.toBool() == true && !autoplace())) {
            // Switching autoplacement on. Save user-defined
            // placement properties to undo stack.
            undoPushProperty(Pid::OFFSET2);
            // other will be saved in Element::undoChangeProperty
            }
      Element::undoChangeProperty(pid, val, ps);
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
//   triggerLayout
//---------------------------------------------------------

void SpannerSegment::triggerLayout() const
      {
      if (_spanner)
            _spanner->triggerLayout();
      }

//---------------------------------------------------------
//   Spanner
//---------------------------------------------------------

Spanner::Spanner(Score* s, ElementFlags f)
   : Element(s, f)
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
      qDeleteAll(segments);
      qDeleteAll(unusedSegments);
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
//      ls->setAutoplace(autoplace());
      segments.push_back(ls);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Spanner::remove(Element* e)
      {
      SpannerSegment* ss = toSpannerSegment(e);
      if (ss->system())
            ss->system()->remove(ss);
      segments.erase(std::remove(segments.begin(), segments.end(), ss), segments.end());
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
//   insertTimeUnmanaged
//---------------------------------------------------------

void Spanner::insertTimeUnmanaged(const Fraction& fromTick, const Fraction& len)
      {
      Fraction newTick1 = tick();
      Fraction newTick2 = tick2();

      // check spanner start and end point
      if (len > Fraction(0,1)) {          // adding time
            if (tick() > fromTick)        // start after insertion point: shift start to right
                  newTick1 += len;
            if (tick2() > fromTick)       // end after insertion point: shift end to right
                  newTick2 += len;
            }
      if (len < Fraction(0,1)) {          // removing time
            Fraction toTick = fromTick - len;
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
                  setProperty(Pid::SPANNER_TICKS, newTick2-newTick1);
            if (newTick1 != tick())
                  setProperty(Pid::SPANNER_TICK, newTick1);
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

QVariant Spanner::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::SPANNER_TICK:
                  return _tick;
            case Pid::SPANNER_TICKS:
                  return _ticks;
            case Pid::SPANNER_TRACK2:
                  return track2();
            case Pid::ANCHOR:
                  return int(anchor());
            case Pid::LOCATION_STAVES:
                  return (track2() / VOICES) - (track() / VOICES);
            case Pid::LOCATION_VOICES:
                  return (track2() % VOICES) - (track() / VOICES);
            case Pid::LOCATION_FRACTIONS:
                  return _ticks;
            case Pid::LOCATION_MEASURES:
            case Pid::LOCATION_GRACE:
            case Pid::LOCATION_NOTE:
                  return Location::getLocationProperty(propertyId, startElement(), endElement());
            default:
                  break;
            }
      return Element::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Spanner::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::SPANNER_TICK:
                  setTick(v.value<Fraction>());
                  setStartElement(0);     // invalidate
                  setEndElement(0);       //
                  if (score() && score()->spannerMap().removeSpanner(this))
                        score()->addSpanner(this);
                  break;
            case Pid::SPANNER_TICKS:
                  setTicks(v.value<Fraction>());
                  setEndElement(0);       // invalidate
                  break;
            case Pid::TRACK:
                  setTrack(v.toInt());
                  setStartElement(0);     // invalidate
                  break;
            case Pid::SPANNER_TRACK2:
                  setTrack2(v.toInt());
                  setEndElement(0);       // invalidate
                  break;
            case Pid::ANCHOR:
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

QVariant Spanner::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::ANCHOR:
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
                  if (ticks().isZero() && isTextLine() && parent())   // special case palette
                        setTicks(score()->lastSegment()->tick() - _tick);
                  // find last cr on this staff that ends before tick2

                  _endElement = score()->findCRinStaff(tick2(), track2() / VOICES);
                  if (!_endElement) {
                        qDebug("%s no end element for tick %d", name(), tick2().ticks());
                        return;
                        }
                  if (!endCR()->measure()->isMMRest()) {
                        ChordRest* cr = endCR();
                        Fraction nticks = cr->tick() + cr->actualTicks() - _tick;
                        if ((_ticks - nticks).isNotZero()) {
                              qDebug("%s ticks changed, %d -> %d", name(), _ticks.ticks(), nticks.ticks());
                              setTicks(nticks);
                              if (isOttava())
                                    staff()->updateOttava();
                              }
                        }
                  }
                  break;

            case Anchor::MEASURE:
                  _endElement = score()->tick2measure(tick2() - Fraction(1, 1920));
                  if (!_endElement) {
                        qDebug("Spanner::computeEndElement(), measure not found for tick %d\n", tick2().ticks()-1);
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
            Segment* s  = score()->tick2segmentMM(tick2(), false, SegmentType::ChordRest);
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
          auto range = mmap.equal_range(tick().ticks());
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
      auto range = mmap.equal_range(tick().ticks());
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

void Spanner::setTick(const Fraction& v)
      {
      _tick = v;
      if (score())
            score()->spannerMap().setDirty();
      }

//---------------------------------------------------------
//   setTick2
//---------------------------------------------------------

void Spanner::setTick2(const Fraction& f)
      {
      setTicks(f - _tick);
      }

//---------------------------------------------------------
//   setTicks
//---------------------------------------------------------

void Spanner::setTicks(const Fraction& f)
      {
      _ticks = f;
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
//   pushUnusedSegment
//---------------------------------------------------------

void Spanner::pushUnusedSegment(SpannerSegment* seg)
      {
      if (!seg)
            return;
      seg->setSystem(nullptr);
      unusedSegments.push_back(seg);
      }

//---------------------------------------------------------
//   popUnusedSegment
//    Take the next unused segment for reusing it.
//    If there is no unused segments left returns nullptr.
//---------------------------------------------------------

SpannerSegment* Spanner::popUnusedSegment()
      {
      if (unusedSegments.empty())
            return nullptr;
      SpannerSegment* seg = unusedSegments.front();
      unusedSegments.pop_front();
      return seg;
      }

//---------------------------------------------------------
//   reuse
//    called when segment from unusedSegments is added
//    back to the spanner.
//---------------------------------------------------------

void Spanner::reuse(SpannerSegment* seg)
      {
      add(seg);
      }

//---------------------------------------------------------
//   reuseSegments
//    Adds \p number segments from unusedSegments to this
//    spanner via reuse() call. Returns number of new
//    segments that still need to be created, that is,
//    returns (number - nMovedSegments).
//---------------------------------------------------------

int Spanner::reuseSegments(int number)
      {
      while (number > 0) {
            SpannerSegment* seg = popUnusedSegment();
            if (!seg)
                  break;
            reuse(seg);
            --number;
            }
      return number;
      }

//---------------------------------------------------------
//   fixupSegments
//    Makes number of segments match targetNumber.
//    Tries to reuse unused segments. If there are no
//    unused segments left, uses \p createSegment to create
//    the needed segments.
//    Previously unused segments are added via reuse() call
//---------------------------------------------------------

void Spanner::fixupSegments(unsigned int targetNumber, std::function<SpannerSegment*()> createSegment)
      {
      const int diff = targetNumber - int(nsegments());
      if (diff == 0)
            return;
      if (diff > 0) {
            const int ncreate = reuseSegments(diff);
            for (int i = 0; i < ncreate; ++i)
                  add(createSegment());
            }
      else { // diff < 0
            const int nremove = -diff;
            for (int i = 0; i < nremove; ++i) {
                  SpannerSegment* seg = segments.back();
                  segments.pop_back();
                  pushUnusedSegment(seg);
                  }
            }
      }

//---------------------------------------------------------
//   eraseSpannerSegments
//    Completely erase all spanner segments, both used and
//    unused.
//---------------------------------------------------------

void Spanner::eraseSpannerSegments()
      {
      qDeleteAll(segments);
      qDeleteAll(unusedSegments);
      segments.clear();
      unusedSegments.clear();
      }

//---------------------------------------------------------
//   layoutSystem
//---------------------------------------------------------

SpannerSegment* Spanner::layoutSystem(System*)
      {
      qDebug(" %s", name());
      return 0;
      }

//---------------------------------------------------------
//   getNextLayoutSystemSegment
//---------------------------------------------------------

SpannerSegment* Spanner::getNextLayoutSystemSegment(System* system, std::function<SpannerSegment*()> createSegment)
      {
      SpannerSegment* seg = nullptr;
      for (SpannerSegment* ss : spannerSegments()) {
            if (!ss->system()) {
                  seg = ss;
                  break;
                  }
            }
      if (!seg) {
            if ((seg = popUnusedSegment()))
                  reuse(seg);
            else {
                  seg = createSegment();
                  Q_ASSERT(seg);
                  add(seg);
                  }
            }
      seg->setSystem(system);
      seg->setSpanner(this);
      seg->setTrack(track());
      seg->setVisible(visible());
      return seg;
      }

//---------------------------------------------------------
//   layoutSystemsDone
//    Called after layout of all systems is done so precise
//    number of systems for this spanner becomes available.
//---------------------------------------------------------

void Spanner::layoutSystemsDone()
      {
      std::vector<SpannerSegment*> validSegments;
      for (SpannerSegment* seg : segments) {
            if (seg->system())
                  validSegments.push_back(seg);
            else // TODO: score()->selection().remove(ss); needed?
                  pushUnusedSegment(seg);
            }
      segments = std::move(validSegments);
      }

//--------------------------------------------------
//   fraction
//---------------------------------------------------------

#if 0
static Fraction fraction(const XmlWriter& xml, const Element* current, const Fraction& t)
      {
      Fraction tick(t);
      if (!xml.clipboardmode()) {
            const Measure* m = toMeasure(current->findMeasure());
            if (m)
                  tick -= m->tick();
            }
      return tick;
      }
#endif

//--------------------------------------------------
//   Spanner::writeSpannerStart
//---------------------------------------------------------

void Spanner::writeSpannerStart(XmlWriter& xml, const Element* current, int track, Fraction frac) const
      {
      SpannerWriter w(xml, current, this, track, frac, true);
      w.write();
      }

//--------------------------------------------------
//   Spanner::writeSpannerEnd
//---------------------------------------------------------

void Spanner::writeSpannerEnd(XmlWriter& xml, const Element* current, int track, Fraction frac) const
      {
      SpannerWriter w(xml, current, this, track, frac, false);
      w.write();
      }

//--------------------------------------------------
//   Spanner::readSpanner
//---------------------------------------------------------

void Spanner::readSpanner(XmlReader& e, Element* current, int track)
      {
      std::unique_ptr<ConnectorInfoReader> info(new ConnectorInfoReader(e, current, track));
      ConnectorInfoReader::readConnector(std::move(info), e);
      }

//--------------------------------------------------
//   Spanner::readSpanner
//---------------------------------------------------------

void Spanner::readSpanner(XmlReader& e, Score* current, int track)
      {
      std::unique_ptr<ConnectorInfoReader> info(new ConnectorInfoReader(e, current, track));
      ConnectorInfoReader::readConnector(std::move(info), e);
      }

//---------------------------------------------------------
//   SpannerWriter::fillSpannerPosition
//---------------------------------------------------------

void SpannerWriter::fillSpannerPosition(Location& l, const MeasureBase* m, const Fraction& tick, bool clipboardmode)
      {
      if (clipboardmode) {
            l.setMeasure(0);
            l.setFrac(tick);
            }
      else {
            if (!m) {
                  qWarning("fillSpannerPosition: couldn't find spanner's endpoint's measure");
                  l.setMeasure(0);
                  l.setFrac(tick);
                  return;
                  }
            l.setMeasure(m->measureIndex());
            l.setFrac(tick - m->tick());
            }
      }

//---------------------------------------------------------
//   SpannerWriter::SpannerWriter
//---------------------------------------------------------

SpannerWriter::SpannerWriter(XmlWriter& xml, const Element* current, const Spanner* sp, int track, Fraction frac, bool start)
   : ConnectorInfoWriter(xml, current, sp, track, frac)
      {
      const bool clipboardmode = xml.clipboardmode();
      if (!sp->startElement() || !sp->endElement()) {
            qWarning("SpannerWriter: spanner (%s) doesn't have an endpoint!", sp->name());
            return;
            }
      if (current->isMeasure() || current->isSegment() || (sp->startElement()->type() != current->type())) {
            // (The latter is the hairpins' case, for example, though they are
            // covered by the other checks too.)
            // We cannot determine position of the spanner from its start/end
            // elements and will try to obtain this info from the spanner itself.
            if (!start) {
                  _prevLoc.setTrack(sp->track());
                  Measure* m = sp->score()->tick2measure(sp->tick());
                  fillSpannerPosition(_prevLoc, m, sp->tick(), clipboardmode);
                  }
            else {
                  const int track2 = (sp->track2() != -1) ? sp->track2() : sp->track();
                  _nextLoc.setTrack(track2);
                  Measure* m = sp->score()->tick2measure(sp->tick2());
                  fillSpannerPosition(_nextLoc, m, sp->tick2(), clipboardmode);
                  }
            }
      else {
            // We can obtain the spanner position info from its start/end
            // elements and will prefer this source of information.
            // Reason: some spanners contain no or wrong information (e.g. Ties).
            if (!start)
                  updateLocation(sp->startElement(), _prevLoc, clipboardmode);
            else
                  updateLocation(sp->endElement(), _nextLoc, clipboardmode);
            }
      }

//---------------------------------------------------------
//   autoplaceSpannerSegment
//---------------------------------------------------------

void SpannerSegment::autoplaceSpannerSegment(qreal minDistance)
      {
      if (!parent()) {
            setOffset(QPointF());
            return;
            }
      if (isStyled(Pid::OFFSET))
            setOffset(spanner()->propertyDefault(Pid::OFFSET).toPointF());

      if (spanner()->anchor() == Spanner::Anchor::NOTE)
            return;

      if (visible() && autoplace()) {
            SkylineLine sl(!spanner()->placeAbove());
            sl.add(shape().translated(pos()));
            if (spanner()->placeAbove()) {
                  qreal d  = system()->topDistance(staffIdx(), sl);
                  if (d > -minDistance)
                        rypos() += -(d + minDistance);
                  }
            else {
                  qreal d  = system()->bottomDistance(staffIdx(), sl);
                  if (d > -minDistance)
                        rypos() += d + minDistance;
                  }
            }
      }

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void Spanner::undoChangeProperty(Pid id, const QVariant& v, PropertyFlags ps)
      {
      if (id == Pid::PLACEMENT) {
            ScoreElement::undoChangeProperty(id, v, ps);
            // change offset of all segments if styled

            for (SpannerSegment* s : segments) {
                  if (s->isStyled(Pid::OFFSET)) {
                        s->setOffset(s->propertyDefault(Pid::OFFSET).toPointF());
                        s->triggerLayout();
                        }
                  }
            MuseScoreCore::mscoreCore->updateInspector();
            return;
            }
      Element::undoChangeProperty(id, v, ps);
      }

}

