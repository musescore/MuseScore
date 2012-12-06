//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: segment.cpp 5589 2012-04-28 13:48:19Z wschweer $
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
#include "spanner.h"
#include "line.h"
#include "hairpin.h"
#include "ottava.h"
#include "sig.h"
#include "staffstate.h"
#include "instrchange.h"
#include "clef.h"
#include "timesig.h"
#include "system.h"

//---------------------------------------------------------
//   subTypeName
//---------------------------------------------------------

const char* Segment::subTypeName() const
      {
      switch(subtype()) {
            case SegClef:                 return "Clef";
            case SegKeySig:               return "Key Signature";
            case SegTimeSig:              return "Time Signature";
            case SegStartRepeatBarLine:   return "Begin Repeat";
            case SegBarLine:              return "BarLine";
            case SegGrace:                return "Grace";
            case SegChordRest:            return "ChordRest";
            case SegBreath:               return "Breath";
            case SegEndBarLine:           return "EndBarLine";
            case SegTimeSigAnnounce:      return "Time Sig Precaution";
            case SegKeySigAnnounce:       return "Key Sig Precaution";
            default:
                  return "";
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
      setSubtype(st);
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

      empty = s.empty;           // cached value
      _tick = s._tick;

      foreach(Element* e, s._annotations) {
            Element* ne = e->clone();
            add(ne);
            }

      _elist.reserve(s._elist.size());
      foreach(Element* e, s._elist) {
            Element* ne = 0;
            if (e) {
                  ne = e->clone();
                  ne->setParent(this);
                  }
            _elist.append(ne);
            }
      _dotPosX = s._dotPosX;
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
      foreach(Spanner* s, _spannerFor)
            s->setScore(score);
      foreach(Element* e, _annotations)
            e->setScore(score);
      }

Segment::~Segment()
      {
      foreach(Element* e, _elist) {
            if (!e)
                  continue;
            if (e->type() == CLEF)
                  e->staff()->removeClef(static_cast<Clef*>(e));
            else if (e->type() == TIMESIG)
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

Segment* Segment::next1(SegmentTypes types) const
      {
      for (Segment* s = next1(); s; s = s->next1()) {
            if (s->subtype() & types)
                  return s;
            }
      return 0;
      }

//---------------------------------------------------------
//   next
//    got to next segment which has subtype in types
//---------------------------------------------------------

Segment* Segment::next(SegmentTypes types) const
      {
      for (Segment* s = next(); s; s = s->next()) {
            if (s->subtype() & types)
                  return s;
            }
      return 0;
      }

//---------------------------------------------------------
//   prev
//    got to previous segment which has subtype in types
//---------------------------------------------------------

Segment* Segment::prev(SegmentTypes types) const
      {
      for (Segment* s = prev(); s; s = s->prev()) {
            if (s->subtype() & types)
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

Segment* Segment::prev1(SegmentTypes types) const
      {
      for (Segment* s = prev1(); s; s = s->prev1()) {
            if (s->subtype() & types)
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
            if (seg->subtype() == SegChordRest) {
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
            if (staffIdx > staff)
                  e->setTrack(e->track() - VOICES);
            }

      fixStaffIdx();
      }

//---------------------------------------------------------
//   addSpanner
//---------------------------------------------------------

void Segment::addSpanner(Spanner* l)
      {
      Element* e = l->endElement();
      if (e)
            static_cast<Segment*>(e)->addSpannerBack(l);
      _spannerFor.append(l);
      foreach(SpannerSegment* ss, l->spannerSegments()) {
            Q_ASSERT(ss->spanner() == l);
            if (ss->system())
                  ss->system()->add(ss);
            }
      }

//---------------------------------------------------------
//   removeSpanner
//---------------------------------------------------------

void Segment::removeSpanner(Spanner* l)
      {
      if (!static_cast<Segment*>(l->endElement())->removeSpannerBack(l)) {
            qDebug("Segment(%p): cannot remove spannerBack %s %p, size %d", this, l->name(), l, _spannerFor.size());
            // abort();
            }
      if (!_spannerFor.removeOne(l)) {
            qDebug("Segment(%p): cannot remove spannerFor %s %p, size %d", this, l->name(), l, _spannerFor.size());
            // abort();
            }
      foreach(SpannerSegment* ss, l->spannerSegments()) {
            if (ss->system())
                  ss->system()->remove(ss);
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Segment::add(Element* el)
      {
// qDebug("%p segment add(%d, %d, %s %p)", this, tick(), el->track(), el->name(), el);
      el->setParent(this);

      int track = el->track();
      if (track == -1) {
            qDebug("element <%s> has invalid track -1", el->name());
            abort();
            }
      int staffIdx = track / VOICES;

      switch(el->type()) {
            case REPEAT_MEASURE:
                  measure()->setRepeatFlags(measure()->repeatFlags() | RepeatMeasureFlag);
                  _elist[track] = el;
                  empty = false;
                  break;

            case HAIRPIN:
                  addSpanner(static_cast<Spanner*>(el));
                  score()->updateHairpin(static_cast<Hairpin*>(el));
                  score()->setPlaylistDirty(true);
                  break;

            case OTTAVA:
                  {
                  addSpanner(static_cast<Spanner*>(el));
                  Ottava* o   = static_cast<Ottava*>(el);
                  Segment* es = static_cast<Segment*>(o->endElement());
                  if (es) {
                        int tick2   = es->tick();
                        int shift   = o->pitchShift();
                        Staff* st = o->staff();
                        st->pitchOffsets().setPitchOffset(tick(), shift);
                        st->pitchOffsets().setPitchOffset(tick2, 0);
                        }
                  score()->setPlaylistDirty(true);
                  }
                  break;

            case VOLTA:
            case TRILL:
            case PEDAL:
            case TEXTLINE:
                  addSpanner(static_cast<Spanner*>(el));
                  break;

            case DYNAMIC:
            case HARMONY:
            case SYMBOL:
            case FRET_DIAGRAM:
            case TEMPO_TEXT:
            case STAFF_TEXT:
            case REHEARSAL_MARK:
            case MARKER:
            case IMAGE:
            case TEXT:
            case TAB_DURATION_SYMBOL:
            case FIGURED_BASS:
                  _annotations.append(el);
                  break;
            case JUMP:
                  measure()->setRepeatFlags(measure()->repeatFlags() | RepeatJump);
                  _annotations.append(el);
                  break;

            case STAFF_STATE:
                  if (static_cast<StaffState*>(el)->subtype() == STAFF_STATE_INSTRUMENT) {
                        StaffState* ss = static_cast<StaffState*>(el);
                        Part* part = el->staff()->part();
                        part->setInstrument(ss->instrument(), tick());
                        }
                  _annotations.append(el);
                  break;

            case INSTRUMENT_CHANGE: {
                  InstrumentChange* is = static_cast<InstrumentChange*>(el);
                  Part* part = is->staff()->part();
                  part->setInstrument(is->instrument(), tick());
                  _annotations.append(el);
                  break;
                  }

            case CLEF:
                  _elist[track] = el;
                  el->staff()->addClef(static_cast<Clef*>(el));
                  empty = false;
                  break;

            case TIMESIG:
                  _elist[track] = el;
                  el->staff()->addTimeSig(static_cast<TimeSig*>(el));
                  empty = false;
                  break;

            case CHORD:
            case REST:
                  if (_elist[track]) {
                        qDebug("Segment::add(%s) there is already an %s at %s(%d) track %d",
                           el->name(), _elist[track]->name(),
                           score()->sigmap()->pos(tick()), tick(), track);
                        ChordRest* cr = static_cast<ChordRest*>(el);
                        ChordRest* cr1  = static_cast<ChordRest*>(_elist[track]);
                        qDebug("   %d/%d -> %d/%d",
                           cr->duration().numerator(), cr->duration().denominator(),
                           cr1->duration().numerator(), cr1->duration().denominator());
                        // abort();
                        return;
                        }
                  if (track % VOICES)
                        measure()->mstaff(staffIdx)->hasVoices = true;

                  // fall through

            case KEYSIG:
            case BAR_LINE:
            case BREATH:
                  _elist[track] = el;
                  empty = false;
                  break;

            default:
                  qDebug("Segment::add() unknown %s", el->name());
                  abort();
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
            case CHORD:
            case REST:
                  {
                  _elist[track] = 0;
                  int staffIdx = el->staffIdx();
                  measure()->checkMultiVoices(staffIdx);
                  }
                  break;

            case REPEAT_MEASURE:
                  measure()->setRepeatFlags(measure()->repeatFlags() & ~RepeatMeasureFlag);
                  _elist[track] = 0;
                  break;

            case OTTAVA:
                  {
                  Ottava* o   = static_cast<Ottava*>(el);
                  Segment* es = static_cast<Segment*>(o->endElement());
                  int tick2   = es->tick();
                  Staff* st   = o->staff();
                  st->pitchOffsets().remove(tick());
                  st->pitchOffsets().remove(tick2);
                  removeSpanner(static_cast<Spanner*>(el));
                  score()->setPlaylistDirty(true);
                  }
                  break;

            case HAIRPIN:
                  score()->removeHairpin(static_cast<Hairpin*>(el));
                  removeSpanner(static_cast<Spanner*>(el));
                  score()->setPlaylistDirty(true);
                  break;

            case VOLTA:
            case TRILL:
            case PEDAL:
            case TEXTLINE:
                  removeSpanner(static_cast<Spanner*>(el));
                  break;

            case DYNAMIC:
            case HARMONY:
            case SYMBOL:
            case FRET_DIAGRAM:
            case TEMPO_TEXT:
            case STAFF_TEXT:
            case REHEARSAL_MARK:
            case MARKER:
            case IMAGE:
            case TEXT:
            case TAB_DURATION_SYMBOL:
            case FIGURED_BASS:
                  _annotations.removeOne(el);
                  break;

            case JUMP:
                  measure()->setRepeatFlags(measure()->repeatFlags() & ~RepeatJump);
                  _annotations.removeOne(el);
                  break;

            case STAFF_STATE:
                  if (static_cast<StaffState*>(el)->subtype() == STAFF_STATE_INSTRUMENT) {
                        Part* part = el->staff()->part();
                        part->removeInstrument(tick());
                        }
                  _annotations.removeOne(el);
                  break;

            case INSTRUMENT_CHANGE:
                  {
                  InstrumentChange* is = static_cast<InstrumentChange*>(el);
                  Part* part = is->staff()->part();
                  part->removeInstrument(tick());
                  }
                  _annotations.removeOne(el);
                  break;

            case CLEF:
                  _elist[track] = 0;
                  el->staff()->removeClef(static_cast<Clef*>(el));
                  break;

            case TIMESIG:
                  _elist[track] = 0;
                  el->staff()->removeTimeSig(static_cast<TimeSig*>(el));
                  break;

            case KEYSIG:
            case BAR_LINE:
            case BREATH:
                  _elist[track] = 0;
                  break;

            default:
                  qDebug("Segment::remove() unknown %s", el->name());
                  abort();

            }
      checkEmpty();
      }

//---------------------------------------------------------
//   segmentType
//    returns segment type suitable for storage of Element
//---------------------------------------------------------

Segment::SegmentType Segment::segmentType(ElementType type)
      {
      switch (type) {
            case CHORD:
            case REST:
            case REPEAT_MEASURE:
            case JUMP:
            case MARKER:
                  return SegChordRest;
            case CLEF:
                  return SegClef;
            case KEYSIG:
                  return SegKeySig;
            case TIMESIG:
                  return SegTimeSig;
            case BAR_LINE:
                  return SegStartRepeatBarLine;
            case BREATH:
                  return SegBreath;
            default:
                  qDebug("Segment:segmentType():  bad type: <%s>", Element::name(type));
                  return (SegmentType)-1;
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

const QList<Lyrics*>* Segment::lyricsList(int staffIdx) const
      {
      if (!(subtype() & (SegChordRestGrace))) {
            if (MScore::debugMode)
                  qDebug("warning : lyricsList  bad segment type <%s><%s>", name(), subTypeName());
            return 0;
            }
      int strack = staffIdx * VOICES;
      int etrack = strack + VOICES;
      for (int track = strack; track < etrack; ++track) {
            ChordRest* cr = static_cast<ChordRest*>(element(track));
            if (cr)
                  return &cr->lyricsList();
            }
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
      xml.tag("subtype", _subtype);
      xml.tag("leadingSpace", _extraLeadingSpace.val());
      xml.tag("trailingSpace", _extraTrailingSpace.val());
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Segment::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());

            if (tag == "subtype")
                  _subtype = SegmentType(val.toInt());
            else if (tag == "leadingSpace")
                  _extraLeadingSpace = Spatium(val.toDouble());
            else if (tag == "trailingSpace")
                  _extraTrailingSpace = Spatium(val.toDouble());
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Segment::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_LEADING_SPACE:   return extraLeadingSpace().val();
            case P_TRAILING_SPACE:  return extraTrailingSpace().val();
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
            case P_LEADING_SPACE: setExtraLeadingSpace(Spatium(v.toDouble())); break;
            case P_TRAILING_SPACE: setExtraTrailingSpace(Spatium(v.toDouble())); break;
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
      if (subtype() != SegChordRest)
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


