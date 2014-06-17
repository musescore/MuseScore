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

#include "mscore.h"
#include "segment.h"
#include "element.h"
#include "chord.h"
#include "note.h"
#include "score.h"
#include "beam.h"
#include "tuplet.h"
#include "text.h"
#include "measure.h"
#include "barline.h"
#include "part.h"
#include "lyrics.h"
#include "repeat.h"
#include "staff.h"
#include "line.h"
#include "hairpin.h"
#include "ottava.h"
#include "sig.h"
#include "keysig.h"
#include "staffstate.h"
#include "instrchange.h"
#include "clef.h"
#include "timesig.h"
#include "system.h"
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
            case SegmentType::Clef:                 return "Clef";
            case SegmentType::KeySig:               return "Key Signature";
            case SegmentType::Ambitus:              return "Ambitus";
            case SegmentType::TimeSig:              return "Time Signature";
            case SegmentType::StartRepeatBarLine:   return "Begin Repeat";
            case SegmentType::BarLine:              return "BarLine";
            case SegmentType::ChordRest:            return "ChordRest";
            case SegmentType::Breath:               return "Breath";
            case SegmentType::EndBarLine:           return "EndBarLine";
            case SegmentType::TimeSigAnnounce:      return "Time Sig Precaution";
            case SegmentType::KeySigAnnounce:       return "Key Sig Precaution";
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
            empty = false;
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
   : Element(m->score())
      {
      setParent(m);
      init();
      empty = true;
      }

Segment::Segment(Measure* m, SegmentType st, int t)
   : Element(m->score())
      {
      setParent(m);
      _segmentType = st;
      setTick(t);
      init();
      empty = true;
      }

//---------------------------------------------------------
//   Segment
//---------------------------------------------------------

Segment::Segment(const Segment& s)
   : Element(s)
      {
      _next = 0;
      _prev = 0;

      empty               = s.empty;           // cached value
      _segmentType        = s._segmentType;
      _tick               = s._tick;
      _extraLeadingSpace  = s._extraLeadingSpace;
      _extraTrailingSpace = s._extraTrailingSpace;

      foreach (Element* e, s._annotations) {
            Element* ne = e->clone();
            add(ne);
            }

      _elist.reserve(s._elist.size());
      foreach (Element* e, s._elist) {
            Element* ne = 0;
            if (e) {
                  ne = e->clone();
                  ne->setParent(this);
                  }
            _elist.append(ne);
            }
      _dotPosX = s._dotPosX;
      }

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
      foreach(Element* e, _elist) {
            if (e)
                  e->setScore(score);
            }
      foreach(Element* e, _annotations)
            e->setScore(score);
      }

