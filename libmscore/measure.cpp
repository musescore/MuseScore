//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer
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
#include "stafftypechange.h"
#include "stafflines.h"
#include "bracketItem.h"

namespace Ms {

//---------------------------------------------------------
//   MStaff
///   Per staff values of measure.
//---------------------------------------------------------

class MStaff {
      Shape _shape;
      Text* _noText         { 0 };         ///< Measure number text object
      StaffLines*  _lines   { 0 };
      Spacer* _vspacerUp    { 0 };
      Spacer* _vspacerDown  { 0 };
      bool _hasVoices       { false };    ///< indicates that MStaff contains more than one voice,
                                          ///< this changes some layout rules
      bool _visible         { true  };
      bool _slashStyle      { false };
#ifndef NDEBUG
      bool _corrupted       { false };
#endif

   public:
      MStaff()  {}
      ~MStaff();
      MStaff(const MStaff&);

      void setScore(Score*);
      void setTrack(int);

      Shape shape() const            { return _shape; }
      Shape& shape()                 { return _shape; }

      Text* noText() const           { return _noText;     }
      void setNoText(Text* t)        { _noText = t;        }

      StaffLines* lines() const      { return _lines; }
      void setLines(StaffLines* l)   { _lines = l;    }

      Spacer* vspacerUp() const      { return _vspacerUp;   }
      void setVspacerUp(Spacer* s)   { _vspacerUp = s;      }
      Spacer* vspacerDown() const    { return _vspacerDown; }
      void setVspacerDown(Spacer* s) { _vspacerDown = s;    }

      bool hasVoices() const         { return _hasVoices;  }
      void setHasVoices(bool val)    { _hasVoices = val;   }

      bool visible() const           { return _visible;    }
      void setVisible(bool val)      { _visible = val;     }

