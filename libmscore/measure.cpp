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

/**
 \file
 Implementation of most part of class Measure.
*/

#include "measure.h"
#include "accidental.h"
#include "ambitus.h"
#include "articulation.h"
#include "barline.h"
#include "beam.h"
#include "box.h"
#include "bracket.h"
#include "breath.h"
#include "chord.h"
#include "clef.h"
#include "drumset.h"
#include "duration.h"
#include "dynamic.h"
#include "fret.h"
#include "glissando.h"
#include "hairpin.h"
#include "harmony.h"
#include "hook.h"
#include "icon.h"
#include "image.h"
#include "key.h"
#include "keysig.h"
#include "layoutbreak.h"
#include "layout.h"
#include "note.h"
#include "ottava.h"
#include "page.h"
#include "part.h"
#include "pedal.h"
#include "pitchspelling.h"
#include "repeat.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "select.h"
#include "sig.h"
#include "slur.h"
#include "spacer.h"
#include "staff.h"
#include "stafftext.h"
#include "stafftype.h"
#include "stringdata.h"
#include "style.h"
#include "sym.h"
#include "system.h"
#include "tempotext.h"
#include "text.h"
#include "tie.h"
#include "tiemap.h"
#include "timesig.h"
#include "tremolo.h"
#include "trill.h"
#include "tuplet.h"
#include "tupletmap.h"
#include "undo.h"
#include "utils.h"
#include "volta.h"
#include "xml.h"
#include "systemdivider.h"

