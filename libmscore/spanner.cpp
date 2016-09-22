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

int   Spanner::editTick;
int   Spanner::editTick2;
int   Spanner::editTrack2;
Note* Spanner::editEndNote;
Note* Spanner::editStartNote;
QList<QPointF> Spanner::userOffsets2;
QList<QPointF> Spanner::userOffsets;

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
            ss->_selected = f;
      _spanner->_selected = f;
      }

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void SpannerSegment::setVisible(bool f)
      {
      if (_spanner) {
            for (SpannerSegment* ss : _spanner->spannerSegments())
                  ss->_visible = f;
            _spanner->_visible = f;
            }
      else
            _visible = f;
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
//   nextElement
//---------------------------------------------------------

Element* SpannerSegment::nextElement()
      {
      return spanner()->nextElement();
      }

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

Element* SpannerSegment::prevElement()
      {
      return spanner()->prevElement();
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
      SpannerSegment* ls = static_cast<SpannerSegment*>(e);
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
      SpannerSegment* ss = static_cast<SpannerSegment*>(e);
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
//   startEdit
//---------------------------------------------------------

void Spanner::startEdit(MuseScoreView*, const QPointF&)
      {
      editTick   = _tick;
      editTick2  = tick2();
      editTrack2 = _track2;
      if (_anchor == Spanner::Anchor::NOTE) {
            editEndNote       = static_cast<Note*>(_endElement);
            editStartNote     = static_cast<Note*>(_startElement);
            }

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
      bool rebuild = false;
      if (_anchor == Spanner::Anchor::NOTE) {
            if (_endElement != editEndNote || _startElement != editStartNote) {
                  // swap original anchor elements into the spanner
                  // and set the new one via an undoable operation
                  Note* newStartNote      = static_cast<Note*>(_startElement);
                  Note* newEndNote        = static_cast<Note*>(_endElement);
                  _startElement           = editStartNote;
                  _endElement             = editEndNote;
                  score()->undo(new ChangeSpannerElements(this, newStartNote, newEndNote));
                  }
            }
      else {
            if (editTick != tick()) {
                  score()->undoPropertyChanged(this, P_ID::SPANNER_TICK, editTick);
                  rebuild = true;
                  }
            // ticks may also change by moving initial anchor, without moving ending anchor
            if (editTick2 != tick2() || editTick2 - editTick != tick2() - tick()) {
                  score()->undoPropertyChanged(this, P_ID::SPANNER_TICKS, editTick2 - editTick);
                  rebuild = true;
                  }
            if (editTrack2 != track2()) {
                  score()->undoPropertyChanged(this, P_ID::SPANNER_TRACK2, editTrack2);
                  rebuild = true;
                  }
            }

      if (rebuild)
            score()->rebuildBspTree();

      if (spannerSegments().size() != userOffsets2.size()) {
            qDebug("Spanner::endEdit(): segment size changed");
            return;
            }

      for (int i = 0; i < userOffsets2.size(); ++i) {
            SpannerSegment* ss = segments[i];
            score()->undoPropertyChanged(ss, P_ID::USER_OFF, userOffsets[i]);
            score()->undoPropertyChanged(ss, P_ID::USER_OFF2, userOffsets2[i]);
            }
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
                  break;
            case P_ID::SPANNER_TICKS:
                  setTicks(v.toInt());
                  break;
            case P_ID::TRACK:
                  setTrack(v.toInt());
                  setStartElement(0);
                  break;
            case P_ID::SPANNER_TRACK2:
                  setTrack2(v.toInt());
                  setEndElement(0);
                  break;
            case P_ID::ANCHOR:
                  setAnchor(Anchor(v.toInt()));
                  break;
            default:
                  if (!Element::setProperty(propertyId, v))
                        return false;
                  break;
            }
      score()->setLayoutAll();
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
                  Segment* seg = score()->tick2segmentMM(tick(), false, Segment::Type::ChordRest);
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
                  if (ticks() == 0 && isTextLine())
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
                              if (type() == Element::Type::OTTAVA)
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

      Note*  oldStart   = static_cast<Note*>(sp->startElement());
      Note*  oldEnd     = static_cast<Note*>(sp->endElement());
      Note*  newStart   = nullptr;
      Score* score      = newEnd->score();
      // determine the track where to expect the 'parallel' start element
      int   newTrack    = newEnd->track() + (oldEnd->track() - oldStart->track());
      // look in notes linked to oldStart for a note with the
      // same score as new score and appropriate track
      for (ScoreElement* newEl : oldStart->linkList())
            if (static_cast<Note*>(newEl)->score() == score
                        && static_cast<Note*>(newEl)->track() == newTrack) {
                  newStart = static_cast<Note*>(newEl);
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

      Note*  oldStart   = static_cast<Note*>(sp->startElement());
      Note*  oldEnd     = static_cast<Note*>(sp->endElement());
      Note*  newEnd     = nullptr;
      Score* score      = newStart->score();
      // determine the track where to expect the 'parallel' start element
      int   newTrack    = newStart->track() + (oldEnd->track() - oldStart->track());
      // look in notes linked to oldEnd for a note with the
      // same score as new score and appropriate track
      for (ScoreElement* newEl : oldEnd->linkList())
            if (static_cast<Note*>(newEl)->score() == score
                        && static_cast<Note*>(newEl)->track() == newTrack) {
                  newEnd = static_cast<Note*>(newEl);
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
      Q_ASSERT(_startElement->type() == Element::Type::CHORD);
      return static_cast<Chord*>(_startElement);
      }

//---------------------------------------------------------
//   endChord
//---------------------------------------------------------

Chord* Spanner::endChord()
      {
      Q_ASSERT(_anchor == Anchor::CHORD);

      if (!_endElement && type() == Element::Type::SLUR) {
            Segment* s = score()->tick2segmentMM(tick2(), false, Segment::Type::ChordRest);
            _endElement = s ? static_cast<ChordRest*>(s->element(track2())) : nullptr;
            if (_endElement->type() != Element::Type::CHORD)
                  _endElement = nullptr;
            }
      return static_cast<Chord*>(_endElement);
      }

//---------------------------------------------------------
//   startCR
//---------------------------------------------------------

ChordRest* Spanner::startCR()
      {
      Q_ASSERT(_anchor == Anchor::SEGMENT || _anchor == Anchor::CHORD);
      if (!_startElement)
            _startElement = score()->findCR(tick(), track());
      return static_cast<ChordRest*>(_startElement);
      }

//---------------------------------------------------------
//   endCR
//---------------------------------------------------------

ChordRest* Spanner::endCR()
      {
      Q_ASSERT(_anchor == Anchor::SEGMENT || _anchor == Anchor::CHORD);
      if (!_endElement && type() == Element::Type::SLUR) {
            Segment* s = score()->tick2segmentMM(tick2(), false, Segment::Type::ChordRest);
            _endElement = s ? static_cast<ChordRest*>(s->element(track2())) : nullptr;
            }
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

//---------------------------------------------------------
//   startMeasure
//---------------------------------------------------------

Measure* Spanner::startMeasure() const
      {
      Q_ASSERT(!_endElement || _endElement->type() == Element::Type::MEASURE);
      return static_cast<Measure*>(_startElement);
      }

//---------------------------------------------------------
//   endMeasure
//---------------------------------------------------------

Measure* Spanner::endMeasure() const
      {
      Q_ASSERT(!_endElement || _endElement->type() == Element::Type::MEASURE);
      return static_cast<Measure*>(_endElement);
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Spanner::setSelected(bool f)
      {
      for (SpannerSegment* ss : spannerSegments())
            ss->setSelected(f);
      _selected = f;
      }

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void Spanner::setVisible(bool f)
      {
      for (SpannerSegment* ss : spannerSegments())
            ss->setVisible(f);
      _visible = f;
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
            Q_ASSERT(!e || e->type() == Element::Type::NOTE);
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
            Q_ASSERT(!e || e->type() == Element::Type::NOTE);
#endif
      _endElement = e;
      }

//---------------------------------------------------------
//   nextElement
//---------------------------------------------------------

Element* Spanner::nextElement()
      {
      Segment* s = startSegment();
      if (s)
            return s->firstElement(staffIdx());
      return score()->firstElement();
      }

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

Element* Spanner::prevElement()
      {
      Segment* s = endSegment();
      if (s)
            return s->lastElement(staffIdx());
      return score()->lastElement();
      }

//---------------------------------------------------------
//   setTick
//   //no: @warning Alters spannerMap - Do not call from within a loop over spannerMap
//---------------------------------------------------------

void Spanner::setTick(int v)
      {
      _tick = v;
// WS: this is a low level function and should have no side effects
//      if (score()) {
//our starting tick changed, we'd need to occupy a different position in the spannerMap
//            if (score()->spannerMap().removeSpanner(this))
//                  score()->addSpanner(this);
//            }
      }

//---------------------------------------------------------
//   setTick2
//---------------------------------------------------------

void Spanner::setTick2(int v)
      {
      _ticks = v - _tick;
      if (score())
            score()->spannerMap().setDirty();
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