      bool slashStyle() const        { return _slashStyle; }
      void setSlashStyle(bool val)   { _slashStyle = val;  }

#ifndef NDEBUG
      bool corrupted() const         { return _corrupted; }
      void setCorrupted(bool val)    { _corrupted = val; }
#endif
      };

MStaff::~MStaff()
      {
      delete _noText;
      delete _lines;
      delete _vspacerUp;
      delete _vspacerDown;
      }

MStaff::MStaff(const MStaff& m)
      {
      _noText      = 0;
      _lines       = new StaffLines(*m._lines);
      _hasVoices   = m._hasVoices;
      _vspacerUp   = 0;
      _vspacerDown = 0;
      _visible     = m._visible;
      _slashStyle  = m._slashStyle;
#ifndef NDEBUG
      _corrupted   = m._corrupted;
#endif
      }

//---------------------------------------------------------
//   MStaff::setScore
//---------------------------------------------------------

void MStaff::setScore(Score* score)
      {
      if (_lines)
            _lines->setScore(score);
      if (_vspacerUp)
            _vspacerUp->setScore(score);
      if (_vspacerDown)
            _vspacerDown->setScore(score);
      if (_noText)
            _noText->setScore(score);
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void MStaff::setTrack(int track)
      {
      if (_lines)
            _lines->setTrack(track);
      if (_vspacerUp)
            _vspacerUp->setTrack(track);
      if (_vspacerDown)
            _vspacerDown->setTrack(track);
      if (_noText)
            _noText->setTrack(track);
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
            s->setLines(new StaffLines(score()));
            s->lines()->setTrack(staffIdx * VOICES);
            s->lines()->setParent(this);
            s->lines()->setVisible(!staff->invisible());
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
//   layoutStaffLines
//---------------------------------------------------------

void Measure::layoutStaffLines()
      {
      for (MStaff* ms : _mstaves)
            ms->lines()->layout();
      }

//---------------------------------------------------------
//   createStaves
//---------------------------------------------------------

void Measure::createStaves(int staffIdx)
      {
      for (int n = _mstaves.size(); n <= staffIdx; ++n) {
            Staff* staff = score()->staff(n);
            MStaff* s    = new MStaff;
            s->setLines(new StaffLines(score()));
            s->lines()->setParent(this);
            s->lines()->setTrack(n * VOICES);
            s->lines()->setVisible(!staff->invisible());
            _mstaves.push_back(s);
            }
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

bool Measure::hasVoices(int staffIdx) const                     { return _mstaves[staffIdx]->hasVoices(); }
void Measure::setHasVoices(int staffIdx, bool v)                { return _mstaves[staffIdx]->setHasVoices(v); }
StaffLines* Measure::staffLines(int staffIdx)                   { return _mstaves[staffIdx]->lines(); }
Spacer* Measure::vspacerDown(int staffIdx) const                { return _mstaves[staffIdx]->vspacerDown(); }
Spacer* Measure::vspacerUp(int staffIdx) const                  { return _mstaves[staffIdx]->vspacerUp(); }
void Measure::setStaffVisible(int staffIdx, bool visible)       { _mstaves[staffIdx]->setVisible(visible); }
void Measure::setStaffSlashStyle(int staffIdx, bool slashStyle) { _mstaves[staffIdx]->setSlashStyle(slashStyle); }
#ifndef NDEBUG
bool Measure::corrupted(int staffIdx) const                     { return _mstaves[staffIdx]->corrupted(); }
void Measure::setCorrupted(int staffIdx, bool val)              { _mstaves[staffIdx]->setCorrupted(val); }
#endif
void Measure::setNoText(int staffIdx, Text* t)                  { _mstaves[staffIdx]->setNoText(t); }
Text* Measure::noText(int staffIdx) const                       { return _mstaves[staffIdx]->noText(); }
Shape Measure::staffShape(int staffIdx) const                   { return _mstaves[staffIdx]->shape(); }
Shape& Measure::staffShape(int staffIdx)                        { return _mstaves[staffIdx]->shape(); }

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
      tversatz.init(chord->staff()->keySigEvent(tick()), chord->staff()->clef(tick()));

      for (Segment* segment = first(); segment; segment = segment->next()) {
            int startTrack = chord->staffIdx() * VOICES;
            if (segment->isKeySigType()) {
                  KeySig* ks = toKeySig(segment->element(startTrack));
                  if (!ks)
                        continue;
                  tversatz.init(chord->staff()->keySigEvent(segment->tick()), chord->staff()->clef(segment->tick()));
                  }
            else if (segment->segmentType() == SegmentType::ChordRest) {
                  int endTrack   = startTrack + VOICES;
                  for (int track = startTrack; track < endTrack; ++track) {
                        Element* e = segment->element(track);
                        if (!e || !e->isChord())
                              continue;
                        Chord* chord = toChord(e);
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
      tversatz.init(staff->keySigEvent(tick()), staff->clef(tick()));

      SegmentType st = SegmentType::ChordRest;
      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;
      for (Segment* segment = first(st); segment; segment = segment->next(st)) {
            if (segment == s && staff->isPitchedStaff(tick())) {
                  ClefType clef = staff->clef(s->tick());
                  int l = relStep(line, clef);
                  return tversatz.accidentalVal(l, error);
                  }
            for (int track = startTrack; track < endTrack; ++track) {
                  Element* e = segment->element(track);
                  if (!e || !e->isChord())
                        continue;
                  Chord* chord = toChord(e);
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
      tck -= ticks();
      if (isMMRest()) {
            Segment* s = first(SegmentType::ChordRest);
            qreal x1   = s->x();
            qreal w    = width() - x1;
            return x1 + (tck * w) / (ticks() * mmRestCount());
            }

      Segment* s;
      qreal x1  = 0;
      qreal x2  = 0;
      int tick1 = 0;
      int tick2 = 0;
      for (s = first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            x2    = s->x();
            tick2 = s->rtick();
            if (tck == tick2)
                  return x2 + pos().x();
            if (tck <= tick2)
                  break;
            x1    = x2;
            tick1 = tick2;
            }
      if (s == 0) {
            x2    = width();
            tick2 = ticks();
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
            Spacer* sp = ms->vspacerDown();
            if (sp) {
                  sp->layout();
                  int n = score()->staff(staffIdx)->lines(tick()) - 1;
                  qreal y = system()->staff(staffIdx)->y();
                  sp->setPos(_spatium * .5, y + n * _spatium);
                  }
            sp = ms->vspacerUp();
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
                              ( ((no()+1) % score()->styleI(StyleIdx::measureNumberInterval)) == 0 );
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
                        t = new Text(SubStyle::MEASURE_NUMBER, score());
                        t->setFlag(ElementFlag::ON_STAFF, true);
                        t->setTrack(staffIdx * VOICES);
                        t->setGenerated(true);
                        t->setParent(this);
                        add(t);
                        }
                  t->setXmlText(s);
                  t->layout();
                  }
            else {
                  if (t) {
                        if (t->generated())
                              score()->removeElement(t);
                        else
                              score()->undo(new RemoveElement(t));
                        }
                  }
            }

      //---------------------------------------------------
      //    layout ties, spanners and tuples
      //---------------------------------------------------

      int tracks = score()->ntracks();
      static const SegmentType st { SegmentType::ChordRest };
      for (int track = 0; track < tracks; ++track) {
            if (!score()->staff(track / VOICES)->show()) {
                  track += VOICES-1;
                  continue;
                  }
            for (Segment* s = first(st); s; s = s->next(st)) {
                  ChordRest* cr = s->cr(track);
                  if (!cr)
                        continue;

                  if (cr->isChord()) {
                        Chord* c = toChord(cr);
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

Chord* Measure::findChord(int t, int track)
      {
      t -= tick();
      for (Segment* seg = last(); seg; seg = seg->prev()) {
            if (seg->rtick() < t)
                  return 0;
            if (seg->rtick() == t) {
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

ChordRest* Measure::findChordRest(int t, int track)
      {
      t -= tick();
      for (const Segment& seg : _segments) {
            if (seg.rtick() > t)
                  return 0;
            if (seg.rtick() == t) {
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

Segment* Measure::tick2segment(int t, SegmentType st)
      {
      t -= tick();
      for (Segment& s : _segments) {
            if (s.rtick() == t) {
                  if (s.segmentType() & st)
                        return &s;
                  }
            if (s.rtick() > t)
                  break;
            }
      return 0;
      }

//---------------------------------------------------------
//   findSegment
/// Search for a segment of type \a st at position \a t.
//---------------------------------------------------------

Segment* Measure::findSegment(SegmentType st, int t) const
      {
      t -= tick();
      Segment* s;
      if (t > ticks()/2) {
            // search backwards
            for (s = last(); s && s->rtick() > t; s = s->prev())
                  ;
            while (s && s->prev() && s->prev()->rtick() == t)
                  s = s->prev();
            }
      else {
            // search forwards
            for (s = first(); s && s->rtick() < t; s = s->next())
                  ;
            }
      for (; s && s->rtick() == t; s = s->next()) {
            if (s->segmentType() & st)
                  return s;
            }
      return 0;
      }

//---------------------------------------------------------
//   getSegment
///   Get a segment of type \a st at tick position \a t.
///   If the segment does not exist, it is created.
//---------------------------------------------------------

Segment* Measure::getSegment(SegmentType st, int t)
      {
      return getSegmentR(st, t - tick());
      }

//---------------------------------------------------------
//   findSegmentR
//    Search for a segment of type st at relative
//    position t.
//---------------------------------------------------------

Segment* Measure::findSegmentR(SegmentType st, int t) const
      {
      Segment* s;
      if (t > ticks()/2) {
            // search backwards
            for (s = last(); s && s->rtick() > t; s = s->prev())
                  ;
            while (s && s->prev() && s->prev()->rtick() == t)
                  s = s->prev();
            }
      else {
            // search forwards
            for (s = first(); s && s->rtick() < t; s = s->next())
                  ;
            }
      for (; s && s->rtick() == t; s = s->next()) {
            if (s->segmentType() & st)
                  return s;
            }
      return 0;
      }

//---------------------------------------------------------
//   findFirst
//    return first segment of type st at relative
//    position t.
//---------------------------------------------------------

Segment* Measure::findFirst(SegmentType st, int t) const
      {
      Segment* s;
      // search forwards
      for (s = first(); s && s->rtick() <= t; s = s->next()) {
            if (s->segmentType() == st)
                  return s;
            }
      return 0;
      }

//---------------------------------------------------------
//   getSegmentR
///   Get a segment of type \a st at relative tick position \a t.
///   If the segment does not exist, it is created.
//---------------------------------------------------------

Segment* Measure::getSegmentR(SegmentType st, int t)
      {
      Segment* s = findSegmentR(st, t);
      if (!s) {
            s = new Segment(this, st, t);
            add(s);
            }
      return s;
      }

//---------------------------------------------------------
//   undoGetSegment
//---------------------------------------------------------

Segment* Measure::undoGetSegment(SegmentType type, int t)
      {
      return undoGetSegmentR(type, t-tick());
      }

//---------------------------------------------------------
//   undoGetSegmentR
//---------------------------------------------------------

Segment* Measure::undoGetSegmentR(SegmentType type, int t)
      {
      Segment* s = findSegmentR(type, t);
      if (s == 0) {
            s = new Segment(this, type, t);
            score()->undoAddElement(s);
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
      ElementType type = e->type();

      switch (type) {
            case ElementType::SEGMENT:
                  {
                  Segment* seg     = toSegment(e);
                  int t            = seg->rtick();
                  SegmentType st = seg->segmentType();
                  Segment* s;

                  for (s = first(); s && s->rtick() < t; s = s->next())
                        ;
                  while (s && s->rtick() == t) {
                        if (s->segmentType() > st)
                              break;
                        s = s->next();
                        }
                  seg->setParent(this);
                  _segments.insert(seg, s);
                  //
                  // update measure flags
                  //
                  if (seg->header())
                        seg->measure()->setHeader(true);
                  if (seg->trailer())
                        seg->measure()->setTrailer(true);
                  }
                  break;

            case ElementType::TEXT:
                  if (e->staffIdx() < int(_mstaves.size()))
                        _mstaves[e->staffIdx()]->setNoText(toText(e));
                  break;

            case ElementType::SPACER:
                  {
                  Spacer* sp = toSpacer(e);
                  switch (sp->spacerType()) {
                        case SpacerType::UP:
                              _mstaves[e->staffIdx()]->setVspacerUp(sp);
                              break;
                        case SpacerType::DOWN:
                        case SpacerType::FIXED:
                              _mstaves[e->staffIdx()]->setVspacerDown(sp);
                              break;
                        }
                  }
                  break;
            case ElementType::JUMP:
                  setRepeatJump(true);
                  // fall through

            case ElementType::MARKER:
                  el().push_back(e);
                  break;

            case ElementType::HBOX:
                  if (e->staff())
                        e->setMag(e->staff()->mag(tick()));     // ?!
                  el().push_back(e);
                  break;

            case ElementType::MEASURE:
                  _mmRest = toMeasure(e);
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
            case ElementType::SEGMENT:
                  {
                  Segment* s = toSegment(e);
                  _segments.remove(s);
                  //
                  // update measure flags
                  //
                  if (s->header())
                        s->measure()->checkHeader();
                  if (s->trailer())
                        s->measure()->checkTrailer();
                  }
                  break;

            case ElementType::TEXT:
                  _mstaves[e->staffIdx()]->setNoText(nullptr);
                  break;

            case ElementType::SPACER:
                  switch (toSpacer(e)->spacerType()) {
                        case SpacerType::DOWN:
                        case SpacerType::FIXED:
                              _mstaves[e->staffIdx()]->setVspacerDown(0);
                              break;
                        case SpacerType::UP:
                              _mstaves[e->staffIdx()]->setVspacerUp(0);
                              break;
                        }
                  break;

            case ElementType::JUMP:
                  setRepeatJump(false);
                  // fall through

            case ElementType::MARKER:
            case ElementType::HBOX:
                  if (!el().remove(e)) {
                        qDebug("Measure(%p)::remove(%s,%p) not found", this, e->name(), e);
                        }
                  break;

            case ElementType::CLEF:
            case ElementType::CHORD:
            case ElementType::REST:
            case ElementType::TIMESIG:
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

            case ElementType::MEASURE:
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
      if (o->isTuplet()) {
            Tuplet* t = toTuplet(n);
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
      for (Segment* segment = last(); segment; segment = segment->prev()) {
            if (segment->segmentType() & (SegmentType::EndBarLine | SegmentType::TimeSigAnnounce))
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

      Segment* ts = findSegment(SegmentType::TimeSig, tick());

      for (int i = sStaff; i < eStaff; ++i) {
            Staff* staff = score()->staff(i);
            MStaff* ms   = new MStaff;
            ms->setLines(new StaffLines(score()));
            ms->lines()->setTrack(i * VOICES);
            ms->lines()->setParent(this);
            ms->lines()->setVisible(!staff->invisible());
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
                              ots = toTimeSig(ts->element(track));
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
                        score()->undoAddElement(timesig);
                        if (constructed)
                              delete ots;
                        }
                  }
            }
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
      ms->setLines(new StaffLines(score()));
      ms->lines()->setParent(this);
      ms->lines()->setTrack(staffIdx * VOICES);
      ms->lines()->setVisible(!staff->invisible());
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

bool Measure::acceptDrop(EditData& data) const
      {
      MuseScoreView* viewer = data.view;
      QPointF pos           = data.pos;
      Element* e            = data.element;


      int staffIdx;
      Segment* seg;
      if (!score()->pos2measure(pos, &staffIdx, 0, &seg, 0))
            return false;

      QRectF staffR = system()->staff(staffIdx)->bbox().translated(system()->canvasPos());
      staffR &= canvasBoundingRect();

      switch (e->type()) {
            case ElementType::MEASURE_LIST:
            case ElementType::JUMP:
            case ElementType::MARKER:
            case ElementType::LAYOUT_BREAK:
            case ElementType::STAFF_LIST:
                  viewer->setDropRectangle(canvasBoundingRect());
                  return true;

            case ElementType::KEYSIG:
            case ElementType::TIMESIG:
                  if (data.modifiers & Qt::ControlModifier)
                        viewer->setDropRectangle(staffR);
                  else
                        viewer->setDropRectangle(canvasBoundingRect());
                  return true;

            case ElementType::BRACKET:
            case ElementType::REPEAT_MEASURE:
            case ElementType::MEASURE:
            case ElementType::SPACER:
            case ElementType::IMAGE:
            case ElementType::BAR_LINE:
            case ElementType::SYMBOL:
            case ElementType::CLEF:
            case ElementType::STAFFTYPE_CHANGE:
                  viewer->setDropRectangle(staffR);
                  return true;

            case ElementType::ICON:
                  switch (toIcon(e)->iconType()) {
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

Element* Measure::drop(EditData& data)
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
      //bool fromPalette = (e->track() == -1);

      switch (e->type()) {
            case ElementType::MEASURE_LIST:
                  delete e;
                  break;

            case ElementType::STAFF_LIST:
//TODO                  score()->pasteStaff(e, this, staffIdx);
                  delete e;
                  break;

            case ElementType::MARKER:
            case ElementType::JUMP:
                  e->setParent(this);
                  e->setTrack(0);
                  {
                  // code borrowed from ChordRest::drop()
//                  Text* t = static_cast<Text*>(e);
//                  StyledPropertyListIdx st = t->textStyleType();
                  // for palette items, we want to use current score text style settings
                  // except where the source element had explicitly overridden these via text properties
                  // palette text style will be relative to baseStyle, so rebase this to score
//                  if (st >= StyledPropertyListIdx::DEFAULT && fromPalette)
//                        t->textStyle().restyle(MScore::baseStyle().textStyle(st), score()->textStyle(st));
                  }
                  score()->undoAddElement(e);
                  return e;

            case ElementType::DYNAMIC:
            case ElementType::FRET_DIAGRAM:
                  e->setParent(seg);
                  e->setTrack(staffIdx * VOICES);
                  score()->undoAddElement(e);
                  return e;

            case ElementType::IMAGE:
            case ElementType::SYMBOL:
                  e->setParent(seg);
                  e->setTrack(staffIdx * VOICES);
                  e->layout();
                  {
                  QPointF uo(data.pos - e->canvasPos() - data.dragOffset);
                  e->setUserOff(uo);
                  }
                  score()->undoAddElement(e);
                  return e;

            case ElementType::BRACKET:
                  {
                  Bracket* b = toBracket(e);
                  int level = 0;
                  int firstStaff = 0;
                  for (Staff* s : score()->staves()) {
                        for (const BracketItem* bi : s->brackets()) {
                              int lastStaff = firstStaff + bi->bracketSpan() - 1;
                              if (staffIdx >= firstStaff && staffIdx <= lastStaff)
                                    ++level;
                              }
                        firstStaff++;
                        }
                  score()->undoAddBracket(staff, level, b->bracketType(), 1);
                  delete b;
                  }
                  return 0;

            case ElementType::CLEF:
                  score()->undoChangeClef(staff, first(), toClef(e)->clefType());
                  delete e;
                  break;

            case ElementType::KEYSIG:
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

            case ElementType::TIMESIG:
                  score()->cmdAddTimeSig(this, staffIdx, toTimeSig(e),
                     data.modifiers & Qt::ControlModifier);
                  return 0;

            case ElementType::LAYOUT_BREAK: {
                  LayoutBreak* b = toLayoutBreak(e);
                  Measure* measure = isMMRest() ? mmRestLast() : this;
                  switch (b->layoutBreakType()) {
                        case  LayoutBreak::PAGE:
                              if (measure->pageBreak()) {
                                    delete b;
                                    b = 0;
                                    }
                              else
                                    measure->setLineBreak(false);
                              break;
                        case  LayoutBreak::LINE:
                              if (measure->lineBreak()) {
                                    delete b;
                                    b = 0;
                                    }
                              else
                                    measure->setPageBreak(false);
                              break;
                        case  LayoutBreak::SECTION:
                              if (measure->sectionBreak()) {
                                    delete b;
                                    b = 0;
                                    }
                              else
                                    measure->setLineBreak(false);
                              break;
                        case LayoutBreak::NOBREAK:
                              if (measure->noBreak()) {
                                    delete b;
                                    b = 0;
                                    }
                              else {
                                    measure->setLineBreak(false);
                                    measure->setPageBreak(false);
                                    }
                              break;
                        }
                  if (b) {
                        b->setTrack(-1);       // these are system elements
                        b->setParent(measure);
                        score()->undoAddElement(b);
                        }
                  measure->cleanupLayoutBreaks(true);
                  return b;
                  }

            case ElementType::SPACER:
                  {
                  Spacer* spacer = toSpacer(e);
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

            case ElementType::BAR_LINE:
                  {
                  BarLine* bl = toBarLine(e);

                  // if dropped bar line refers to span rather than to subtype
                  // or if Ctrl key used
                  if ((bl->spanFrom() && bl->spanTo()) || data.control()) {
                        // get existing bar line for this staff, and drop the change to it
                        Segment* seg = undoGetSegment(SegmentType::EndBarLine, tick() + ticks());
                        BarLine* cbl = toBarLine(seg->element(staffIdx * VOICES));
                        if (cbl)
                              cbl->drop(data);
                        }
                  else {
                        // if dropped bar line refers to line subtype
                        score()->undoChangeBarLine(this, bl->barLineType(), SegmentType::EndBarLine);
                        delete e;
                        }
                  break;
                  }

            case ElementType::REPEAT_MEASURE:
                  {
                  delete e;
                  return cmdInsertRepeatMeasure(staffIdx);
                  }
            case ElementType::ICON:
                  switch(toIcon(e)->iconType()) {
                        case IconType::VFRAME:
                              score()->insertMeasure(ElementType::VBOX, this);
                              break;
                        case IconType::HFRAME:
                              score()->insertMeasure(ElementType::HBOX, this);
                              break;
                        case IconType::TFRAME:
                              score()->insertMeasure(ElementType::TBOX, this);
                              break;
                        case IconType::FFRAME:
                              score()->insertMeasure(ElementType::FBOX, this);
                              break;
                        case IconType::MEASURE:
                              score()->insertMeasure(ElementType::MEASURE, this);
                              break;
                        default:
                              break;
                        }
                  break;

            case ElementType::STAFFTYPE_CHANGE:
                  {
                  StaffTypeChange* stc = toStaffTypeChange(e);
                  e->setParent(this);
                  e->setTrack(staffIdx * VOICES);
                  StaffType* st = stc->staffType();
                  Staff* staff = score()->staff(staffIdx);

                  StaffType* nst;
                  if (st) {
                        nst = staff->setStaffType(tick(), st);
                        delete st;
                        }
                  else {
                        // dragged from palette
                        st  = staff->staffType(tick());
                        nst = staff->setStaffType(tick(), st);
                        }
                  stc->setStaffType(nst);
                  score()->undoAddElement(e);
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
            if (s->segmentType() & SegmentType::ChordRest) {
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
      Segment* seg = undoGetSegment(SegmentType::ChordRest, tick());
      RepeatMeasure* rm = new RepeatMeasure(score());
      rm->setTrack(staffIdx * VOICES);
      rm->setParent(seg);
      rm->setDurationType(TDuration::DurationType::V_MEASURE);
      rm->setDuration(stretchedLen(score()->staff(staffIdx)));
      score()->undoAddCR(rm, this, tick());
      for (Element* e : el()) {
            if (e->isSlur() && e->staffIdx() == staffIdx)
                  score()->undoRemoveElement(e);
            }
      return rm;
      }

//---------------------------------------------------------
//   adjustToLen
//    change actual measure len, adjust elements to
//    new len
//---------------------------------------------------------

void Measure::adjustToLen(Fraction nf, bool appendRestsIfNecessary)
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
                        if (s->segmentType() & (SegmentType::EndBarLine|SegmentType::TimeSigAnnounce|SegmentType::KeySigAnnounce)) {
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
                              if (segment->segmentType() == SegmentType::ChordRest) {
                                    for (Element* a : segment->annotations())
                                          if (a->track() == trk)
                                                s->undoRemoveElement(a);
                                    Element* e = segment->element(trk);
                                    if (e && e->isChordRest()) {
                                          ChordRest* cr = toChordRest(e);
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
                                    }
                              else if (segment->segmentType() == SegmentType::Breath) {
                                    Element* e = segment->element(trk);
                                    if (e)
                                          s->undoRemoveElement(e);
                                    }
                              segment = pseg;
                              }
                        rFlag = true;
                        }
                  int voice = trk % VOICES;
                  if (appendRestsIfNecessary && (n > 0) && (rFlag || voice == 0)) {
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
            foreach (Element* e, m->el()) {
                  if (e->isSlur())
                        s->undoRemoveElement(e);
                  }
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Measure::write(XmlWriter& xml, int staff, bool writeSystemElements, bool forceTimeSig) const
      {
      int mno = no() + 1;
      if (_len != _timesig) {
            // this is an irregular measure
            xml.stag(QString("Measure number=\"%1\" len=\"%2/%3\"").arg(mno).arg(_len.numerator()).arg(_len.denominator()));
            }
      else
            xml.stag(QString("Measure number=\"%1\"").arg(mno));

      xml.setCurTick(tick());

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

      if (mstaff->vspacerUp())
            xml.tag("vspacerUp", mstaff->vspacerUp()->gap() / _spatium);
      if (mstaff->vspacerDown()) {
            if (mstaff->vspacerDown()->spacerType() == SpacerType::FIXED)
                  xml.tag("vspacerFixed", mstaff->vspacerDown()->gap() / _spatium);
            else
                  xml.tag("vspacerDown", mstaff->vspacerDown()->gap() / _spatium);
            }
      if (!mstaff->visible())
            xml.tag("visible", mstaff->visible());
      if (mstaff->slashStyle())
            xml.tag("slashStyle", mstaff->slashStyle());

      int strack = staff * VOICES;
      int etrack = strack + VOICES;
      for (const Element* e : el()) {
            if (!e->generated() && ((e->staffIdx() == staff) || (e->systemFlag() && writeSystemElements)))
                  e->write(xml);
            }
      Q_ASSERT(first());
      Q_ASSERT(last());
      score()->writeSegments(xml, strack, etrack, first(), last()->next1(), writeSystemElements, false, false, forceTimeSig);
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
            s->setLines(new StaffLines(score()));
            s->lines()->setParent(this);
            s->lines()->setTrack(n * VOICES);
            s->lines()->setVisible(!staff->invisible());
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

                  //
                  //  StartRepeatBarLine: at rtick == 0, always BarLineType::START_REPEAT
                  //  BarLine:            in the middle of a measure, has no semantic
                  //  EndBarLine:         at the end of a measure
                  //  BeginBarLine:       first segment of a measure, systemic barline

                  SegmentType st;
                  int t = e.tick() - tick();
                  if (t && (t != ticks()))
                        st = SegmentType::BarLine;
                  else if (barLine->barLineType() == BarLineType::START_REPEAT && t == 0)
                        st = SegmentType::StartRepeatBarLine;
                  else if (barLine->barLineType() == BarLineType::START_REPEAT && t == ticks()) {
                        // old version, ignore
                        delete barLine;
                        barLine = 0;
                        }
                  else if (t == 0 && segment == 0)
                        st = SegmentType::BeginBarLine;
                  else
                        st = SegmentType::EndBarLine;
                  if (barLine) {
                        segment = getSegmentR(st, t);
                        segment->add(barLine);
                        }
                  barLine->read(e);
                  barLine->layout();
                  }
            else if (tag == "Chord") {
                  Chord* chord = new Chord(score());
                  chord->setTrack(e.track());
                  chord->read(e);
                  segment = getSegment(SegmentType::ChordRest, e.tick());
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
                  segment = getSegment(SegmentType::ChordRest, e.tick());
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
                  segment = getSegment(SegmentType::Breath, tick);
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
                  segment = getSegment(SegmentType::ChordRest, e.tick());
                  segment->add(rm);
                  e.incTick(ticks());
                  }
            else if (tag == "Clef") {
                  Clef* clef = new Clef(score());
                  clef->setTrack(e.track());
                  clef->read(e);
                  clef->setGenerated(false);

                  // there may be more than one clef segment for same tick position
                  // the first clef may be missing and is added later in layout

                  bool header;
                  if (e.tick() != tick())
                        header = false;
                  else if (!segment)
                        header = true;
                  else {
                        header = true;
                        for (Segment* s = _segments.first(); s && !s->rtick(); s = s->next()) {
                              if (s->isKeySigType() || s->isTimeSigType()) {
                                    // hack: there may be other segment types which should
                                    // generate a clef at current position
                                    header = false;
                                    break;
                                    }
                              }
                        }
                  segment = getSegment(header ? SegmentType::HeaderClef : SegmentType::Clef, e.tick());
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
                        segment = getSegment(SegmentType::TimeSigAnnounce, currTick);
                        segment->add(ts);
                        }
                  else {
                        // if 'real' time sig., do full process
                        segment = getSegment(SegmentType::TimeSig, currTick);
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
                        segment = getSegment(courtesySig ? SegmentType::KeySigAnnounce : SegmentType::KeySig, curTick);
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
                        segment = getSegment(SegmentType::ChordRest, e.tick());
                        segment->add(t);
                        }
                  }

            //----------------------------------------------------
            // Annotation

            else if (tag == "Dynamic") {
                  Dynamic* dyn = new Dynamic(score());
                  dyn->setTrack(e.track());
                  dyn->read(e);
                  segment = getSegment(SegmentType::ChordRest, e.tick());
                  segment->add(dyn);
                  }
            else if (tag == "Harmony"
               || tag == "FretDiagram"
               || tag == "TremoloBar"
               || tag == "Symbol"
               || tag == "Tempo"
               || tag == "StaffText"
               || tag == "SystemText"
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
                  segment = getSegment(SegmentType::ChordRest, e.tick());
                  segment->add(el);
                  }
            else if (tag == "Marker" || tag == "Jump"
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
                        segment = getSegment(SegmentType::ChordRest, e.tick());
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
                  segment = getSegmentR(SegmentType::BeginBarLine, 0);
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
                  if (!_mstaves[staffIdx]->vspacerDown()) {
                        Spacer* spacer = new Spacer(score());
                        spacer->setSpacerType(SpacerType::DOWN);
                        spacer->setTrack(staffIdx * VOICES);
                        add(spacer);
                        }
                  _mstaves[staffIdx]->vspacerDown()->setGap(e.readDouble() * _spatium);
                  }
            else if (tag == "vspacerFixed") {
                  if (!_mstaves[staffIdx]->vspacerDown()) {
                        Spacer* spacer = new Spacer(score());
                        spacer->setSpacerType(SpacerType::FIXED);
                        spacer->setTrack(staffIdx * VOICES);
                        add(spacer);
                        }
                  _mstaves[staffIdx]->vspacerDown()->setGap(e.readDouble() * _spatium);
                  }
            else if (tag == "vspacerUp") {
                  if (!_mstaves[staffIdx]->vspacerUp()) {
                        Spacer* spacer = new Spacer(score());
                        spacer->setSpacerType(SpacerType::UP);
                        spacer->setTrack(staffIdx * VOICES);
                        add(spacer);
                        }
                  _mstaves[staffIdx]->vspacerUp()->setGap(e.readDouble() * _spatium);
                  }
            else if (tag == "visible")
                  _mstaves[staffIdx]->setVisible(e.readInt());
            else if (tag == "slashStyle")
                  _mstaves[staffIdx]->setSlashStyle(e.readInt());
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
                  Text* noText = new Text(SubStyle::MEASURE_NUMBER, score());
                  noText->read(e);
                  noText->setFlag(ElementFlag::ON_STAFF, true);
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
                  segment = getSegment(SegmentType::Ambitus, e.tick());
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
      return score()->staff(staffIdx)->show() && _mstaves[staffIdx]->visible();
      }

//---------------------------------------------------------
//   slashStyle
//---------------------------------------------------------

bool Measure::slashStyle(int staffIdx) const
      {
      Staff* staff = score()->staff(staffIdx);
      return staff->slashStyle(tick()) || _mstaves[staffIdx]->slashStyle() || staff->staffType(tick())->slashStyle();
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
            func(data, ms->lines());
            if (ms->vspacerUp())
                  func(data, ms->vspacerUp());
            if (ms->vspacerDown())
                  func(data, ms->vspacerDown());
            if (ms->noText())
                  func(data, ms->noText());
            }

      for (Segment* s = first(); s; s = s->next()) {
            if (!s->enabled())
                  continue;
            s->scanElements(data, func, all);
            }
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
            if (s->segmentType() != SegmentType::ChordRest)
                  continue;
            if (s->element(track) == 0)
                  score()->setRest(s->tick(), track, len(), true, 0);
            break;
            }
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
            _mstaves[staffIdx]->lines()->setTrack(staffIdx * VOICES);
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
      for (Segment* s = first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
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
            if (sp->isSlur() && (spStart >= start || spEnd < end)) {
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
      _mstaves[staffIdx]->setHasVoices(false);

      for (Segment* s = first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
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
                              _mstaves[staffIdx]->setHasVoices(true);
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
      if (track >= score()->ntracks())
            return false;
      for (Segment* s = first(); s; s = s->next()) {
            if (s->segmentType() != SegmentType::ChordRest)
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
      for (Segment* s = first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            for (int track = strack; track < etrack; ++track) {
                  Element* e = s->element(track);
                  if (e && !e->isRest())
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

      Segment* s = first(SegmentType::ChordRest);
      for (int track = strack; track < etrack; ++track) {
            Element* e = s->element(track);
            if (e) {
                  if (!e->isRest())
                        return false;
                  Rest* rest = toRest(e);
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
      Segment* s   = first(SegmentType::ChordRest);

      if (s == 0)
            return false;

      for (int track = strack; track < etrack; ++track) {
            Element* e = s->element(track);
            if (e && e->isRepeatMeasure())
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
      static const SegmentType st = SegmentType::ChordRest ;
      for (const Segment* s = first(st); s; s = s->next(st)) {
            bool restFound = false;
            for (int track = 0; track < tracks; ++track) {
                  if ((track % VOICES) == 0 && !score()->staff(track/VOICES)->show()) {
                        track += VOICES-1;
                        continue;
                        }
                  if (s->element(track))  {
                        if (!s->element(track)->isRest())
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
      static const SegmentType st = SegmentType::ChordRest;
      for (const Segment* s = first(st); s; s = s->next(st)) {
            if (s->segmentType() != st || !s->element(track))
                  continue;
            if (!s->element(track)->isRest())
                  return false;
            }
      return true;
      }

//---------------------------------------------------------
//   isOnlyDeletedRests
//---------------------------------------------------------

bool Measure::isOnlyDeletedRests(int track) const
      {
      static const SegmentType st { SegmentType::ChordRest };
      for (const Segment* s = first(st); s; s = s->next(st)) {
            if (s->segmentType() != st || !s->element(track))
                  continue;
            if (s->element(track)->isRest() ? !toRest(s->element(track))->isGap() : !s->element(track)->isRest())
                  return false;
            }
      return true;
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
            Segment* s = new Segment(m, oseg->segmentType(), oseg->rtick());
            m->_segments.push_back(s);
            for (int track = 0; track < tracks; ++track) {
                  Element* oe = oseg->element(track);
                  for (Element* e : oseg->annotations()) {
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
                        ChordRest* ocr = toChordRest(oe);
                        ChordRest* ncr = toChordRest(ne);
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
                        if (oe->isChord()) {
                              Chord* och = toChord(ocr);
                              Chord* nch = toChord(ncr);
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
      switch (propertyId) {
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
      score()->setLayout(tick());
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
            return toMeasure(prev()->next());
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
            return toMeasure(next()->prev());
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
      Element* e = score()->selection().element();
      if (!e && !score()->selection().elements().isEmpty())
            e = score()->selection().elements().first();
      for (; e && e->type() != ElementType::SEGMENT; e = e->parent()) {
            ;
      }
      Segment* seg = static_cast<Segment*>(e);
      Segment* nextSegment = seg->next();
      Element* next = seg->firstElementOfSegment(nextSegment, staff);
      if (next)
            return next;

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
      while (ns && !ns->enabled())
            ns = ns->next();
      while (ns) {
            Segment* s = ns;
            ns         = s->nextEnabled();
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

      Segment* s = first();
      while (s && !s->enabled())
            s = s->next();
      qreal minimumWidth = s ? s->x() : 0.0;
      for (Segment& s : _segments) {
            if (!s.enabled())
                  continue;
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

            Segment* s = first();
            while (s && !s->enabled())
                  s = s->next();
            qreal x = s->pos().x();
            while (s) {
                  s->rxpos() = x;
                  x += s->width();
                  s = s->nextEnabled();
                  }
            }

      //---------------------------------------------------
      //    layout individual elements
      //---------------------------------------------------

      for (Segment& s : _segments) {
            if (!s.enabled())
                  continue;
            for (Element* e : s.elist()) {
                  if (!e)
                        continue;
                  ElementType t = e->type();
                  int staffIdx    = e->staffIdx();
                  if (t == ElementType::REPEAT_MEASURE || (t == ElementType::REST && (isMMRest() || toRest(e)->isFullMeasureRest()))) {
                        //
                        // element has to be centered in free space
                        //    x1 - left measure position of free space
                        //    x2 - right measure position of free space

                        Segment* s1;
                        for (s1 = s.prev(); s1 && !s1->enabled(); s1 = s1->prev())
                              ;
                        Segment* s2;
                        for (s2 = s.next(); s2; s2 = s2->next()) {
                              if (s2->enabled() && !s2->isChordRestType() && s2->element(staffIdx * VOICES))
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
                  else if (t == ElementType::REST)
                        e->rxpos() = 0;
                  else if (t == ElementType::CHORD) {
                        Chord* c = toChord(e);
                        c->layout2();
                        if (c->tremolo())
                              c->tremolo()->layout();
                        }
                  else if (t == ElementType::BAR_LINE) {
                        e->rypos() = 0.0;
                        e->rxpos() = 0.0;
//                        e->rxpos() = s.isEndBarLineType() ? s.width() * .5 : 0.0;
                        e->adjustReadPos();
                        }
                  else
                        e->adjustReadPos();
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
      while (s && !s->isEndBarLineType())
            s = s->prev();
      // search first element
      if (s) {
            for (const Element* e : s->elist()) {
                  if (e)
                        return toBarLine(e);
                  }
            }
      return 0;
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
      score()->setLayout(tick());
      score()->setLayout(endTick());
      }

//---------------------------------------------------------
//   setEndBarLineType
//     Create a *generated* barline with the given type and
//     properties if none exists. Modify if it exists.
//     Useful for import filters.
//---------------------------------------------------------

void Measure::setEndBarLineType(BarLineType val, int track, bool visible, QColor color)
      {
      Segment* seg = undoGetSegment(SegmentType::EndBarLine, endTick());
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
      bl->setColor(color.isValid() ? color : curColor());
      }

//---------------------------------------------------------
//   barLinesSetSpan
//---------------------------------------------------------

void Measure::barLinesSetSpan(Segment* seg)
      {
      int track = 0;
      for (Staff* staff : score()->staves()) {
            BarLine* bl = toBarLine(seg->element(track));  // get existing bar line for this staff, if any
            if (bl) {
                  if (bl->generated()) {
                        bl->setSpanStaff(staff->barLineSpan());
                        bl->setSpanFrom(staff->barLineFrom());
                        bl->setSpanTo(staff->barLineTo());
                        }
                  }
            else {
                  bl = new BarLine(score());
                  bl->setParent(seg);
                  bl->setTrack(track);
                  bl->setGenerated(true);
                  bl->setSpanStaff(staff->barLineSpan());
                  bl->setSpanFrom(staff->barLineFrom());
                  bl->setSpanTo(staff->barLineTo());
                  bl->layout();
                  score()->addElement(bl);
                  }
            track += VOICES;
            }
      }

//---------------------------------------------------------
//   createEndBarLines
//    actually creates or modifies barlines
//    return the width change for measure
//---------------------------------------------------------

qreal Measure::createEndBarLines(bool isLastMeasureInSystem)
      {
      int nstaves  = score()->nstaves();
      Segment* seg = findSegmentR(SegmentType::EndBarLine, ticks());
      Measure* nm  = nextMeasure();

#ifndef NDEBUG
      computeMinWidth();
#endif
      qreal oldWidth = width();

      if (nm && nm->repeatStart() && !isLastMeasureInSystem && !repeatEnd()) {
            // no barline, use StartBarLine of next measure
            if (!seg)
                  return 0.0;
            seg->setEnabled(false);
            }
      else {
            BarLineType t = nm ? BarLineType::NORMAL : BarLineType::END;
            if (!seg)
                  seg = undoGetSegmentR(SegmentType::EndBarLine, ticks());
            seg->setEnabled(true);
            //
            //  Set flag "hasCourtesyKeySig" if this measure needs a courtesy key sig.
            //  This flag is later used to set a double end bar line and to actually
            //  create the courtesy key sig.
            //

            bool show = score()->styleB(StyleIdx::genCourtesyKeysig) && !sectionBreak() && nm;

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
                              Segment* s = nm->findSegment(SegmentType::KeySig, tick);
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
            if (repeatEnd()) {
                  t = BarLineType::END_REPEAT;
                  force = true;
                  }
            else if (isLastMeasureInSystem && nextMeasure() && nextMeasure()->repeatStart()) {
                  t = BarLineType::NORMAL;
                  force = true;
                  }

            for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                  int track    = staffIdx * VOICES;
                  BarLine* bl  = toBarLine(seg->element(track));
                  Staff* staff = score()->staff(staffIdx);
                  if (!bl) {
                        bl = new BarLine(score());
                        bl->setParent(seg);
                        bl->setTrack(track);
                        bl->setGenerated(true);
                        bl->setSpanStaff(staff->barLineSpan());
                        bl->setSpanFrom(staff->barLineFrom());
                        bl->setSpanTo(staff->barLineTo());
                        bl->setBarLineType(t);
                        score()->addElement(bl);
                        }
                  else {
                        // do not change bar line type if bar line is user modified
                        // and its not a repeat start/end barline (forced)

                        if (bl->generated()) {
                              bl->setSpanStaff(staff->barLineSpan());
                              bl->setSpanFrom(staff->barLineFrom());
                              bl->setSpanTo(staff->barLineTo());
                              bl->setBarLineType(t);
                              }
                        else {
                              if (bl->barLineType() != t) {
                                    if (force) {
                                          bl->undoChangeProperty(P_ID::BARLINE_TYPE, QVariant::fromValue(t));
                                          bl->setGenerated(true);
                                          }
                                    }
                              }
                        }
                  bl->layout();
                  }
            seg->createShapes();
            }

      // fix segment layout
      Segment* s = seg->prevEnabled();
      qreal x    = s->rxpos();
      computeMinWidth(s, x, false);

#if 0
#ifndef NDEBUG
      qreal w = width();
      computeMinWidth();
      if (!qFuzzyCompare(w, width()))
            qDebug("width mismatch %f != %f at %d", w, width(), tick());
#endif
#endif
      return width() - oldWidth;
      }

//---------------------------------------------------------
//   basicStretch
//---------------------------------------------------------

qreal Measure::basicStretch() const
      {
      qreal stretch = userStretch() * score()->styleD(StyleIdx::measureSpacing);
      if (stretch < 1.0)
            stretch = 1.0;
      return stretch;
      }

//---------------------------------------------------------
//   basicWidth
//---------------------------------------------------------

qreal Measure::basicWidth() const
      {
      Segment* ls = last();
      qreal w = (ls->x() + ls->width()) * basicStretch();
      qreal minMeasureWidth = score()->styleP(StyleIdx::minMeasureWidth);
      if (w < minMeasureWidth)
            w = minMeasureWidth;
      return w;
      }

//-------------------------------------------------------------------
//   addSystemHeader
///   Add elements to make this measure suitable as the first measure
///   of a system.
//    The system header can contain a starting BarLine, a Clef,
//    and a KeySig
//-------------------------------------------------------------------

void Measure::addSystemHeader(bool isFirstSystem)
      {
      int staffIdx = 0;
      Segment* kSegment = findFirst(SegmentType::KeySig, 0);
      Segment* cSegment = findFirst(SegmentType::HeaderClef, 0);

      for (Staff* staff : score()->staves()) {
            const int track = staffIdx * VOICES;

            // keep key sigs in TABs: TABs themselves should hide them
            bool needKeysig = isFirstSystem || score()->styleB(StyleIdx::genKeysig);

            // If we need a Key::C KeySig (which would be invisible) and there is
            // a courtesy key sig, dont create it and switch generated flags.
            // This avoids creating an invisible KeySig which can distort layout.

            KeySigEvent keyIdx = staff->keySigEvent(tick());
            KeySig* ksAnnounce = 0;
            if (needKeysig && (keyIdx.key() == Key::C)) {
                  Measure* pm = prevMeasure();
                  if (pm && pm->hasCourtesyKeySig()) {
                        Segment* ks = pm->first(SegmentType::KeySigAnnounce);
                        if (ks) {
                              ksAnnounce = toKeySig(ks->element(track));
                              if (ksAnnounce) {
                                    needKeysig = false;
//                                    if (keysig) {
//                                          ksAnnounce->setGenerated(false);
//TODO                                      keysig->setGenerated(true);
//                                          }
                                    }
                              }
                        }
                  }

            needKeysig = needKeysig && (keyIdx.key() != Key::C || keyIdx.custom() || keyIdx.isAtonal());

            if (needKeysig) {
                  KeySig* keysig;
                  if (!kSegment) {
                        kSegment = new Segment(this, SegmentType::KeySig, 0);
                        kSegment->setHeader(true);
                        add(kSegment);
                        keysig = 0;
                        }
                  else
                        keysig  = toKeySig(kSegment->element(track));
                  if (!keysig) {
                        //
                        // create missing key signature
                        //
                        keysig = new KeySig(score());
                        keysig->setTrack(track);
                        keysig->setGenerated(true);
                        keysig->setParent(kSegment);
                        kSegment->add(keysig);
                        }
                  keysig->setKeySigEvent(keyIdx);
                  keysig->layout();
                  kSegment->createShape(staffIdx);
                  kSegment->setEnabled(true);
                  }
            else {
                  if (kSegment) {
                        // do not disable user modified keysigs
                        bool disable = true;
                        for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
                              Element* e = kSegment->element(staffIdx * VOICES);
                              if (e && !e->generated()) {
                                    disable = false;
                                    break;
                                    }
                              }
                        if (disable)
                              kSegment->setEnabled(false);
                        }
                  }

            if (isFirstSystem || score()->styleB(StyleIdx::genClef)) {
                  ClefTypeList cl = staff->clefType(tick());
                  Clef* clef;
                  if (!cSegment) {
                        cSegment = new Segment(this, SegmentType::HeaderClef, 0);
                        cSegment->setHeader(true);
                        add(cSegment);
                        clef = 0;
                        }
                  else
                        clef = toClef(cSegment->element(track));
                  if (staff->staffType(tick())->genClef()) {
                        if (!clef) {
                              //
                              // create missing clef
                              //
                              clef = new Clef(score());
                              clef->setTrack(track);
                              clef->setGenerated(true);
                              clef->setParent(cSegment);
                              cSegment->add(clef);
                              }
                        if (clef->generated())
                              clef->setClefType(cl);
                        clef->setSmall(false);
                        clef->layout();
                        }
                  else if (clef) {
                        clef->parent()->remove(clef);
                        delete clef;
                        }
                  cSegment->createShape(staffIdx);
                  cSegment->setEnabled(true);
                  }
            else {
                  if (cSegment)
                        cSegment->setEnabled(false);
                  }
            ++staffIdx;
            }
      //
      // create systemic barline
      //
      Segment* s  = findSegment(SegmentType::BeginBarLine, tick());
      int n       = score()->nstaves();
      if ((n > 1 && score()->styleB(StyleIdx::startBarlineMultiple)) || (n == 1 && score()->styleB(StyleIdx::startBarlineSingle))) {
            if (!s) {
                  s = new Segment(this, SegmentType::BeginBarLine, 0);
                  add(s);
                  }
            for (int track = 0; track < score()->ntracks(); track += VOICES) {
                  BarLine* bl = toBarLine(s->element(track));
                  if (!bl) {
                        bl = new BarLine(score());
                        bl->setTrack(track);
                        bl->setGenerated(true);
                        bl->setParent(s);
                        bl->setBarLineType(BarLineType::NORMAL);
                        bl->setSpanStaff(true);
                        bl->layout();
                        s->add(bl);
                        s->createShapes();
                        }
                  }
            s->setEnabled(true);
            s->setHeader(true);
            setHeader(true);
            }
      else if (s)
            s->setEnabled(false);
      checkHeader();
      }

//---------------------------------------------------------
//   addSystemTrailer
//---------------------------------------------------------

void Measure::addSystemTrailer(Measure* nm)
      {
      int _rtick = ticks();
      bool isFinalMeasure = isFinalMeasureOfSection();

      // locate a time sig. in the next measure and, if found,
      // check if it has court. sig. turned off
      TimeSig* ts;
      bool showCourtesySig = false;
      Segment* s = findSegmentR(SegmentType::TimeSigAnnounce, _rtick);
      if (score()->genCourtesyTimesig() && !isFinalMeasure && !score()->floatMode()) {
            Segment* tss = nm->findSegmentR(SegmentType::TimeSig, 0);
            if (tss) {
                  ts = toTimeSig(tss->element(0));
                  if (ts && ts->showCourtesySig()) {
                        showCourtesySig = true;
                        // if due, create a new courtesy time signature for each staff
                        if (!s) {
                              s  = new Segment(this, SegmentType::TimeSigAnnounce, _rtick);
                              s->setTrailer(true);
                              add(s);
                              }
                        s->setEnabled(true);
                        int nstaves = score()->nstaves();
                        for (int track = 0; track < nstaves * VOICES; track += VOICES) {
                              TimeSig* nts = toTimeSig(tss->element(track));
                              if (!nts)
                                    continue;
                              ts = toTimeSig(s->element(track));
                              if (!ts) {
                                    ts = new TimeSig(score());
                                    ts->setTrack(track);
                                    ts->setGenerated(true);
                                    ts->setParent(s);
                                    score()->undoAddElement(ts);
                                    }
                              ts->setFrom(nts);
                              ts->layout();
                              s->createShape(track / VOICES);
                              }
                        }
                  }
            }
      if (!showCourtesySig && s) {
            // remove any existing time signatures
            s->setEnabled(false);
            }

      // courtesy key signatures

      int n      = score()->nstaves();
      bool show  = hasCourtesyKeySig();
      s          = findSegmentR(SegmentType::KeySigAnnounce, _rtick);

      Segment* clefSegment = findSegmentR(SegmentType::Clef, ticks());

      for (int staffIdx = 0; staffIdx < n; ++staffIdx) {
            int track    = staffIdx * VOICES;
            Staff* staff = score()->staff(staffIdx);

            if (show) {
                  if (!s) {
                        s = new Segment(this, SegmentType::KeySigAnnounce, _rtick);
                        s->setTrailer(true);
                        add(s);
                        }
                  KeySig* ks = toKeySig(s->element(track));
                  KeySigEvent key2 = staff->keySigEvent(endTick());

                  if (!ks) {
                        ks = new KeySig(score());
                        ks->setTrack(track);
                        ks->setGenerated(true);
                        ks->setParent(s);
                        s->add(ks);
                        }
                  //else if (!(ks->keySigEvent() == key2)) {
                  //      score()->undo(new ChangeKeySig(ks, key2, ks->showCourtesy()));
                  //      }
                  ks->setKeySigEvent(key2);
                  ks->layout();
                  s->createShape(track / VOICES);
                  s->setEnabled(true);
                  }
            else {
                  // remove any existent courtesy key signature
                  if (s)
                        s->setEnabled(false);
                  }
            if (clefSegment) {
                  Clef* clef = toClef(clefSegment->element(track));
                  if (clef) {
                        clef->setSmall(true);
                        if (!score()->genCourtesyClef() || repeatEnd() || isFinalMeasure || !clef->showCourtesy())
                              clef->clear();          // make invisible
                        }
                  }
            }
      checkTrailer();
      }

//---------------------------------------------------------
//   removeSystemHeader
//---------------------------------------------------------

void Measure::removeSystemHeader()
      {
      if (!header())
            return;
      for (Segment* seg = first(); seg; seg = seg->next()) {
            if (!seg->header())
                  break;
            seg->setEnabled(false);
            }
      setHeader(false);
      }

//---------------------------------------------------------
//   removeSystemTrailer
//---------------------------------------------------------

void Measure::removeSystemTrailer()
      {
      bool changed = false;
      for (Segment* seg = last(); seg != first(); seg = seg->prev()) {
            if (!seg->trailer())
                  break;
            if (seg->enabled())
                  seg->setEnabled(false);
            changed = true;
            }
      setTrailer(false);
      if (changed)
            computeMinWidth();
      }

//---------------------------------------------------------
//   checkHeader
//---------------------------------------------------------

void Measure::checkHeader()
      {
      for (Segment* seg = first(); seg; seg = seg->next()) {
            if (seg->enabled() && seg->header()) {
                  setHeader(seg->header());
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   checkTrailer
//---------------------------------------------------------

void Measure::checkTrailer()
      {
      for (Segment* seg = last(); seg != first(); seg = seg->prev()) {
            if (seg->enabled() && seg->trailer()) {
                  setTrailer(seg->trailer());
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   setStretchedWidth
//---------------------------------------------------------

void Measure::setStretchedWidth(qreal w)
      {
      qreal minWidth = score()->styleP(StyleIdx::minMeasureWidth);
      if (w < minWidth)
            w = minWidth;
      qreal stretchableWidth = 0.0;
      for (Segment* s = first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            stretchableWidth += s->width();
            }
      w += stretchableWidth * (basicStretch()-1.0) * ticks() / 1920.0;
      setWidth(w);
      }

//---------------------------------------------------------
//   computeMinWidth
//    sets the minimum stretched width of segment list s
//    set the width and x position for all segments
//---------------------------------------------------------

void Measure::computeMinWidth(Segment* s, qreal x, bool isSystemHeader)
      {
      Segment* fs = s;
      bool first  = system()->firstMeasure() == this;
      const Shape ls(first ? QRectF(0.0, -1000000.0, 0.0, 2000000.0) : QRectF(0.0, 0.0, 0.0, spatium() * 4));
      while (s) {
            s->rxpos() = x;
            if (!s->enabled()) {
                  s->setWidth(0);
                  s = s->next();
                  continue;
                  }
            Segment* ns = s->nextEnabled();
            qreal w;

            if (ns) {
                  if (isSystemHeader && !ns->header()) {        // this is the system header gap
                        w = s->minHorizontalDistance(ns, true);
                        isSystemHeader = false;
                        }
                  else {
                        w = s->minHorizontalDistance(ns, false);
                        }
// printf("  min %f <%s>(%d) <%s>(%d)\n", s->x(), s->subTypeName(), s->enabled(), ns->subTypeName(), ns->enabled());
#if 1
                  // look back for collisions with previous segments
                  // this is time consuming (ca. +5%) and probably requires more optimization

                  int n = 1;
                  for (Segment* ps = s; ps != fs;) {
                        qreal ww;
                        ps = ps->prevEnabled();
                        if (ps == fs)
                              ww = ns->minLeft(ls) - s->x();
                        else {
                              if (ps->isChordRestType())
                                    ++n;
                              ww = ps->minHorizontalDistance(ns, false) - (s->x() - ps->x());
                              }
                        if (ww > w) {
                              // overlap !
                              // distribute extra space between segments ps - ss;
                              // only ChordRest segments get more space
                              // TODO: is there a special case n == 0 ?

                              qreal d = (ww - w) / n;
                              qreal xx = ps->x();
                              for (Segment* ss = ps; ss != s;) {
                                    Segment* ns = ss->nextEnabled();
                                    qreal ww    = ss->width();
                                    if (ss->isChordRestType()) {
                                          ww += d;
                                          ss->setWidth(ww);
                                          }
                                    xx += ww;
                                    ns->rxpos() = xx;
                                    ss = ns;
                                    }
                              w += d;
                              x = xx;
                              break;
                              }
                        }
#endif
                  }
            else
                  w = s->minRight();
            s->setWidth(w);
            x += w;
            s = s->next();
            }
      setStretchedWidth(x);
      }

//---------------------------------------------------------
//   hasAccidental
//---------------------------------------------------------

static bool hasAccidental(Segment* s)
      {
      for (int track = 0; track < s->score()->ntracks(); ++track) {
            Element* e = s->element(track);
            if (!e || !e->isChord())
                  continue;
            Chord* c = toChord(e);
            for (Note* n : c->notes()) {
                  if (n->accidental())
                        return true;
                  }
            }
      return false;
      }

void Measure::computeMinWidth()
      {
      Segment* s;

      //
      // skip disabled segment
      //
      for (s = first(); s && !s->enabled(); s = s->next()) {
            s->rxpos() = 0;
            s->setWidth(0);
            }
      if (!s) {
            setWidth(0.0);
            return;
            }
      qreal x;
      bool first = system()->firstMeasure() == this;

      // left barriere:
      //    Make sure no elements crosses the left boarder if first measure in a system.
      //
      Shape ls(first ? QRectF(0.0, -1000000.0, 0.0, 2000000.0) : QRectF(0.0, 0.0, 0.0, spatium() * 4));

      x = s->minLeft(ls);
      if (s->isChordRestType()) {
            x += score()->styleP(hasAccidental(s) ? StyleIdx::barAccidentalDistance : StyleIdx::barNoteDistance);
            }
      else if (s->isClefType())
            x += score()->styleP(StyleIdx::clefLeftMargin);
      else if (s->isKeySigType())
            x = qMax(x, score()->styleP(StyleIdx::keysigLeftMargin));
      else if (s->isTimeSigType())
            x = qMax(x, score()->styleP(StyleIdx::timesigLeftMargin));
      x += s->extraLeadingSpace().val() * spatium();
      bool isSystemHeader = s->header();

      computeMinWidth(s, x, isSystemHeader);
      }

}

