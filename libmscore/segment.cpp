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

#include "barline.h"
#include "beam.h"
#include "clef.h"
#include "chord.h"
#include "element.h"
#include "hairpin.h"
#include "harmony.h"
#include "instrchange.h"
#include "line.h"
#include "log.h"
#include "keysig.h"
#include "mscore.h"
#include "measure.h"
#include "note.h"
#include "part.h"
#include "repeat.h"
#include "score.h"
#include "segment.h"
#include "sig.h"
#include "staff.h"
#include "staffstate.h"
#include "system.h"
#include "tuplet.h"
#include "timesig.h"
#include "undo.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   subTypeName
//---------------------------------------------------------

const char* Segment::subTypeName() const
      {
      return subTypeName(_segmentType);
      }

const char* Segment::subTypeName(SegmentType t)
      {
      switch(t) {
            case SegmentType::Invalid:              return "Invalid";
            case SegmentType::BeginBarLine:         return "BeginBarLine";
            case SegmentType::HeaderClef:           return "HeaderClef";
            case SegmentType::Clef:                 return "Clef";
            case SegmentType::KeySig:               return "Key Signature";
            case SegmentType::Ambitus:              return "Ambitus";
            case SegmentType::TimeSig:              return "Time Signature";
            case SegmentType::StartRepeatBarLine:   return "Begin Repeat";
            case SegmentType::BarLine:              return "BarLine";
            case SegmentType::Breath:               return "Breath";
            case SegmentType::ChordRest:            return "ChordRest";
            case SegmentType::EndBarLine:           return "EndBarLine";
            case SegmentType::KeySigAnnounce:       return "Key Sig Precaution";
            case SegmentType::TimeSigAnnounce:      return "Time Sig Precaution";
            default:
                  return "??";
            }
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void Segment::setElement(int track, Element* el)
      {
      if (el) {
            el->setParent(this);
            _elist[track] = el;
            setEmpty(false);
            }
      else {
            _elist[track] = 0;
            checkEmpty();
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Segment::removeElement(int track)
      {
      Element* el = element(track);
      if (el->isChordRest()) {
            ChordRest* cr = (ChordRest*)el;
            Beam* beam = cr->beam();
            if (beam)
                  beam->remove(cr);
            Tuplet* tuplet = cr->tuplet();
            if (tuplet)
                  tuplet->remove(cr);
            }
      }

//---------------------------------------------------------
//   Segment
//---------------------------------------------------------

Segment::Segment(Measure* m)
   : Element(m->score(), ElementFlag::EMPTY | ElementFlag::ENABLED | ElementFlag::NOT_SELECTABLE)
      {
      setParent(m);
      init();
      }

Segment::Segment(Measure* m, SegmentType st, const Fraction& t)
   : Element(m->score(), ElementFlag::EMPTY | ElementFlag::ENABLED | ElementFlag::NOT_SELECTABLE)
      {
      setParent(m);
//      Q_ASSERT(t >= Fraction(0,1));
//      Q_ASSERT(t <= m->ticks());
      _segmentType = st;
      _tick = t;
      init();
      }

//---------------------------------------------------------
//   Segment
//---------------------------------------------------------

Segment::Segment(const Segment& s)
   : Element(s)
      {
      _segmentType        = s._segmentType;
      _tick               = s._tick;
      _extraLeadingSpace  = s._extraLeadingSpace;

      for (Element* e : s._annotations)
            add(e->clone());

      _elist.reserve(s._elist.size());
      for (Element* e : s._elist) {
            Element* ne = 0;
            if (e) {
                  ne = e->clone();
                  ne->setParent(this);
                  }
            _elist.push_back(ne);
            }
      _dotPosX = s._dotPosX;
      _shapes  = s._shapes;
      }

//---------------------------------------------------------
//   setSegmentType
//---------------------------------------------------------

void Segment::setSegmentType(SegmentType t)
      {
      Q_ASSERT(_segmentType != SegmentType::Clef || t != SegmentType::ChordRest);
      _segmentType = t;
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Segment::setScore(Score* score)
      {
      Element::setScore(score);
      for (Element* e : _elist) {
            if (e)
                  e->setScore(score);
            }
      for (Element* e : _annotations)
            e->setScore(score);
      }

Segment::~Segment()
      {
      for (Element* e : _elist) {
            if (!e)
                  continue;
            if (e->isTimeSig())
                  e->staff()->removeTimeSig(toTimeSig(e));
            delete e;
            }
      qDeleteAll(_annotations);
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Segment::init()
      {
      int staves = score()->nstaves();
      int tracks = staves * VOICES;
      _elist.assign(tracks, 0);
      _dotPosX.assign(staves, 0.0);
      _shapes.assign(staves, Shape());
      }

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

Fraction Segment::tick() const
      {
      return _tick + measure()->tick();
      }

//---------------------------------------------------------
//   next1
///   return next \a Segment, don’t stop searching at end
///   of \a Measure
//---------------------------------------------------------

Segment* Segment::next1() const
      {
      if (next())
            return next();
      Measure* m = measure()->nextMeasure();
      return m ? m->first() : 0;
      }

//---------------------------------------------------------
//   next1enabled
//---------------------------------------------------------

Segment* Segment::next1enabled() const
      {
      Segment* s = next1();
      while (s && !s->enabled())
            s = s->next1();
      return s;
      }

//---------------------------------------------------------
//   next1MM
//---------------------------------------------------------

Segment* Segment::next1MM() const
      {
      if (next())
            return next();
      Measure* m = measure()->nextMeasureMM();
      return m ? m->first() : 0;
      }

Segment* Segment::next1(SegmentType types) const
      {
      for (Segment* s = next1(); s; s = s->next1()) {
            if (s->segmentType() & types)
                  return s;
            }
      return 0;
      }

Segment* Segment::next1MM(SegmentType types) const
      {
      for (Segment* s = next1MM(); s; s = s->next1MM()) {
            if (s->segmentType() & types)
                  return s;
            }
      return 0;
      }

Segment* Segment::next1MMenabled() const
      {
      Segment* s = next1MM();
      while (s && !s->enabled())
            s = s->next1MM();
      return s;
      }

//---------------------------------------------------------
//   next
//    got to next segment which has subtype in types
//---------------------------------------------------------

Segment* Segment::next(SegmentType types) const
      {
      for (Segment* s = next(); s; s = s->next()) {
            if (s->segmentType() & types)
                  return s;
            }
      return 0;
      }

//---------------------------------------------------------
//   nextInStaff
///   Returns next \c Segment in the staff with given index
//---------------------------------------------------------

Segment* Segment::nextInStaff(int staffIdx, SegmentType type) const
      {
      Segment* s = next(type);
      const int minTrack = staffIdx * VOICES;
      const int maxTrack = (staffIdx + 1) * VOICES - 1;
      while (s && !s->hasElements(minTrack, maxTrack))
            s = s->next(type);
      return s;
      }

//---------------------------------------------------------
//   prev
//    got to previous segment which has subtype in types
//---------------------------------------------------------

Segment* Segment::prev(SegmentType types) const
      {
      for (Segment* s = prev(); s; s = s->prev()) {
            if (s->segmentType() & types)
                  return s;
            }
      return 0;
      }

//---------------------------------------------------------
//   prev1
///   return previous \a Segment, don’t stop searching at
///   \a Measure begin
//---------------------------------------------------------

Segment* Segment::prev1() const
      {
      if (prev())
            return prev();
      Measure* m = measure()->prevMeasure();
      return m ? m->last() : 0;
      }

Segment* Segment::prev1enabled() const
      {
      Segment* s = prev1();
      while (s && !s->enabled())
            s = s->prev1();
      return s;
      }

Segment* Segment::prev1MM() const
      {
      if (prev())
            return prev();
      Measure* m = measure()->prevMeasureMM();
      return m ? m->last() : 0;
      }

Segment* Segment::prev1MMenabled() const
      {
      Segment* s = prev1MM();
      while (s && !s->enabled())
            s = s->prev1MM();
      return s;
      }

Segment* Segment::prev1(SegmentType types) const
      {
      for (Segment* s = prev1(); s; s = s->prev1()) {
            if (s->segmentType() & types)
                  return s;
            }
      return 0;
      }

Segment* Segment::prev1MM(SegmentType types) const
      {
      for (Segment* s = prev1MM(); s; s = s->prev1MM()) {
            if (s->segmentType() & types)
                  return s;
            }
      return 0;
      }

//---------------------------------------------------------
//   nextCR
//    get next ChordRest Segment
//---------------------------------------------------------

Segment* Segment::nextCR(int track, bool sameStaff) const
      {
      int strack = track;
      int etrack;
      if (sameStaff) {
            strack &= ~(VOICES-1);
            etrack = strack + VOICES;
            }
      else {
            etrack = strack + 1;
            }
      for (Segment* seg = next1(); seg; seg = seg->next1()) {
            if (seg->isChordRestType()) {
                  if (track == -1)
                        return seg;
                  for (int t = strack; t < etrack; ++t) {
                        if (seg->element(t))
                              return seg;
                        }
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   nextChordRest
//    get the next ChordRest, start at this segment
//---------------------------------------------------------

ChordRest* Segment::nextChordRest(int track, bool backwards) const
      {
      for (const Segment* seg = this; seg; seg = backwards ? seg->prev1() : seg->next1()) {
            Element* el = seg->element(track);
            if (el && el->isChordRest())
                  return toChordRest(el);
            }
      return 0;
      }

Element* Segment::element(int track) const
{
    int elementsCount = static_cast<int>(_elist.size());
    if (track < 0 || track >= elementsCount) {
        return nullptr;
    }

    return _elist[track];
}

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Segment::insertStaff(int staff)
      {
      int track = staff * VOICES;
      for (int voice = 0; voice < VOICES; ++voice)
            _elist.insert(_elist.begin() + track, 0);
      _dotPosX.insert(_dotPosX.begin()+staff, 0.0);
      _shapes.insert(_shapes.begin()+staff, Shape());

      for (Element* e : _annotations) {
            int staffIdx = e->staffIdx();
            if (staffIdx >= staff && !e->systemFlag())
                  e->setTrack(e->track() + VOICES);
            }
      fixStaffIdx();
      }

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void Segment::removeStaff(int staff)
      {
      int track = staff * VOICES;
      _elist.erase(_elist.begin() + track, _elist.begin() + track + VOICES);
      _dotPosX.erase(_dotPosX.begin() + staff);
      _shapes.erase(_shapes.begin()+staff);

      for (Element* e : _annotations) {
            int staffIdx = e->staffIdx();
            if (staffIdx > staff && !e->systemFlag())
                  e->setTrack(e->track() - VOICES);
            }

      fixStaffIdx();
      }

//---------------------------------------------------------
//   checkElement
//---------------------------------------------------------

void Segment::checkElement(Element* el, int track)
      {
      // prevent segmentation fault on out of bounds index
      IF_ASSERT_FAILED(track < static_cast<int>(_elist.size()))
            return;

      // generated elements can be overwritten
      if (_elist[track] && !_elist[track]->generated()) {
            qDebug("add(%s): there is already a %s at track %d tick %d",
               el->name(),
               _elist[track]->name(),
               track,
               tick().ticks()
               );
//            abort();
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Segment::add(Element* el)
      {
//      qDebug("%p segment %s add(%d, %d, %s)", this, subTypeName(), tick(), el->track(), el->name());

      el->setParent(this);

      int track = el->track();
      Q_ASSERT(track != -1);
      Q_ASSERT(el->score() == score());
      Q_ASSERT(score()->nstaves() * VOICES == int(_elist.size()));
      // make sure offset is correct for staff
      if (el->isStyled(Pid::OFFSET))
            el->setOffset(el->propertyDefault(Pid::OFFSET).toPointF());

      switch (el->type()) {
            case ElementType::REPEAT_MEASURE:
                  _elist[track] = el;
                  setEmpty(false);
                  break;

            case ElementType::TEMPO_TEXT:
            case ElementType::DYNAMIC:
            case ElementType::HARMONY:
            case ElementType::SYMBOL:
            case ElementType::FRET_DIAGRAM:
            case ElementType::STAFF_TEXT:
            case ElementType::SYSTEM_TEXT:
            case ElementType::REHEARSAL_MARK:
            case ElementType::MARKER:
            case ElementType::IMAGE:
            case ElementType::TEXT:
            case ElementType::TREMOLOBAR:
            case ElementType::TAB_DURATION_SYMBOL:
            case ElementType::FIGURED_BASS:
            case ElementType::FERMATA:
            case ElementType::STICKING:
                  _annotations.push_back(el);
                  break;

            case ElementType::STAFF_STATE:
                  if (toStaffState(el)->staffStateType() == StaffStateType::INSTRUMENT) {
                        StaffState* ss = toStaffState(el);
                        Part* part = el->part();
                        part->setInstrument(ss->instrument(), tick());
                        }
                  _annotations.push_back(el);
                  break;

            case ElementType::INSTRUMENT_CHANGE: {
                  InstrumentChange* is = toInstrumentChange(el);
                  Part* part = is->part();
                  part->setInstrument(is->instrument(), tick());
                  _annotations.push_back(el);
                  break;
                  }

            case ElementType::CLEF:
                  Q_ASSERT(_segmentType == SegmentType::Clef || _segmentType == SegmentType::HeaderClef);
                  checkElement(el, track);
                  _elist[track] = el;
                  if (!el->generated()) {
                        el->staff()->setClef(toClef(el));
//                        updateNoteLines(this, el->track());   TODO::necessary?
                        }
                  setEmpty(false);
                  break;

            case ElementType::TIMESIG:
                  Q_ASSERT(segmentType() == SegmentType::TimeSig || segmentType() == SegmentType::TimeSigAnnounce);
                  checkElement(el, track);
                  _elist[track] = el;
                  el->staff()->addTimeSig(toTimeSig(el));
                  setEmpty(false);
                  break;

            case ElementType::KEYSIG:
                  Q_ASSERT(_segmentType == SegmentType::KeySig || _segmentType == SegmentType::KeySigAnnounce);
                  checkElement(el, track);
                  _elist[track] = el;
                  if (!el->generated())
                        el->staff()->setKey(tick(), toKeySig(el)->keySigEvent());
                  setEmpty(false);
                  break;

            case ElementType::CHORD:
            case ElementType::REST:
                  Q_ASSERT(_segmentType == SegmentType::ChordRest);
                  {
                  if (track % VOICES) {
                        bool v;
                        if (el->isChord()) {
                              v = false;
                              // consider chord visible if any note is visible
                              Chord* c = toChord(el);
                              for (Note* n : c->notes()) {
                                    if (n->visible()) {
                                          v = true;
                                          break;
                                          }
                                    }
                              }
                        else
                              v = el->visible();

                        if (v && measure()->score()->ntracks() > track)
                              measure()->setHasVoices(track / VOICES, true);
                        }
                  // the tick position of a tuplet is the tick position of its
                  // first element:
//                  ChordRest* cr = toChordRest(el);
//                  if (cr->tuplet() && !cr->tuplet()->elements().empty() && cr->tuplet()->elements().front() == cr && cr->tuplet()->tick() < 0)
//                        cr->tuplet()->setTick(cr->tick());
                  score()->setPlaylistDirty();
                  }
                  // fall through

            case ElementType::BAR_LINE:
            case ElementType::BREATH:
                  if (track < score()->nstaves() * VOICES) {
                        checkElement(el, track);
                        _elist[track] = el;
                        }
                  setEmpty(false);
                  break;

            case ElementType::AMBITUS:
                  Q_ASSERT(_segmentType == SegmentType::Ambitus);
                  checkElement(el, track);
                  _elist[track] = el;
                  setEmpty(false);
                  break;

            default:
                  qFatal("Segment::add() unknown %s", el->name());
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Segment::remove(Element* el)
      {
// qDebug("%p Segment::remove %s %p", this, el->name(), el);

      if (!el) return;

      int track = el->track();

      switch(el->type()) {
            case ElementType::CHORD:
            case ElementType::REST:
                  {
                  _elist[track] = 0;
                  int staffIdx = el->staffIdx();
                  measure()->checkMultiVoices(staffIdx);
                  // spanners with this cr as start or end element will need relayout
                  SpannerMap& smap = score()->spannerMap();
                  auto spanners = smap.findOverlapping(tick().ticks(), tick().ticks());
                  for (auto interval : spanners) {
                        Spanner* s = interval.value;
                        Element* start = s->startElement();
                        Element* end = s->endElement();
                        if (s->startElement() == el)
                              start = nullptr;
                        if (s->endElement() == el)
                              end = nullptr;
                        if (start != s->startElement() || end != s->endElement())
                              score()->undo(new ChangeStartEndSpanner(s, start, end));
                        }
                  score()->setPlaylistDirty();
                  }
                  break;

            case ElementType::REPEAT_MEASURE:
                  _elist[track] = 0;
                  break;

            case ElementType::DYNAMIC:
            case ElementType::FIGURED_BASS:
            case ElementType::FRET_DIAGRAM:
            case ElementType::HARMONY:
            case ElementType::IMAGE:
            case ElementType::MARKER:
            case ElementType::REHEARSAL_MARK:
            case ElementType::STAFF_TEXT:
            case ElementType::SYSTEM_TEXT:
            case ElementType::SYMBOL:
            case ElementType::TAB_DURATION_SYMBOL:
            case ElementType::TEMPO_TEXT:
            case ElementType::TEXT:
            case ElementType::TREMOLOBAR:
            case ElementType::FERMATA:
            case ElementType::STICKING:
                  removeAnnotation(el);
                  break;

            case ElementType::STAFF_STATE:
                  if (toStaffState(el)->staffStateType() == StaffStateType::INSTRUMENT) {
                        Part* part = el->part();
                        part->removeInstrument(tick());
                        }
                  removeAnnotation(el);
                  break;

            case ElementType::INSTRUMENT_CHANGE:
                  {
                  InstrumentChange* is = toInstrumentChange(el);
                  Part* part = is->part();
                  part->removeInstrument(tick());
                  }
                  removeAnnotation(el);
                  break;

            case ElementType::TIMESIG:
                  _elist[track] = 0;
                  el->staff()->removeTimeSig(toTimeSig(el));
                  break;

            case ElementType::KEYSIG:
                  Q_ASSERT(_elist[track] == el);

                  _elist[track] = 0;
                  if (!el->generated())
                        el->staff()->removeKey(tick());
                  break;

            case ElementType::CLEF:
                  el->staff()->removeClef(toClef(el));
                  // updateNoteLines(this, el->track());
                  // fall through

            case ElementType::BAR_LINE:
            case ElementType::AMBITUS:
                  _elist[track] = 0;
                  break;

            case ElementType::BREATH:
                  _elist[track] = 0;
                  score()->setPause(tick(), 0);
                  break;

            default:
                  qFatal("Segment::remove() unknown %s", el->name());

            }
      triggerLayout();
      checkEmpty();
      }

//---------------------------------------------------------
//   segmentType
//    returns segment type suitable for storage of Element
//---------------------------------------------------------

SegmentType Segment::segmentType(ElementType type)
      {
      switch (type) {
            case ElementType::CHORD:
            case ElementType::REST:
            case ElementType::REPEAT_MEASURE:
            case ElementType::JUMP:
            case ElementType::MARKER:
                  return SegmentType::ChordRest;
            case ElementType::CLEF:
                  return SegmentType::Clef;
            case ElementType::KEYSIG:
                  return SegmentType::KeySig;
            case ElementType::TIMESIG:
                  return SegmentType::TimeSig;
            case ElementType::BAR_LINE:
                  return SegmentType::StartRepeatBarLine;
            case ElementType::BREATH:
                  return SegmentType::Breath;
            default:
                  qDebug("Segment:segmentType():  bad type: <%s>", Element::name(type));
                  return SegmentType::Invalid;
            }
      }

//---------------------------------------------------------
//   sortStaves
//---------------------------------------------------------

void Segment::sortStaves(QList<int>& dst)
      {
      std::vector<Element*> dl;
      dl.reserve(dst.size());

      for (int i = 0; i < dst.size(); ++i) {
            int startTrack = dst[i] * VOICES;
            int endTrack   = startTrack + VOICES;
            for (int k = startTrack; k < endTrack; ++k)
                  dl.push_back(_elist[k]);
            }
      std::swap(_elist, dl);
      QMap<int, int> map;
      for (int k = 0; k < dst.size(); ++k) {
            map.insert(dst[k], k);
            }
      for (Element* e : _annotations) {
            if (!e->systemFlag())
                  e->setTrack(map[e->staffIdx()] * VOICES + e->voice());
            }
      fixStaffIdx();
      }

//---------------------------------------------------------
//   fixStaffIdx
//---------------------------------------------------------

void Segment::fixStaffIdx()
      {
      int track = 0;
      for (Element* e : _elist) {
            if (e)
                  e->setTrack(track);
            ++track;
            }
      }

//---------------------------------------------------------
//   checkEmpty
//---------------------------------------------------------

void Segment::checkEmpty() const
      {
      if (!_annotations.empty()) {
            setEmpty(false);
            return;
            }
      setEmpty(true);
      for (const Element* e : _elist) {
            if (e) {
                  setEmpty(false);
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   swapElements
//---------------------------------------------------------

void Segment::swapElements(int i1, int i2)
      {
      std::iter_swap(_elist.begin() + i1, _elist.begin() + i2);
      if (_elist[i1])
            _elist[i1]->setTrack(i1);
      if (_elist[i2])
            _elist[i2]->setTrack(i2);
      triggerLayout();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Segment::write(XmlWriter& xml) const
      {
      if (written())
            return;
      setWritten(true);
      if (_extraLeadingSpace.isZero())
            return;
      xml.stag(this);
      xml.tag("leadingSpace", _extraLeadingSpace.val());
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Segment::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "subtype")
                  e.skipCurrentElement();
            else if (tag == "leadingSpace")
                  _extraLeadingSpace = Spatium(e.readDouble());
            else if (tag == "trailingSpace")          // obsolete
                  e.readDouble();
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Segment::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::TICK:
                  return _tick;
            case Pid::LEADING_SPACE:
                  return extraLeadingSpace();
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Segment::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::LEADING_SPACE:
                  return Spatium(0.0);
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Segment::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::TICK:
                  setRtick(v.value<Fraction>());
                  break;
            case Pid::LEADING_SPACE:
                  setExtraLeadingSpace(v.value<Spatium>());
                  for (Element* e : _elist) {
                        if(e) 
                              e->setGenerated(false);
                        }
                  break;
            default:
                  return Element::setProperty(propertyId, v);
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   widthInStaff
//---------------------------------------------------------

qreal Segment::widthInStaff(int staffIdx, SegmentType t) const
      {
      const qreal segX = x();
      qreal nextSegX = segX;

      Segment* nextSeg = nextInStaff(staffIdx, t);
      if (nextSeg)
            nextSegX = nextSeg->x();
      else {
            Segment* lastSeg = measure()->lastEnabled();
            if (lastSeg->segmentType() & t)
                  nextSegX = lastSeg->x() + lastSeg->width();
            else
                  nextSegX = lastSeg->x();
            }

      return nextSegX - segX;
      }

//---------------------------------------------------------
//   ticksInStaff
//---------------------------------------------------------

Fraction Segment::ticksInStaff(int staffIdx) const
      {
      const Fraction segTick = tick();
      Fraction nextSegTick = segTick;

      Segment* nextSeg = nextInStaff(staffIdx, durationSegmentsMask);
      if (nextSeg)
            nextSegTick = nextSeg->tick();
      else {
            Segment* lastSeg = measure()->last();
            nextSegTick = lastSeg->tick() + lastSeg->ticks();
            }

      return nextSegTick - segTick;
      }

//---------------------------------------------------------
//   splitsTuplet
//---------------------------------------------------------

bool Segment::splitsTuplet() const
      {
      for (Element* e : _elist) {
            if (!(e && e->isChordRest()))
                  continue;
            ChordRest* cr = toChordRest(e);
            Tuplet* t = cr->tuplet();
            while (t) {
                  if (cr != t->elements().front())
                        return true;
                  t = t->tuplet();
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   operator<
///   return true if segment is before s in list
//---------------------------------------------------------

bool Segment::operator<(const Segment& s) const
      {
      if (tick() < s.tick())
            return true;
      if (tick() > s.tick())
            return false;
      for (Segment* ns = next1(); ns && (ns->tick() == s.tick()); ns = ns->next1()) {
            if (ns == &s)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   operator>
///   return true if segment is after s in list
//---------------------------------------------------------

bool Segment::operator>(const Segment& s) const
      {
      if (tick() > s.tick())
            return true;
      if (tick() < s.tick())
            return false;
      for (Segment* ns = prev1(); ns && (ns->tick() == s.tick()); ns = ns->prev1()) {
            if (ns == &s)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   hasElements
///  Returns true if the segment has at least one element.
///  Annotations are not considered.
//---------------------------------------------------------

bool Segment::hasElements() const
      {
      for (const Element* e : _elist) {
            if (e)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   hasElements
///  return true if an annotation of type type or and element is found in the track range
//---------------------------------------------------------

bool Segment::hasElements(int minTrack, int maxTrack) const
      {
      for (int curTrack = minTrack; curTrack <= maxTrack; curTrack++)
            if (element(curTrack))
                  return true;
      return false;
      }

//---------------------------------------------------------
//   allElementsInvisible
///  return true if all elements in the segment are invisible
//---------------------------------------------------------

bool Segment::allElementsInvisible() const
      {
      if (isType(SegmentType::BarLineType | SegmentType::ChordRest))
            return false;

      for (Element* e : _elist) {
            if (e && e->visible() && !qFuzzyCompare(e->width(), 0.0))
                  return false;
            }

      return true;
      }

//---------------------------------------------------------
//   hasAnnotationOrElement
///  return true if an annotation of type type or and element is found in the track range
//---------------------------------------------------------

bool Segment::hasAnnotationOrElement(ElementType type, int minTrack, int maxTrack) const
      {
      for (const Element* e : _annotations)
            if (e->type() == type && e->track() >= minTrack && e->track() <= maxTrack)
                  return true;
      return hasElements(minTrack, maxTrack);
      }

//---------------------------------------------------------
//   findAnnotation
///  Returns the first found annotation of type type
///  or nullptr if nothing was found.
//---------------------------------------------------------

Element* Segment::findAnnotation(ElementType type, int minTrack, int maxTrack)
      {
      for (Element* e : _annotations)
            if (e->type() == type && e->track() >= minTrack && e->track() <= maxTrack)
                  return e;
      return nullptr;
      }

//---------------------------------------------------------
//   findAnnotations
///  Returns the list of found annotations
///  or nullptr if nothing was found.
//---------------------------------------------------------

std::vector<Element*> Segment::findAnnotations(ElementType type, int minTrack, int maxTrack)
      {
      std::vector<Element*> found;
      for (Element* e : _annotations)
            if (e->type() == type && e->track() >= minTrack && e->track() <= maxTrack)
                  found.push_back(e);
      return found;
      }

//---------------------------------------------------------
//   removeAnnotation
//---------------------------------------------------------

void Segment::removeAnnotation(Element* e)
      {
      for (auto i = _annotations.begin(); i != _annotations.end(); ++i) {
            if (*i == e) {
                  _annotations.erase(i);
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   clearAnnotations
//---------------------------------------------------------

void Segment::clearAnnotations()
      {
      _annotations.clear();
      }

//---------------------------------------------------------
//   elementAt
//    A variant of the element(int) function,
//    specifically intended to be called from QML plugins
//---------------------------------------------------------

Ms::Element* Segment::elementAt(int track) const
      {
      Element* e = track < int(_elist.size()) ? _elist[track] : 0;
      return e;
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Segment::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      for (int track = 0; track < score()->nstaves() * VOICES; ++track) {
            int staffIdx = track/VOICES;
            if (!all && !(score()->staff(staffIdx)->show() && system() && !system()->staves()->empty() && system()->staff(staffIdx)->show())) {
                  track += VOICES - 1;
                  continue;
                  }
            Element* e = element(track);
            if (e == 0)
                  continue;
            // if measure is not visible, handle visible End Bar Lines and Courtesy Clefs in certain conditions (for ossias and cutaway):
            if (!measure()->visible(staffIdx)) {
                  if (isEndBarLineType()) {
                        if (!measure()->nextMeasure())
                              continue;  // skip EndBarLines if next measure == NULL
                        else if (!measure()->isCutawayClef(staffIdx) &&
                                 !(measure()->nextMeasure()->visible(staffIdx) && measure()->nextMeasure()->system() == measure()->system()))
                              continue;  // skip if not on courtesy clef measures and if next measure in system is not visible
                        }
                  else if (isClefType()) {
                        if (!measure()->isCutawayClef(staffIdx))
                              continue;  // skip clefs except for courtesy clefs at the end of invisible measures
                        }
                  else
                        continue;
                  }
            e->scanElements(data, func, all);
            }
      for (Element* e : annotations()) {
            if (all || e->systemFlag() || measure()->visible(e->staffIdx()))
                  e->scanElements(data,  func, all);
            }
      }

//---------------------------------------------------------
//   firstElementForNavigation
//   This function returns the first main element from a
//   segment, or a barline if it spanns in the staff
//---------------------------------------------------------

Element* Segment::firstElementForNavigation(int staff)
      {
      if (isChordRestType()) {
            int strack = staff * VOICES;
            int etrack = strack + VOICES;
            for (int v = strack; v < etrack; ++v) {
                  Element* el = element(v);
                  if (!el)
                        continue;
                  return el->isChord() ? toChord(el)->notes().back() : el;
                  }
            }
      else
            return getElement(staff);
      return 0;
      }

//---------------------------------------------------------
//   lastElementForNavigation
//   This function returns the last main element from a
//   segment, or a barline if it spanns in the staff
//---------------------------------------------------------

Element* Segment::lastElementForNavigation(int staff)
      {
      if (segmentType() == SegmentType::ChordRest) {
            for (int voice = staff * VOICES + (VOICES - 1); voice/VOICES == staff; voice--) {
                  Element* el = element(voice);
                  if (!el) {      //there is no chord or rest on this voice
                        continue;
                        }
                  if (el->isChord()) {
                        return toChord(el)->notes().front();
                        }
                  else {
                        return el;
                        }
                 }
            }
      else {
            return getElement(staff);
            }

      return 0;
      }

//---------------------------------------------------------
//   getElement
//   protected because it is used by the firstElementForNavigation and
//   lastElementForNavigation functions when segment types that have
//   just one element to avoid duplicated code
//
//   Use firstElementForNavigation, or lastElementForNavigation instead of this
//---------------------------------------------------------

Element* Segment::getElement(int staff)
      {
      segmentType();
      if (segmentType() == SegmentType::ChordRest) {
            return firstElementForNavigation(staff);
      }
      else if (segmentType() & (SegmentType::EndBarLine | SegmentType::BarLine | SegmentType::StartRepeatBarLine)) {
            for (int i = staff; i >= 0; i--) {
                  if (!element(i * VOICES))
                        continue;
                  BarLine* b = toBarLine(element(i*VOICES));
                  if (i + b->spanStaff() >= staff)
                        return element(i*VOICES);
                  }
            }
      else
            return element(staff * VOICES);
      return 0;
      }

//---------------------------------------------------------
//   nextAnnotation
//   return next element in _annotations
//---------------------------------------------------------

Element* Segment::nextAnnotation(Element* e)
      {
      if (_annotations.empty() || e == _annotations.back())
            return nullptr;
      auto ei = std::find(_annotations.begin(), _annotations.end(), e);
      if (ei == _annotations.end())
            return nullptr;               // element not found

      // TODO: firstVisibleStaff() for system elements? see Spanner::nextSpanner()
      auto resIt = std::find_if(ei + 1, _annotations.end(), [e](Element* nextElem){
            return nextElem && nextElem->staffIdx() == e->staffIdx();
            });
      
      return _annotations.end() == resIt ? nullptr : *resIt;
      }

//---------------------------------------------------------
//   prevAnnotation
//   return previous element in _annotations
//---------------------------------------------------------

Element* Segment::prevAnnotation(Element* e)
      {
      if (e == _annotations.front())
          return nullptr;
      auto reverseIt = std::find(_annotations.rbegin(), _annotations.rend(), e);
      if (reverseIt == _annotations.rend())
            return nullptr;               // element not found

      // TODO: firstVisibleStaff() for system elements? see Spanner::nextSpanner()
      auto resIt = std::find_if(reverseIt + 1, _annotations.rend(), [e](Element* prevElem){
            return prevElem && prevElem->staffIdx() == e->staffIdx();
            });
      
      return _annotations.rend() == resIt ? nullptr : *resIt;
      }

//---------------------------------------------------------
//   firstAnnotation
//---------------------------------------------------------

Element* Segment::firstAnnotation(Segment* s, int activeStaff)
      {
      for (auto i = s->annotations().begin(); i != s->annotations().end(); ++i) {
            // TODO: firstVisibleStaff() for system elements? see Spanner::nextSpanner()
            if ((*i)->staffIdx() == activeStaff)
                  return *i;
            }
      return nullptr;
      }

//---------------------------------------------------------
//   lastAnnotation
//---------------------------------------------------------

Element* Segment::lastAnnotation(Segment* s, int activeStaff)
      {
      for (auto i = --s->annotations().end(); i != s->annotations().begin(); --i) {
            // TODO: firstVisibleStaff() for system elements? see Spanner::nextSpanner()
            if ((*i)->staffIdx() == activeStaff)
                  return *i;
            }
      auto i = s->annotations().begin();
      if ((*i)->staffIdx() == activeStaff)
            return *i;
      return nullptr;
      }

//--------------------------------------------------------
//   firstInNextSegments
//   Searches for the next segment that has elements on the
//   active staff and returns its first element
//
//   Uses firstElementForNavigation so it also returns a barline if it
//   spans into the active staff
//--------------------------------------------------------

Element* Segment::firstInNextSegments(int activeStaff)
      {
      Element* re = 0;
      Segment* seg = this;
      while (!re) {
            seg = seg->next1MMenabled();
            if (!seg) //end of staff, or score
                  break;

            re = seg->firstElementForNavigation(activeStaff);
            }

      if (re)
            return re;

      if (!seg) { //end of staff
            if (activeStaff + 1 >= score()->nstaves()) //end of score
                  return 0;
            seg = score()->firstSegmentMM(SegmentType::All);
            return seg->element((activeStaff + 1) * VOICES);
            }
      return 0;
      }

//---------------------------------------------------------
//   firstElementOfSegment
//   returns the first non null element in the given segment
//---------------------------------------------------------

Element* Segment::firstElementOfSegment(Segment* s, int activeStaff)
      {
      for (auto i: s->elist()) {
            if (i && i->staffIdx() == activeStaff) {
                  if (i->isDurationElement()) {
                        DurationElement* de = toDurationElement(i);
                        Tuplet* tuplet = de->tuplet();
                        if (tuplet && de == tuplet->elements().front())
                              return tuplet;
                        }
                  if (i->type() == ElementType::CHORD) {
                        Chord* chord = toChord(i);
                        return chord->firstGraceOrNote();
                        }
                  else
                        return i;
                  }
            }
      return nullptr;
      }

//---------------------------------------------------------
//   nextElementOfSegment
//   returns the next element in the given segment
//---------------------------------------------------------

Element* Segment::nextElementOfSegment(Segment* s, Element* e, int activeStaff)
      {
      for (int track = 0; track < score()->nstaves() * VOICES - 1; ++track) {
            if (s->element(track) == 0)
                  continue;
             Element* el = s->element(track);
             if (el == e) {
                 Element* next = s->element(track+1);
                 while (track < score()->nstaves() * VOICES - 1 &&
                        (!next || next->staffIdx() != activeStaff)) {
                       next = s->element(++track);
                       }
                 if (!next || next->staffIdx() != activeStaff)
                       return nullptr;
                 if (next->isChord())
                       return toChord(next)->notes().back();
                 else
                       return next;
             }
             if (el->type() == ElementType::CHORD) {
                   std::vector<Note*> notes = toChord(el)->notes();
                   auto i = std::find(notes.begin(), notes.end(), e);
                   if (i == notes.end())
                         continue;
                   if (i!= notes.begin()) {
                         return *(i-1);
                         }
                   else {
                         Element* nextEl = s->element(++track);
                         while (track < score()->nstaves() * VOICES - 1 &&
                                (!nextEl || nextEl->staffIdx() != activeStaff)) {
                               nextEl = s->element(++track);
                               }
                         if (!nextEl || nextEl->staffIdx() != activeStaff)
                               return nullptr;
                         if (nextEl->isChord())
                               return toChord(nextEl)->notes().back();
                         return nextEl;
                         }
                   }
            }
      return nullptr;
      }

//---------------------------------------------------------
//   prevElementOfSegment
//   returns the previous element in the given segment
//---------------------------------------------------------

Element* Segment::prevElementOfSegment(Segment* s, Element* e, int activeStaff)
      {
      for (int track = score()->nstaves() * VOICES - 1; track > 0; --track) {
            if (s->element(track) == 0)
                  continue;
             Element* el = s->element(track);
             if (el == e) {
                 Element* prev = s->element(track-1);
                 while (track > 0 &&
                        (!prev || prev->staffIdx() != activeStaff)) {
                       prev = s->element(--track);
                       }
                 if (!prev)
                       return nullptr;
                 if (prev->staffIdx() == e->staffIdx()) {
                 if (prev->isChord())
                       return toChord(prev)->notes().front();
                 else
                       return prev;
                       }
                 return nullptr;
             }
             if (el->isChord()) {
                   std::vector<Note*> notes = toChord(el)->notes();
                   auto i = std::find(notes.begin(), notes.end(), e);
                   if (i == notes.end())
                         continue;
                   if (i!= --notes.end()) {
                         return *(i+1);
                         }
                   else {
                         Element* prevEl = s->element(--track);
                         while (track > 0 &&
                                (!prevEl || prevEl->staffIdx() != activeStaff)) {
                               prevEl = s->element(--track);
                               }
                         if (!prevEl)
                               return nullptr;
                         if (prevEl->staffIdx() == e->staffIdx()) {
                         if (prevEl->isChord())
                               return toChord(prevEl)->notes().front();
                         return prevEl;
                               }
                         return nullptr;
                         }
                   }
            }
      return nullptr;
      }

//---------------------------------------------------------
//   lastElementOfSegment
//   returns the last element in the given segment
//---------------------------------------------------------

Element* Segment::lastElementOfSegment(Segment* s, int activeStaff)
      {
      std::vector<Element*> elements = s->elist();
      for (auto i = --elements.end(); i != elements.begin(); --i) {
            if (*i && (*i)->staffIdx() == activeStaff) {
                  if ((*i)->isChord())
                      return toChord(*i)->notes().front();
                  else
                        return *i;
                  }
            }
      auto i = elements.begin();
      if (*i && (*i)->staffIdx() == activeStaff) {
            if ((*i)->type() == ElementType::CHORD)
                  return toChord(*i)->notes().front();
            else
                  return *i;
            }
      return nullptr;
      }

//---------------------------------------------------------
//   firstSpanner
//---------------------------------------------------------

Spanner* Segment::firstSpanner(int activeStaff)
      {
      std::multimap<int, Spanner*> mmap = score()->spanner();
      auto range = mmap.equal_range(tick().ticks());
      if (range.first != range.second){ // range not empty
            for (auto i = range.first; i != range.second; ++i) {
                  Spanner* s = i->second;
                  Element* e = s->startElement();
                  if (!e)
                        continue;
                  if (s->startSegment() == this) {
                        if (e->staffIdx() == activeStaff)
                              return s;
#if 1
                        else if (e->isMeasure() && activeStaff == 0)
                              return s;
#else
                        // TODO: see Spanner::nextSpanner()
                        else if (e->isMeasure()) {
                              SpannerSegment* ss = s->frontSegment();
                              int top = ss && ss->system() ? ss->system()->firstVisibleStaff() : 0;
                              if (activeStaff == top)
                                    return s;
                              }
#endif
                        }
                  }
            }
      return nullptr;
      }

//---------------------------------------------------------
//   lastSpanner
//---------------------------------------------------------

Spanner* Segment::lastSpanner(int activeStaff)
      {
      std::multimap<int, Spanner*> mmap = score()->spanner();
      auto range = mmap.equal_range(tick().ticks());
      if (range.first != range.second){ // range not empty
            for (auto i = --range.second; ; --i) {
                  Spanner* s = i->second;
                  Element* e = s->startElement();
                  if (!e)
                        continue;
                  if (s->startSegment() == this) {
                        if (e->staffIdx() == activeStaff)
                              return s;
#if 1
                        else if (e->isMeasure() && activeStaff == 0)
                              return s;
#else
                        // TODO: see Spanner::nextSpanner()
                        else if (e->isMeasure()) {
                              SpannerSegment* ss = s->frontSegment();
                              int top = ss && ss->system() ? ss->system()->firstVisibleStaff() : 0;
                              if (activeStaff == top)
                                    return s;
                              }
#endif
                        }
                  if (i == range.first)
                        break;
                  }
            }
      return nullptr;
      }



//---------------------------------------------------------
//   notChordRestType
//---------------------------------------------------------

bool Segment::notChordRestType(Segment* s)
      {
      if (s->segmentType() == SegmentType::KeySig ||
          s->segmentType() == SegmentType::TimeSig ||
          s->segmentType() == SegmentType::Clef ||
          s->segmentType() == SegmentType::HeaderClef ||
          s->segmentType() == SegmentType::BeginBarLine ||
          s->segmentType() == SegmentType::EndBarLine ||
          s->segmentType() == SegmentType::BarLine ||
          s->segmentType() == SegmentType::KeySigAnnounce ||
          s->segmentType() == SegmentType::TimeSigAnnounce) {
            return true;
            }
      else {
            return false;
            }
      }

//---------------------------------------------------------
//   nextElement
//---------------------------------------------------------

Element* Segment::nextElement(int activeStaff)
      {
      Element* e = score()->selection().element();
      if (!e && !score()->selection().elements().isEmpty() )
            e = score()->selection().elements().first();
      if (!e)
            return nullptr;
      switch (e->type()) {
            case ElementType::DYNAMIC:
            case ElementType::HARMONY:
            case ElementType::SYMBOL:
            case ElementType::FERMATA:
            case ElementType::FRET_DIAGRAM:
            case ElementType::TEMPO_TEXT:
            case ElementType::STAFF_TEXT:
            case ElementType::SYSTEM_TEXT:
            case ElementType::REHEARSAL_MARK:
            case ElementType::MARKER:
            case ElementType::IMAGE:
            case ElementType::TEXT:
            case ElementType::TREMOLOBAR:
            case ElementType::TAB_DURATION_SYMBOL:
            case ElementType::FIGURED_BASS:
            case ElementType::STAFF_STATE:
            case ElementType::INSTRUMENT_CHANGE:
            case ElementType::STICKING:
            case ElementType::TUPLET:{
                  Element* next = nullptr;
                  if (e->parent() == this)
                        next = nextAnnotation(e);
                  if (next)
                        return next;
                  else {
                        Spanner* s = firstSpanner(activeStaff);
                        if (s)
                              return s->spannerSegments().front();
                        }
                  Segment* nextSegment = this->next1MMenabled();
                  while (nextSegment) {
                        Element* nextEl = nextSegment->firstElementOfSegment(nextSegment, activeStaff);
                        if (nextEl)
                              return nextEl;
                        nextSegment = nextSegment->next1MMenabled();
                        }
                  break;
                  }
            case ElementType::SEGMENT: {
                  if (!_annotations.empty()) {
                        Element* next = firstAnnotation(this, activeStaff);
                        if (next)
                              return next;
                        }
                  Spanner* sp = firstSpanner(activeStaff);
                  if (sp)
                        return sp->spannerSegments().front();

                  Segment* nextSegment = this->next1MMenabled();
                  while (nextSegment) {
                        Element* nextEl = nextSegment->firstElementOfSegment(nextSegment, activeStaff);
                        if (nextEl)
                              return nextEl;
                        nextSegment = nextSegment->next1MMenabled();
                        }
                  break;
                  }
            default: {
                  Element* p;
                  if (e->isTieSegment() || e->isGlissandoSegment()) {
                        SpannerSegment* s = toSpannerSegment(e);
                        Spanner* sp = s->spanner();
                        p = sp->startElement();
                        }
                  else {
                        p = e;
                        Element* pp = p->parent();
                        if (pp->isNote() || pp->isRest() || (pp->isChord() && !p->isNote()))
                              p = pp;
                        }
                  Element* el = p;
                  for (; p && p->type() != ElementType::SEGMENT; p = p->parent()) {
                        ;
                       }
                  Segment* seg = toSegment(p);
                  // next in _elist
                  Element* nextEl = nextElementOfSegment(seg, el, activeStaff);
                  if (nextEl)
                        return nextEl;
                  if (!_annotations.empty()) {
                        Element* next = firstAnnotation(seg, activeStaff);
                        if (next)
                              return next;
                        }
                  Spanner* s = firstSpanner(activeStaff);
                  if (s)
                        return s->spannerSegments().front();
                  Segment* nextSegment =  seg->next1MMenabled();
                  if (!nextSegment) {
                        MeasureBase* mb = measure()->next();
                        return mb && mb->isBox() ? mb : score()->lastElement();
                        }

                  Measure* nsm = nextSegment->measure();
                  if (nsm != measure()) {
                        // check for frame, measure elements
                        MeasureBase* nmb = measure()->nextMM();
                        Element* nme = nsm->el().empty() ? nullptr : nsm->el().front();
                        if (nsm != nmb)
                              return nmb;
                        else if (nme && nme->isTextBase() && nme->staffIdx() == e->staffIdx())
                              return nme;
                        else if (nme && nme->isLayoutBreak() && e->staffIdx() == 0)
                              return nme;
                        }

                  while (nextSegment) {
                        nextEl = nextSegment->firstElementOfSegment(nextSegment, activeStaff);
                        if (nextEl)
                              return nextEl;
                        nextSegment = nextSegment->next1MMenabled();
                        }
                  }
                  break;
            }
            return nullptr;
      }

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

Element* Segment::prevElement(int activeStaff)
      {
      Element* e = score()->selection().element();
      if (!e && !score()->selection().elements().isEmpty() )
            e = score()->selection().elements().last();
      if (!e)
            return nullptr;
      switch (e->type()) {
            case ElementType::DYNAMIC:
            case ElementType::HARMONY:
            case ElementType::SYMBOL:
            case ElementType::FERMATA:
            case ElementType::FRET_DIAGRAM:
            case ElementType::TEMPO_TEXT:
            case ElementType::STAFF_TEXT:
            case ElementType::SYSTEM_TEXT:
            case ElementType::REHEARSAL_MARK:
            case ElementType::MARKER:
            case ElementType::IMAGE:
            case ElementType::TEXT:
            case ElementType::TREMOLOBAR:
            case ElementType::TAB_DURATION_SYMBOL:
            case ElementType::FIGURED_BASS:
            case ElementType::STAFF_STATE:
            case ElementType::INSTRUMENT_CHANGE:
            case ElementType::STICKING: {
                  Element* prev = nullptr;
                  if (e->parent() == this)
                        prev = prevAnnotation(e);
                  if (prev)
                        return prev;
                  if (notChordRestType(this)) {
                        Element* lastEl = lastElementOfSegment(this, activeStaff);
                        if (lastEl)
                              return lastEl;
                        }
                   int track = score()->nstaves() * VOICES - 1;
                   Segment* s = this;
                   Element* el = s->element(track);
                   while (track > 0 && (!el || el->staffIdx() != activeStaff)) {
                         el = s->element(--track);
                         if (track == 0) {
                               track = score()->nstaves() * VOICES - 1;
                               s = s->prev1MMenabled();
                               }
                         }
                   if (el->staffIdx() != activeStaff)
                         return nullptr;
                   if (el->type() == ElementType::CHORD || el->type() == ElementType::REST
                            || el->type() == ElementType::REPEAT_MEASURE) {
                         ChordRest* cr = this->cr(el->track());
                         if (cr) {
                               Element* elCr = cr->lastElementBeforeSegment();
                               if (elCr) {
                                     return elCr;
                                     }
                               }
                          }
                   if (el->type() == ElementType::CHORD) {
                         return toChord(el)->lastElementBeforeSegment();
                         }
                   else if (el->type() == ElementType::NOTE) {
                         Chord* c = toNote(el)->chord();
                         return c->lastElementBeforeSegment();
                         }
                   else {
                         return el;
                         }
                  }
            case ElementType::ARPEGGIO:
            case ElementType::TREMOLO: {
                  Element* el = this->element(e->track());
                  Q_ASSERT(el->type() == ElementType::CHORD);
                  return toChord(el)->prevElement();
                  }
            default: {
                  Element* el = e;
                  Segment* seg = this;
                  if (e->type() == ElementType::TIE_SEGMENT ||
                      e->type() == ElementType::GLISSANDO_SEGMENT) {
                        SpannerSegment* s = toSpannerSegment(e);
                        Spanner* sp = s->spanner();
                        el = sp->startElement();
                        seg = sp->startSegment();
                        }
                  else {
                        Element* ep = e->parent();
                        if (ep->isNote() || ep->isRest() || (ep->isChord() && !e->isNote()))
                              el = e->parent();
                        }

                 Element* prev = seg->prevElementOfSegment(seg, el, activeStaff);
                  if (prev) {
                        if (prev->type() == ElementType::CHORD || prev->type() == ElementType::REST
                               || prev->type() == ElementType::REPEAT_MEASURE) {
                              ChordRest* cr = seg->cr(prev->track());
                              if (cr) {
                                    Element* elCr = cr->lastElementBeforeSegment();
                                    if (elCr) {
                                          return elCr;
                                          }
                                    }
                              }
                        if (prev->type() == ElementType::CHORD) {
                              return toChord(prev)->lastElementBeforeSegment();
                              }
                        else if (prev->type() == ElementType::NOTE) {
                              Chord* c = toNote(prev)->chord();
                              return c->lastElementBeforeSegment();
                              }
                        else {
                              return prev;
                              }
                        }
                   Segment* prevSeg = seg->prev1MMenabled();
                   if (!prevSeg) {
                         MeasureBase* mb = measure()->prev();
                         return mb && mb->isBox() ? mb : score()->firstElement();
                         }

                   Measure* psm = prevSeg->measure();
                   if (psm != measure()) {
                         // check for frame, measure elements
                         MeasureBase* pmb = measure()->prevMM();
                         Element* me = measure()->el().empty() ? nullptr : measure()->el().back();
                         if (me && me->isTextBase() && me->staffIdx() == e->staffIdx())
                               return me;
                         else if (me && me->isLayoutBreak() && e->staffIdx() == 0)
                               return me;
                         else if (psm != pmb)
                              return pmb;
                         }

                   prev = lastElementOfSegment(prevSeg, activeStaff);
                   while (!prev && prevSeg) {
                         prevSeg = prevSeg->prev1MMenabled();
                         prev = lastElementOfSegment(prevSeg, activeStaff);
                         }
                   if (!prevSeg)
                         return score()->firstElement();

                   if (notChordRestType(prevSeg)) {
                         Element* lastEl = lastElementOfSegment(prevSeg, activeStaff);
                         if (lastEl)
                               return lastEl;
                         }
                   Spanner* s1 = prevSeg->lastSpanner(activeStaff);
                   if (s1) {
                         return s1->spannerSegments().front();
                         }
                   else if (!prevSeg->annotations().empty()) {
                         Element* next = lastAnnotation(prevSeg, activeStaff);
                         if (next)
                               return next;
                         }
                   if (prev->type() == ElementType::CHORD || prev->type() == ElementType::REST
                            || prev->type() == ElementType::REPEAT_MEASURE || prev->type() == ElementType::NOTE) {
                         ChordRest* cr = prevSeg->cr(prev->track());
                         if (cr) {
                               Element* elCr = cr->lastElementBeforeSegment();
                               if (elCr) {
                                     return elCr;
                                     }
                               }
                         }
                   if (prev->type() == ElementType::CHORD) {
                         return toChord(prev)->lastElementBeforeSegment();
                         }
                   else if (prev->type() == ElementType::NOTE) {
                         Chord* c = toNote(prev)->chord();
                         return c->lastElementBeforeSegment();
                         }
                   else {
                         return prev;
                         }
                  }
            }
      }

Element* Segment::firstElement(int staffIdx) const
      {
      int startTrack = staffIdx * VOICES;
      int endTrack = startTrack + VOICES;
      for (int track =  startTrack; track < endTrack; ++track) {
            if (Element* item = _elist[track])
                  return item;
            }
      return nullptr;
      }

//--------------------------------------------------------
//   lastInPrevSegments
//   Searches for the previous segment that has elements on
//   the active staff and returns its last element
//
//   Uses lastElementForNavigation so it also returns a barline if it
//   spans into the active staff
//--------------------------------------------------------

Element* Segment::lastInPrevSegments(int activeStaff)
      {
      Element* re = 0;
      Segment* seg = this;

      while (!re) {
            seg = seg->prev1MMenabled();
            if (!seg) //end of staff, or score
                  break;

            re = seg->lastElementOfSegment(seg, activeStaff);
            }

      if (re)
            return re;

      if (!seg) { //end of staff
            if (activeStaff - 1 < 0) //end of score
                  return 0;

            re = 0;
            seg = score()->lastSegmentMM();
            while (true) {
                  //if (seg->segmentType() == SegmentType::EndBarLine)
                  //      score()->inputState().setTrack((activeStaff - 1) * VOICES ); //correction

                  if ((re = seg->firstElementForNavigation(activeStaff - 1)) != 0)
                        return re;

                  seg = seg->prev1MMenabled();
                  }
            }

      return 0;
      }

//---------------------------------------------------------
//   accessibleExtraInfo
//---------------------------------------------------------

QString Segment::accessibleExtraInfo() const
      {
      QString rez = "";
      if (!annotations().empty()) {
            QString temp = "";
            for (const Element* a : annotations()) {
                  if (!score()->selectionFilter().canSelect(a)) continue;
                  switch(a->type()) {
                        case ElementType::DYNAMIC:
                              //they are added in the chordrest, because they are for only one staff
                               break;
                        default:
                               temp = temp + " " + a->accessibleInfo();
                        }
                  }
            if(!temp.isEmpty())
                  rez = rez + QObject::tr("Annotations:") + temp;
            }

      QString startSpanners = "";
      QString endSpanners = "";

      auto spanners = score()->spannerMap().findOverlapping(tick().ticks(), tick().ticks());
      for (auto interval : spanners) {
            Spanner* s = interval.value;
            if (!score()->selectionFilter().canSelect(s)) continue;
            if (segmentType() == SegmentType::EndBarLine       ||
               segmentType() == SegmentType::BarLine           ||
               segmentType() == SegmentType::StartRepeatBarLine) {
                  if (s->isVolta())
                        continue;
                  }
            else {
                  if (s->isVolta() || s->isTie()) //ties are added in Note
                        continue;
                  }

            if (s->tick() == tick())
                  startSpanners += QObject::tr("Start of %1").arg(s->accessibleInfo());

            const Segment* seg = 0;
            switch (s->type()) {
                  case ElementType::VOLTA:
                  case ElementType::SLUR:
                        seg = this;
                        break;
                  default:
                        seg = next1MM(SegmentType::ChordRest);
                        break;
                  }

            if (seg && s->tick2() == seg->tick())
                  endSpanners += QObject::tr("End of %1").arg(s->accessibleInfo());
            }
      return rez + " " + startSpanners + " " + endSpanners;
      }

//---------------------------------------------------------
//   createShapes
//---------------------------------------------------------

void Segment::createShapes()
      {
      setVisible(false);
      for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx)
            createShape(staffIdx);
      }

//---------------------------------------------------------
//   createShape
//---------------------------------------------------------

void Segment::createShape(int staffIdx)
      {
      Shape& s = _shapes[staffIdx];
      s.clear();

      if (segmentType() & (SegmentType::BarLine | SegmentType::EndBarLine | SegmentType::StartRepeatBarLine | SegmentType::BeginBarLine)) {
            setVisible(true);
            BarLine* bl = toBarLine(element(staffIdx * VOICES));
            if (bl) {
                  QRectF r = bl->layoutRect();
#ifndef NDEBUG
                  s.add(r.translated(bl->pos()), bl->name());
#else
                  s.add(r.translated(bl->pos()));
#endif
                  }
            s.addHorizontalSpacing(Shape::SPACING_GENERAL, 0, 0);
            s.addHorizontalSpacing(Shape::SPACING_LYRICS, 0, 0);
            return;
            }
#if 0
      for (int track = staffIdx * VOICES; track < (staffIdx + 1) * VOICES; ++track) {
            Element* e = _elist[track];
            if (e)
                  s.add(e->shape().translated(e->pos()));
            }
#endif

      if (!score()->staff(staffIdx)->show())
            return;

      int strack = staffIdx * VOICES;
      int etrack = strack + VOICES;
      for (Element* e : _elist) {
            if (!e)
                  continue;
            int effectiveTrack = e->vStaffIdx() * VOICES + e->voice();
            if (effectiveTrack >= strack && effectiveTrack < etrack) {
                  // TODO: we could choose to ignore invisible/no-autoplace notes & rests
                  // but these might be relied upon by some
                  setVisible(true);
                  if (e->addToSkyline() || e->isChord())
                        s.add(e->shape().translated(e->pos()));
                  }
            }

      for (Element* e : _annotations) {
            if (!e || e->staffIdx() != staffIdx)
                  continue;
            if (!e->addToSkyline())
                  continue;
            // TODO: will skipping segments that contain only invisible annotations adversely affect playback or anything else?
            setVisible(true);

            if (e->isHarmony()) {
                  // use same spacing calculation as for chordrest
                  auto h = toHarmony(e);
                  if (h->bbox().isEmpty())
                        h->layout1();
                  const qreal margin = styleP(Sid::minHarmonyDistance) * 0.5;
                  qreal x1 = e->bbox().x() - margin + e->pos().x();
                  qreal x2 = e->bbox().x() + e->bbox().width() + margin + e->pos().x();
                  s.addHorizontalSpacing(Shape::SPACING_HARMONY, x1, x2);
                  }
            else if (!e->isRehearsalMark()
               && !e->isFretDiagram()
               && !e->isHarmony()
               && !e->isTempoText()
               && !e->isDynamic()
               && !e->isFiguredBass()
               && !e->isSymbol()
               && !e->isFSymbol()
               && !e->isSystemText()
               && !e->isInstrumentChange()
               && !e->isArticulation()
               && !e->isFermata()
               && !e->isStaffText()) {
                  // annotations added here are candidates for collision detection
                  // lyrics, ...
                  s.add(e->shape().translated(e->pos()));
                  }
            }
      }

//---------------------------------------------------------
//   minRight
//    calculate minimum distance needed to the right
//---------------------------------------------------------

qreal Segment::minRight() const
      {
      qreal distance = 0.0;
      for (const Shape& sh : shapes())
            distance = qMax(distance, sh.right());
      if (isClefType())
            distance += score()->styleP(Sid::clefBarlineDistance);
      if (trailer())
            distance += score()->styleP(Sid::systemTrailerRightMargin);
      return distance;
      }

//---------------------------------------------------------
//   minLeft
//    Calculate minimum distance needed to the left shape
//    sl. Sl is the same for all staves.
//---------------------------------------------------------

qreal Segment::minLeft(const Shape& sl) const
      {
      qreal distance = 0.0;
      for (const Shape& sh : shapes()) {
            qreal d = sl.minHorizontalDistance(sh);
            if (d > distance)
                  distance = d;
            }
      return distance;
      }

qreal Segment::minLeft() const
      {
      qreal distance = 0.0;
      for (const Shape& sh : shapes()) {
            qreal l = sh.left();
            if (l > distance)
                  distance = l;
            }
      return distance;
      }

//---------------------------------------------------------
//   minHorizontalCollidingDistance
//    calculate the minimum distance to ns avoiding collisions
//---------------------------------------------------------

qreal Segment::minHorizontalCollidingDistance(Segment* ns) const
      {
      qreal w = 0.0;
      for (unsigned staffIdx = 0; staffIdx < _shapes.size(); ++staffIdx) {
            if (score()->staff(staffIdx) && !score()->staff(staffIdx)->show())
                  continue;
            qreal d = staffShape(staffIdx).minHorizontalDistance(ns->staffShape(staffIdx));
            w       = qMax(w, d);
            }
      return w;
      }

//---------------------------------------------------------
//   minHorizontalDistance
//    calculate the minimum layout distance to Segment ns
//---------------------------------------------------------

qreal Segment::minHorizontalDistance(Segment* ns, bool systemHeaderGap) const
      {

      qreal ww = -DBL_MAX;        // can remain negative
      for (unsigned staffIdx = 0; staffIdx < _shapes.size(); ++staffIdx) {
            if (score()->staff(staffIdx) && !score()->staff(staffIdx)->show())
                  continue;
            qreal d = ns ? staffShape(staffIdx).minHorizontalDistance(ns->staffShape(staffIdx)) : 0.0;
            // first chordrest of a staff should clear the widest header for any staff
            // so make sure segment is as wide as it needs to be
            if (systemHeaderGap)
                  d = qMax(d, staffShape(staffIdx).right());
            ww      = qMax(ww, d);
            }
      qreal w = qMax(ww, 0.0);      // non-negative

      SegmentType st  = segmentType();
      SegmentType nst = ns ? ns->segmentType() : SegmentType::Invalid;

      if (isChordRestType()) {
            if (nst == SegmentType::EndBarLine) {
                  w = qMax(w, score()->noteHeadWidth());
                  w += score()->styleP(Sid::noteBarDistance);
                  }
            else if (nst == SegmentType::Clef) {
                  // clef likely does not exist on all staves
                  // and can cause very uneven spacing
                  // so use ww to avoid forcing margin except as necessary
                  w = ww + score()->styleP(Sid::clefLeftMargin);
                  }
            else {
                  bool isGap = false;
                  for (int i = 0; i < score()->nstaves() * VOICES; i++) {
                        Element* el = element(i);
                        if (!el)
                              continue;
                        if (el->isRest() && toRest(el)->isGap())
                              isGap = true;
                        else {
                              isGap = false;
                              break;
                              }
                        }
                  if (isGap)
                        return 0.0;
                  // minimum distance between notes is one note head width
                  w = qMax(w, score()->noteHeadWidth()) + score()->styleP(Sid::minNoteDistance);
                  }
            }
      else if (nst == SegmentType::ChordRest) {
            // <non ChordRest> - <ChordRest>
            if (systemHeaderGap) {
                  if (st == SegmentType::TimeSig)
                        w += score()->styleP(Sid::systemHeaderTimeSigDistance);
                  else
                        w += score()->styleP(Sid::systemHeaderDistance);
                  }
            else {
//                  qreal d = score()->styleP(Sid::barNoteDistance);
//                  qreal dd = minRight() + ns->minLeft() + spatium();
//                  w = qMax(d, dd);
                  // not header
                  if (st == SegmentType::Clef)
                        w = ww + score()->styleP(Sid::midClefKeyRightMargin);
                  else if (st == SegmentType::KeySig)
                        w += score()->styleP(Sid::midClefKeyRightMargin);
                  else
                        w += score()->styleP(Sid::barNoteDistance);

                  if (st == SegmentType::StartRepeatBarLine) {
                        if (Element* barLine = element(0)) {
                              const qreal blWidth = barLine->width();
                              if (w < blWidth)
                                    w += blWidth;
                              }
                        }
                  }
            // d -= ns->minLeft() * .7;      // hack
            // d = qMax(d, ns->minLeft());
            // d = qMax(d, spatium());       // minimum distance is one spatium
            // w = qMax(w, minRight()) + d;
            }
      else if (systemHeaderGap) {
            // first segment after header is *not* a chordrest
            // could be a clef
            if (st == SegmentType::TimeSig)
                  w += score()->styleP(Sid::systemHeaderTimeSigDistance);
            else
                  w += score()->styleP(Sid::systemHeaderDistance);
            }
      else if (st & (SegmentType::Clef | SegmentType::HeaderClef)) {
            if (nst == SegmentType::KeySig || nst == SegmentType::KeySigAnnounce)
                  w += score()->styleP(Sid::clefKeyDistance);
            else if (nst == SegmentType::TimeSig || nst == SegmentType::TimeSigAnnounce)
                  w += score()->styleP(Sid::clefTimesigDistance);
            else if (nst & (SegmentType::EndBarLine | SegmentType::StartRepeatBarLine))
                  w += score()->styleP(Sid::clefBarlineDistance);
            else if (nst == SegmentType::Ambitus)
                  w += score()->styleP(Sid::ambitusMargin);
            }
      else if ((st & (SegmentType::KeySig | SegmentType::KeySigAnnounce))
         && (nst & (SegmentType::TimeSig | SegmentType::TimeSigAnnounce))) {
            w += score()->styleP(Sid::keyTimesigDistance);
            }
      else if (st == SegmentType::KeySig && nst == SegmentType::StartRepeatBarLine)
            w += score()->styleP(Sid::keyBarlineDistance);
      else if (st == SegmentType::StartRepeatBarLine)
            w += score()->styleP(Sid::noteBarDistance);
      else if (st == SegmentType::BeginBarLine && (nst & (SegmentType::HeaderClef | SegmentType::Clef)))
            w += score()->styleP(Sid::clefLeftMargin);
      else if (st == SegmentType::BeginBarLine && nst == SegmentType::KeySig)
            w += score()->styleP(Sid::keysigLeftMargin);
      else if (st == SegmentType::EndBarLine) {
            if (nst == SegmentType::KeySigAnnounce)
                  w += score()->styleP(Sid::keysigLeftMargin);
            else if (nst == SegmentType::TimeSigAnnounce)
                  w += score()->styleP(Sid::timesigLeftMargin);
            else if (nst == SegmentType::Clef)
                  w += score()->styleP(Sid::clefLeftMargin);
            }
      else if (st == SegmentType::TimeSig && nst == SegmentType::StartRepeatBarLine)
            w += score()->styleP(Sid::timesigBarlineDistance);
      else if (st == SegmentType::Breath)
            w += spatium() * 1.5;
      else if (st == SegmentType::Ambitus)
            w += score()->styleP(Sid::ambitusMargin);

      if (w < 0.0)
            w = 0.0;
      if (ns)
            w += ns->extraLeadingSpace().val() * spatium();
      return w;
      }

}           // namespace Ms
