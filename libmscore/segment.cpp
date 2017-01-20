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
#include "undo.h"

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
            case Type::BeginBarLine:         return "BeginBarLine";
            case Type::HeaderClef:           return "HeaderClef";
            case Type::Clef:                 return "Clef";
            case Type::KeySig:               return "Key Signature";
            case Type::Ambitus:              return "Ambitus";
            case Type::TimeSig:              return "Time Signature";
            case Type::StartRepeatBarLine:   return "Begin Repeat";
            case Type::BarLine:              return "BarLine";
            case Type::Breath:               return "Breath";
            case Type::ChordRest:            return "ChordRest";
            case Type::EndBarLine:           return "EndBarLine";
            case Type::KeySigAnnounce:       return "Key Sig Precaution";
            case Type::TimeSigAnnounce:      return "Time Sig Precaution";
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
   : Element(m->score())
      {
      setParent(m);
      init();
      }

Segment::Segment(Measure* m, Type st, int t)
   : Element(m->score())
      {
      setParent(m);
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
      _next               = 0;
      _prev               = 0;
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
      return m ? m->last() : 0;
      }

Segment* Segment::prev1MM() const
      {
      if (prev())
            return prev();
      Measure* m = measure()->prevMeasureMM();
      return m ? m->last() : 0;
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
      if (_elist[track]) {
            qDebug("Segment::add(%s) there is already a %s at %s(%d) track %d. score %p",
               el->name(), _elist[track]->name(),
               qPrintable(score()->sigmap()->pos(tick())), tick(), track, score());
            // abort();
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

      switch (el->type()) {
            case ElementType::REPEAT_MEASURE:
                  _elist[track] = el;
                  setEmpty(false);
                  break;

            case ElementType::DYNAMIC:
            case ElementType::HARMONY:
            case ElementType::SYMBOL:
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
                  Q_ASSERT(_segmentType == Type::Clef || _segmentType == Type::HeaderClef);
                  checkElement(el, track);
                  _elist[track] = el;
                  if (!el->generated()) {
                        el->staff()->setClef(toClef(el));
                        updateNoteLines(this, el->track());
                        }
                  setEmpty(false);
                  break;

            case ElementType::TIMESIG:
                  Q_ASSERT(segmentType() == Type::TimeSig || segmentType() == Type::TimeSigAnnounce);
                  checkElement(el, track);
                  _elist[track] = el;
                  el->staff()->addTimeSig(toTimeSig(el));
                  setEmpty(false);
                  break;

            case ElementType::KEYSIG:
                  Q_ASSERT(_segmentType == Type::KeySig || _segmentType == Type::KeySigAnnounce);
                  checkElement(el, track);
                  _elist[track] = el;
                  if (!el->generated())
                        el->staff()->setKey(tick(), toKeySig(el)->keySigEvent());
                  setEmpty(false);
                  break;

            case ElementType::CHORD:
            case ElementType::REST:
                  Q_ASSERT(_segmentType == Type::ChordRest);
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
                  ChordRest* cr = toChordRest(el);
                  if (cr->tuplet() && !cr->tuplet()->elements().empty() && cr->tuplet()->elements().front() == cr && cr->tuplet()->tick() < 0)
                        cr->tuplet()->setTick(cr->tick());
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
                  Q_ASSERT(_segmentType == Type::Ambitus);
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
                  auto spanners = smap.findOverlapping(tick(), tick());
                  for (auto interval : spanners) {
                        Spanner* s = interval.value;
                        if (s->startElement() == el)
                              s->setStartElement(nullptr);
                        if (s->endElement() == el)
                              s->setEndElement(nullptr);
                        }
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
      score()->setLayout(tick());
      checkEmpty();
      }

//---------------------------------------------------------
//   segmentType
//    returns segment type suitable for storage of Element
//---------------------------------------------------------

Segment::Type Segment::segmentType(ElementType type)
      {
      switch (type) {
            case ElementType::CHORD:
            case ElementType::REST:
            case ElementType::REPEAT_MEASURE:
            case ElementType::JUMP:
            case ElementType::MARKER:
                  return Type::ChordRest;
            case ElementType::CLEF:
                  return Type::Clef;
            case ElementType::KEYSIG:
                  return Type::KeySig;
            case ElementType::TIMESIG:
                  return Type::TimeSig;
            case ElementType::BAR_LINE:
                  return Type::StartRepeatBarLine;
            case ElementType::BREATH:
                  return Type::Breath;
            default:
                  qDebug("Segment:segmentType():  bad type: <%s>", Element::name(type));
                  return Type::Invalid;
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
//   fpos
//    return relative position of segment in measure
//---------------------------------------------------------

Fraction Segment::fpos() const
      {
      return Fraction::fromTicks(_tick);
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
      score()->setLayout(tick());
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
      xml.stag(name());
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

QVariant Segment::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::TICK:
                  return _tick;
            case P_ID::LEADING_SPACE:
                  return extraLeadingSpace();
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Segment::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::LEADING_SPACE:
                  return Spatium(0.0);
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Segment::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::TICK:
                  _tick = v.toInt();
                  break;
            case P_ID::LEADING_SPACE:
                  setExtraLeadingSpace(v.value<Spatium>());
                  break;
            default:
                  return Element::setProperty(propertyId, v);
            }
      score()->setLayout(tick());
      return true;
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

//---------------------------------------------------------
//   elementAt
//    A variant of the element(int) function,
//    specifically intended to be called from QML plugins
//---------------------------------------------------------

Ms::Element* Segment::elementAt(int track) const
      {
      Element* e = track < int(_elist.size()) ? _elist[track] : 0;

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
      for (int track = 0; track < score()->nstaves() * VOICES; ++track) {
            int staffIdx = track/VOICES;
            if (!all && !(measure()->visible(staffIdx) && score()->staff(staffIdx)->show())) {
                  track += VOICES - 1;
                  continue;
                  }
            Element* e = element(track);
            if (e == 0)
                  continue;
            e->scanElements(data, func, all);
            }
      for (Element* e : annotations()) {
            if (all || e->systemFlag() || measure()->visible(e->staffIdx()))
                  e->scanElements(data,  func, all);
            }
      }

//---------------------------------------------------------
//   firstElement
//   This function returns the first main element from a
//   segment, or a barline if it spanns in the staff
//---------------------------------------------------------

Element* Segment::firstElement(int staff)
      {
      if (segmentType() == Segment::Type::ChordRest) {
            for (int v = staff * VOICES; v/VOICES == staff; v++) {
                Element* el = element(v);
                if (!el) {      //there is no chord or rest on this voice
                      continue;
                      }
                if (el->isChord()) {
                      return toChord(el)->notes().back();
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
//   lastElement
//   This function returns the last main element from a
//   segment, or a barline if it spanns in the staff
//---------------------------------------------------------

Element* Segment::lastElement(int staff)
      {
      if (segmentType() == Segment::Type::ChordRest) {
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
//   protected because it is used by the firstElement and
//   lastElement functions when segment types that have
//   just one elemnt to avoid duplicated code
//
//   Use firstElement, or lastElement instead of this
//---------------------------------------------------------

Element* Segment::getElement(int staff)
      {
      if (segmentType() == Segment::Type::ChordRest)
            return firstElement(staff);
      else if (segmentType() & (Type::EndBarLine | Type::BarLine | Type::StartRepeatBarLine)) {
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

//--------------------------------------------------------
//   firstInNextSegments
//   Searches for the next segment that has elements on the
//   active staff and returns its first element
//
//   Uses firstElement so it also returns a barline if it
//   spans into the active staff
//--------------------------------------------------------

Element* Segment::firstInNextSegments(int activeStaff)
      {
      Element* re = 0;
      Segment* seg = this;
      while (!re) {
            seg = seg->next1MM(Segment::Type::All);
            if (!seg) //end of staff, or score
                  break;

            re = seg->firstElement(activeStaff);
            }

      if (re)
            return re;

      if (!seg) { //end of staff
            seg = score()->firstSegment();
            return seg->element( (activeStaff + 1) * VOICES );
            }

      return 0;
      }

//--------------------------------------------------------
//   firstInNextSegments
//   Searches for the previous segment that has elements on
//   the active staff and returns its last element
//
//   Uses lastElement so it also returns a barline if it
//   spans into the active staff
//--------------------------------------------------------

Element* Segment::lastInPrevSegments(int activeStaff)
      {
      Element* re = 0;
      Segment* seg = this;

      while (!re) {
            seg = seg->prev1MM(Segment::Type::All);
            if (!seg) //end of staff, or score
                  break;

            re = seg->lastElement(activeStaff);
            }

      if (re)
            return re;

      if (!seg) { //end of staff
            if (activeStaff -1 < 0) //end of score
                  return 0;

            re = 0;
            seg = score()->lastSegment();
            while (true) {
                  if (seg->segmentType() == Segment::Type::EndBarLine)
                        score()->inputState().setTrack( (activeStaff -1) * VOICES ); //correction

                  if ((re = seg->lastElement(activeStaff -1)) != 0)
                        return re;

                  seg = seg->prev1(Segment::Type::All);
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
                  rez = rez + tr("Annotations:") + temp;
            }

      QString startSpanners = "";
      QString endSpanners = "";

      auto spanners = score()->spannerMap().findOverlapping(this->tick(), this->tick());
      for (auto interval : spanners) {
            Spanner* s = interval.value;
            if (!score()->selectionFilter().canSelect(s)) continue;
            if (segmentType() == Segment::Type::EndBarLine       ||
               segmentType() == Segment::Type::BarLine           ||
               segmentType() == Segment::Type::StartRepeatBarLine) {
                  if (s->isVolta())
                        continue;
                  }
            else {
                  if (s->isVolta() || s->isTie()) //ties are added in Note
                        continue;
                  }

            if (s->tick() == tick())
                  startSpanners += tr("Start of ") + s->accessibleInfo();

            const Segment* seg = 0;
            switch (s->type()) {
                  case ElementType::VOLTA:
                  case ElementType::SLUR:
                        seg = this;
                        break;
                  default:
                        seg = next1MM(Segment::Type::ChordRest);
                        break;
                  }

            if (seg && s->tick2() == seg->tick())
                  endSpanners += tr("End of ") + s->accessibleInfo();
            }
      return rez + " " + startSpanners + " " + endSpanners;
      }

//---------------------------------------------------------
//   createShapes
//---------------------------------------------------------

void Segment::createShapes()
      {
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
#if 1
      if (segmentType() & (Type::BarLine | Type::EndBarLine | Type::StartRepeatBarLine | Type::BeginBarLine)) {
            BarLine* bl = toBarLine(element(0));
            if (bl) {
                  qreal w = BarLine::layoutWidth(score(), bl->barLineType(), 1.0);
                  s.add(QRectF(bl->x(), 0.0, w, spatium() * 4.0));
                  }
            return;
            }
#endif

      for (Element* e : _elist) {
            if (e && e->vStaffIdx() == staffIdx)
                  s.add(e->shape().translated(e->pos()));
            }
      for (Element* e : _annotations) {
            // probably only allow for lyrics and chordnames
            if (e->staffIdx() == staffIdx
               && !e->isRehearsalMark()
               && !e->isTempoText()
               && !e->isDynamic()
               && !e->isSymbol()
               && !e->isFSymbol()
               && !e->isSystemText()
               && !e->isStaffText())
                  s.add(e->shape().translated(e->pos()));
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
            distance += score()->styleP(StyleIdx::clefBarlineDistance);
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
//   minHorizontalDistance
//---------------------------------------------------------

qreal Segment::minHorizontalDistance(Segment* ns, bool systemHeaderGap) const
      {
      Segment::Type st  = segmentType();
      Segment::Type nst = ns ? ns->segmentType() : Segment::Type::Invalid;

      qreal w = 0.0;
      for (unsigned staffIdx = 0; staffIdx < _shapes.size(); ++staffIdx) {
            qreal d = staffShape(staffIdx).minHorizontalDistance(ns->staffShape(staffIdx));
            w = qMax(w, d);
            }

      if (isChordRestType()) {
            if (nst == Segment::Type::EndBarLine)
                  w += score()->styleP(StyleIdx::noteBarDistance);
            else if (nst == Segment::Type::Clef)
                  w = qMax(w, score()->styleP(StyleIdx::clefLeftMargin));
            else {
                  bool isGap = false;
                  for (int i = 0; i < score()->nstaves() * VOICES; i++) {
                        Element* el = element(i);
                        if (el && el->isRest() && toRest(el)->isGap())
                              isGap = true;
                        else if (el) {
                              isGap = false;
                              break;
                              }
                        }
                  if (isGap)
                        return 0.0;

                  w = qMax(w, score()->noteHeadWidth()) + score()->styleP(StyleIdx::minNoteDistance);
                  }
            }
      else if (nst == Segment::Type::ChordRest) {
            qreal d;
            if (systemHeaderGap) {
                  if (st == Segment::Type::TimeSig)
                        d = score()->styleP(StyleIdx::systemHeaderTimeSigDistance);
                  else
                        d = score()->styleP(StyleIdx::systemHeaderDistance);
                  }
            else
                  d = score()->styleP(StyleIdx::barNoteDistance);
            qreal dd = minRight() + ns->minLeft() + spatium();
            w = qMax(d, dd);
            // d -= ns->minLeft() * .7;      // hack
            // d = qMax(d, ns->minLeft());
            // d = qMax(d, spatium());       // minimum distance is one spatium
            // w = qMax(w, minRight()) + d;
            }
      else if (st & (Segment::Type::Clef | Segment::Type::HeaderClef)) {
            if (nst == Segment::Type::KeySig)
                  w += score()->styleP(StyleIdx::clefKeyDistance);
            else if (nst == Segment::Type::TimeSig)
                  w += score()->styleP(StyleIdx::clefTimesigDistance);
            else if (nst & (Segment::Type::EndBarLine | Segment::Type::StartRepeatBarLine))
                  w += score()->styleP(StyleIdx::clefBarlineDistance);
            else if (nst == Segment::Type::Ambitus)
                  w += score()->styleP(StyleIdx::ambitusMargin);
            }
      else if ((st & (Segment::Type::KeySig | Segment::Type::KeySigAnnounce))
         && (nst & (Segment::Type::TimeSig | Segment::Type::TimeSigAnnounce))) {
            w += score()->styleP(StyleIdx::keyTimesigDistance);
            }
      else if (st == Segment::Type::KeySig && nst == Segment::Type::StartRepeatBarLine)
            w += score()->styleP(StyleIdx::keyBarlineDistance);
      else if (st == Segment::Type::StartRepeatBarLine)
            w += score()->styleP(StyleIdx::noteBarDistance);
      else if (st == Segment::Type::BeginBarLine && (nst & (Segment::Type::HeaderClef | Segment::Type::Clef)))
            w += score()->styleP(StyleIdx::clefLeftMargin);
      else if (st == Segment::Type::EndBarLine) {
            if (nst == Segment::Type::KeySigAnnounce)
                  w += score()->styleP(StyleIdx::keysigLeftMargin);
            else if (nst == Segment::Type::TimeSigAnnounce)
                  w += score()->styleP(StyleIdx::timesigLeftMargin);
            }
      else if (st == Segment::Type::TimeSig && nst == Segment::Type::StartRepeatBarLine)
            w += score()->styleP(StyleIdx::timesigBarlineDistance);
      else if (st == Segment::Type::Breath)
            w += spatium() * 1.5;
      else if (st == Segment::Type::Ambitus)
            w += score()->styleP(StyleIdx::ambitusMargin);

      if (w < 0.0)
            w = 0.0;
      if (ns)
            w += ns->extraLeadingSpace().val() * spatium();
      return w;
      }

}           // namespace Ms
