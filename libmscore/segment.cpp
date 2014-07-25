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

const char* Segment::subTypeName(Type t)
      {
      switch(t) {
            case Type::Invalid:              return "Invalid";
            case Type::Clef:                 return "Clef";
            case Type::KeySig:               return "Key Signature";
            case Type::Ambitus:              return "Ambitus";
            case Type::TimeSig:              return "Time Signature";
            case Type::StartRepeatBarLine:   return "Begin Repeat";
            case Type::BarLine:              return "BarLine";
            case Type::ChordRest:            return "ChordRest";
            case Type::Breath:               return "Breath";
            case Type::EndBarLine:           return "EndBarLine";
            case Type::TimeSigAnnounce:      return "Time Sig Precaution";
            case Type::KeySigAnnounce:       return "Key Sig Precaution";
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

Segment::Segment(Measure* m, Type st, int t)
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

void Segment::setSegmentType(Type t)
      {
      Q_ASSERT(_segmentType != Type::Clef || t != Type::ChordRest);
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
            if (e->type() == Element::Type::TIMESIG)
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

Segment* Segment::next1(Type types) const
      {
      for (Segment* s = next1(); s; s = s->next1()) {
            if (s->segmentType() & types)
                  return s;
            }
      return 0;
      }

Segment* Segment::next1MM(Type types) const
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

Segment* Segment::next(Type types) const
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

Segment* Segment::prev(Type types) const
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

Segment* Segment::prev1(Type types) const
      {
      for (Segment* s = prev1(); s; s = s->prev1()) {
            if (s->segmentType() & types)
                  return s;
            }
      return 0;
      }

Segment* Segment::prev1MM(Type types) const
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
            if (seg->segmentType() == Type::ChordRest) {
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
            case Element::Type::REPEAT_MEASURE:
                  measure()->setRepeatFlags(measure()->repeatFlags() | Repeat::MEASURE);
                  _elist[track] = el;
                  empty = false;
                  break;

            case Element::Type::DYNAMIC:
            case Element::Type::HARMONY:
            case Element::Type::SYMBOL:
            case Element::Type::FRET_DIAGRAM:
            case Element::Type::TEMPO_TEXT:
            case Element::Type::STAFF_TEXT:
            case Element::Type::REHEARSAL_MARK:
            case Element::Type::MARKER:
            case Element::Type::IMAGE:
            case Element::Type::TEXT:
            case Element::Type::TAB_DURATION_SYMBOL:
            case Element::Type::FIGURED_BASS:
                  _annotations.push_back(el);
                  break;
            case Element::Type::JUMP:
                  measure()->setRepeatFlags(measure()->repeatFlags() | Repeat::JUMP);
                  _annotations.push_back(el);
                  break;

            case Element::Type::STAFF_STATE:
                  if (static_cast<StaffState*>(el)->staffStateType() == StaffStateType::INSTRUMENT) {
                        StaffState* ss = static_cast<StaffState*>(el);
                        Part* part = el->staff()->part();
                        part->setInstrument(ss->instrument(), tick());
                        }
                  _annotations.push_back(el);
                  break;

            case Element::Type::INSTRUMENT_CHANGE: {
                  InstrumentChange* is = static_cast<InstrumentChange*>(el);
                  Part* part = is->staff()->part();
                  part->setInstrument(is->instrument(), tick());
                  _annotations.push_back(el);
                  break;
                  }

            case Element::Type::CLEF:
                  Q_ASSERT(_segmentType == Type::Clef);
                  checkElement(el, track);
                  _elist[track] = el;
                  if (!el->generated())
                        el->staff()->setClef(static_cast<Clef*>(el));
                  empty = false;
                  break;

            case Element::Type::TIMESIG:
                  Q_ASSERT(segmentType() == Type::TimeSig || segmentType() == Type::TimeSigAnnounce);
                  checkElement(el, track);
                  _elist[track] = el;
                  el->staff()->addTimeSig(static_cast<TimeSig*>(el));
                  empty = false;
                  break;

            case Element::Type::KEYSIG:
                  Q_ASSERT(_segmentType == Type::KeySig || _segmentType == Type::KeySigAnnounce);
                  checkElement(el, track);
                  _elist[track] = el;
                  if (!el->generated())
                        el->staff()->setKey(tick(), static_cast<KeySig*>(el)->key());
                  empty = false;
                  break;

            case Element::Type::CHORD:
            case Element::Type::REST:
                  Q_ASSERT(_segmentType == Type::ChordRest);
                  if (track % VOICES)
                        measure()->mstaff(track / VOICES)->hasVoices = true;

                  // fall through

            case Element::Type::BAR_LINE:
            case Element::Type::BREATH:
                  checkElement(el, track);
                  _elist[track] = el;
                  empty = false;
                  break;

            case Element::Type::AMBITUS:
                  Q_ASSERT(_segmentType == Type::Ambitus);
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
            case Element::Type::CHORD:
            case Element::Type::REST:
                  {
                  _elist[track] = 0;
                  int staffIdx = el->staffIdx();
                  measure()->checkMultiVoices(staffIdx);
                  }
                  break;

            case Element::Type::REPEAT_MEASURE:
                  measure()->resetRepeatFlag(Repeat::MEASURE);
                  _elist[track] = 0;
                  break;

            case Element::Type::DYNAMIC:
            case Element::Type::HARMONY:
            case Element::Type::SYMBOL:
            case Element::Type::FRET_DIAGRAM:
            case Element::Type::TEMPO_TEXT:
            case Element::Type::STAFF_TEXT:
            case Element::Type::REHEARSAL_MARK:
            case Element::Type::MARKER:
            case Element::Type::IMAGE:
            case Element::Type::TEXT:
            case Element::Type::TAB_DURATION_SYMBOL:
            case Element::Type::FIGURED_BASS:
                  removeAnnotation(el);
                  break;

            case Element::Type::JUMP:
                  measure()->resetRepeatFlag(Repeat::JUMP);
                  removeAnnotation(el);
                  break;

            case Element::Type::STAFF_STATE:
                  if (static_cast<StaffState*>(el)->staffStateType() == StaffStateType::INSTRUMENT) {
                        Part* part = el->staff()->part();
                        part->removeInstrument(tick());
                        }
                  removeAnnotation(el);
                  break;

            case Element::Type::INSTRUMENT_CHANGE:
                  {
                  InstrumentChange* is = static_cast<InstrumentChange*>(el);
                  Part* part = is->staff()->part();
                  part->removeInstrument(tick());
                  }
                  removeAnnotation(el);
                  break;

            case Element::Type::TIMESIG:
                  _elist[track] = 0;
                  el->staff()->removeTimeSig(static_cast<TimeSig*>(el));
                  break;

            case Element::Type::KEYSIG:
                  Q_ASSERT(_elist[track] == el);

                  _elist[track] = 0;
                  if (!el->generated())
                        el->staff()->removeKey(tick());
                  empty = false;
                  break;

            case Element::Type::CLEF:
                  if (!el->generated())
                        el->staff()->removeClef(static_cast<Clef*>(el));

            case Element::Type::BAR_LINE:
            case Element::Type::BREATH:
            case Element::Type::AMBITUS:
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

Segment::Type Segment::segmentType(Element::Type type)
      {
      switch (type) {
            case Element::Type::CHORD:
            case Element::Type::REST:
            case Element::Type::REPEAT_MEASURE:
            case Element::Type::JUMP:
            case Element::Type::MARKER:
                  return Type::ChordRest;
            case Element::Type::CLEF:
                  return Type::Clef;
            case Element::Type::KEYSIG:
                  return Type::KeySig;
            case Element::Type::TIMESIG:
                  return Type::TimeSig;
            case Element::Type::BAR_LINE:
                  return Type::StartRepeatBarLine;
            case Element::Type::BREATH:
                  return Type::Breath;
            default:
                  qDebug("Segment:segmentType():  bad type: <%s>", Element::name(type));
                  return Type::Invalid;
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
      if (!(segmentType() & (Type::ChordRest))) {
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
      if (segmentType() != Type::ChordRest)
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

bool Segment::findAnnotationOrElement(Element::Type type, int minTrack, int maxTrack)
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

//---------------------------------------------------------
//   elementAt
//    A variant of the element(int) function,
//    specifically intended to be called from QML plugins
//---------------------------------------------------------

Ms::Element* Segment::elementAt(int track) const {
      Element* e = _elist.value(track);
#ifdef SCRIPT_INTERFACE
// if called from QML/JS, tell QML engine not to garbage collect this object
      if (e)
            QQmlEngine::setObjectOwnership(e, QQmlEngine::CppOwnership);
#endif
      return e;
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Segment::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      // bar line visibility depends on spanned staves,
      // not simply on visibility of first staff
      if (segmentType() == Segment::Type::BarLine || segmentType() == Segment::Type::EndBarLine
          || segmentType() == Segment::Type::StartRepeatBarLine) {
            for (int staffIdx = 0; staffIdx < _score->nstaves(); ++staffIdx) {
                  Element* e = element(staffIdx*VOICES);
                  if (e == 0)             // if no element, skip
                        continue;
                  // if staff not visible
                  if (!all && !(measure()->visible(staffIdx) && _score->staff(staffIdx)->show())) {
                        // if bar line spans just this staff...
                        if (static_cast<BarLine*>(e)->span() <= 1
                            // ...or span another staff but without entering INTO it...
                            || (static_cast<BarLine*>(e)->span() < 2 &&
                                static_cast<BarLine*>(e)->spanTo() < 1) )
                              continue;         // ...skip
                        }
                  e->scanElements(data, func, all);
                  }
            }
      else
            for (int track = 0; track < _score->nstaves() * VOICES; ++track) {
                  int staffIdx = track/VOICES;
                  if (!all && !(measure()->visible(staffIdx) && _score->staff(staffIdx)->show())) {
                        track += VOICES - 1;
                        continue;
                        }
                  Element* e = element(track);
                  if (e == 0)
                        continue;
                  e->scanElements(data, func, all);
                  }
      foreach(Element* e, annotations()) {
            if (all || e->systemFlag() || measure()->visible(e->staffIdx()))
                  e->scanElements(data,  func, all);
            }
      }

}           // namespace Ms