namespace Ms {

//---------------------------------------------------------
//   MStaff
//---------------------------------------------------------

MStaff::~MStaff()
      {
      delete _noText;
      delete lines;
      delete _vspacerUp;
      delete _vspacerDown;
      }

MStaff::MStaff(const MStaff& m)
      {
      _noText      = 0;
      lines        = new StaffLines(*m.lines);
      hasVoices    = m.hasVoices;
      _vspacerUp   = 0;
      _vspacerDown = 0;
      _visible     = m._visible;
      _slashStyle  = m._slashStyle;
#ifndef NDEBUG
      _corrupted   = m._corrupted;
#endif
      }

//---------------------------------------------------------
//   Measure
//---------------------------------------------------------

Measure::Measure(Score* s)
   : MeasureBase(s), _timesig(4,4), _len(4,4)
      {
      _repeatCount           = 2;

      int n = score()->nstaves();
      _mstaves.reserve(n);
      for (int staffIdx = 0; staffIdx < n; ++staffIdx) {
            MStaff* s    = new MStaff;
            Staff* staff = score()->staff(staffIdx);
            s->lines     = new StaffLines(score());
            s->lines->setTrack(staffIdx * VOICES);
            s->lines->setParent(this);
            s->lines->setVisible(!staff->invisible());
            _mstaves.push_back(s);
            }
      setIrregular(false);
      _noMode                   = MeasureNumberMode::AUTO;
      _userStretch              = 1.0;
      _breakMultiMeasureRest    = false;
      _mmRest                   = 0;
      _mmRestCount              = 0;
      setFlag(ElementFlag::MOVABLE, true);
      }

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

Measure::Measure(const Measure& m)
   : MeasureBase(m)
      {
      _segments     = m._segments.clone();
      _timesig      = m._timesig;
      _len          = m._len;
      _repeatCount  = m._repeatCount;
      _userStretch  = m._userStretch;

      _mstaves.reserve(m._mstaves.size());
      for (MStaff* ms : m._mstaves)
            _mstaves.push_back(new MStaff(*ms));

      _breakMultiMeasureRest = m._breakMultiMeasureRest;
      _mmRest                = m._mmRest;
      _mmRestCount           = m._mmRestCount;
      _playbackCount         = m._playbackCount;
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Measure::setScore(Score* score)
      {
      MeasureBase::setScore(score);
      for (Segment* s = first(); s; s = s->next())
            s->setScore(score);
      }

//---------------------------------------------------------
//   MStaff::setScore
//---------------------------------------------------------

void MStaff::setScore(Score* score)
      {
      lines->setScore(score);
      if (_vspacerUp)
            _vspacerUp->setScore(score);
      if (_vspacerDown)
            _vspacerDown->setScore(score);
      }

//---------------------------------------------------------
//   Measure
//---------------------------------------------------------

Measure::~Measure()
      {
      for (Segment* s = first(); s;) {
            Segment* ns = s->next();
            delete s;
            s = ns;
            }
      qDeleteAll(_mstaves);
      }

//---------------------------------------------------------
//   AcEl
//---------------------------------------------------------

struct AcEl {
      Note* note;
      qreal x;
      };

//---------------------------------------------------------
//   findAccidental
///   return current accidental value at note position
//---------------------------------------------------------

AccidentalVal Measure::findAccidental(Note* note) const
      {
      Chord* chord = note->chord();
      AccidentalState tversatz;  // state of already set accidentals for this measure
      tversatz.init(chord->staff()->key(tick()));

      for (Segment* segment = first(); segment; segment = segment->next()) {
            int startTrack = chord->staffIdx() * VOICES;
            if (segment->segmentType() == Segment::Type::KeySig) {
                  KeySig* ks = static_cast<KeySig*>(segment->element(startTrack));
                  if (!ks)
                        continue;
                  tversatz.init(chord->staff()->key(segment->tick()));
                  }
            else if (segment->segmentType() == Segment::Type::ChordRest) {
                  int endTrack   = startTrack + VOICES;
                  for (int track = startTrack; track < endTrack; ++track) {
                        Element* e = segment->element(track);
                        if (!e || e->type() != Element::Type::CHORD)
                              continue;
                        Chord* chord = static_cast<Chord*>(e);
                        for (Chord* chord1 : chord->graceNotes()) {
                              for (Note* note1 : chord1->notes()) {
                                    if (note1->tieBack())
                                          continue;
                                    //
                                    // compute accidental
                                    //
                                    int tpc  = note1->tpc();
                                    int line = absStep(tpc, note1->epitch());

                                    if (note == note1)
                                          return tversatz.accidentalVal(line);
                                    tversatz.setAccidentalVal(line, tpc2alter(tpc));
                                    }
                              }
                        for (Note* note1 : chord->notes()) {
                              if (note1->tieBack())
                                    continue;
                              //
                              // compute accidental
                              //
                              int tpc  = note1->tpc();
                              int line = absStep(tpc, note1->epitch());

                              if (note == note1)
                                    return tversatz.accidentalVal(line);
                              tversatz.setAccidentalVal(line, tpc2alter(tpc));
                              }
                        }
                  }
            }
      qDebug("Measure::findAccidental: note not found");
      return AccidentalVal::NATURAL;
      }

//---------------------------------------------------------
//   findAccidental
///   Compute accidental state at segment/staffIdx for
///   relative staff line.
//---------------------------------------------------------

AccidentalVal Measure::findAccidental(Segment* s, int staffIdx, int line, bool &error) const
      {
      AccidentalState tversatz;  // state of already set accidentals for this measure
      Staff* staff = score()->staff(staffIdx);
      tversatz.init(staff->key(tick()));

      Segment::Type st = Segment::Type::ChordRest;
      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;
      for (Segment* segment = first(st); segment; segment = segment->next(st)) {
            if (segment == s && staff->isPitchedStaff()) {
                  ClefType clef = staff->clef(s->tick());
                  int l = relStep(line, clef);
                  return tversatz.accidentalVal(l, error);
                  }
            for (int track = startTrack; track < endTrack; ++track) {
                  Element* e = segment->element(track);
                  if (!e || e->type() != Element::Type::CHORD)
                        continue;
                  Chord* chord = static_cast<Chord*>(e);
                  for (Chord* chord1 : chord->graceNotes()) {
                        for (Note* note : chord1->notes()) {
                              if (note->tieBack())
                                    continue;
                              int tpc  = note->tpc();
                              int l    = absStep(tpc, note->epitch());
                              tversatz.setAccidentalVal(l, tpc2alter(tpc));
                              }
                        }

                  for (Note* note : chord->notes()) {
                        if (note->tieBack())
                              continue;
                        int tpc    = note->tpc();
                        int l      = absStep(tpc, note->epitch());
                        tversatz.setAccidentalVal(l, tpc2alter(tpc));
                        }
                  }
            }
      qDebug("segment not found");
      return AccidentalVal::NATURAL;
      }

//---------------------------------------------------------
//   tick2pos
//    return x position for tick relative to System
//---------------------------------------------------------

qreal Measure::tick2pos(int tck) const
      {
      if (isMMRest()) {
            Segment* s = first(Segment::Type::ChordRest);
            qreal x1 = s->x();
            qreal w  = width() - x1;
            return x1 + (tck * w) / (ticks() * mmRestCount());
            }

      Segment* s;
      qreal x1 = 0;
      qreal x2 = 0;
      int tick1 = tick();
      int tick2 = tick1;
      for (s = first(Segment::Type::ChordRest); s; s = s->next(Segment::Type::ChordRest)) {
            x2    = s->x();
            tick2 = s->tick();
            if (tck == tick2)
                  return x2 + pos().x();
            if (tck <= tick2)
                  break;
            x1    = x2;
            tick1 = tick2;
            }
      if (s == 0) {
            x2    = width();
            tick2 = endTick();
            }
      qreal dx = x2 - x1;
      int dt   = tick2 - tick1;
      x1      += (dt == 0) ? 0.0 : (dx * (tck - tick1) / dt);
      return x1 + pos().x();
      }

//---------------------------------------------------------
//   layout2
//    called after layout of page
//---------------------------------------------------------

void Measure::layout2()
      {
      Q_ASSERT(parent());
      Q_ASSERT(score()->nstaves() == int(_mstaves.size()));

      qreal _spatium = spatium();

      for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
            MStaff* ms = _mstaves[staffIdx];
//            ms->lines->setWidth(width());
            Spacer* sp = ms->_vspacerDown;
            if (sp) {
                  sp->layout();
                  int n = score()->staff(staffIdx)->lines() - 1;
                  qreal y = system()->staff(staffIdx)->y();
                  sp->setPos(_spatium * .5, y + n * _spatium);
                  }
            sp = ms->_vspacerUp;
            if (sp) {
                  sp->layout();
                  qreal y = system()->staff(staffIdx)->y();
                  sp->setPos(_spatium * .5, y - sp->gap());
                  }
            }

      MeasureBase::layout();  // layout LAYOUT_BREAK elements

      //---------------------------------------------------
      //   set measure number
      //---------------------------------------------------

      bool smn = false;

      if (_noMode == MeasureNumberMode::SHOW)
            smn = true;
      else if (_noMode == MeasureNumberMode::HIDE)
            smn = false;
      else {
            if (score()->styleB(StyleIdx::showMeasureNumber)
               && !irregular()
               && (no() || score()->styleB(StyleIdx::showMeasureNumberOne))) {
                  if (score()->styleB(StyleIdx::measureNumberSystem))
                        smn = system()->firstMeasure() == this;
                  else {
                        smn = (no() == 0 && score()->styleB(StyleIdx::showMeasureNumberOne)) ||
                              ( ((no()+1) % score()->style(StyleIdx::measureNumberInterval).toInt()) == 0 );
                        }
                  }
            }
      QString s;
      if (smn)
            s = QString("%1").arg(no() + 1);
      int nn = 1;
      bool nas = score()->styleB(StyleIdx::measureNumberAllStaffs);

      if (!nas) {
            //find first non invisible staff
            for (unsigned staffIdx = 0; staffIdx < _mstaves.size(); ++staffIdx) {
                  MStaff* ms = _mstaves[staffIdx];
                  SysStaff* s  = system()->staff(staffIdx);
                  Staff* staff = score()->staff(staffIdx);
                  if (ms->visible() && staff->show() && s->show()) {
                        nn = staffIdx;
                        break;
                        }
                  }
            }
      for (int staffIdx = 0; staffIdx < int(_mstaves.size()); ++staffIdx) {
            MStaff* ms = _mstaves[staffIdx];
            Text* t = ms->noText();
            if (t)
                  t->setTrack(staffIdx * VOICES);
            if (smn && ((staffIdx == nn) || nas)) {
                  if (t == 0) {
                        t = new Text(score());
                        t->setFlag(ElementFlag::ON_STAFF, true);
                        t->setTrack(staffIdx * VOICES);
                        t->setGenerated(true);
                        t->setTextStyleType(TextStyleType::MEASURE_NUMBER);
                        t->setParent(this);
                        score()->undo(new AddElement(t));
                        // score()->undoAddElement(t);
                        }
                  t->setXmlText(s);
                  t->layout();
                  }
            else {
                  if (t)
                        score()->undo(new RemoveElement(t));
                  }
            }

      //---------------------------------------------------
      //    layout ties, spanners and tuples
      //---------------------------------------------------

      int tracks = score()->ntracks();
      static const Segment::Type st { Segment::Type::ChordRest };
      for (int track = 0; track < tracks; ++track) {
            if (!score()->staff(track / VOICES)->show()) {
                  track += VOICES-1;
                  continue;
                  }
            for (Segment* s = first(st); s; s = s->next(st)) {
                  ChordRest* cr = s->cr(track);
                  if (!cr)
                        continue;

                  if (cr->type() == Element::Type::CHORD) {
                        Chord* c = static_cast<Chord*>(cr);
                        for (const Note* note : c->notes()) {
                              Tie* tie = note->tieFor();
                              if (tie)
                                    tie->layout();
                              for (Spanner* sp : note->spannerFor())
                                    sp->layout();
                              }
                        }
                  DurationElement* de = cr;
                  while (de->tuplet() && de->tuplet()->elements().front() == de) {
                        de->tuplet()->layout();
                        de = de->tuplet();
                        }
                  }
            }
      }

//---------------------------------------------------------
//   findChord
///   Search for chord at position \a tick in \a track
//---------------------------------------------------------

Chord* Measure::findChord(int tick, int track)
      {
      for (Segment* seg = last(); seg; seg = seg->prev()) {
            if (seg->tick() < tick)
                  return 0;
            if (seg->tick() == tick) {
                  Element* el = seg->element(track);
                  if (el && el->isChord())
                        return toChord(el);
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   findChordRest
///   Search for chord or rest at position \a tick at \a staff in \a voice.
//---------------------------------------------------------

ChordRest* Measure::findChordRest(int tick, int track)
      {
      for (const Segment& seg : _segments) {
            if (seg.tick() > tick)
                  return 0;
            if (seg.tick() == tick) {
                  Element* el = seg.element(track);
                  if (el && el->isChordRest())
                        return toChordRest(el);
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   tick2segment
//---------------------------------------------------------

Segment* Measure::tick2segment(int tick, Segment::Type st)
      {
      for (Segment& s : _segments) {
            if (s.tick() == tick) {
                  if (s.segmentType() & st)
                        return &s;
                  }
            if (s.tick() > tick)
                  break;
            }
      return 0;
      }

//---------------------------------------------------------
//   findSegment
/// Search for a segment of type \a st at position \a t.
//---------------------------------------------------------

Segment* Measure::findSegment(Segment::Type st, int t)
      {
      Segment* s;
      for (s = first(); s && s->tick() < t; s = s->next())
            ;
      for (; s && s->tick() == t; s = s->next()) {
            if (s->segmentType() & st)
                  return s;
            }
      return 0;
      }

//---------------------------------------------------------
//   undoGetSegment
//---------------------------------------------------------

Segment* Measure::undoGetSegment(Segment::Type type, int tick)
      {
      Segment* s = findSegment(type, tick);
      if (s == 0) {
            s = new Segment(this, type, tick);
            score()->undoAddElement(s);
            }
      return s;
      }

//---------------------------------------------------------
//   getSegment
//---------------------------------------------------------

Segment* Measure::getSegment(Element* e, int tick)
      {
      return getSegment(Segment::segmentType(e->type()), tick);
      }

//---------------------------------------------------------
//   getSegment
///   Get a segment of type \a st at tick position \a t.
///   If the segment does not exist, it is created.
//---------------------------------------------------------

Segment* Measure::getSegment(Segment::Type st, int tick)
      {
      Segment* s = findSegment(st, tick);
      if (!s) {
            s = new Segment(this, st, tick);
            add(s);
            }
      return s;
      }

//---------------------------------------------------------
//   add
///   Add new Element \a el to Measure.
//---------------------------------------------------------

void Measure::add(Element* e)
      {
      e->setParent(this);
      Element::Type type = e->type();

      switch (type) {
            case Element::Type::TEXT:
                  if (e->staffIdx() < int(_mstaves.size()))
                        _mstaves[e->staffIdx()]->setNoText(static_cast<Text*>(e));
                  break;

            case Element::Type::SPACER:
                  {
                  Spacer* sp = toSpacer(e);
                  switch (sp->spacerType()) {
                        case SpacerType::UP:
                              _mstaves[e->staffIdx()]->_vspacerUp = sp;
                              break;
                        case SpacerType::DOWN:
                        case SpacerType::FIXED:
                              _mstaves[e->staffIdx()]->_vspacerDown = sp;
                              break;
                        }
                  }
                  break;
            case Element::Type::SEGMENT:
                  {
                  Segment* seg     = toSegment(e);
                  int t            = seg->tick();
                  Segment::Type st = seg->segmentType();
                  Segment* s;
                  for (s = first(); s && s->tick() < t; s = s->next())
                        ;
                  if (s) {
                        if (st == Segment::Type::ChordRest) {
                              // add ChordRest segment after all other segments with same tick
                              // except EndBarLine
                              while (s && s->segmentType() != st && s->tick() == t) {
                                    if (s->segmentType() == Segment::Type::EndBarLine) {
                                          break;
                                          }
                                    s = s->next();
                                    }
                              }
                        else {
                              // use order of segments in segment.h
                              if (s && s->tick() == t) {
                                    while (s && s->segmentType() <= st) {
                                          s = s->next();
                                          if (s && s->tick() != t)
                                                break;
                                          }
                                    }
                              }
                        }
                  seg->setParent(this);
                  _segments.insert(seg, s);
                  }
                  break;

            case Element::Type::JUMP:
                  setRepeatJump(true);
                  // fall through

            case Element::Type::MARKER:
                  el().push_back(e);
                  break;

            case Element::Type::HBOX:
                  if (e->staff())
                        e->setMag(e->staff()->mag());     // ?!
                  el().push_back(e);
                  break;

            case Element::Type::MEASURE:
                  _mmRest = static_cast<Measure*>(e);
                  break;

            default:
                  MeasureBase::add(e);
                  break;
            }
      }

//---------------------------------------------------------
//   remove
///   Remove Element \a el from Measure.
//---------------------------------------------------------

void Measure::remove(Element* e)
      {
      Q_ASSERT(e->parent() == this);
      Q_ASSERT(e->score() == score());

      switch (e->type()) {
            case Element::Type::TEXT:
                  _mstaves[e->staffIdx()]->setNoText(static_cast<Text*>(0));
                  break;

            case Element::Type::SPACER:
                  switch (toSpacer(e)->spacerType()) {
                        case SpacerType::DOWN:
                        case SpacerType::FIXED:
                              _mstaves[e->staffIdx()]->_vspacerDown = 0;
                              break;
                        case SpacerType::UP:
                              _mstaves[e->staffIdx()]->_vspacerUp = 0;
                              break;
                        }
                  break;

            case Element::Type::SEGMENT:
                  _segments.remove(toSegment(e));
                  break;

            case Element::Type::JUMP:
                  setRepeatJump(false);
                  // fall through

            case Element::Type::MARKER:
            case Element::Type::HBOX:
                  if (!el().remove(e)) {
                        qDebug("Measure(%p)::remove(%s,%p) not found", this, e->name(), e);
                        }
                  break;

            case Element::Type::CLEF:
            case Element::Type::CHORD:
            case Element::Type::REST:
            case Element::Type::TIMESIG:
                  for (Segment* segment = first(); segment; segment = segment->next()) {
                        int staves = score()->nstaves();
                        int tracks = staves * VOICES;
                        for (int track = 0; track < tracks; ++track) {
                              Element* ee = segment->element(track);
                              if (ee == e) {
                                    segment->setElement(track, 0);
                                    return;
                                    }
                              }
                        }
                  qDebug("Measure::remove: %s %p not found", e->name(), e);
                  break;

            case Element::Type::MEASURE:
                  _mmRest = 0;
                  break;

            default:
                  MeasureBase::remove(e);
                  break;
            }
      }

//---------------------------------------------------------
//   change
//---------------------------------------------------------

void Measure::change(Element* o, Element* n)
      {
      if (o->type() == Element::Type::TUPLET) {
            Tuplet* t = static_cast<Tuplet*>(n);
            for (DurationElement* e : t->elements())
                  e->setTuplet(t);
            }
      else {
            remove(o);
            add(n);
            }
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Measure::spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/)
      {
      }

//-------------------------------------------------------------------
//   moveTicks
//    Also adjust endBarLine if measure len has changed. For this
//    diff == 0 cannot be optimized away
//-------------------------------------------------------------------

void Measure::moveTicks(int diff)
      {
      setTick(tick() + diff);
//      for (Segment* segment = first(); segment; segment = segment->next()) {
//            if (segment->segmentType() & (Segment::Type::EndBarLine | Segment::Type::TimeSigAnnounce))
//                  segment->setTick(tick() + ticks());
//            }
      for (Segment* segment = last(); segment; segment = segment->prev()) {
            if (segment->segmentType() & (Segment::Type::EndBarLine | Segment::Type::TimeSigAnnounce))
                  segment->setTick(tick() + ticks());
            else if (segment->isChordRestType())
                  break;
            }
      }

//---------------------------------------------------------
//   removeStaves
//---------------------------------------------------------

void Measure::removeStaves(int sStaff, int eStaff)
      {
      for (Segment* s = first(); s; s = s->next()) {
            for (int staff = eStaff-1; staff >= sStaff; --staff) {
                  s->removeStaff(staff);
                  }
            }
      for (Element* e : el()) {
            if (e->track() == -1)
                  continue;
            int voice = e->voice();
            int staffIdx = e->staffIdx();
            if (staffIdx >= eStaff) {
                  staffIdx -= eStaff - sStaff;
                  e->setTrack(staffIdx * VOICES + voice);
                  }
            }
      }

//---------------------------------------------------------
//   insertStaves
//---------------------------------------------------------

void Measure::insertStaves(int sStaff, int eStaff)
      {
      for (Element* e : el()) {
            if (e->track() == -1)
                  continue;
            int staffIdx = e->staffIdx();
            if (staffIdx >= sStaff && !e->systemFlag()) {
                  int voice = e->voice();
                  staffIdx += eStaff - sStaff;
                  e->setTrack(staffIdx * VOICES + voice);
                  }
            }
      for (Segment* s = first(); s; s = s->next()) {
            for (int staff = sStaff; staff < eStaff; ++staff) {
                  s->insertStaff(staff);
                  }
            }
      }

//---------------------------------------------------------
//   cmdRemoveStaves
//---------------------------------------------------------

void Measure::cmdRemoveStaves(int sStaff, int eStaff)
      {
      int sTrack = sStaff * VOICES;
      int eTrack = eStaff * VOICES;
      for (Segment* s = first(); s; s = s->next()) {
            for (int track = eTrack - 1; track >= sTrack; --track) {
                  Element* el = s->element(track);
                  if (el) {
                        el->undoUnlink();
                        score()->undo(new RemoveElement(el));
                        }
                  }
            foreach (Element* e, s->annotations()) {
                  int staffIdx = e->staffIdx();
                  if ((staffIdx >= sStaff) && (staffIdx < eStaff) && !e->systemFlag()) {
                        e->undoUnlink();
                        score()->undo(new RemoveElement(e));
                        }
                  }
            }
      for (Element* e : el()) {
            if (e->track() == -1)
                  continue;
            int staffIdx = e->staffIdx();
            if (staffIdx >= sStaff && (staffIdx < eStaff) && !e->systemFlag()) {
                  e->undoUnlink();
                  score()->undo(new RemoveElement(e));
                  }
            }

      score()->undo(new RemoveStaves(this, sStaff, eStaff));

      for (int i = eStaff - 1; i >= sStaff; --i) {
            MStaff* ms = *(_mstaves.begin()+i);
            Text* t = ms->noText();
            if (t) {
//                  t->undoUnlink();
//                  score()->undo(new RemoveElement(t));
                  }
            score()->undo(new RemoveMStaff(this, ms, i));
            }

      // barLine
      // TODO
      }

//---------------------------------------------------------
//   cmdAddStaves
//---------------------------------------------------------

void Measure::cmdAddStaves(int sStaff, int eStaff, bool createRest)
      {
      score()->undo(new InsertStaves(this, sStaff, eStaff));

      Segment* ts = findSegment(Segment::Type::TimeSig, tick());

      for (int i = sStaff; i < eStaff; ++i) {
            Staff* staff = score()->staff(i);
            MStaff* ms   = new MStaff;
            ms->lines    = new StaffLines(score());
            ms->lines->setTrack(i * VOICES);
            ms->lines->setParent(this);
            ms->lines->setVisible(!staff->invisible());
            score()->undo(new InsertMStaff(this, ms, i));
            }

      if (!createRest && !ts)
            return;


      // create list of unique staves (only one instance for linked staves):

      QList<int> sl;
      for (int staffIdx = sStaff; staffIdx < eStaff; ++staffIdx) {
            Staff* s = score()->staff(staffIdx);
            if (s->linkedStaves()) {
                  bool alreadyInList = false;
                  for (int idx : sl) {
                        if (s->linkedStaves()->staves().contains(score()->staff(idx))) {
                              alreadyInList = true;
                              break;
                              }
                        }
                  if (alreadyInList)
                        continue;
                  }
            sl.append(staffIdx);
            }

      for (int staffIdx : sl) {
            if (createRest)
                  score()->setRest(tick(), staffIdx * VOICES, len(), false, 0, _timesig == len());

            // replicate time signature
            if (ts) {
                  TimeSig* ots = 0;
                  bool constructed = false;
                  for (unsigned track = 0; track < _mstaves.size() * VOICES; ++track) {
                        if (ts->element(track)) {
                              ots = static_cast<TimeSig*>(ts->element(track));
                              break;
                              }
                        }
                  if (!ots) {
                        // no time signature found; use measure length to construct one
                        ots = new TimeSig(score());
                        ots->setSig(len());
                        constructed = true;
                        }
                  // do no replicate local time signatures
                  if (ots && !ots->isLocal()) {
                        TimeSig* timesig = new TimeSig(*ots);
                        timesig->setTrack(staffIdx * VOICES);
                        timesig->setParent(ts);
                        timesig->setSig(ots->sig(), ots->timeSigType());
                        timesig->setNeedLayout(true);
                        score()->undoAddElement(timesig);
                        if (constructed)
                              delete ots;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void MStaff::setTrack(int track)
      {
      lines->setTrack(track);
      if (_vspacerUp)
            _vspacerUp->setTrack(track);
      if (_vspacerDown)
            _vspacerDown->setTrack(track);
      }

//---------------------------------------------------------
//   insertMStaff
//---------------------------------------------------------

void Measure::insertMStaff(MStaff* staff, int idx)
      {
      _mstaves.insert(_mstaves.begin()+idx, staff);
      for (unsigned staffIdx = 0; staffIdx < _mstaves.size(); ++staffIdx)
            _mstaves[staffIdx]->setTrack(staffIdx * VOICES);
      }

//---------------------------------------------------------
//   removeMStaff
//---------------------------------------------------------

void Measure::removeMStaff(MStaff* /*staff*/, int idx)
      {
      _mstaves.erase(_mstaves.begin()+idx);
      for (unsigned staffIdx = 0; staffIdx < _mstaves.size(); ++staffIdx)
            _mstaves[staffIdx]->setTrack(staffIdx * VOICES);
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Measure::insertStaff(Staff* staff, int staffIdx)
      {
      for (Segment* s = first(); s; s = s->next())
            s->insertStaff(staffIdx);

      MStaff* ms = new MStaff;
      ms->lines  = new StaffLines(score());
      ms->lines->setParent(this);
      ms->lines->setTrack(staffIdx * VOICES);
      ms->lines->setVisible(!staff->invisible());
      insertMStaff(ms, staffIdx);
      }

//---------------------------------------------------------
//   staffabbox
//---------------------------------------------------------

QRectF Measure::staffabbox(int staffIdx) const
      {
      System* s = system();
      QRectF sb(s->staff(staffIdx)->bbox());
      QRectF rrr(sb.translated(s->pagePos()));
      QRectF rr(abbox());
      QRectF r(rr.x(), rrr.y(), rr.width(), rrr.height());
      return r;
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

/**
 Return true if an Element of type \a type can be dropped on a Measure
*/

bool Measure::acceptDrop(const DropData& data) const
      {
      MuseScoreView* viewer = data.view;
      QPointF pos           = data.pos;
      Element* e            = data.element;

      int staffIdx;
      Segment* seg;
      if (score()->pos2measure(pos, &staffIdx, 0, &seg, 0) == nullptr)
            return false;

      QRectF staffR = system()->staff(staffIdx)->bbox().translated(system()->canvasPos());
      staffR &= canvasBoundingRect();

      switch (e->type()) {
            case Element::Type::MEASURE_LIST:
            case Element::Type::JUMP:
            case Element::Type::MARKER:
            case Element::Type::LAYOUT_BREAK:
            case Element::Type::STAFF_LIST:
                  viewer->setDropRectangle(canvasBoundingRect());
                  return true;

            case Element::Type::KEYSIG:
            case Element::Type::TIMESIG:
                  if (data.modifiers & Qt::ControlModifier)
                        viewer->setDropRectangle(staffR);
                  else
                        viewer->setDropRectangle(canvasBoundingRect());
                  return true;

            case Element::Type::BRACKET:
            case Element::Type::REPEAT_MEASURE:
            case Element::Type::MEASURE:
            case Element::Type::SPACER:
            case Element::Type::IMAGE:
            case Element::Type::BAR_LINE:
            case Element::Type::SYMBOL:
            case Element::Type::CLEF:
                  viewer->setDropRectangle(staffR);
                  return true;

            case Element::Type::ICON:
                  switch(static_cast<Icon*>(e)->iconType()) {
                        case IconType::VFRAME:
                        case IconType::HFRAME:
                        case IconType::TFRAME:
                        case IconType::FFRAME:
                        case IconType::MEASURE:
                              viewer->setDropRectangle(canvasBoundingRect());
                              return true;
                        default:
                              break;
                        }
                  break;

            default:
                  break;
            }
      return false;
      }

//---------------------------------------------------------
//   drop
///   Drop element.
///   Handle a dropped element at position \a pos of given
///   element \a type and \a subtype.
//---------------------------------------------------------

Element* Measure::drop(const DropData& data)
      {
      Element* e = data.element;
      int staffIdx = -1;
      Segment* seg;
      score()->pos2measure(data.pos, &staffIdx, 0, &seg, 0);

      if (e->systemFlag())
            staffIdx = 0;
      if (staffIdx < 0)
            return 0;
      Staff* staff = score()->staff(staffIdx);
      bool fromPalette = (e->track() == -1);

      switch (e->type()) {
            case Element::Type::MEASURE_LIST:
                  delete e;
                  break;

            case Element::Type::STAFF_LIST:
//TODO                  score()->pasteStaff(e, this, staffIdx);
                  delete e;
                  break;

            case Element::Type::MARKER:
            case Element::Type::JUMP:
                  e->setParent(this);
                  e->setTrack(0);
                  {
                  // code borrowed from ChordRest::drop()
                  Text* t = static_cast<Text*>(e);
                  TextStyleType st = t->textStyleType();
                  // for palette items, we want to use current score text style settings
                  // except where the source element had explicitly overridden these via text properties
                  // palette text style will be relative to baseStyle, so rebase this to score
                  if (st >= TextStyleType::DEFAULT && fromPalette)
                        t->textStyle().restyle(MScore::baseStyle()->textStyle(st), score()->textStyle(st));
                  }
                  score()->undoAddElement(e);
                  return e;

            case Element::Type::DYNAMIC:
            case Element::Type::FRET_DIAGRAM:
                  e->setParent(seg);
                  e->setTrack(staffIdx * VOICES);
                  score()->undoAddElement(e);
                  return e;

            case Element::Type::IMAGE:
            case Element::Type::SYMBOL:
                  e->setParent(seg);
                  e->setTrack(staffIdx * VOICES);
                  e->layout();
                  {
                  QPointF uo(data.pos - e->canvasPos() - data.dragOffset);
                  e->setUserOff(uo);
                  }
                  score()->undoAddElement(e);
                  return e;

            case Element::Type::BRACKET:
                  {
                  Bracket* b = static_cast<Bracket*>(e);
                  int level = 0;
                  int firstStaff = 0;
                  for (Staff* s : score()->staves()) {
                        for (const BracketItem& bi : s->brackets()) {
                              int lastStaff = firstStaff + bi._bracketSpan - 1;
                              if (staffIdx >= firstStaff && staffIdx <= lastStaff)
                                    ++level;
                              }
                        firstStaff++;
                        }
                  score()->undoAddBracket(staff, level, b->bracketType(), 1);
                  delete b;
                  }
                  return 0;

            case Element::Type::CLEF:
                  score()->undoChangeClef(staff, first(), static_cast<Clef*>(e)->clefType());
                  delete e;
                  break;

            case Element::Type::KEYSIG:
                  {
                  KeySigEvent k = toKeySig(e)->keySigEvent();
                  delete e;

                  if (data.modifiers & Qt::ControlModifier) {
                        // apply only to this stave
                        score()->undoChangeKeySig(staff, tick(), k);
                        }
                  else {
                        // apply to all staves:
                        for (Staff* s : score()->staves())
                              score()->undoChangeKeySig(s, tick(), k);
                        }

                  break;
                  }

            case Element::Type::TIMESIG:
                  score()->cmdAddTimeSig(this, staffIdx, static_cast<TimeSig*>(e),
                     data.modifiers & Qt::ControlModifier);
                  return 0;

            case Element::Type::LAYOUT_BREAK: {
                  LayoutBreak* b = toLayoutBreak(e);
                  switch (b->layoutBreakType()) {
                        case  LayoutBreak::PAGE:
                              if (pageBreak()) {
                                    delete b;
                                    b = 0;
                                    }
                              else
                                    setLineBreak(false);
                              break;
                        case  LayoutBreak::LINE:
                              if (lineBreak()) {
                                    delete b;
                                    b = 0;
                                    }
                              else
                                    setPageBreak(false);
                              break;
                        case  LayoutBreak::SECTION:
                              if (sectionBreak()) {
                                    delete b;
                                    b = 0;
                                    }
                              else
                                    setLineBreak(false);
                              break;
                        case LayoutBreak::NOBREAK:
                              if (noBreak()) {
                                    delete b;
                                    b = 0;
                                    }
                              else {
                                    setLineBreak(false);
                                    setPageBreak(false);
                                    }
                              break;
                        }
                  if (b) {
                        b->setTrack(-1);       // these are system elements
                        b->setParent(this);
                        score()->undoAddElement(b);
                        }
                  cleanupLayoutBreaks(true);
                  return b;
                  }

            case Element::Type::SPACER:
                  {
                  Spacer* spacer = static_cast<Spacer*>(e);
                  spacer->setTrack(staffIdx * VOICES);
                  spacer->setParent(this);
                  if (spacer->spacerType() == SpacerType::FIXED) {
                        qreal gap = spatium() * 10;
                        System* s = system();
                        if (staffIdx == score()->nstaves()-1) {
                              System* ns = 0;
                              for (System* ts : score()->systems()) {
                                    if (ns) {
                                          ns = ts;
                                          break;
                                          }
                                    if (ts  == s)
                                          ns = ts;
                                    }
                              if (ns) {
                                    qreal y1 = s->staffYpage(staffIdx);
                                    qreal y2 = ns->staffYpage(0);
                                    printf("=====%f  %f\n", y1, y2);
                                    gap = y2 - y1 - score()->staff(staffIdx)->height();
                                    }
                              }
                        else {
                              qreal y1 = s->staffYpage(staffIdx);
                              qreal y2 = s->staffYpage(staffIdx+1);
                              gap = y2 - y1 - score()->staff(staffIdx)->height();
                              }
                        spacer->setGap(gap);
                        }
                  score()->undoAddElement(spacer);
                  return spacer;
                  }

            case Element::Type::BAR_LINE:
                  {
                  BarLine* bl = static_cast<BarLine*>(e);

                  // if dropped bar line refers to span rather than to subtype
                  // or if Ctrl key used
                  if ((bl->spanFrom() != 0 && bl->spanTo() != DEFAULT_BARLINE_TO) || data.control()) {
                        // get existing bar line for this staff, and drop the change to it
                        Segment* seg = undoGetSegment(Segment::Type::EndBarLine, tick() + ticks());
                        BarLine* cbl = static_cast<BarLine*>(seg->element(staffIdx * VOICES));
                        if (cbl)
                              cbl->drop(data);
                        }
                  else {
                        // if dropped bar line refers to line subtype
                        score()->undoChangeBarLine(this, bl->barLineType());
                        delete e;
                        }
                  break;
                  }

            case Element::Type::REPEAT_MEASURE:
                  {
                  delete e;
                  return cmdInsertRepeatMeasure(staffIdx);
                  }
            case Element::Type::ICON:
                  switch(static_cast<Icon*>(e)->iconType()) {
                        case IconType::VFRAME:
                              score()->insertMeasure(Element::Type::VBOX, this);
                              break;
                        case IconType::HFRAME:
                              score()->insertMeasure(Element::Type::HBOX, this);
                              break;
                        case IconType::TFRAME:
                              score()->insertMeasure(Element::Type::TBOX, this);
                              break;
                        case IconType::FFRAME:
                              score()->insertMeasure(Element::Type::FBOX, this);
                              break;
                        case IconType::MEASURE:
                              score()->insertMeasure(Element::Type::MEASURE, this);
                              break;
                        default:
                              break;
                        }
                  break;

            default:
                  qDebug("Measure: cannot drop %s here", e->name());
                  delete e;
                  break;
            }
      return 0;
      }

//---------------------------------------------------------
//   cmdInsertRepeatMeasure
//---------------------------------------------------------

RepeatMeasure* Measure::cmdInsertRepeatMeasure(int staffIdx)
      {
      //
      // see also cmdDeleteSelection()
      //
      score()->select(0, SelectType::SINGLE, 0);
      for (Segment* s = first(); s; s = s->next()) {
            if (s->segmentType() & Segment::Type::ChordRest) {
                  int strack = staffIdx * VOICES;
                  int etrack = strack + VOICES;
                  for (int track = strack; track < etrack; ++track) {
                        Element* el = s->element(track);
                        if (el)
                              score()->undoRemoveElement(el);
                        }
                  }
            }
      //
      // add repeat measure
      //
      Segment* seg = undoGetSegment(Segment::Type::ChordRest, tick());
      RepeatMeasure* rm = new RepeatMeasure(score());
      rm->setTrack(staffIdx * VOICES);
      rm->setParent(seg);
      rm->setDurationType(TDuration::DurationType::V_MEASURE);
      rm->setDuration(stretchedLen(score()->staff(staffIdx)));
      score()->undoAddCR(rm, this, tick());
      for (Element* e : el()) {
            if (e->type() == Element::Type::SLUR && e->staffIdx() == staffIdx)
                  score()->undoRemoveElement(e);
            }
      return rm;
      }

//---------------------------------------------------------
//   adjustToLen
//    change actual measure len, adjust elements to
//    new len
//---------------------------------------------------------

void Measure::adjustToLen(Fraction nf)
      {
      int ol   = len().ticks();
      int nl   = nf.ticks();
      int diff = nl - ol;

      int startTick = endTick();
      if (diff < 0)
            startTick += diff;

      score()->undoInsertTime(startTick, diff);
      score()->undo(new InsertTime(score(), startTick, diff));

      for (Score* s : score()->scoreList()) {
            Measure* m = s->tick2measure(tick());
            s->undo(new ChangeMeasureLen(m, nf));
            if (nl > ol) {
                  // move EndBarLine, TimeSigAnnounce, KeySigAnnounce
                  for (Segment* s = m->first(); s; s = s->next()) {
                        if (s->segmentType() & (Segment::Type::EndBarLine|Segment::Type::TimeSigAnnounce|Segment::Type::KeySigAnnounce)) {
                              s->setTick(tick() + nl);
                              }
                        }
                  }
            }
      Score* s      = score()->masterScore();
      Measure* m    = s->tick2measure(tick());
      QList<int> sl = s->uniqueStaves();

      for (int staffIdx : sl) {
            int rests  = 0;
            int chords = 0;
            Rest* rest = 0;
            for (Segment* segment = m->first(); segment; segment = segment->next()) {
                  int strack = staffIdx * VOICES;
                  int etrack = strack + VOICES;
                  for (int track = strack; track < etrack; ++track) {
                        Element* e = segment->element(track);
                        if (e) {
                              if (e->isRest()) {
                                    ++rests;
                                    rest = toRest(e);
                                    }
                              else if (e->isChord())
                                    ++chords;
                              }
                        }
                  }
            // if just a single rest
            if (rests == 1 && chords == 0) {
                  // if measure value didn't change, stick to whole measure rest
                  if (_timesig == nf) {
                        rest->undoChangeProperty(P_ID::DURATION, QVariant::fromValue<Fraction>(nf));
                        rest->undoChangeProperty(P_ID::DURATION_TYPE, QVariant::fromValue<TDuration>(TDuration::DurationType::V_MEASURE));
                        }
                  else {      // if measure value did change, represent with rests actual measure value
                        // convert the measure duration in a list of values (no dots for rests)
                        std::vector<TDuration> durList = toDurationList(nf, false, 0);

                        // set the existing rest to the first value of the duration list
                        for (ScoreElement* e : rest->linkList()) {
                              e->undoChangeProperty(P_ID::DURATION, QVariant::fromValue<Fraction>(durList[0].fraction()));
                              e->undoChangeProperty(P_ID::DURATION_TYPE, QVariant::fromValue<TDuration>(durList[0]));
                              }

                        // add rests for any other duration list value
                        int tickOffset = tick() + durList[0].ticks();
                        for (unsigned i = 1; i < durList.size(); i++) {
                              Rest* newRest = new Rest(s);
                              newRest->setDurationType(durList.at(i));
                              newRest->setDuration(durList.at(i).fraction());
                              newRest->setTrack(rest->track());
                              score()->undoAddCR(newRest, this, tickOffset);
                              tickOffset += durList.at(i).ticks();
                              }
                        }
                  continue;
                  }

            int strack = staffIdx * VOICES;
            int etrack = strack + VOICES;

            for (int trk = strack; trk < etrack; ++trk) {
                  int n = diff;
                  bool rFlag = false;
                  if (n < 0)  {
                        for (Segment* segment = m->last(); segment;) {
                              Segment* pseg = segment->prev();
                              Element* e = segment->element(trk);
                              if (e && e->isChordRest()) {
                                    ChordRest* cr = static_cast<ChordRest*>(e);
                                    if (cr->durationType() == TDuration::DurationType::V_MEASURE) {
                                          int actualTicks = cr->actualTicks();
                                          n += actualTicks;
                                          cr->setDurationType(TDuration(actualTicks));
                                          }
                                    else
                                          n += cr->actualTicks();
                                    s->undoRemoveElement(e);
                                    if (n >= 0)
                                          break;
                                    }
                              segment = pseg;
                              }
                        rFlag = true;
                        }
                  int voice = trk % VOICES;
                  if ((n > 0) && (rFlag || voice == 0)) {
                        // add rest to measure
                        int rtick = tick() + nl - n;
                        int track = staffIdx * VOICES + voice;
                        s->setRest(rtick, track, Fraction::fromTicks(n), false, 0, false);
                        }
                  }
            }
      if (diff < 0) {
            //
            //  CHECK: do not remove all slurs
            //
            foreach(Element* e, m->el()) {
                  if (e->type() == Element::Type::SLUR)
                        s->undoRemoveElement(e);
                  }
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Measure::write(Xml& xml, int staff, bool writeSystemElements) const
      {
      int mno = no() + 1;
      if (_len != _timesig) {
            // this is an irregular measure
            xml.stag(QString("Measure number=\"%1\" len=\"%2/%3\"").arg(mno).arg(_len.numerator()).arg(_len.denominator()));
            }
      else
            xml.stag(QString("Measure number=\"%1\"").arg(mno));

      xml.curTick = tick();

      if (_mmRestCount > 0)
            xml.tag("multiMeasureRest", _mmRestCount);
      if (writeSystemElements) {
            if (repeatStart())
                  xml.tagE("startRepeat");
            if (repeatEnd())
                  xml.tag("endRepeat", _repeatCount);
            writeProperty(xml, P_ID::IRREGULAR);
            writeProperty(xml, P_ID::BREAK_MMR);
            writeProperty(xml, P_ID::USER_STRETCH);
            writeProperty(xml, P_ID::NO_OFFSET);
            writeProperty(xml, P_ID::MEASURE_NUMBER_MODE);
            }
      qreal _spatium = spatium();
      MStaff* mstaff = _mstaves[staff];
      if (mstaff->noText() && !mstaff->noText()->generated()) {
            xml.stag("MeasureNumber");
            mstaff->noText()->writeProperties(xml);
            xml.etag();
            }

      if (mstaff->_vspacerUp)
            xml.tag("vspacerUp", mstaff->_vspacerUp->gap() / _spatium);
      if (mstaff->_vspacerDown) {
            if (mstaff->_vspacerDown->spacerType() == SpacerType::FIXED)
                  xml.tag("vspacerFixed", mstaff->_vspacerDown->gap() / _spatium);
            else
                  xml.tag("vspacerDown", mstaff->_vspacerDown->gap() / _spatium);
            }
      if (!mstaff->_visible)
            xml.tag("visible", mstaff->_visible);
      if (mstaff->_slashStyle)
            xml.tag("slashStyle", mstaff->_slashStyle);

      int strack = staff * VOICES;
      int etrack = strack + VOICES;
      for (const Element* e : el()) {
            if (!e->generated() && ((e->staffIdx() == staff) || (e->systemFlag() && writeSystemElements)))
                  e->write(xml);
            }
      Q_ASSERT(first());
      Q_ASSERT(last());
      score()->writeSegments(xml, strack, etrack, first(), last()->next1(), writeSystemElements, false, false);
      xml.etag();
      }

//---------------------------------------------------------
//   ticks
//---------------------------------------------------------

int Measure::ticks() const
      {
      return _len.ticks();
      }

//---------------------------------------------------------
//   Measure::read
//---------------------------------------------------------

void Measure::read(XmlReader& e, int staffIdx)
      {
      Segment* segment = 0;
      qreal _spatium = spatium();

      QList<Chord*> graceNotes;
      e.tuplets().clear();
      e.setTrack(staffIdx * VOICES);

      for (int n = _mstaves.size(); n <= staffIdx; ++n) {
            Staff* staff = score()->staff(n);
            MStaff* s    = new MStaff;
            s->lines     = new StaffLines(score());
            s->lines->setParent(this);
            s->lines->setTrack(n * VOICES);
            s->lines->setVisible(!staff->invisible());
            _mstaves.push_back(s);
            }

      // tick is obsolete
      if (e.hasAttribute("tick"))
            e.initTick(score()->fileDivision(e.intAttribute("tick")));

      bool irregular;
      if (e.hasAttribute("len")) {
            QStringList sl = e.attribute("len").split('/');
            if (sl.size() == 2)
                  _len = Fraction(sl[0].toInt(), sl[1].toInt());
            else
                  qDebug("illegal measure size <%s>", qPrintable(e.attribute("len")));
            irregular = true;
            score()->sigmap()->add(tick(), SigEvent(_len, _timesig));
            score()->sigmap()->add(tick() + ticks(), SigEvent(_timesig));
            }
      else
            irregular = false;

      Staff* staff = score()->staff(staffIdx);
      Fraction timeStretch(staff->timeStretch(tick()));

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "move")
                  e.initTick(e.readFraction().ticks() + tick());
            else if (tag == "tick") {
                  e.initTick(score()->fileDivision(e.readInt()));
                  }
            else if (tag == "BarLine") {
                  BarLine* barLine = new BarLine(score());
                  barLine->setTrack(e.track());
                  barLine->read(e);

                  //
                  //  StartRepeatBarLine: always at the beginning tick of a measure, always BarLineType::START_REPEAT
                  //  BarLine:            in the middle of a measure, has no semantic
                  //  EndBarLine:         at the end tick of a measure
                  //  BeginBarLine:       first segment of a measure

                  Segment::Type st;
                  if ((e.tick() != tick()) && (e.tick() != endTick()))
                        st = Segment::Type::BarLine;
                  else if (barLine->barLineType() == BarLineType::START_REPEAT && e.tick() == tick())
                        st = Segment::Type::StartRepeatBarLine;
                  else if (e.tick() == tick() && segment == 0)
                        st = Segment::Type::BeginBarLine;
                  else
                        st = Segment::Type::EndBarLine;
                  segment = getSegment(st, e.tick());
                  segment->add(barLine);
                  }
            else if (tag == "Chord") {
                  Chord* chord = new Chord(score());
                  chord->setTrack(e.track());
                  chord->read(e);
                  segment = getSegment(Segment::Type::ChordRest, e.tick());
                  if (chord->noteType() != NoteType::NORMAL)
                        graceNotes.push_back(chord);
                  else {
                        segment->add(chord);
                        for (int i = 0; i < graceNotes.size(); ++i) {
                              Chord* gc = graceNotes[i];
                              gc->setGraceIndex(i);
                              chord->add(gc);
                              }
                        graceNotes.clear();
                        int crticks = chord->actualTicks();
                        e.incTick(crticks);
                        }
                  }
            else if (tag == "Rest") {
                  Rest* rest = new Rest(score());
                  rest->setDurationType(TDuration::DurationType::V_MEASURE);
                  rest->setDuration(timesig()/timeStretch);
                  rest->setTrack(e.track());
                  rest->read(e);
                  segment = getSegment(rest, e.tick());
                  segment->add(rest);

                  if (!rest->duration().isValid())     // hack
                        rest->setDuration(timesig()/timeStretch);

                  e.incTick(rest->actualTicks());
                  }
            else if (tag == "Breath") {
                  Breath* breath = new Breath(score());
                  breath->setTrack(e.track());
                  int tick = e.tick();
                  breath->read(e);
                  segment = getSegment(Segment::Type::Breath, tick);
                  segment->add(breath);
                  }
            else if (tag == "endSpanner") {
                  int id = e.attribute("id").toInt();
                  Spanner* spanner = e.findSpanner(id);
                  if (spanner) {
                        spanner->setTicks(e.tick() - spanner->tick());
                        // if (spanner->track2() == -1)
                              // the absence of a track tag [?] means the
                              // track is the same as the beginning of the slur
                        if (spanner->track2() == -1)
                              spanner->setTrack2(spanner->track() ? spanner->track() : e.track());
                        }
                  else {
                        // remember "endSpanner" values
                        SpannerValues sv;
                        sv.spannerId = id;
                        sv.track2    = e.track();
                        sv.tick2     = e.tick();
                        e.addSpannerValues(sv);
                        }
                  e.readNext();
                  }
            else if (tag == "Slur") {
                  Slur *sl = new Slur(score());
                  sl->setTick(e.tick());
                  sl->read(e);
                  //
                  // check if we already saw "endSpanner"
                  //
                  int id = e.spannerId(sl);
                  const SpannerValues* sv = e.spannerValues(id);
                  if (sv) {
                        sl->setTick2(sv->tick2);
                        sl->setTrack2(sv->track2);
                        }
                  score()->addSpanner(sl);
                  }
            else if (tag == "HairPin"
               || tag == "Pedal"
               || tag == "Ottava"
               || tag == "Trill"
               || tag == "TextLine"
               || tag == "Volta") {
                  Spanner* sp = static_cast<Spanner*>(Element::name2Element(tag, score()));
                  sp->setTrack(e.track());
                  sp->setTick(e.tick());
                  // ?? sp->setAnchor(Spanner::Anchor::SEGMENT);
                  sp->read(e);
                  score()->addSpanner(sp);
                  //
                  // check if we already saw "endSpanner"
                  //
                  int id = e.spannerId(sp);
                  const SpannerValues* sv = e.spannerValues(id);
                  if (sv) {
                        sp->setTicks(sv->tick2 - sp->tick());
                        sp->setTrack2(sv->track2);
                        }
                  }
            else if (tag == "RepeatMeasure") {
                  RepeatMeasure* rm = new RepeatMeasure(score());
                  rm->setTrack(e.track());
                  rm->read(e);
                  segment = getSegment(Segment::Type::ChordRest, e.tick());
                  segment->add(rm);
                  e.incTick(ticks());
                  }
            else if (tag == "Clef") {
                  Clef* clef = new Clef(score());
                  clef->setTrack(e.track());
                  clef->read(e);
                  clef->setGenerated(false);

                  // there may be more than one clef segment for same tick position
                  if (!segment) {
                        // this is the first segment of measure
                        segment = getSegment(Segment::Type::Clef, e.tick());
                        }
                  else {
                        bool firstSegment = false;
                        // the first clef may be missing and is added later in layout
                        for (Segment* s = _segments.first(); s && s->tick() == e.tick(); s = s->next()) {
                              if (s->segmentType() == Segment::Type::Clef
                                    // hack: there may be other segment types which should
                                    // generate a clef at current position
                                 || s->segmentType() == Segment::Type::StartRepeatBarLine
                                 ) {
                                    firstSegment = true;
                                    break;
                                    }
                              }
                        if (firstSegment) {
                              Segment* ns = 0;
                              if (segment->next()) {
                                    ns = segment->next();
                                    while (ns && ns->tick() < e.tick())
                                          ns = ns->next();
                                    }
                              segment = 0;
                              for (Segment* s = ns; s && s->tick() == e.tick(); s = s->next()) {
                                    if (s->segmentType() == Segment::Type::Clef) {
                                          segment = s;
                                          break;
                                          }
                                    }
                              if (!segment) {
                                    segment = new Segment(this, Segment::Type::Clef, e.tick());
                                    _segments.insert(segment, ns);
                                    }
                              }
                        else {
                              // this is the first clef: move to left
                              segment = getSegment(Segment::Type::Clef, e.tick());
                              }
                        }
                  if (e.tick() != tick())
                        clef->setSmall(true);         // layout does this ?
                  segment->add(clef);
                  }
            else if (tag == "TimeSig") {
                  TimeSig* ts = new TimeSig(score());
                  ts->setTrack(e.track());
                  ts->read(e);
                  // if time sig not at begining of measure => courtesy time sig
                  int currTick = e.tick();
                  bool courtesySig = (currTick > tick());
                  if (courtesySig) {
                        // if courtesy sig., just add it without map processing
                        segment = getSegment(Segment::Type::TimeSigAnnounce, currTick);
                        segment->add(ts);
                        }
                  else {
                        // if 'real' time sig., do full process
                        segment = getSegment(Segment::Type::TimeSig, currTick);
                        segment->add(ts);

                        timeStretch = ts->stretch().reduced();
                        _timesig    = ts->sig() / timeStretch;

                        if (irregular) {
                              score()->sigmap()->add(tick(), SigEvent(_len, _timesig));
                              score()->sigmap()->add(tick() + ticks(), SigEvent(_timesig));
                              }
                        else {
                              _len = _timesig;
                              score()->sigmap()->add(tick(), SigEvent(_timesig));
                              }
                        }
                  }
            else if (tag == "KeySig") {
                  KeySig* ks = new KeySig(score());
                  ks->setTrack(e.track());
                  ks->read(e);
                  int curTick = e.tick();
                  if (!ks->isCustom() && !ks->isAtonal() && ks->key() == Key::C && curTick == 0) {
                        // ignore empty key signature
                        qDebug("remove keysig c at tick 0");
                        if (ks->links()) {
                              if (ks->links()->size() == 1)
                                    e.linkIds().remove(ks->links()->lid());
                              }
                        }
                  else {
                        // if key sig not at beginning of measure => courtesy key sig
//                        bool courtesySig = (curTick > tick());
                        bool courtesySig = (curTick == endTick());
                        segment = getSegment(courtesySig ? Segment::Type::KeySigAnnounce : Segment::Type::KeySig, curTick);
                        segment->add(ks);
                        if (!courtesySig)
                              staff->setKey(curTick, ks->keySigEvent());
                        }
                  }
            else if (tag == "Text") {
                  Text* t = new StaffText(score());
                  t->setTrack(e.track());
                  t->read(e);
                  if (t->empty()) {
                        qDebug("reading empty text: deleted");
                        delete t;
                        }
                  else {
                        segment = getSegment(Segment::Type::ChordRest, e.tick());
                        segment->add(t);
                        }
                  }

            //----------------------------------------------------
            // Annotation

            else if (tag == "Dynamic") {
                  Dynamic* dyn = new Dynamic(score());
                  dyn->setTrack(e.track());
                  dyn->read(e);
                  segment = getSegment(Segment::Type::ChordRest, e.tick());
                  segment->add(dyn);
                  }
            else if (tag == "Harmony"
               || tag == "FretDiagram"
               || tag == "TremoloBar"
               || tag == "Symbol"
               || tag == "Tempo"
               || tag == "StaffText"
               || tag == "RehearsalMark"
               || tag == "InstrumentChange"
               || tag == "StaffState"
               || tag == "FiguredBass"
               ) {
                  Element* el = Element::name2Element(tag, score());
                  // hack - needed because tick tags are unreliable in 1.3 scores
                  // for symbols attached to anything but a measure
                  el->setTrack(e.track());
                  el->read(e);
                  segment = getSegment(Segment::Type::ChordRest, e.tick());
                  segment->add(el);
                  }
            else if (tag == "Marker"
               || tag == "Jump"
               ) {
                  Element* el = Element::name2Element(tag, score());
                  el->setTrack(e.track());
                  el->read(e);
                  add(el);
                  }
            else if (tag == "Image") {
                  if (MScore::noImages)
                        e.skipCurrentElement();
                  else {
                        Element* el = Element::name2Element(tag, score());
                        el->setTrack(e.track());
                        el->read(e);
                        segment = getSegment(Segment::Type::ChordRest, e.tick());
                        segment->add(el);
                        }
                  }
            //----------------------------------------------------
            else if (tag == "stretch") {
                  double val = e.readDouble();
                  if (val < 0.0)
                        val = 0;
                  setUserStretch(val);
                  }
            else if (tag == "noOffset")
                  setNoOffset(e.readInt());
            else if (tag == "measureNumberMode")
                  setMeasureNumberMode(MeasureNumberMode(e.readInt()));
            else if (tag == "irregular")
                  setIrregular(e.readBool());
            else if (tag == "breakMultiMeasureRest")
                  _breakMultiMeasureRest = e.readBool();
            else if (tag == "sysInitBarLineType") {
                  const QString& val(e.readElementText());
                  BarLine* barLine = new BarLine(score());
                  barLine->setTrack(e.track());
                  barLine->setBarLineType(val);
                  segment = getSegment(Segment::Type::BeginBarLine, tick());
                  segment->add(barLine);
                  }
            else if (tag == "Tuplet") {
                  Tuplet* tuplet = new Tuplet(score());
                  tuplet->setTrack(e.track());
                  tuplet->setTick(e.tick());
                  tuplet->setParent(this);
                  tuplet->read(e);
                  e.addTuplet(tuplet);
                  }
            else if (tag == "startRepeat") {
                  setRepeatStart(true);
                  e.readNext();
                  }
            else if (tag == "endRepeat") {
                  _repeatCount = e.readInt();
                  setRepeatEnd(true);
                  }
            else if (tag == "vspacer" || tag == "vspacerDown") {
                  if (_mstaves[staffIdx]->_vspacerDown == 0) {
                        Spacer* spacer = new Spacer(score());
                        spacer->setSpacerType(SpacerType::DOWN);
                        spacer->setTrack(staffIdx * VOICES);
                        add(spacer);
                        }
                  _mstaves[staffIdx]->_vspacerDown->setGap(e.readDouble() * _spatium);
                  }
            else if (tag == "vspacerFixed") {
                  if (_mstaves[staffIdx]->_vspacerDown == 0) {
                        Spacer* spacer = new Spacer(score());
                        spacer->setSpacerType(SpacerType::FIXED);
                        spacer->setTrack(staffIdx * VOICES);
                        add(spacer);
                        }
                  _mstaves[staffIdx]->_vspacerDown->setGap(e.readDouble() * _spatium);
                  }
            else if (tag == "vspacer" || tag == "vspacerUp") {
                  if (_mstaves[staffIdx]->_vspacerUp == 0) {
                        Spacer* spacer = new Spacer(score());
                        spacer->setSpacerType(SpacerType::UP);
                        spacer->setTrack(staffIdx * VOICES);
                        add(spacer);
                        }
                  _mstaves[staffIdx]->_vspacerUp->setGap(e.readDouble() * _spatium);
                  }
            else if (tag == "visible")
                  _mstaves[staffIdx]->_visible = e.readInt();
            else if (tag == "slashStyle")
                  _mstaves[staffIdx]->_slashStyle = e.readInt();
            else if (tag == "Beam") {
                  Beam* beam = new Beam(score());
                  beam->setTrack(e.track());
                  beam->read(e);
                  beam->setParent(0);
                  e.addBeam(beam);
                  }
            else if (tag == "Segment")
                  segment->read(e);
            else if (tag == "MeasureNumber") {
                  Text* noText = new Text(score());
                  noText->read(e);
                  noText->setFlag(ElementFlag::ON_STAFF, true);
                  // noText->setFlag(ElementFlag::MOVABLE, false); ??
                  noText->setTrack(e.track());
                  noText->setParent(this);
                  _mstaves[noText->staffIdx()]->setNoText(noText);
                  }
            else if (tag == "SystemDivider") {
                  SystemDivider* sd = new SystemDivider(score());
                  sd->read(e);
                  add(sd);
                  }
            else if (tag == "Ambitus") {
                  Ambitus* range = new Ambitus(score());
                  range->read(e);
                  segment = getSegment(Segment::Type::Ambitus, e.tick());
                  range->setParent(segment);          // a parent segment is needed for setTrack() to work
                  range->setTrack(trackZeroVoice(e.track()));
                  segment->add(range);
                  }
            else if (tag == "multiMeasureRest") {
                  _mmRestCount = e.readInt();
                  // set tick to previous measure
                  setTick(e.lastMeasure()->tick());
                  e.initTick(e.lastMeasure()->tick());
                  }
            else if (MeasureBase::readProperties(e))
                  ;
            else
                  e.unknown();
            }
      e.checkTuplets();
      }

//---------------------------------------------------------
//   checkMeasure
//    after opening / paste and every read operation
//    this method checks for gaps and fills them
//    with invisible rests
//---------------------------------------------------------

void Measure::checkMeasure(int staffIdx)
      {
      score()->staff(staffIdx)->setExcerpt(score()->excerpt());
      int n = VOICES;
      if (score()->staff(staffIdx)->excerpt())
            n = 1;

      if (isMMRest())
            return;

      for (int track = staffIdx * VOICES; (n || track % n) && hasVoice(track); track++) {
            Segment* seg = first();
            if (!seg->element(track))
                  seg = seg->nextCR(track, false);
            if (!seg->element(track))
                  continue;

            Segment* pseg = 0;
            int stick = tick();
            int ticks = seg->tick() - stick;

            while (seg) {
                  // !HACK, it was > 1, but for some tuplets it can happen to have 1 tick difference...
                  // 4 is a 512th...
                  if (ticks > 3) {
                        TDuration d;
                        d.setVal(ticks);
                        if (d.isValid()) {
                              Fraction f = Fraction::fromTicks(ticks);
                              Rest* rest = new Rest(score());
                              rest->setDuration(f);
                              rest->setDurationType(d);
                              rest->setTrack(track);
                              rest->setGap(true);
                              score()->undoAddCR(rest, this, stick);
                              }
                        }

                  pseg = seg;
                  if (!seg->element(track)->isChordRest())
                        break;
                  stick = seg->tick() + toChordRest(seg->element(track))->actualTicks();
                  for (Segment* s = seg->nextCR(track, true); s; s = s->nextCR(track, true)) {
                        if (!s->element(track))
                              continue;
                        if (s->parent() != this) {
                              stick = -1;
                              break;
                              }

                        seg = s;
                        if (seg->tick() == stick) {
                              pseg = seg;
                              stick = seg->tick() + toChordRest(seg->element(track))->actualTicks();
                              continue;
                              }
                        else {
                              ticks = seg->tick() - stick;
                              break;
                              }
                        }

                  if (stick == -1) {
                        break;
                        }
                  //reached last segment in measure
                  if (pseg == seg || stick == tick() + _len.ticks()) {
                        if (stick + ticks < tick() + _len.ticks()) {
                              ticks = tick() + _len.ticks() - stick;
                              if (ticks > 0)
                                    continue;
                              }
                        break;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   visible
//---------------------------------------------------------

bool Measure::visible(int staffIdx) const
      {
      if (staffIdx >= score()->staves().size()) {
            qDebug("Measure::visible: bad staffIdx: %d", staffIdx);
            return false;
            }
      if (system() && (system()->staves()->empty() || !system()->staff(staffIdx)->show()))
            return false;
      if (score()->staff(staffIdx)->cutaway() && isMeasureRest(staffIdx))
            return false;
      return score()->staff(staffIdx)->show() && _mstaves[staffIdx]->_visible;
      }

//---------------------------------------------------------
//   slashStyle
//---------------------------------------------------------

bool Measure::slashStyle(int staffIdx) const
      {
      return score()->staff(staffIdx)->slashStyle() || _mstaves[staffIdx]->_slashStyle || score()->staff(staffIdx)->staffType()->slashStyle();
      }

//---------------------------------------------------------
//   isFinalMeasureOfSection
//    returns true if this measure is final actual measure of a section
//    takes into consideration fact that subsequent measures base objects
//    may have section break before encountering next actual measure
//---------------------------------------------------------

bool Measure::isFinalMeasureOfSection() const
      {
      const MeasureBase* mb = static_cast<const MeasureBase*>(this);

      do {
            if (mb->sectionBreak())
                  return true;

            mb = mb->next();
            } while (mb && !mb->isMeasure());   // loop until reach next actual measure or end of score

      return false;
      }

//---------------------------------------------------------
//   isAnacrusis
//---------------------------------------------------------

bool Measure::isAnacrusis() const
      {
      TimeSigFrac timeSig = score()->sigmap()->timesig(tick()).nominal();
      return irregular() && ticks() < timeSig.ticksPerMeasure();
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Measure::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      MeasureBase::scanElements(data, func, all);

      int nstaves = score()->nstaves();
      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            if (!all && !(visible(staffIdx) && score()->staff(staffIdx)->show()))
                  continue;
            MStaff* ms = _mstaves[staffIdx];
            func(data, ms->lines);
            if (ms->_vspacerUp)
                  func(data, ms->_vspacerUp);
            if (ms->_vspacerDown)
                  func(data, ms->_vspacerDown);
            if (ms->noText())
                  func(data, ms->noText());
            }

      for (Segment* s = first(); s; s = s->next())
            s->scanElements(data, func, all);
      }

//---------------------------------------------------------
//   createVoice
//    Create a voice on demand by filling the measure
//    with a whole measure rest.
//    Check if there are any chord/rests in track; if
//    not create a whole measure rest
//---------------------------------------------------------

void Measure::createVoice(int track)
      {
      for (Segment* s = first(); s; s = s->next()) {
            if (s->segmentType() != Segment::Type::ChordRest)
                  continue;
            if (s->element(track) == 0)
                  score()->setRest(s->tick(), track, len(), true, 0);
            break;
            }
      }

//---------------------------------------------------------
//   setStartRepeatBarLine
//    return true if bar line type changed
//---------------------------------------------------------

void Measure::setStartRepeatBarLine()
      {
      bool val        = repeatStart();
      Segment* s      = findSegment(Segment::Type::StartRepeatBarLine, tick());
      bool customSpan = false;
      int numStaves   = score()->nstaves();

      for (int staffIdx = 0; staffIdx < numStaves;) {
            int track    = staffIdx * VOICES;
            Staff* staff = score()->staff(staffIdx);
            BarLine* bl  = s ? static_cast<BarLine*>(s->element(track)) : 0;
            int span, spanFrom, spanTo;
            // if there is a bar line, take span from it
            if (bl && bl->customSpan()) {     // if there is a bar line and has custom span,
                  span       = bl->span();
                  spanFrom   = bl->spanFrom();
                  spanTo     = bl->spanTo();
                  customSpan = true;
                  }
            else {
                  span       = staff->barLineSpan();
                  spanFrom   = staff->barLineFrom();
                  spanTo     = staff->barLineTo();
                  if (span == 0 && customSpan) {
                        // spanned staves have already been skipped by the loop at the end;
                        // if a staff with span 0 is found and the previous bar line had custom span
                        // this staff shall have an aditional bar line, because the previous staff bar
                        // line has been shortened
                        int staffLines = staff->lines();
                        span     = 1;
                        spanFrom = staffLines == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : 0;
                        spanTo   = staffLines == 1 ? BARLINE_SPAN_1LINESTAFF_TO   : (staffLines-1) * 2;
                        }
                  }
            // make sure we do not span more staves than actually exist
            if (staffIdx + span > numStaves)
                  span = numStaves - staffIdx;

            if (span && val && !bl) {
                  // no barline were we need one:
                  if (s == 0)
                        s = undoGetSegment(Segment::Type::StartRepeatBarLine, tick());
                  bl = new BarLine(score());
                  bl->setBarLineType(BarLineType::START_REPEAT);
                  bl->setGenerated(true);
                  bl->setTrack(track);
                  bl->setParent(s);
                  score()->undoAddElement(bl);
                  }
            else if (bl && !val) {
                  score()->undoRemoveElement(bl);     // barline were we do not need one
                  }
            if (bl && val && span) {
                  bl->setSpan(span);
                  bl->setSpanFrom(spanFrom);
                  bl->setSpanTo(spanTo);
                  }

            ++staffIdx;
            //
            // remove any unwanted barlines:
            //
            // if spanning several staves but not entering INTO last staff,
            if (span > 1 && spanTo <= 0)
                  span--;                 // count one span less
            if (s) {
                  for (int i = 1; i < span; ++i) {
                        Element* e  = s->element(staffIdx * VOICES);
                        if (e)
                              score()->undoRemoveElement(toBarLine(e));
                        ++staffIdx;
                        }
                  }
            }
      if (s)
            s->createShapes();
      }

//---------------------------------------------------------
//   setEndBarLineType
//     Create a *generated* barline with the given type and
//     properties if none exists. Modify if it exists.
//     Useful for import filters.
//---------------------------------------------------------

void Measure::setEndBarLineType(BarLineType val, int track, bool visible, QColor color)
      {
      Segment* seg = undoGetSegment(Segment::Type::EndBarLine, endTick());
      // get existing bar line for this staff, if any
      BarLine* bl = toBarLine(seg->element(track));
      if (!bl) {
            // no suitable bar line: create a new one
            bl = new BarLine(score());
            bl->setParent(seg);
            bl->setTrack(track);
            score()->addElement(bl);
            }
      bl->setGenerated(false);
      bl->setBarLineType(val);
      bl->setVisible(visible);
      if (color.isValid())
            bl->setColor(color);
      else
            bl->setColor(curColor());
      }

//---------------------------------------------------------
//   createEndBarLines
//    actually creates or modifies barlines
//    return the width change for measure
//---------------------------------------------------------

qreal Measure::createEndBarLines(bool isLastMeasureInSystem)
      {
      int nstaves  = score()->nstaves();
      Segment* seg = undoGetSegment(Segment::Type::EndBarLine, endTick());
      BarLine* bl  = 0;
      int span     = 0;        // span counter
      int aspan    = 0;        // actual span
      bool mensur  = false;    // keep note of Mensurstrich case

      int spanTot;             // to keep track of the target span as we count down
      int lastIdx;
      int spanFrom = 0;
      int spanTo = 0;
      static const int unknownSpanFrom = 9999;

      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            Staff* staff   = score()->staff(staffIdx);
            int track      = staffIdx * VOICES;
            int staffLines = staff->lines();

            // get existing bar line for this staff, if any
            BarLine* cbl = toBarLine(seg->element(track));

            // if span counter has been counted off, get new span values
            // and forget about any previous bar line

            if (span == 0) {
                  if (cbl && cbl->customSpan()) {     // if there is a bar line and has custom span,
                        span     = cbl->span();       // get span values from it
                        spanFrom = cbl->spanFrom();
                        spanTo   = cbl->spanTo();
                        }
                  else {                              // otherwise, get from staff
                        span = staff->barLineSpan();
                        // if some span OR last staff (span==0) of a Mensurstrich case, get From/To from staff
                        if (span || mensur) {
                              spanFrom = staff->barLineFrom();
                              spanTo   = staff->barLineTo();
                              mensur   = false;
                              }
                        // but if staff is set to no span, a multi-staff spanning bar line
                        // has been shortened to span less staves and following staves left without bars;
                        // set bar line span values to default

                        else if (staff->show()) {
                              span        = 1;
                              spanFrom    = staffLines == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : 0;
                              spanTo      = staffLines == 1 ? BARLINE_SPAN_1LINESTAFF_TO : (staff->lines() - 1) * 2;
                              }
                        }
                  if (!staff->show()) {
                        // this staff is not visible
                        // we should recalculate spanFrom when we find a valid staff
                        spanFrom = unknownSpanFrom;
                        }
                  if ((staffIdx + span) > nstaves)    // sanity check, don't span more than available staves
                        span = nstaves - staffIdx;
                  spanTot = span;
                  lastIdx = staffIdx + span - 1;
                  bl      = nullptr;
                  }
            else if (spanFrom == unknownSpanFrom && staff->show()) {
                  // we started a span earlier, but had not found a valid staff yet
                  spanFrom = staffLines == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : 0;
                  }
            if (staff->show() && span) {
                  //
                  // there should be a barline in this staff
                  // this is true even for a staff not shown because of hide empty staves
                  // but not for a staff not shown because it is made invisible
                  //
                  // if we already have a bar line, keep extending this bar line down until span exhausted;
                  // if no barline yet, re-use the bar line existing in this staff if any,
                  // restarting actual span

                  if (!bl) {
                        bl    = cbl;
                        aspan = 0;
                        }
                  if (!bl) {
                        // no suitable bar line: create a new one
                        bl = new BarLine(score());
                        bl->setParent(seg);
                        bl->setTrack(track);
                        bl->setGenerated(true);
                        score()->addElement(bl);
                        }
                  else {
                        // if a bar line exists for this staff (cbl) but
                        // it is not the bar line we are dealing with (bl),
                        // we are extending down the bar line of a staff above (bl)
                        // and the bar line for this staff (cbl) is not needed:
                        // DELETE it

                        if (cbl && cbl != bl) {

                              // Mensurstrich special case:
                              // if span arrives inside the end staff (spanTo>0) OR
                              //          span is not multi-staff (spanTot<=1) OR
                              //          current staff is not the last spanned staff (span!=1) OR
                              //          staff is the last score staff
                              //    remove bar line for this staff
                              // If NONE of the above conditions holds, the staff is the last staff of
                              // a Mensurstrich(-like) span: keep its bar line, as it may span to next staff

                              if (spanTo > 0 || spanTot <= 1 || span != 1 || staffIdx == nstaves-1)
                                    score()->undoRemoveElement(cbl);
                              }
                        }
                  }
            else {
                  //
                  // there should be no barline in this staff
                  //
                  if (cbl)
                        score()->undoRemoveElement(cbl);
                  }

            // if span not counted off AND we have a bar line AND this staff is shown,
            // set bar line span values (this may result in extending down a bar line
            // for a previous staff, if we are counting off a span > 1)

            if (span) {
                  if (bl) {
                        ++aspan;
                        if (staff->show()) {          // count visible staves only (whether hidden or not)
                              bl->setSpan(aspan);     // need to update span & spanFrom even for hidden staves
                              bl->setSpanFrom(spanFrom);
                              // if current actual span < target span, set spanTo to full staff height
                              if (aspan < spanTot && staffIdx < lastIdx)
                                    bl->setSpanTo(staffLines == 1 ? BARLINE_SPAN_1LINESTAFF_TO : (staffLines - 1) * 2);
                              // if we reached target span, set spanTo to intended value
                              else
                                    bl->setSpanTo(spanTo);
                              }
                        }
                  --span;
                  }
            // if just finished (span==0) a multi-staff span (spanTot>1) ending at the top of a staff (spanTo<=0)
            // scan this staff again, as it may have its own bar lines (Mensurstrich(-like) span)
            if (spanTot > 1 && spanTo <= 0 && span == 0) {
                  mensur = true;
                  staffIdx--;
                  }
            }

      //
      //  Set flag "hasCourtesyKeySig" if this measure needs a courtesy key sig.
      //  This flag is later used to set a double end bar line and to actually
      //  create the courtesy key sig.
      //

      Measure* nm   = nextMeasure();
      BarLineType t = nm ? BarLineType::NORMAL : BarLineType::END;
      bool show     = score()->styleB(StyleIdx::genCourtesyKeysig) && !sectionBreak() && nm;

      setHasCourtesyKeySig(false);

      if (isLastMeasureInSystem && show) {
            int tick = endTick();
            for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                  Staff* staff     = score()->staff(staffIdx);
                  KeySigEvent key1 = staff->keySigEvent(tick - 1);
                  KeySigEvent key2 = staff->keySigEvent(tick);
                  if (!(key1 == key2)) {
                        // locate a key sig. in next measure and, if found,
                        // check if it has court. sig turned off
                        Segment* s = nm->findSegment(Segment::Type::KeySig, tick);
                        if (s) {
                              KeySig* ks = toKeySig(s->element(staffIdx * VOICES));
                              if (ks && !ks->showCourtesy())
                                    continue;
                              }
                        setHasCourtesyKeySig(true);
                        t = BarLineType::DOUBLE;
                        break;
                        }
                  }
            }

      bool force = false;
      if (!isLastMeasureInSystem && repeatEnd() && nextMeasure()->repeatStart()) {
            t = BarLineType::END_START_REPEAT;
            force = true;
            }
      else if (repeatEnd()) {
            t = BarLineType::END_REPEAT;
            force = true;
            }
      else if (!isLastMeasureInSystem && nextMeasure() && nextMeasure()->repeatStart()) {
            t = BarLineType::START_REPEAT;
            force = true;
            }

      qreal w = 0.0;
      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            BarLine* bl = static_cast<BarLine*>(seg->element(staffIdx * VOICES));
            if (bl) {
                  // do not change bar line type if bar line is user modified
                  // and its not a repeat start/end barline

                  if (bl->generated())
                        bl->setBarLineType(t);
                  else {
                        if (force) {
                              score()->undoChangeProperty(bl, P_ID::BARLINE_TYPE, QVariant::fromValue(t));
                              bl->setGenerated(true);
                              }
                        }
                  bl->layout();
                  w = bl->width();
                  }
            }
      seg->setWidth(w);
      seg->createShapes();

      // fix previous segment width
      Segment* ps = seg->prev();
      qreal www   = ps->minHorizontalDistance(seg, false);
      w          += www - ps->width();
      ps->setWidth(www);

      setWidth(width() + w);

      return w;
      }

//---------------------------------------------------------
//   sortStaves
//---------------------------------------------------------

void Measure::sortStaves(QList<int>& dst)
      {
      std::vector<MStaff*> ms;
      for (int idx : dst)
            ms.push_back(_mstaves[idx]);
      _mstaves = ms;

      for (unsigned staffIdx = 0; staffIdx < _mstaves.size(); ++staffIdx)
            _mstaves[staffIdx]->lines->setTrack(staffIdx * VOICES);
      for (Segment& s : _segments)
            s.sortStaves(dst);

      for (Element* e : el()) {
            if (e->track() == -1 || e->systemFlag())
                  continue;
            int voice    = e->voice();
            int staffIdx = e->staffIdx();
            int idx = dst.indexOf(staffIdx);
            e->setTrack(idx * VOICES + voice);
            }
      }

//---------------------------------------------------------
//   exchangeVoice
//---------------------------------------------------------

void Measure::exchangeVoice(int strack, int dtrack, int staffIdx)
      {

      for (Segment* s = first(Segment::Type::ChordRest); s; s = s->next(Segment::Type::ChordRest)) {
            s->swapElements(strack, dtrack);
            }

      auto spanners = score()->spannerMap().findOverlapping(tick(), endTick()-1);
      int start = tick();
      int end = start + ticks();
      for (auto i = spanners.begin(); i < spanners.end(); i++) {
            Spanner* sp = i->value;
            int spStart = sp->tick();
            int spEnd = spStart + sp->ticks();
            qDebug("Start %d End %d Diff %d \n Measure Start %d End %d", spStart, spEnd, spEnd-spStart, start, end);
            if (sp->type() == Element::Type::SLUR && (spStart >= start || spEnd < end)) {
                if (sp->track() == strack && spStart >= start){
                        sp->setTrack(dtrack);
                        }
                else if (sp->track() == dtrack && spStart >= start){
                        sp->setTrack(strack);
                        }
                if (sp->track2() == strack && spEnd < end){
                        sp->setTrack2(dtrack);
                        }
                else if (sp->track2() == dtrack && spEnd < end){
                        sp->setTrack2(strack);
                        }
                  }
            }
      // MStaff* ms = mstaff(staffIdx);
      // ms->hasVoices = true;
      checkMultiVoices(staffIdx);   // probably true, but check for invisible notes & rests
      }

//---------------------------------------------------------
//   checkMultiVoices
///   Check for more than on voice in this measure and staff and
///   set MStaff->hasVoices
//---------------------------------------------------------

void Measure::checkMultiVoices(int staffIdx)
      {
      int strack = staffIdx * VOICES + 1;
      int etrack = staffIdx * VOICES + VOICES;
      _mstaves[staffIdx]->hasVoices = false;

      for (Segment* s = first(Segment::Type::ChordRest); s; s = s->next(Segment::Type::ChordRest)) {
            for (int track = strack; track < etrack; ++track) {
                  Element* e = s->element(track);
                  if (e) {
                        bool v;
                        if (e->isChord()) {
                              v = false;
                              // consider chord visible if any note is visible
                              Chord* c = toChord(e);
                              for (Note* n : c->notes()) {
                                    if (n->visible()) {
                                          v = true;
                                          break;
                                          }
                                    }
                              }
                        else
                              v = e->visible();
                        if (v) {
                              _mstaves[staffIdx]->hasVoices = true;
                              return;
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   hasVoice
//---------------------------------------------------------

bool Measure::hasVoice(int track) const
      {
      if (track >= int(mstaves().size() * VOICES))
            return false;
      for (Segment* s = first(); s; s = s->next()) {
            if (s->segmentType() != Segment::Type::ChordRest)
                  continue;
            if (s->element(track))
                  return true;
            }
      return false;
      }

//-------------------------------------------------------------------
//   isMeasureRest
///   Check if the measure is filled by a full-measure rest or full
///   of rests on this staff. If staff is -1, then check for
///   all staves.
//-------------------------------------------------------------------

bool Measure::isMeasureRest(int staffIdx) const
      {
      int strack;
      int etrack;
      if (staffIdx < 0) {
            strack = 0;
            etrack = score()->nstaves() * VOICES;
            }
      else {
            strack = staffIdx * VOICES;
            etrack = strack + VOICES;
            }
      for (Segment* s = first(Segment::Type::ChordRest); s; s = s->next(Segment::Type::ChordRest)) {
            for (int track = strack; track < etrack; ++track) {
                  Element* e = s->element(track);
                  if (e && e->type() != Element::Type::REST)
                        return false;
                  }
            for (Element* a : s->annotations()) {
                  if (!a || a->systemFlag())
                        continue;
                  int atrack = a->track();
                  if (atrack >= strack && atrack < etrack)
                        return false;
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   isFullMeasureRest
//    Check for an empty measure, filled with full measure
//    rests.
//---------------------------------------------------------

bool Measure::isFullMeasureRest() const
      {
      int strack = 0;
      int etrack = score()->nstaves() * VOICES;

      Segment* s = first(Segment::Type::ChordRest);
      for (int track = strack; track < etrack; ++track) {
            Element* e = s->element(track);
            if (e) {
                  if (e->type() != Element::Type::REST)
                        return false;
                  Rest* rest = static_cast<Rest*>(e);
                  if (rest->durationType().type() != TDuration::DurationType::V_MEASURE)
                        return false;
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   isRepeatMeasure
//---------------------------------------------------------

bool Measure::isRepeatMeasure(Staff* staff) const
      {
      int staffIdx = staff->idx();
      int strack   = staffIdx * VOICES;
      int etrack   = (staffIdx + 1) * VOICES;
      Segment* s   = first(Segment::Type::ChordRest);

      if (s == 0)
            return false;

      for (int track = strack; track < etrack; ++track) {
            Element* e = s->element(track);
            if (e && e->type() == Element::Type::REPEAT_MEASURE)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   isEmpty
//---------------------------------------------------------

bool Measure::empty() const
      {
      if (irregular())
            return false;
      int n = 0;
      int tracks = _mstaves.size() * VOICES;
      static const Segment::Type st = Segment::Type::ChordRest ;
      for (const Segment* s = first(st); s; s = s->next(st)) {
            bool restFound = false;
            for (int track = 0; track < tracks; ++track) {
                  if ((track % VOICES) == 0 && !score()->staff(track/VOICES)->show()) {
                        track += VOICES-1;
                        continue;
                        }
                  if (s->element(track))  {
                        if (s->element(track)->type() != Element::Type::REST)
                              return false;
                        restFound = true;
                        }
                  }
            if (restFound)
                  ++n;
            // measure is not empty if there is more than one rest
            if (n > 1)
                  return false;
            }
      return true;
      }

//---------------------------------------------------------
//   isOnlyRests
//---------------------------------------------------------

bool Measure::isOnlyRests(int track) const
      {
      static const Segment::Type st = Segment::Type::ChordRest;
      for (const Segment* s = first(st); s; s = s->next(st)) {
            if (s->segmentType() != st || !s->element(track))
                  continue;
            if (s->element(track)->type() != Element::Type::REST)
                  return false;
            }
      return true;
      }

//---------------------------------------------------------
//   isOnlyDeletedRests
//---------------------------------------------------------

bool Measure::isOnlyDeletedRests(int track) const
      {
      static const Segment::Type st { Segment::Type::ChordRest };
      for (const Segment* s = first(st); s; s = s->next(st)) {
            if (s->segmentType() != st || !s->element(track))
                  continue;
            if (s->element(track)->isRest() ? !toRest(s->element(track))->isGap() : !s->element(track)->isRest())
                  return false;
            }
      return true;
      }

//---------------------------------------------------------
//   minWidth1
///   return minimum width of measure excluding system
///   header
//---------------------------------------------------------

qreal Measure::minWidth1() const
      {
      int nstaves = score()->nstaves();
      Segment::Type st = Segment::Type::Clef | Segment::Type::KeySig
         | Segment::Type::StartRepeatBarLine | Segment::Type::BeginBarLine;

      Segment* s = first();
      while ((s->segmentType() & st) && s->next()) {
            // found a segment that we might be able to skip
            // we can do so only if it contains no non-generated elements
            // note that it is possible for the same segment to contain both generated and non-generated elements
            // consider, a keysig segment at the start of a system in which one staff has a local key change
            bool generated = true;
            for (int i = 0; i < nstaves; ++i) {
                  Element* e = s->element(i * VOICES);
                  if (e && !e->generated()) {
                        generated = false;
                        break;
                        }
                  }
            if (!generated)
                  break;
            s = s->next();
            }
      return score()->computeMinWidth(s, false);
      }

//---------------------------------------------------------
//   stretchedLen
//---------------------------------------------------------

Fraction Measure::stretchedLen(Staff* staff) const
      {
      return len() * staff->timeStretch(tick());
      }

//---------------------------------------------------------
//   cloneMeasure
//---------------------------------------------------------

Measure* Measure::cloneMeasure(Score* sc, TieMap* tieMap)
      {
      Measure* m      = new Measure(sc);
      m->_timesig     = _timesig;
      m->_len         = _len;
      m->_repeatCount = _repeatCount;

      for (MStaff* ms : _mstaves)
            m->_mstaves.push_back(new MStaff(*ms));

      m->setNo(no());
      m->setNoOffset(noOffset());
      m->setIrregular(irregular());
      m->_userStretch           = _userStretch;
      m->_breakMultiMeasureRest = _breakMultiMeasureRest;
      m->_playbackCount         = _playbackCount;

      m->setTick(tick());
      m->setLineBreak(lineBreak());
      m->setPageBreak(pageBreak());
      m->setSectionBreak(sectionBreak() ? new LayoutBreak(*sectionBreakElement()) : 0);

      int tracks = sc->nstaves() * VOICES;
      TupletMap tupletMap;

      for (Segment* oseg = first(); oseg; oseg = oseg->next()) {
            Segment* s = new Segment(m);
            s->setSegmentType(oseg->segmentType());
            s->setRtick(oseg->rtick());
            m->_segments.push_back(s);
            for (int track = 0; track < tracks; ++track) {
                  Element* oe = oseg->element(track);
                  foreach (Element* e, oseg->annotations()) {
                        if (e->generated() || e->track() != track)
                              continue;
                        Element* ne = e->clone();
                        ne->setTrack(track);
                        ne->setUserOff(e->userOff());
                        ne->setScore(sc);
                        s->add(ne);
                        }
                  if (!oe)
                        continue;
                  Element* ne = oe->clone();
                  if (oe->isChordRest()) {
                        ChordRest* ocr = static_cast<ChordRest*>(oe);
                        ChordRest* ncr = static_cast<ChordRest*>(ne);
                        Tuplet* ot     = ocr->tuplet();
                        if (ot) {
                              Tuplet* nt = tupletMap.findNew(ot);
                              if (nt == 0) {
                                    nt = new Tuplet(*ot);
                                    nt->clear();
                                    nt->setTrack(track);
                                    nt->setScore(sc);
                                    tupletMap.add(ot, nt);
                                    }
                              ncr->setTuplet(nt);
                              nt->add(ncr);
                              }
                        if (oe->type() == Element::Type::CHORD) {
                              Chord* och = static_cast<Chord*>(ocr);
                              Chord* nch = static_cast<Chord*>(ncr);
                              int n = och->notes().size();
                              for (int i = 0; i < n; ++i) {
                                    Note* on = och->notes().at(i);
                                    Note* nn = nch->notes().at(i);
                                    if (on->tieFor()) {
                                          Tie* tie = on->tieFor()->clone();
                                          tie->setScore(sc);
                                          nn->setTieFor(tie);
                                          tie->setStartNote(nn);
                                          tieMap->add(on->tieFor(), tie);
                                          }
                                    if (on->tieBack()) {
                                          Tie* tie = tieMap->findNew(on->tieBack());
                                          if (tie) {
                                                nn->setTieBack(tie);
                                                tie->setEndNote(nn);
                                                }
                                          else {
                                                qDebug("cloneMeasure: cannot find tie, track %d", track);
                                                }
                                          }
                                    }
                              }
                        }
                  ne->setUserOff(oe->userOff());
                  ne->setScore(sc);
                  s->add(ne);
                  }
            }
      foreach(Element* e, el()) {
            Element* ne = e->clone();
            ne->setScore(sc);
            ne->setUserOff(e->userOff());
            m->add(ne);
            }
      return m;
      }

//---------------------------------------------------------
//   pos2sel
//---------------------------------------------------------

int Measure::snap(int tick, const QPointF p) const
      {
      Segment* s = first();
      for (; s->next(); s = s->next()) {
            qreal x  = s->x();
            qreal dx = s->next()->x() - x;
            if (s->tick() == tick)
                  x += dx / 3.0 * 2.0;
            else  if (s->next()->tick() == tick)
                  x += dx / 3.0;
            else
                  x += dx * .5;
            if (p.x() < x)
                  break;
            }
      return s->tick();
      }

//---------------------------------------------------------
//   snapNote
//---------------------------------------------------------

int Measure::snapNote(int /*tick*/, const QPointF p, int staff) const
      {
      Segment* s = first();
      for (;;) {
            Segment* ns = s->next();
            while (ns && ns->element(staff) == 0)
                  ns = ns->next();
            if (ns == 0)
                  break;
            qreal x  = s->x();
            qreal nx = x + (ns->x() - x) * .5;
            if (p.x() < nx)
                  break;
            s = ns;
            }
      return s->tick();
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Measure::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::TIMESIG_NOMINAL:
                  return QVariant::fromValue(_timesig);
            case P_ID::TIMESIG_ACTUAL:
                  return QVariant::fromValue(_len);
            case P_ID::MEASURE_NUMBER_MODE:
                  return int(measureNumberMode());
            case P_ID::BREAK_MMR:
                  return breakMultiMeasureRest();
            case P_ID::REPEAT_COUNT:
                  return repeatCount();
            case P_ID::USER_STRETCH:
                  return userStretch();
            case P_ID::NO_OFFSET:
                  return noOffset();
            case P_ID::IRREGULAR:
                  return irregular();
            default:
                  return MeasureBase::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Measure::setProperty(P_ID propertyId, const QVariant& value)
      {
      switch(propertyId) {
            case P_ID::TIMESIG_NOMINAL:
                  _timesig = value.value<Fraction>();
                  break;
            case P_ID::TIMESIG_ACTUAL:
                  _len = value.value<Fraction>();
                  break;
            case P_ID::MEASURE_NUMBER_MODE:
                  setMeasureNumberMode(MeasureNumberMode(value.toInt()));
                  break;
            case P_ID::BREAK_MMR:
                  setBreakMultiMeasureRest(value.toBool());
                  break;
            case P_ID::REPEAT_COUNT:
                  setRepeatCount(value.toInt());
                  break;
            case P_ID::USER_STRETCH:
                  setUserStretch(value.toDouble());
                  break;
            case P_ID::NO_OFFSET:
                  setNoOffset(value.toInt());
                  break;
            case P_ID::IRREGULAR:
                  setIrregular(value.toBool());
                  break;
            default:
                  return MeasureBase::setProperty(propertyId, value);
            }
      score()->setLayoutAll();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Measure::propertyDefault(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::TIMESIG_NOMINAL:
            case P_ID::TIMESIG_ACTUAL:
                  return QVariant();
            case P_ID::MEASURE_NUMBER_MODE:
                  return int(MeasureNumberMode::AUTO);
            case P_ID::BREAK_MMR:
                  return false;
            case P_ID::REPEAT_COUNT:
                  return 2;
            case P_ID::USER_STRETCH:
                  return 1.0;
            case P_ID::NO_OFFSET:
                  return 0;
            case P_ID::IRREGULAR:
                  return false;
            default:
                  break;
            }
      return MeasureBase::propertyDefault(propertyId);
      }

//-------------------------------------------------------------------
//   mmRestFirst
//    this is a multi measure rest
//    returns first measure of replaced sequence of empty measures
//-------------------------------------------------------------------

Measure* Measure::mmRestFirst() const
      {
      Q_ASSERT(isMMRest());
      if (prev())
            return static_cast<Measure*>(prev()->next());
      return score()->firstMeasure();
      }

//-------------------------------------------------------------------
//   mmRestLast
//    this is a multi measure rest
//    returns last measure of replaced sequence of empty measures
//-------------------------------------------------------------------

Measure* Measure::mmRestLast() const
      {
      Q_ASSERT(isMMRest());
      if (next())
            return static_cast<Measure*>(next()->prev());
      return score()->lastMeasure();
      }

//---------------------------------------------------------
//   mmRest1
//    return the multi measure rest this measure is covered
//    by
//---------------------------------------------------------

const Measure* Measure::mmRest1() const
      {
      if (_mmRest)
            return _mmRest;
      if (_mmRestCount != -1)
            // return const_cast<Measure*>(this);
            return this;
      const Measure* m = this;
      while (m && !m->_mmRest)
            m = m->prevMeasure();
      if (m)
            return const_cast<Measure*>(m->_mmRest);
      return 0;
      }

//-------------------------------------------------------------------
//   userStretch
//-------------------------------------------------------------------

qreal Measure::userStretch() const
      {
      return (score()->layoutMode() == LayoutMode::FLOAT ? 1.0 : _userStretch);
      }

//---------------------------------------------------------
//   nextElementStaff
//---------------------------------------------------------

Element* Measure::nextElementStaff(int staff)
      {
      Segment* firstSeg = segments().first();
      if (firstSeg)
            return firstSeg->firstElement(staff);
      return score()->firstElement();
      }

//---------------------------------------------------------
//   prevElementStaff
//---------------------------------------------------------

Element* Measure::prevElementStaff(int staff)
      {
      Measure* prevM = prevMeasureMM();
      if (prevM) {
            Segment* seg = prevM->last();
            if (seg)
                  seg->lastElement(staff);
            }
      return score()->lastElement();
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Measure::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(QString::number(no() + 1));
      }

//-----------------------------------------------------------------------------
//    stretchMeasure
//    resize width of measure to targetWidth
//-----------------------------------------------------------------------------

void Measure::stretchMeasure(qreal targetWidth)
      {
      bbox().setWidth(targetWidth);

      //---------------------------------------------------
      //    compute minTick and set ticks for all segments
      //---------------------------------------------------

      int minTick = ticks();
      if (minTick <= 0) {
            qDebug("=====minTick %d measure %p", minTick, this);
            }
      Q_ASSERT(minTick > 0);

      Segment* ns = first();
      while (ns) {
            Segment* s = ns;
            ns         = s->next();
            int nticks = (ns ? ns->rtick() : ticks()) - s->rtick();
            if (nticks) {
                  if (nticks < minTick)
                        minTick = nticks;
                  }
            s->setTicks(nticks);
            }

      //---------------------------------------------------
      //    compute stretch
      //---------------------------------------------------

      std::multimap<qreal, Segment*> springs;

      qreal minimumWidth = first()->pos().x();
      for (Segment& s : _segments) {
            int t = s.ticks();
            if (t) {
                  qreal str = 1.0 + 0.865617 * log(qreal(t) / qreal(minTick)); // .6 * log(t / minTick) / log(2);
                  qreal d   = s.width() / str;
                  s.setStretch(str);
                  springs.insert(std::pair<qreal, Segment*>(d, &s));
                  }
            minimumWidth += s.width();
            }

      //---------------------------------------------------
      //    compute 1/Force for a given Extend
      //---------------------------------------------------

      if (targetWidth > minimumWidth) {
            qreal force = 0;
            qreal c     = 0.0;
            for (auto i = springs.begin();;) {
                  c            += i->second->stretch();
                  minimumWidth -= i->second->width();
                  qreal f       = (targetWidth - minimumWidth) / c;
                  ++i;
                  if (i == springs.end() || f <= i->first) {
                        force = f;
                        break;
                        }
                  }

            //---------------------------------------------------
            //    distribute stretch to segments
            //---------------------------------------------------

            for (auto& i : springs) {
                  qreal width = force * i.second->stretch();
                  if (width > i.second->width())
                        i.second->setWidth(width);
                  }

            //---------------------------------------------------
            //    move segments to final position
            //---------------------------------------------------

            qreal x = first()->pos().x();
            for (Segment& s : _segments) {
                  s.rxpos() = x;
                  x += s.width();
                  }
            }

      //---------------------------------------------------
      //    layout individual elements
      //---------------------------------------------------

      for (Segment& s : _segments) {
            for (Element* e : s.elist()) {
                  if (!e)
                        continue;
                  Element::Type t = e->type();
                  int staffIdx    = e->staffIdx();
                  if (t == Element::Type::REPEAT_MEASURE || (t == Element::Type::REST && (isMMRest() || toRest(e)->isFullMeasureRest()))) {
                        //
                        // element has to be centered in free space
                        //    x1 - left measure position of free space
                        //    x2 - right measure position of free space

                        Segment* s1 = s.prev() ? s.prev() : 0;
                        Segment* s2;
                        for (s2 = s.next(); s2; s2 = s2->next()) {
                              if (!s2->isChordRestType())
                                    break;
                              }
                        qreal x1 = s1 ? s1->x() + s1->minRight() : 0;
                        qreal x2 = s2 ? s2->x() - s2->minLeft() : targetWidth;

                        if (isMMRest()) {
                              Rest* rest = toRest(e);
                              //
                              // center multi measure rest
                              //
                              qreal d  = point(score()->styleS(StyleIdx::multiMeasureRestMargin));
                              qreal w = x2 - x1 - 2 * d;

                              rest->setMMWidth(w);
                              qreal x = x1 - s.x() + d;
                              e->setPos(x, e->staff()->height() * .5);   // center vertically in measure
                              rest->layout();
                              s.createShape(staffIdx);
                              }
                        else { // if (rest->isFullMeasureRest()) {
                              //
                              // center full measure rest
                              //
                              e->rxpos() = (x2 - x1 - e->width()) * .5 + x1 - s.x() - e->bbox().x();
                              e->adjustReadPos();
                              s.createShape(staffIdx);  // DEBUG
                              }
                        }
                  else if (t == Element::Type::REST)
                        e->rxpos() = 0;
                  else if (t == Element::Type::CHORD) {
                        Chord* c = toChord(e);
                        c->layout2();
                        if (c->tremolo())
                              c->tremolo()->layout();
                        }
                  else if (t == Element::Type::BAR_LINE) {
                        e->setPos(QPointF());
                        e->adjustReadPos();
                        }
                  else
                        e->adjustReadPos();
                  }
            }
      }

//---------------------------------------------------------
//   removeSystemHeader
//---------------------------------------------------------

void Measure::removeSystemHeader()
      {
      for (Segment* seg = first(); seg; seg = seg->next()) {
            Segment::Type st = seg->segmentType();
            if (st & (Segment::Type::BeginBarLine | Segment::Type::StartRepeatBarLine))
                  score()->undoRemoveElement(seg);
            else if (st == Segment::Type::ChordRest)
                  break;
            else if (st & (Segment::Type::Clef | Segment::Type::KeySig)) {
                  for (int staffIdx = 0;  staffIdx < score()->nstaves(); ++staffIdx) {
                        Element* el = seg->element(staffIdx * VOICES);
                        // remove Clefs and Keysigs if generated
                        if (el && el->generated())
                              score()->undoRemoveElement(el);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   removeSystemTrailer
//---------------------------------------------------------

void Measure::removeSystemTrailer()
      {
      for (Segment* seg = last(); seg != first(); seg = seg->prev()) {
            Segment::Type st = seg->segmentType();
            if (st == Segment::Type::ChordRest)
                  break;
            else if (st & (Segment::Type::TimeSigAnnounce | Segment::Type::KeySigAnnounce)) {
                  for (int staffIdx = 0;  staffIdx < score()->nstaves(); ++staffIdx) {
                        Element* el = seg->element(staffIdx * VOICES);
                        // courtesy time sigs and key sigs: remove if not in last measure
                        // (generated or not!)
                        if (el)
                              score()->undoRemoveElement(el);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   endBarLine
//      return the first one
//---------------------------------------------------------

const BarLine* Measure::endBarLine() const
      {
      // search barline segment:
      Segment* s = last();
      while (s && s->segmentType() != Segment::Type::EndBarLine)
            s = s->prev();
      // search first element
      if (s) {
            for (const Element* e : s->elist()) {
                  if (e)
                        return toBarLine(e);
                  }
            }
      return nullptr;
      }

//---------------------------------------------------------
//   endBarLineType
//    Assume all barlines have same type if there is more
//    than one.
//---------------------------------------------------------

BarLineType Measure::endBarLineType() const
      {
      const BarLine* bl = endBarLine();
      return bl ? bl->barLineType() : BarLineType::NORMAL;
      }

//---------------------------------------------------------
//   endBarLineType
//    Assume all barlines have same visiblity if there is more
//    than one.
//---------------------------------------------------------

bool Measure::endBarLineVisible() const
      {
      const BarLine* bl = endBarLine();
      return bl ? bl->visible() : true;
      }
//---------------------------------------------------------
//   triggerLayout
//---------------------------------------------------------

void Measure::triggerLayout() const
      {
      score()->setLayout(first()->tick());
      score()->setLayout(last()->tick());
      }

}