Segment::~Segment()
      {
      foreach(Element* e, _elist) {
            if (!e)
                  continue;
            if (e->type() == ElementType::TIMESIG)
                  e->staff()->removeTimeSig(static_cast<TimeSig*>(e));
            delete e;
            }
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Segment::init()
      {
      int staves = score()->nstaves();
      int tracks = staves * VOICES;
      _elist.reserve(tracks);
      for (int i = 0; i < tracks; ++i)
            _elist.push_back(0);
      _dotPosX.reserve(staves);
      for (int i = 0; i < staves; ++i)
            _dotPosX.push_back(0.0);
      _prev = 0;
      _next = 0;
      }

//---------------------------------------------------------
//   next1
///   return next \a Segment, dont stop searching at end
///   of \a Measure
//---------------------------------------------------------

Segment* Segment::next1() const
      {
      if (next())
            return next();
      Measure* m = measure()->nextMeasure();
      if (m == 0)
            return 0;
      return m->first();
      }

//---------------------------------------------------------
//   next1MM
//---------------------------------------------------------

Segment* Segment::next1MM() const
      {
      if (next())
            return next();
      Measure* m = measure()->nextMeasureMM();
      if (m == 0)
            return 0;
      return m->first();
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
///   return previous \a Segment, dont stop searching at
///   \a Measure begin
//---------------------------------------------------------

Segment* Segment::prev1() const
      {
      if (prev())
            return prev();
      Measure* m = measure()->prevMeasure();
      if (m == 0)
            return 0;
      return m->last();
      }

Segment* Segment::prev1MM() const
      {
      if (prev())
            return prev();
      Measure* m = measure()->prevMeasureMM();
      if (m == 0)
            return 0;
      return m->last();
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

Segment* Segment::nextCR(int track) const
      {
      Segment* seg = next1();
      for (; seg; seg = seg->next1()) {
            if (seg->segmentType() == SegmentType::ChordRest) {
                  if (track != -1 && !seg->element(track))
                        continue;
                  return seg;
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
                  return static_cast<ChordRest*>(el);
            }
      return 0;
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Segment::insertStaff(int staff)
      {
      int track = staff * VOICES;
      for (int voice = 0; voice < VOICES; ++voice)
            _elist.insert(track, 0);
      _dotPosX.insert(staff, 0.0);

      foreach(Element* e, _annotations) {
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
      _dotPosX.removeAt(staff);

      foreach(Element* e, _annotations) {
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
      if (_elist[track]) {
            qDebug("Segment::add(%s) there is already a %s at %s(%d) track %d. score %p",
               el->name(), _elist[track]->name(),
               score()->sigmap()->pos(tick()), tick(), track, score());
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Segment::add(Element* el)
      {
      // qDebug("%p segment %s add(%d, %d, %s)", this, subTypeName(), tick(), el->track(), el->name());
      el->setParent(this);

      int track = el->track();
      Q_ASSERT(track != -1);

      switch (el->type()) {
            case ElementType::REPEAT_MEASURE:
                  measure()->setRepeatFlags(measure()->repeatFlags() | Repeat::MEASURE);
                  _elist[track] = el;
                  empty = false;
                  break;

            case ElementType::DYNAMIC:
            case ElementType::HARMONY:
            case ElementType::SYMBOL:
            case ElementType::FRET_DIAGRAM:
            case ElementType::TEMPO_TEXT:
            case ElementType::STAFF_TEXT:
            case ElementType::REHEARSAL_MARK:
            case ElementType::MARKER:
            case ElementType::IMAGE:
            case ElementType::TEXT:
            case ElementType::TAB_DURATION_SYMBOL:
            case ElementType::FIGURED_BASS:
                  _annotations.push_back(el);
                  break;
            case ElementType::JUMP:
                  measure()->setRepeatFlags(measure()->repeatFlags() | Repeat::JUMP);
                  _annotations.push_back(el);
                  break;

            case ElementType::STAFF_STATE:
                  if (static_cast<StaffState*>(el)->staffStateType() == StaffStateType::INSTRUMENT) {
                        StaffState* ss = static_cast<StaffState*>(el);
                        Part* part = el->staff()->part();
                        part->setInstrument(ss->instrument(), tick());
                        }
                  _annotations.push_back(el);
                  break;

            case ElementType::INSTRUMENT_CHANGE: {
                  InstrumentChange* is = static_cast<InstrumentChange*>(el);
                  Part* part = is->staff()->part();
                  part->setInstrument(is->instrument(), tick());
                  _annotations.push_back(el);
                  break;
                  }

            case ElementType::CLEF:
                  Q_ASSERT(_segmentType == SegmentType::Clef);
                  checkElement(el, track);
                  _elist[track] = el;
                  empty = false;
                  break;

            case ElementType::TIMESIG:
                  Q_ASSERT(segmentType() == SegmentType::TimeSig || segmentType() == SegmentType::TimeSigAnnounce);
                  checkElement(el, track);
                  _elist[track] = el;
                  el->staff()->addTimeSig(static_cast<TimeSig*>(el));
                  empty = false;
                  break;

            case ElementType::KEYSIG:
                  Q_ASSERT(_segmentType == SegmentType::KeySig || _segmentType == SegmentType::KeySigAnnounce);
                  checkElement(el, track);
                  _elist[track] = el;
                  if (!el->generated())
                        el->staff()->setKey(tick(), static_cast<KeySig*>(el)->key());
                  empty = false;
                  break;

            case ElementType::CHORD:
            case ElementType::REST:
                  Q_ASSERT(_segmentType == SegmentType::ChordRest);
                  if (track % VOICES)
                        measure()->mstaff(track / VOICES)->hasVoices = true;

                  // fall through

            case ElementType::BAR_LINE:
            case ElementType::BREATH:
                  checkElement(el, track);
                  _elist[track] = el;
                  empty = false;
                  break;

            case ElementType::AMBITUS:
                  Q_ASSERT(_segmentType == SegmentType::Ambitus);
                  checkElement(el, track);
                  _elist[track] = el;
                  empty = false;
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

      int track = el->track();

      switch(el->type()) {
            case ElementType::CHORD:
            case ElementType::REST:
                  {
                  _elist[track] = 0;
                  int staffIdx = el->staffIdx();
                  measure()->checkMultiVoices(staffIdx);
                  }
                  break;

            case ElementType::REPEAT_MEASURE:
                  measure()->setRepeatFlags(measure()->repeatFlags() & ~Repeat::MEASURE);
                  _elist[track] = 0;
                  break;

            case ElementType::DYNAMIC:
            case ElementType::HARMONY:
            case ElementType::SYMBOL:
            case ElementType::FRET_DIAGRAM:
            case ElementType::TEMPO_TEXT:
            case ElementType::STAFF_TEXT:
            case ElementType::REHEARSAL_MARK:
            case ElementType::MARKER:
            case ElementType::IMAGE:
            case ElementType::TEXT:
            case ElementType::TAB_DURATION_SYMBOL:
            case ElementType::FIGURED_BASS:
                  removeAnnotation(el);
                  break;

            case ElementType::JUMP:
                  measure()->setRepeatFlags(measure()->repeatFlags() & ~Repeat::JUMP);
                  removeAnnotation(el);
                  break;

            case ElementType::STAFF_STATE:
                  if (static_cast<StaffState*>(el)->staffStateType() == StaffStateType::INSTRUMENT) {
                        Part* part = el->staff()->part();
                        part->removeInstrument(tick());
                        }
                  removeAnnotation(el);
                  break;

            case ElementType::INSTRUMENT_CHANGE:
                  {
                  InstrumentChange* is = static_cast<InstrumentChange*>(el);
                  Part* part = is->staff()->part();
                  part->removeInstrument(tick());
                  }
                  removeAnnotation(el);
                  break;

            case ElementType::TIMESIG:
                  _elist[track] = 0;
                  el->staff()->removeTimeSig(static_cast<TimeSig*>(el));
                  break;

            case ElementType::KEYSIG:
                  Q_ASSERT(_elist[track] == el);

                  _elist[track] = 0;
                  if (!el->generated())
                        el->staff()->removeKey(tick());
                  empty = false;
                  break;

            case ElementType::CLEF:
            case ElementType::BAR_LINE:
            case ElementType::BREATH:
            case ElementType::AMBITUS:
                  _elist[track] = 0;
                  break;

            default:
                  qFatal("Segment::remove() unknown %s", el->name());

            }
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
//   removeGeneratedElements
//---------------------------------------------------------

void Segment::removeGeneratedElements()
      {
      for (int i = 0; i < _elist.size(); ++i) {
            if (_elist[i] && _elist[i]->generated()) {
                  _elist[i] = 0;
                  }
            }
      checkEmpty();
      }

//---------------------------------------------------------
//   sortStaves
//---------------------------------------------------------

void Segment::sortStaves(QList<int>& dst)
      {
      QList<Element*>   dl;

      for (int i = 0; i < dst.size(); ++i) {
            int startTrack = dst[i] * VOICES;
            int endTrack   = startTrack + VOICES;
            dl.reserve(VOICES);
            for (int k = startTrack; k < endTrack; ++k)
                  dl.append(_elist[k]);
            }
      _elist = dl;
      QMap<int, int> map;
      for (int k = 0; k < dst.size(); ++k) {
            map.insert(dst[k], k);
            }
      foreach (Element* e, _annotations) {
            if(!e->systemFlag())
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
      foreach(Element* e, _elist) {
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
            empty = false;
            return;
            }
      empty = true;
      foreach(const Element* e, _elist) {
            if (e) {
                  empty = false;
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

int Segment::tick() const
      {
      return _tick + measure()->tick();
      }

//---------------------------------------------------------
//   setTick
//---------------------------------------------------------

void Segment::setTick(int t)
      {
      _tick = t - measure()->tick();
      }

//---------------------------------------------------------
//   segLyricsList
//---------------------------------------------------------

const QList<Lyrics*>* Segment::lyricsList(int track) const
      {
      if (!(segmentType() & (SegmentType::ChordRest))) {
            if (MScore::debugMode)
                  qDebug("warning : lyricsList  bad segment type <%s><%s>", name(), subTypeName());
            return 0;
            }

      ChordRest* cr = static_cast<ChordRest*>(element(track));
      if (cr)
            return &cr->lyricsList();

      return 0;
      }

//---------------------------------------------------------
//   swapElements
//---------------------------------------------------------

void Segment::swapElements(int i1, int i2)
      {
      _elist.swap(i1, i2);
      if (_elist[i1])
            _elist[i1]->setTrack(i1);
      if (_elist[i2])
            _elist[i2]->setTrack(i2);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Segment::write(Xml& xml) const
      {
      if (_written)
            return;
      _written = true;
      if (_extraLeadingSpace.isZero() && _extraTrailingSpace.isZero())
            return;
      xml.stag(name());
      xml.tag("leadingSpace", _extraLeadingSpace.val());
      xml.tag("trailingSpace", _extraTrailingSpace.val());
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
            else if (tag == "trailingSpace")
                  _extraTrailingSpace = Spatium(e.readDouble());
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Segment::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::LEADING_SPACE:   return extraLeadingSpace().val();
            case P_ID::TRAILING_SPACE:  return extraTrailingSpace().val();
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Segment::propertyDefault(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::LEADING_SPACE:   return 0.0;
            case P_ID::TRAILING_SPACE:  return 0.0;
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Segment::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case P_ID::LEADING_SPACE: setExtraLeadingSpace(Spatium(v.toDouble())); break;
            case P_ID::TRAILING_SPACE: setExtraTrailingSpace(Spatium(v.toDouble())); break;
            default:
                  return Element::setProperty(propertyId, v);
            }
      return true;
      }

//---------------------------------------------------------
//   splitsTuplet
//---------------------------------------------------------

bool Segment::splitsTuplet() const
      {
      if (segmentType() != SegmentType::ChordRest)
            return false;
      int tracks = score()->nstaves() * VOICES;
      for (int track = 0; track < tracks; ++track) {
            ChordRest* cr = static_cast<ChordRest*>(element(track));
            if (cr == 0)
                  continue;
            if (cr->tuplet() && cr->tuplet()->elements().front() != cr)
                  return true;
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
//   findAnnotationOrElement
///  return true if an annotation of type type or and element is found in the track range
//---------------------------------------------------------

bool Segment::findAnnotationOrElement(ElementType type, int minTrack, int maxTrack)
      {
      for (const Element* e : _annotations)
            if (e->type() == type && e->track() >= minTrack && e->track() <= maxTrack)
                  return true;
      for (int curTrack = minTrack; curTrack <= maxTrack; curTrack++)
            if (element(curTrack))
                  return true;
      return false;
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

}

