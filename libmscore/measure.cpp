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

#include "global/log.h"

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
#include "fermata.h"
#include "fret.h"
#include "glissando.h"
#include "hairpin.h"
#include "hook.h"
#include "icon.h"
#include "key.h"
#include "keysig.h"
#include "layoutbreak.h"
#include "layout.h"
#include "note.h"
#include "page.h"
#include "part.h"
#include "pitchspelling.h"
#include "repeat.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "select.h"
#include "sig.h"
#include "spacer.h"
#include "staff.h"
#include "stafftext.h"
#include "stafftype.h"
#include "stem.h"
#include "stringdata.h"
#include "style.h"
#include "system.h"
#include "measurenumber.h"
#include "mmrestrange.h"
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
      MeasureNumber* _noText { 0 };         ///< Measure number text object
      MMRestRange* _mmRangeText { 0 };      ///< Multi measure rest range text object
      StaffLines*  _lines    { 0 };
      Spacer* _vspacerUp     { 0 };
      Spacer* _vspacerDown   { 0 };
      bool _hasVoices        { false };    ///< indicates that MStaff contains more than one voice,
                                          ///< this changes some layout rules
      bool _visible          { true  };
      bool _stemless         { false };
#ifndef NDEBUG
      bool _corrupted        { false };
#endif

   public:
      MStaff()  {}
      ~MStaff();
      MStaff(const MStaff&);

      void setScore(Score*);
      void setTrack(int);

      MeasureNumber* noText() const   { return _noText;     }
      void setNoText(MeasureNumber* t) { _noText = t;        }

      MMRestRange *mmRangeText() const { return _mmRangeText; }
      void setMMRangeText(MMRestRange *r) { _mmRangeText = r; }

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

      bool stemless() const          { return _stemless; }
      void setStemless(bool val)     { _stemless = val;  }

#ifndef NDEBUG
      bool corrupted() const         { return _corrupted; }
      void setCorrupted(bool val)    { _corrupted = val; }
#endif
      };

MStaff::~MStaff()
      {
      delete _noText;
      delete _mmRangeText;
      delete _lines;
      delete _vspacerUp;
      delete _vspacerDown;
      }

MStaff::MStaff(const MStaff& m)
      {
      _noText      = 0;
      _mmRangeText = 0;
      _lines       = new StaffLines(*m._lines);
      _hasVoices   = m._hasVoices;
      _vspacerUp   = 0;
      _vspacerDown = 0;
      _visible     = m._visible;
      _stemless  = m._stemless;
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
      if (_mmRangeText)
            _mmRangeText->setScore(score);
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
      if(_mmRangeText)
            _mmRangeText->setTrack(track);
      }

//---------------------------------------------------------
//   Measure
//---------------------------------------------------------

Measure::Measure(Score* s)
   : MeasureBase(s), _timesig(4,4)
      {
      setTicks(Fraction(4,4));
      _repeatCount           = 2;

      int n = score()->nstaves();
      _mstaves.reserve(n);
      for (int staffIdx = 0; staffIdx < n; ++staffIdx) {
            MStaff* ms   = new MStaff;
            Staff* staff = score()->staff(staffIdx);
            ms->setLines(new StaffLines(score()));
            ms->lines()->setTrack(staffIdx * VOICES);
            ms->lines()->setParent(this);
            ms->lines()->setVisible(!staff->invisible(tick()));
            _mstaves.push_back(ms);
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
      _noMode       = m._noMode;
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
      int staffIdx = 0;
      for (MStaff* ms : _mstaves) { 
            if (isCutawayClef(staffIdx) && (score()->staff(staffIdx)->cutaway() || !visible(staffIdx)))
                  // draw short staff lines for a courtesy clef on a hidden measure
                  ms->lines()->layoutPartialWidth(width(), 4.0, true);
            else
                  // normal staff lines
                  ms->lines()->layout();                           
            staffIdx += 1;
            }
      }

//---------------------------------------------------------
//   createStaves
//---------------------------------------------------------

void Measure::createStaves(int staffIdx)
      {
      for (int n = int(_mstaves.size()); n <= staffIdx; ++n) {
            Staff* staff = score()->staff(n);
            MStaff* s    = new MStaff;
            s->setLines(new StaffLines(score()));
            s->lines()->setParent(this);
            s->lines()->setTrack(n * VOICES);
            s->lines()->setVisible(!staff->invisible(tick()));
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
void Measure::setStaffStemless(int staffIdx, bool stemless)     { _mstaves[staffIdx]->setStemless(stemless); }

#ifndef NDEBUG
bool Measure::corrupted(int staffIdx) const                     { return _mstaves[staffIdx]->corrupted(); }
void Measure::setCorrupted(int staffIdx, bool val)              { _mstaves[staffIdx]->setCorrupted(val); }
#endif

void Measure::setNoText(int staffIdx, MeasureNumber* t)         { _mstaves[staffIdx]->setNoText(t); }
MeasureNumber* Measure::noText(int staffIdx) const              { return _mstaves[staffIdx]->noText(); }

void Measure::setMMRangeText(int staffIdx, MMRestRange* t)      { _mstaves[staffIdx]->setMMRangeText(t); }
MMRestRange* Measure::mmRangeText(int staffIdx) const           { return _mstaves[staffIdx]->mmRangeText(); }

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
      Staff* vStaff = chord->score()->staff(chord->vStaffIdx());
      AccidentalState tversatz;  // state of already set accidentals for this measure
      tversatz.init(vStaff->keySigEvent(tick()), chord->staff()->clef(tick()));

      for (Segment* segment = first(); segment; segment = segment->next()) {
            int startTrack = chord->vStaffIdx() * VOICES;
            if (segment->isKeySigType()) {
                  KeySig* ks = toKeySig(segment->element(startTrack));
                  if (!ks)
                        continue;
                  tversatz.init(vStaff->keySigEvent(segment->tick()), chord->staff()->clef(segment->tick()));
                  }
            else if (segment->segmentType() == SegmentType::ChordRest) {
                  int endTrack   = startTrack + VOICES;
                  for (int track = startTrack; track < endTrack; ++track) {
                        Element* e = segment->element(track);
                        if (!e || !e->isChord())
                              continue;
                        Chord* crd = toChord(e);
                        for (Chord* chord1 : crd->graceNotes()) {
                              for (Note* note1 : chord1->notes()) {
                                    if (note1->tieBack() && note1->accidental() == 0)
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
                        for (Note* note1 : crd->notes()) {
                              if (note1->tieBack() && note1->accidental() == 0)
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
                              if (note->tieBack() && note->accidental() == 0)
                                    continue;
                              int tpc  = note->tpc();
                              int l    = absStep(tpc, note->epitch());
                              tversatz.setAccidentalVal(l, tpc2alter(tpc));
                              }
                        }

                  for (Note* note : chord->notes()) {
                        if (note->tieBack() && note->accidental() == 0)
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
///    return x position for tick relative to System
//---------------------------------------------------------

qreal Measure::tick2pos(Fraction tck) const
      {
      tck -= ticks();
      if (isMMRest()) {
            Segment* s = first(SegmentType::ChordRest);
            qreal x1   = s->x();
            qreal w    = width() - x1;
            return x1 + (tck.ticks() * w) / (ticks().ticks() * mmRestCount());
            }

      Segment* s;
      qreal x1  = 0;
      qreal x2  = 0;
      Fraction tick1 = Fraction(0,1);
      Fraction tick2 = Fraction(0,1);
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
      Fraction dt   = tick2 - tick1;
      x1      += dt.isZero() ? 0.0 : (dx * (tck.ticks() - tick1.ticks()) / dt.ticks());
      return x1 + pos().x();
      }

//---------------------------------------------------------
//   showsMeasureNumberInAutoMode
///    Wheter the measure will show measure number(s) when MeasureNumberMode is set to AUTO
//---------------------------------------------------------

bool Measure::showsMeasureNumberInAutoMode()
      {
      // Check wheter any measure number should be shown
      if (!score()->styleB(Sid::showMeasureNumber))
            return false;

      // Measure numbers should not be shown on irregular measures.
      if (irregular())
            return false;

      int interval = score()->styleI(Sid::measureNumberInterval);
      Measure* prevMeasure = this->prevMeasure();

      // Measure numbers should not show on first measure unless specified with Sid::showMeasureNumberOne
      // except, when showing numbers on each measure, and first measure is after anacrusis - then show always
      if (!prevMeasure || prevMeasure->sectionBreak()
          || (prevMeasure->irregular() && prevMeasure->isFirstInSection() && interval != 1))
            return score()->styleB(Sid::showMeasureNumberOne);

      if (score()->styleB(Sid::measureNumberSystem))
            // Show either if
            //   1) This is the first measure of the system OR
            //   2) The previous measure in the system is the first, and is irregular.
            return (isFirstInSystem()
                    || (prevMeasure && prevMeasure->irregular() && prevMeasure->isFirstInSystem()));
      else {
            // In the case of an interval, we should show the measure number either if:
            //   1) We should show them every measure
            if (interval == 1)
                  return true;

            //   2) (measureNumber + 1) % interval == 0 (or 1 if measure number one is numbered.)
            // If measure number 1 is numbered, and the interval is let's say 5, then we should number #1, 6, 11, 16, etc.
            // If measure number 1 is not numbered, with the same interval (5), then we should number #5, 10, 15, 20, etc.
            return (((no() + 1) % interval) == (score()->styleB(Sid::showMeasureNumberOne) ? 1 : 0));
            }
      }

//---------------------------------------------------------
//   showsMeasureNumber
///     Wheter the Measure shows a MeasureNumber
//---------------------------------------------------------

bool Measure::showsMeasureNumber()
      {
      if (_noMode == MeasureNumberMode::SHOW)
            return true;
      else if (_noMode == MeasureNumberMode::HIDE)
            return false;
      else {
            return showsMeasureNumberInAutoMode();
            }
      }

//---------------------------------------------------------
//   layoutMeasureNumber
///    Layouts the Measure Numbers according to the Measure's MeasureNumberMode
//---------------------------------------------------------

void Measure::layoutMeasureNumber()
      {
      bool smn = showsMeasureNumber();

      QString s;
      if (smn)
            s = QString("%1").arg(no() + 1);

      unsigned nn = 1;
      bool nas = score()->styleB(Sid::measureNumberAllStaves);

      if (!nas) {
            //find first non invisible staff
            for (unsigned staffIdx = 0; staffIdx < _mstaves.size(); ++staffIdx) {
                  if (visible(staffIdx)) {
                        nn = staffIdx;
                        break;
                        }
                  }
            }
      for (unsigned staffIdx = 0; staffIdx < _mstaves.size(); ++staffIdx) {
            MStaff* ms       = _mstaves[staffIdx];
            MeasureNumber* t = ms->noText();
            if (t)
                  t->setTrack(staffIdx * VOICES);
            if (smn && ((staffIdx == nn) || nas)) {
                  if (t == 0) {
                        t = new MeasureNumber(score());
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
      }

void Measure::layoutMMRestRange()
      {
      if (!isMMRest() || !score()->styleB(Sid::mmRestShowMeasureNumberRange)) {
            // Remove existing
            for (unsigned staffIdx = 0; staffIdx < _mstaves.size(); ++staffIdx) {
                  MStaff *ms = _mstaves[staffIdx];
                  MMRestRange *rr = ms->mmRangeText();
                  if (rr) {
                        if (rr->generated())
                              score()->removeElement(rr);
                        else
                              score()->undo(new RemoveElement(rr));
                        }
                  }

            return;
            }


      QString s;
      if (mmRestCount() > 1) {
            // middle char is an en dash (not em)
            s = QString("%1–%2").arg(no() + 1).arg(no() + mmRestCount());
            }
      else {
            // If the minimum range to create a mmrest is set to 1,
            // then simply show the measure number as there is no range
            s = QString("%1").arg(no() + 1);
            }

      for (unsigned staffIdx = 0; staffIdx < _mstaves.size(); ++staffIdx) {
            MStaff *ms = _mstaves[staffIdx];
            MMRestRange *rr = ms->mmRangeText();
            if (!rr) {
                  rr = new MMRestRange(score());
                  rr->setTrack(staffIdx * VOICES);
                  rr->setGenerated(true);
                  rr->setParent(this);
                  add(rr);
                  }
            // setXmlText is reimplemented to take care of brackets
            rr->setXmlText(s);
            rr->layout();
            }
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
                  Staff* staff = score()->staff(staffIdx);
                  int n = staff->lines(tick()) - 1;
                  qreal y = system()->staff(staffIdx)->y();
                  sp->setPos(_spatium * .5, y + n * _spatium * staff->mag(tick()));
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
      //    layout ties
      //---------------------------------------------------

      Fraction stick = system()->measures().front()->tick();
      int tracks = score()->ntracks();
      static const SegmentType st { SegmentType::ChordRest };
      for (int track = 0; track < tracks; ++track) {
            if (!score()->staff(track / VOICES)->show()) {
                  track += VOICES-1;
                  continue;
                  }
            for (Segment* seg = first(st); seg; seg = seg->next(st)) {
                  ChordRest* cr = seg->cr(track);
                  if (!cr)
                        continue;

                  if (cr->isChord()) {
                        Chord* c = toChord(cr);
                        c->layoutSpanners(system(), stick);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   findChord
///   Search for chord at position \a tick in \a track
//---------------------------------------------------------

Chord* Measure::findChord(Fraction t, int track)
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

ChordRest* Measure::findChordRest(Fraction t, int track)
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

Segment* Measure::tick2segment(const Fraction& _t, SegmentType st)
      {
      Fraction t = _t - tick();
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
//   findSegmentR
//    Search for a segment of type st at measure relative
//    position t.
//---------------------------------------------------------

Segment* Measure::findSegmentR(SegmentType st, const Fraction& t) const
      {
      Segment* s;
      if (t > (ticks() * Fraction(1,2))) {
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
//   undoGetSegmentR
//---------------------------------------------------------

Segment* Measure::undoGetSegmentR(SegmentType type, const Fraction& t)
      {
      Segment* s = findSegmentR(type, t);
      if (s == 0) {
            s = new Segment(this, type, t);
            score()->undoAddElement(s);
            }
      return s;
      }

//---------------------------------------------------------
//   findFirstR
//    return first segment of type st at relative
//    position t.
//---------------------------------------------------------

Segment* Measure::findFirstR(SegmentType st, const Fraction& t) const
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
///   Get a segment of type st at relative tick position t.
///   If the segment does not exist, it is created.
//---------------------------------------------------------

Segment* Measure::getSegmentR(SegmentType st, const Fraction& t)
      {
      Segment* s = findSegmentR(st, t);
      if (!s) {
            s = new Segment(this, st, t);
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
      ElementType type = e->type();

      switch (type) {
            case ElementType::SEGMENT:
                  {
                  Segment* seg   = toSegment(e);
                  Fraction t     = seg->rtick();
                  SegmentType st = seg->segmentType();
                  Segment* s;

                  for (s = first(); s && s->rtick() < t; s = s->next())
                        ;
                  while (s && s->rtick() == t) {
                        if (!seg->isChordRestType() && (seg->segmentType() == s->segmentType())) {
                              qDebug("there is already a <%s> segment", seg->subTypeName());
                              return;
                              }
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

            case ElementType::MEASURE_NUMBER:
                  if (e->staffIdx() < int(_mstaves.size())) {
                        if (e->isStyled(Pid::OFFSET))
                              e->setOffset(e->propertyDefault(Pid::OFFSET).toPointF());
                        _mstaves[e->staffIdx()]->setNoText(toMeasureNumber(e));
                        }
                  break;

            case ElementType::MMREST_RANGE:
                  if (e->staffIdx() < int(_mstaves.size())) {
                        if (e->isStyled(Pid::OFFSET))
                              e->setOffset(e->propertyDefault(Pid::OFFSET).toPointF());
                        _mstaves[e->staffIdx()]->setMMRangeText(toMMRestRange(e));
                        }
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
                  sp->setGap(sp->gap());  // trigger relayout
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

            case ElementType::STAFFTYPE_CHANGE:
                  {
                  StaffTypeChange* staffTypeChange = toStaffTypeChange(e);
                  const StaffType* templateStaffType = staffTypeChange->staffType();

                  Staff* staff = staffTypeChange->staff();

                  // This will need to point to the stafftype element within the stafftypelist for the staff
                  StaffType* newStaffType = nullptr;

                  if (templateStaffType) {
                        // executed on read, undo/redo, clone
                        // setStaffType adds a copy to stafftypelist and returns a pointer to that element within stafftypelist
                        newStaffType = staff->setStaffType(tick(), *templateStaffType);
                        }
                  else {
                        // executed on add from palette
                        // staffType returns a pointer to the current stafftype element in the list
                        // setStaffType will make a copy and return a pointer to that element within list
                        templateStaffType = staff->staffType(tick());
                        newStaffType = staff->setStaffType(tick(), *templateStaffType);
                        }
                  staff->staffTypeListChanged(tick());
                  staffTypeChange->setStaffType(newStaffType, false);

                  MeasureBase::add(e);
                  }
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

            case ElementType::MEASURE_NUMBER:
                  _mstaves[e->staffIdx()]->setNoText(nullptr);
                  break;

            case ElementType::MMREST_RANGE:
                  _mstaves[e->staffIdx()]->setMMRangeText(nullptr);
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

            case ElementType::STAFFTYPE_CHANGE:
                  {
                  StaffTypeChange* stc = toStaffTypeChange(e);
                  Staff* staff = stc->staff();
                  if (staff) {
                        // st currently points to an list element that is about to be removed
                        // make a copy now to use on undo/redo
                        StaffType* st = new StaffType(*stc->staffType());
                        if (!tick().isZero())
                              staff->removeStaffType(tick());
                        stc->setStaffType(st, true);
                        }
                  MeasureBase::remove(e);
                  }
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

void Measure::moveTicks(const Fraction& diff)
      {
      std::set<Tuplet*> tuplets;
      setTick(tick() + diff);
      for (Segment* segment = last(); segment; segment = segment->prev()) {
            if (segment->segmentType() & (SegmentType::EndBarLine | SegmentType::TimeSigAnnounce))
                  segment->setRtick(ticks());
            else if (segment->isChordRestType()) {
                  // Tuplet ticks are stored as absolute ticks, so they must be adjusted.
                  // But each tuplet must only be adjusted once.
                  for (Element* e : segment->elist()) {
                        if (e) {
                              ChordRest* cr = toChordRest(e);
                              Tuplet* tuplet = cr->tuplet();
                              if (tuplet && tuplets.count(tuplet) == 0) {
                                    tuplet->setTick(tuplet->tick() + diff);
                                    tuplets.insert(tuplet);
                                    }
                              }
                        }
                  }
            }
      tuplets.clear();
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
      Segment* bs = findSegmentR(SegmentType::EndBarLine, ticks());

      for (int i = sStaff; i < eStaff; ++i) {
            Staff* staff = score()->staff(i);
            MStaff* ms   = new MStaff;
            ms->setLines(new StaffLines(score()));
            ms->lines()->setTrack(i * VOICES);
            ms->lines()->setParent(this);
            ms->lines()->setVisible(!staff->invisible(tick()));
            score()->undo(new InsertMStaff(this, ms, i));
            }

      if (!createRest && !ts)
            return;


      // create list of unique staves (only one instance for linked staves):

      QList<int> sl;
      for (int staffIdx = sStaff; staffIdx < eStaff; ++staffIdx) {
            Staff* s = score()->staff(staffIdx);
            if (s->links()) {
                  bool alreadyInList = false;
                  for (int idx : sl) {
                        if (s->links()->contains(score()->staff(idx))) {
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
                  score()->setRest(tick(), staffIdx * VOICES, ticks(), false, 0, _timesig == ticks());

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
                        // no time signature found; use measure timesig to construct one
                        ots = new TimeSig(score());
                        ots->setSig(timesig());
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

            // replicate barline
            if (bs) {
                  BarLine* obl = nullptr;
                  for (unsigned track = 0; track < _mstaves.size() * VOICES; ++track) {
                        Element* e = bs->element(track);
                        if (e && !e->generated()) {
                              obl = toBarLine(e);
                              break;
                              }
                        }
                  if (obl) {
                        BarLine* barline = new BarLine(*obl);
                        barline->setSpanStaff(score()->staff(staffIdx)->barLineSpan());
                        barline->setTrack(staffIdx * VOICES);
                        barline->setParent(bs);
                        barline->setGenerated(false);
                        score()->undoAddElement(barline);
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
      ms->lines()->setVisible(!staff->invisible(tick()));
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
      Element* e            = data.dropElement;

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

            case ElementType::MEASURE_NUMBER:
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
      Element* e = data.dropElement;
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
                  e->setOffset(uo);
                  }
                  score()->undoAddElement(e);
                  return e;

            case ElementType::MEASURE_NUMBER:
                  undoChangeProperty(Pid::MEASURE_NUMBER_MODE, static_cast<int>(MeasureNumberMode::SHOW));
                  delete e;
                  break;

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
                  Selection sel = score()->selection();
                  if (sel.isRange())
                        score()->undoAddBracket(staff, level, b->bracketType(), sel.staffEnd() - sel.staffStart());
                  else
                        score()->undoAddBracket(staff, level, b->bracketType(), 1);
                  delete b;
                  }
                  break;

            case ElementType::CLEF:
                  score()->undoChangeClef(staff, this, toClef(e)->clefType());
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
                  score()->cmdAddTimeSig(this, staffIdx, toTimeSig(e), data.modifiers & Qt::ControlModifier);
                  break;

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
                        const int nextVisStaffIdx = s->nextVisibleStaff(staffIdx);
                        const bool systemEnd = (nextVisStaffIdx == score()->nstaves());
                        if (systemEnd) {
                              System* ns = 0;
                              for (System* ts : score()->systems()) {
                                    if (ns) {
                                          ns = ts;
                                          break;
                                          }
                                    if (ts  == s)
                                          ns = ts;
                                    }
                              if (ns == s) {
                                    qreal y1 = s->staffYpage(staffIdx);
                                    qreal y2 = s->page()->height() - s->page()->bm();
                                    gap = y2 - y1 - score()->staff(staffIdx)->height();
                              } else if (ns && ns->page() == s->page()) {
                                    qreal y1 = s->staffYpage(staffIdx);
                                    qreal y2 = ns->staffYpage(0);
                                    gap = y2 - y1 - score()->staff(staffIdx)->height();
                                    }
                              }
                        else {
                              qreal y1 = s->staffYpage(staffIdx);
                              qreal y2 = s->staffYpage(nextVisStaffIdx);
                              gap = y2 - y1 - score()->staff(staffIdx)->height();
                              }
                        spacer->setGap(gap);
                        }
                  score()->undoAddElement(spacer);
                  triggerLayout();
                  return spacer;
                  }

            case ElementType::BAR_LINE:
                  {
                  BarLine* bl = toBarLine(e);

                  // if dropped bar line refers to span rather than to subtype
                  // or if Ctrl key used
                  if ((bl->spanFrom() && bl->spanTo()) || data.control()) {
                        // get existing bar line for this staff, and drop the change to it
                        seg = undoGetSegmentR(SegmentType::EndBarLine, ticks());
                        BarLine* cbl = toBarLine(seg->element(staffIdx * VOICES));
                        if (cbl)
                              cbl->drop(data);
                        }
                  else if (bl->barLineType() == BarLineType::START_REPEAT) {
                        Measure* m2 = isMMRest() ? mmRestFirst() : this;
                        for (Score* lscore : score()->scoreList()) {
                              Measure* lmeasure = lscore->tick2measure(m2->tick());
                              if (lmeasure)
                                    lmeasure->undoChangeProperty(Pid::REPEAT_START, true);
                              }
                        }
                  else if (bl->barLineType() == BarLineType::END_REPEAT) {
                        Measure* m2 = isMMRest() ? mmRestLast() : this;
                        for (Score* lscore : score()->scoreList()) {
                              Measure* lmeasure = lscore->tick2measure(m2->tick());
                              if (lmeasure)
                                    lmeasure->undoChangeProperty(Pid::REPEAT_END, true);
                              }
                        }
                  else if (bl->barLineType() == BarLineType::END_START_REPEAT) {
                        Measure* m2 = isMMRest() ? mmRestLast() : this;
                        for (Score* lscore : score()->scoreList()) {
                              Measure* lmeasure = lscore->tick2measure(m2->tick());
                              if (lmeasure) {
                                    lmeasure->undoChangeProperty(Pid::REPEAT_END, true);
                                    lmeasure = lmeasure->nextMeasure();
                                    if (lmeasure)
                                          lmeasure->undoChangeProperty(Pid::REPEAT_START, true);
                                    }
                              }
                        }
                  else {
                        // drop to first end barline
                        seg = findSegmentR(SegmentType::EndBarLine, ticks());
                        if (seg) {
                              for (Element* ee : seg->elist()) {
                                    if (ee) {
                                          ee->drop(data);
                                          break;
                                          }
                                    }
                              }
                        else
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
                  e->setParent(this);
                  e->setTrack(staffIdx * VOICES);
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
      rm->setTicks(stretchedLen(score()->staff(staffIdx)));
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
      Fraction ol   = ticks();
      Fraction nl   = nf;
      Fraction diff = nl - ol;

      Fraction startTick = endTick();
      if (diff < Fraction(0,1))
            startTick += diff;

      score()->undoInsertTime(startTick, diff);
      score()->undo(new InsertTime(score(), startTick, diff));

      for (Score* s : score()->scoreList()) {
            Measure* m = s->tick2measure(tick());
            s->undo(new ChangeMeasureLen(m, nf));
            if (nl > ol) {
                  // move EndBarLine, TimeSigAnnounce, KeySigAnnounce
                  for (Segment* seg = m->first(); seg; seg = seg->next()) {
                        if (seg->segmentType() & (SegmentType::EndBarLine|SegmentType::TimeSigAnnounce|SegmentType::KeySigAnnounce)) {
                              seg->setRtick(nl);
                              }
                        }
                  }
            }
      Score* s      = score()->masterScore();
      Measure* m    = s->tick2measure(tick());
      QList<int> sl = s->uniqueStaves();

      for (int staffIdx : qAsConst(sl)) {
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
            Fraction stretch = s->staff(staffIdx)->timeStretch(tick());
            // if just a single rest
            if (rests == 1 && chords == 0) {
                  // if measure value didn't change, stick to whole measure rest
                  if (_timesig == nf) {
                        rest->undoChangeProperty(Pid::DURATION, QVariant::fromValue<Fraction>(nf * stretch));
                        rest->undoChangeProperty(Pid::DURATION_TYPE, QVariant::fromValue<TDuration>(TDuration::DurationType::V_MEASURE));
                        }
                  else {      // if measure value did change, represent with rests actual measure value
#if 0
                        // any reason not to do this instead?
                        s->undoRemoveElement(rest);
                        s->setRest(tick(), staffIdx * VOICES, nf * stretch, false, 0, false);
#else
                        // convert the measure duration in a list of values (no dots for rests)
                        std::vector<TDuration> durList = toDurationList(nf * stretch, false, 0);

                        rest->undoChangeProperty(Pid::DURATION, QVariant::fromValue<Fraction>(durList[0].fraction()));
                        rest->undoChangeProperty(Pid::DURATION_TYPE, QVariant::fromValue<TDuration>(durList[0]));

                        // add rests for any other duration list value
                        Fraction tickOffset = tick() + rest->actualTicks();
                        for (unsigned i = 1; i < durList.size(); i++) {
                              Rest* newRest = new Rest(s);
                              newRest->setDurationType(durList.at(i));
                              newRest->setTicks(durList.at(i).fraction());
                              newRest->setTrack(rest->track());
                              score()->undoAddCR(newRest, this, tickOffset);
                              tickOffset += newRest->actualTicks();
                              }
#endif
                        }
                  continue;
                  }

            int strack = staffIdx * VOICES;
            int etrack = strack + VOICES;

            for (int trk = strack; trk < etrack; ++trk) {
                  Fraction n = diff;
                  bool rFlag = false;
                  if (n < Fraction(0,1))  {
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
                                                Fraction actualTicks = cr->actualTicks();
                                                n += actualTicks;
                                                cr->setDurationType(TDuration(actualTicks));
                                                }
                                          else
                                                n += cr->actualTicks();
                                          s->undoRemoveElement(e);
                                          if (n >= Fraction(0,1))
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
                  if (appendRestsIfNecessary && (n > Fraction(0,1)) && (rFlag || voice == 0)) {
                        // add rest to measure
                        Fraction rtick = tick() + nl - n;
                        int track = staffIdx * VOICES + voice;
                        s->setRest(rtick, track, n * stretch, false, 0, false);
                        }
                  }
            }
      if (diff < Fraction(0,1)) {
            //
            //  CHECK: do not remove all slurs
            //
            for (Element* e : m->el()) {
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
      if (MScore::debugMode) {
            const int mno = no() + 1;
            xml.comment(QString("Measure %1").arg(mno));
            }
      if (_len != _timesig) {
            // this is an irregular measure
            xml.stag(this, QString("len=\"%1/%2\"").arg(_len.numerator()).arg(_len.denominator()));
            }
      else
            xml.stag(this);

      xml.setCurTick(tick());
      xml.setCurTrack(staff * VOICES);

      if (_mmRestCount > 0)
            xml.tag("multiMeasureRest", _mmRestCount);
      if (writeSystemElements) {
            if (repeatStart())
                  xml.tagE("startRepeat");
            if (repeatEnd())
                  xml.tag("endRepeat", _repeatCount);
            writeProperty(xml, Pid::IRREGULAR);
            writeProperty(xml, Pid::BREAK_MMR);
            writeProperty(xml, Pid::USER_STRETCH);
            writeProperty(xml, Pid::NO_OFFSET);
            writeProperty(xml, Pid::MEASURE_NUMBER_MODE);
            }
      qreal _spatium = spatium();
      MStaff* mstaff = _mstaves[staff];
      if (mstaff->noText() && !mstaff->noText()->generated())
            mstaff->noText()->write(xml);

      if (mstaff->mmRangeText() && !mstaff->mmRangeText()->generated())
            mstaff->mmRangeText()->write(xml);

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
      if (mstaff->stemless()) {
            xml.tag("slashStyle", mstaff->stemless()); // for backwards compatibility
            xml.tag("stemless", mstaff->stemless());
            }

      int strack = staff * VOICES;
      int etrack = strack + VOICES;
      for (const Element* e : el()) {
            if (!e->generated() && ((e->staffIdx() == staff) || (e->systemFlag() && writeSystemElements)))
                  e->write(xml);
            }
      Q_ASSERT(first());
      Q_ASSERT(last());
      if (first() && last())
            score()->writeSegments(xml, strack, etrack, first(), last()->next1(), writeSystemElements, forceTimeSig);

      xml.etag();
      }

//---------------------------------------------------------
//   Measure::read
//---------------------------------------------------------

void Measure::read(XmlReader& e, int staffIdx)
      {
      qreal _spatium = spatium();
      e.setCurrentMeasure(this);
      int nextTrack = staffIdx * VOICES;
      e.setTrack(nextTrack);

      for (int n = int(_mstaves.size()); n <= staffIdx; ++n) {
            Staff* staff = score()->staff(n);
            MStaff* s    = new MStaff;
            s->setLines(new StaffLines(score()));
            s->lines()->setParent(this);
            s->lines()->setTrack(n * VOICES);
            s->lines()->setVisible(!staff->invisible(tick()));
            _mstaves.push_back(s);
            }

      bool irregular;
      if (e.hasAttribute("len")) {
            QStringList sl = e.attribute("len").split('/');
            if (sl.size() == 2)
                  _len = Fraction(sl[0].toInt(), sl[1].toInt());
            else
                  qDebug("illegal measure size <%s>", qPrintable(e.attribute("len")));
            irregular = true;
            if (_len.numerator() <= 0 || _len.denominator() <= 0 || _len.denominator() > 128) {
                  e.raiseError(QObject::tr("MSCX error at line %1: invalid measure length: %2").arg(e.lineNumber()).arg(_len.toString()));
                  return;
                  }
            score()->sigmap()->add(tick().ticks(), SigEvent(_len, _timesig));
            score()->sigmap()->add((tick() + ticks()).ticks(), SigEvent(_timesig));
            }
      else
            irregular = false;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "voice") {
                  e.setTrack(nextTrack++);
                  e.setTick(tick());
                  readVoice(e, staffIdx, irregular);
                  }
            else if (tag == "Marker" || tag == "Jump") {
                  Element* el = Element::name2Element(tag, score());
                  el->setTrack(e.track());
                  el->read(e);
                  add(el);
                  }
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
            else if ((tag == "slashStyle") || (tag == "stemless"))
                  _mstaves[staffIdx]->setStemless(e.readInt());
            else if (tag == "SystemDivider") {
                  SystemDivider* sd = new SystemDivider(score());
                  sd->read(e);
                  add(sd);
                  }
            else if (tag == "multiMeasureRest") {
                  _mmRestCount = e.readInt();
                  // set tick to previous measure
                  setTick(e.lastMeasure()->tick());
                  e.setTick(e.lastMeasure()->tick());
                  }
            else if (tag == "MeasureNumber") {
                  MeasureNumber* noText = new MeasureNumber(score());
                  noText->read(e);
                  noText->setTrack(e.track());
                  add(noText);
                  }
            else if (tag == "MMRestRange") {
                  MMRestRange* range = new MMRestRange(score());
                  range->read(e);
                  range->setTrack(e.track());
                  add(range);
                  }
            else if (MeasureBase::readProperties(e))
                  ;
            else
                  e.unknown();
            }
      e.checkConnectors();
      if (isMMRest()) {
            Measure* lm = e.lastMeasure();
            e.setTick(lm->tick() + lm->ticks());
            }
      e.setCurrentMeasure(nullptr);

      connectTremolo();
      }

//---------------------------------------------------------
//   Measure::readVoice
//---------------------------------------------------------

void Measure::readVoice(XmlReader& e, int staffIdx, bool irregular)
      {
      Segment* segment = nullptr;
      QList<Chord*> graceNotes;
      Beam* startingBeam = nullptr;
      Tuplet* tuplet = nullptr;
      Fermata* fermata = nullptr;

      Staff* staff = score()->staff(staffIdx);
      Fraction timeStretch(staff->timeStretch(tick()));

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "location") {
                  Location loc = Location::relative();
                  loc.read(e);
                  e.setLocation(loc);
                  }
            else if (tag == "tick") {           // obsolete?
                  qDebug("read midi tick");
                  e.setTick(Fraction::fromTicks(score()->fileDivision(e.readInt())));
                  }
            else if (tag == "BarLine") {
                  BarLine* barLine = new BarLine(score());
                  barLine->setTrack(e.track());
                  barLine->read(e);
                  //
                  //  StartRepeatBarLine: at rtick == 0, always BarLineType::START_REPEAT
                  //  BarLine:            in the middle of a measure, has no semantic
                  //  EndBarLine:         at the end of a measure
                  //  BeginBarLine:       first segment of a measure, systemic barline

                  SegmentType st = SegmentType::Invalid;
                  Fraction t = e.tick() - tick();
                  if (t.isNotZero() && (t != ticks()))
                        st = SegmentType::BarLine;
                  else if (barLine->barLineType() == BarLineType::START_REPEAT && t.isZero())
                        st = SegmentType::StartRepeatBarLine;
                  else if (barLine->barLineType() == BarLineType::START_REPEAT && t == ticks()) {
                        // old version, ignore
                        delete barLine;
                        barLine = 0;
                        }
                  else if (t.isZero() && segment == 0)
                        st = SegmentType::BeginBarLine;
                  else
                        st = SegmentType::EndBarLine;
                  if (barLine) {
                        segment = getSegmentR(st, t);
                        segment->add(barLine);
                        barLine->layout();
                        }
                  if (fermata) {
                        segment->add(fermata);
                        fermata = nullptr;
                        }
                  }
            else if (tag == "Chord") {
                  Chord* chord = new Chord(score());
                  chord->setTrack(e.track());
                  chord->read(e);
                  if (startingBeam) {
                        startingBeam->add(chord); // also calls chord->setBeam(startingBeam)
                        startingBeam = nullptr;
                        }
//                  if (tuplet && !chord->isGrace())
//                        chord->readAddTuplet(tuplet);
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
                        if (tuplet)
                              tuplet->add(chord);
                        e.incTick(chord->actualTicks());
                        }
                  if (fermata) {
                        segment->add(fermata);
                        fermata = nullptr;
                        }
                  }
            else if (tag == "Rest") {
                  Rest* rest = new Rest(score());
                  rest->setDurationType(TDuration::DurationType::V_MEASURE);
                  rest->setTicks(timesig()/timeStretch);
                  rest->setTrack(e.track());
                  rest->read(e);
                  if (startingBeam) {
                        startingBeam->add(rest); // also calls rest->setBeam(startingBeam)
                        startingBeam = nullptr;
                        }
                  segment = getSegment(SegmentType::ChordRest, e.tick());
                  segment->add(rest);
                  if (fermata) {
                        segment->add(fermata);
                        fermata = nullptr;
                        }

                  if (!rest->ticks().isValid())     // hack
                        rest->setTicks(timesig()/timeStretch);

                  if (tuplet)
                        tuplet->add(rest);
                  e.incTick(rest->actualTicks());
                  }
            else if (tag == "Breath") {
                  Breath* breath = new Breath(score());
                  breath->setTrack(e.track());
                  breath->setPlacement(breath->track() & 1 ? Placement::BELOW : Placement::ABOVE);
                  breath->read(e);
                  segment = getSegment(SegmentType::Breath, e.tick());
                  segment->add(breath);
                  }
            else if (tag == "Spanner")
                  Spanner::readSpanner(e, this, e.track());
            else if (tag == "RepeatMeasure" || tag == "MeasureRepeat" ) {
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
                        for (Segment* s = _segments.first(); s && s->rtick().isZero(); s = s->next()) {
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
                  // if time sig not at beginning of measure => courtesy time sig
                  Fraction currTick = e.tick();
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
                              score()->sigmap()->add(tick().ticks(), SigEvent(_len, _timesig));
                              score()->sigmap()->add((tick() + ticks()).ticks(), SigEvent(_timesig));
                              }
                        else {
                              _len = _timesig;
                              score()->sigmap()->add(tick().ticks(), SigEvent(_timesig));
                              }
                        }
                  }
            else if (tag == "KeySig") {
                  KeySig* ks = new KeySig(score());
                  ks->setTrack(e.track());
                  ks->read(e);
                  Fraction curTick = e.tick();
                  if (!ks->isCustom() && !ks->isAtonal() && ks->key() == Key::C && curTick.isZero()) {
                        // ignore empty key signature
                        qDebug("remove keysig c at tick 0");
                        }
                  else {
                        // if key sig not at beginning of measure => courtesy key sig
                        bool courtesySig = (curTick == endTick());
                        segment = getSegment(courtesySig ? SegmentType::KeySigAnnounce : SegmentType::KeySig, curTick);
                        segment->add(ks);
                        if (!courtesySig)
                              staff->setKey(curTick, ks->keySigEvent());
                        }
                  }
            else if (tag == "Text") {
                  StaffText* t = new StaffText(score());
                  t->setTrack(e.track());
                  t->read(e);
                  if (t->empty()) {
                        qDebug("==reading empty text: deleted");
                        delete t;
                        }
                  else {
                        segment = getSegment(SegmentType::ChordRest, e.tick());
                        segment->add(t);
                        }
                  }
            else if (tag == "Expression") { // 4.x compat
                  StaffText* t = new StaffText(score(), Tid::EXPRESSION);
                  t->setTrack(e.track());
                  t->read(e);
                  if (t->isStyled(Pid::PLACEMENT)) {
                        t->setPlacement(Placement::BELOW);
                        t->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
                        }
                  if (t->empty()) {
                        qDebug("==reading empty text: deleted");
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
               || tag == "Sticking"
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
            else if (tag == "Fermata") {
                  fermata = new Fermata(score());
                  fermata->setTrack(e.track());
                  fermata->setPlacement(fermata->track() & 1 ? Placement::BELOW : Placement::ABOVE);
                  fermata->read(e);
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
            else if (tag == "Tuplet") {
                  Tuplet* oldTuplet = tuplet;
                  tuplet = new Tuplet(score());
                  tuplet->setTrack(e.track());
                  tuplet->setTick(e.tick());
                  tuplet->setParent(this);
                  tuplet->read(e);
                  if (oldTuplet)
                        oldTuplet->add(tuplet);
                  }
            else if (tag == "endTuplet") {
                  if (!tuplet) {
                        qDebug("Measure::read: encountered <endTuplet/> when no tuplet was started");
                        e.skipCurrentElement();
                        continue;
                        }
                  Tuplet* oldTuplet = tuplet;
                  tuplet = tuplet->tuplet();
                  if (oldTuplet->elements().empty()) {
                        // this should not happen and is a sign of input file corruption
                        qDebug("Measure:read: empty tuplet in measure index=%d, input file corrupted?", e.currentMeasureIndex());
                        if (tuplet)
                              tuplet->remove(oldTuplet);
                        delete oldTuplet;
                        }
                  e.readNext();
                  }
            else if (tag == "Beam") {
                  Beam* beam = new Beam(score());
                  beam->setTrack(e.track());
                  beam->read(e);
                  beam->setParent(0);
                  if (startingBeam) {
                        qDebug("The read beam was not used");
                        delete startingBeam;
                        }
                  startingBeam = beam;
                  }
            else if (tag == "Segment" && segment)
                  segment->read(e);
            else if (tag == "Ambitus") {
                  Ambitus* range = new Ambitus(score());
                  range->read(e);
                  segment = getSegment(SegmentType::Ambitus, e.tick());
                  range->setParent(segment);          // a parent segment is needed for setTrack() to work
                  range->setTrack(trackZeroVoice(e.track()));
                  segment->add(range);
                  }
            else
                  e.unknown();
            }
      if (startingBeam) {
            qDebug("The read beam was not used");
            delete startingBeam;
            }
      if (tuplet) {
            qDebug("Measure:readVoice: measure index=%d, <endTuplet/> not found", e.currentMeasureIndex());
            if (tuplet->elements().empty()) {
                  if (tuplet->tuplet())
                        tuplet->tuplet()->remove(tuplet);
                  delete tuplet;
                  }
            }
      if (fermata) {
            SegmentType st = (e.tick() == endTick() ? SegmentType::EndBarLine : SegmentType::ChordRest);
            segment = getSegment(st, e.tick());
            segment->add(fermata);
            fermata = nullptr;
            }
      }

//---------------------------------------------------------
//   Measure::readAddConnector
//---------------------------------------------------------

void Measure::readAddConnector(ConnectorInfoReader* info, bool pasteMode)
      {
      const ElementType type = info->type();
      switch(type) {
            case ElementType::HAIRPIN:
            case ElementType::PEDAL:
            case ElementType::OTTAVA:
            case ElementType::TRILL:
            case ElementType::TEXTLINE:
            case ElementType::LET_RING:
            case ElementType::VIBRATO:
            case ElementType::PALM_MUTE:
            case ElementType::VOLTA:
                  {
                  Spanner* sp = toSpanner(info->connector());
                  const Location& l = info->location();
                  Fraction lTick    = l.frac();
                  Fraction spTick   = pasteMode ? lTick : (tick() + lTick);
                  if (info->isStart()) {
                        sp->setTrack(l.track());
                        sp->setTick(spTick);
                        score()->addSpanner(sp);
                        }
                  else if (info->isEnd()) {
                        sp->setTrack2(l.track());
                        sp->setTick2(spTick);
                        }
                  }
                  break;
            default:
                  break;
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
      if (score()->staff(staffIdx)->cutaway() && isEmpty(staffIdx))
            return false;
      return score()->staff(staffIdx)->show() && _mstaves[staffIdx]->visible();
      }

//---------------------------------------------------------
//   stemless
//---------------------------------------------------------

bool Measure::stemless(int staffIdx) const
      {
      const Staff* staff = score()->staff(staffIdx);
      return staff->stemless(tick()) || _mstaves[staffIdx]->stemless() || staff->staffType(tick())->stemless();
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
      TimeSigFrac timeSig = score()->sigmap()->timesig(tick().ticks()).nominal();
      return irregular() && ticks() < Fraction::fromTicks(timeSig.ticksPerMeasure());
      }

//---------------------------------------------------------
//   isFirstInSystem
//---------------------------------------------------------

bool Measure::isFirstInSystem() const
      {
      IF_ASSERT_FAILED(system()) {
            return false;
            }
      return system()->firstMeasure() == this;
      }

//---------------------------------------------------------
//   isFirstInSection
//---------------------------------------------------------

bool Measure::isFirstInSection() const
      {
      Measure* prevMeasure = this->prevMeasure();
      return !prevMeasure || prevMeasure->sectionBreak();
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Measure::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      MeasureBase::scanElements(data, func, all);

      int nstaves = score()->nstaves();
      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            if (!all && !score()->staff(staffIdx)->show())
                  continue;
            MStaff* ms = _mstaves[staffIdx];
            // show spacers and measure number even on invisible measures (TO DO: also include Staff Type Changes)
            if (ms->vspacerUp())
                  func(data, ms->vspacerUp());
            if (ms->vspacerDown())
                  func(data, ms->vspacerDown());
            if (ms->noText())
                  func(data, ms->noText());
            if (ms->mmRangeText())
                  func(data, ms->mmRangeText());
            // show staff lines only if measure is visible OR if it only has a courtesy clef for cutaway/ossias (short staff lines will be drawn if needed)
            if (visible(staffIdx) || isCutawayClef(staffIdx))
                  func(data, ms->lines());
            }

      for (Segment* s = first(); s; s = s->next()) {
            if (!s->enabled())
                  continue;
            s->scanElements(data, func, all);
            }
      }

//---------------------------------------------------------
//   connectTremolo
///   Connect two-notes tremolo and update duration types
///   for the involved chords.
//---------------------------------------------------------

void Measure::connectTremolo()
      {
      const int ntracks = score()->ntracks();
      constexpr SegmentType st = SegmentType::ChordRest;
      for (Segment* s = first(st); s; s = s->next(st)) {
            for (int i = 0; i < ntracks; ++i) {
                  Element* e = s->element(i);
                  if (!e || !e->isChord())
                        continue;

                  Chord* c = toChord(e);
                  Tremolo* tremolo = c->tremolo();
                  if (tremolo && tremolo->twoNotes()) {
                        // Ensure correct duration type for chord
                        c->setDurationType(tremolo->durationType());

                        // If it is the first tremolo's chord, find the second
                        // chord for tremolo, if needed.
                        if (!tremolo->chord1())
                              tremolo->setChords(c, tremolo->chord2());
                        else if (tremolo->chord1() != c || tremolo->chord2())
                              continue;

                        for (Segment* ls = s->next(st); ls; ls = ls->next(st)) {
                              if (Element* element = ls->element(i)) {
                                    if (!element->isChord()) {
                                          qDebug("cannot connect tremolo");
                                          continue;
                                          }
                                    Chord* nc = toChord(element);
                                    tremolo->setChords(c, nc);
                                    nc->setTremolo(tremolo);
                                    break;
                                    }
                              }
                        }
                  }
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
                  score()->setRest(s->tick(), track, ticks(), true, 0);
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

      auto spanners = score()->spannerMap().findOverlapping(tick().ticks(), endTick().ticks()-1);
      Fraction start = tick();
      Fraction end = start + ticks();
      for (auto i = spanners.begin(); i < spanners.end(); i++) {
            Spanner* sp = i->value;
            Fraction spStart = sp->tick();
            Fraction spEnd = spStart + sp->ticks();
            qDebug("Start %d End %d Diff %d \n Measure Start %d End %d", spStart.ticks(), spEnd.ticks(), (spEnd-spStart).ticks(), start.ticks(), end.ticks());
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
      if (hasVoices(staffIdx, tick(), ticks()))
            _mstaves[staffIdx]->setHasVoices(true);
      else
            _mstaves[staffIdx]->setHasVoices(false);
      }

//---------------------------------------------------------
//   hasVoices
//---------------------------------------------------------

bool Measure::hasVoices(int staffIdx, Fraction stick, Fraction len) const
      {
      Staff* st = score()->staff(staffIdx);
      if (st->isTabStaff(stick)) {
            // TODO: tab staves use different rules for stem directin etc
            // see for example https://musescore.org/en/node/308371
            // we should consider coming up with a more comprehensive solution
            // but for now, we are forcing measures on tab staves to be consider as a whole -
            // either they have voices or not
            // (rather than checking tick ranges)
            stick = tick();
            len = stretchedLen(st);
            }
      int strack = staffIdx * VOICES + 1;
      int etrack = staffIdx * VOICES + VOICES;
      Fraction etick = stick + len;

      for (Segment* s = first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            if (s->tick() >= etick)
                  break;
            for (int track = strack; track < etrack; ++track) {
                  ChordRest* cr = toChordRest(s->element(track));
                  if (cr) {
                        if (cr->tick() + cr->actualTicks() <= stick)
                              continue;
                        bool v = false;
                        if (cr->isChord()) {
                              Chord* c = toChord(cr);
                              // consider a chord visible if stem, hook(s) or beam(s) are visible
                              if ((c->stem() && c->stem()->visible()) ||
                                  (c->hook() && c->hook()->visible()) ||
                                  (c->beam() && c->beam()->visible())) {
                                    v = true;
                                    }
                              else {
                                    // or any of its notes
                                    for (Note* n : c->notes()) {
                                          if (n->visible()) {
                                                v = true;
                                                break;
                                                }
                                          }
                                    }
                              }
                        else if (cr->isRest())
                              v = cr->visible() && !toRest(cr)->isGap();
                        if (v)
                              return true;
                        }
                  }
            }
      return false;
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
//   isEmpty
///   Check if the measure is filled by a full-measure rest, or is
///   full of rests on this staff, that may have fermatas on them.
///   If staff is -1, then check for all staves.
//-------------------------------------------------------------------

bool Measure::isEmpty(int staffIdx) const
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
                  // Check for cross-staff chords
                  bool hasStaves = score()->staff(track / VOICES)->part()->staves()->size() > 1;
                  if (hasStaves) {
                        if (strack >= VOICES) {
                              e = s->element(track - VOICES);
                              if (e && !e->isRest() && e->vStaffIdx() == staffIdx)
                                    return false;
                              }
                        if (etrack < score()->nstaves() * VOICES) {
                              e = s->element(track + VOICES);
                              if (e && !e->isRest() && e->vStaffIdx() == staffIdx)
                                    return false;
                              }
                        }
                  }
            for (Element* a : s->annotations()) {
                  if (a && staffIdx < 0)
                        return false;
                  if (!a || a->systemFlag() || !a->visible() || a->isFermata())
                        continue;
                  int atrack = a->track();
                  if (atrack >= strack && atrack < etrack)
                        return false;
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   isCutawayClef
///    Check for empty measure with only
///    a Courtesy Clef before End Bar Line
//---------------------------------------------------------

bool Measure::isCutawayClef(int staffIdx) const
      {
      if (!score()->staff(staffIdx) || !_mstaves[staffIdx])
            return false;
      bool empty = (score()->staff(staffIdx)->cutaway() && isEmpty(staffIdx)) || !_mstaves[staffIdx]->visible();
      if (!empty)
            return false;
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
      // find segment before EndBarLine
      Segment* s = nullptr;
      for (Segment* ls = last(); ls; ls = ls->prev()) {
            if (ls->segmentType() ==  SegmentType::EndBarLine) {
                  s = ls->prev();
                  break;
                  }
            }
      if (!s)
            return false;
      for (int track = strack; track < etrack; ++track) {
            Element* e = s->element(track);
            if (!e || !e->isClef())
                  continue;
            if ((nextMeasure() && (nextMeasure()->system() == system())) || toClef(e)->showCourtesy())
                  return true;
            }            
      return false;
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

bool Measure::isRepeatMeasure(const Staff* staff) const
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
      int tracks = int(_mstaves.size()) * VOICES;
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
      return ticks() * staff->timeStretch(tick());
      }

//---------------------------------------------------------
//   cloneMeasure
//---------------------------------------------------------

Measure* Measure::cloneMeasure(Score* sc, const Fraction& tick, TieMap* tieMap)
      {
      Measure* m      = new Measure(sc);
      m->_timesig     = _timesig;
      m->_len         = _len;
      m->_repeatCount = _repeatCount;

      Q_ASSERT(sc->staves().size() >= int(_mstaves.size())); // destination score we're cloning into must have at least as many staves as measure being cloned

      m->setNo(no());
      m->setNoOffset(noOffset());
      m->setIrregular(irregular());
      m->_userStretch           = _userStretch;
      m->_breakMultiMeasureRest = _breakMultiMeasureRest;
      m->_playbackCount         = _playbackCount;

      m->setTick(tick);
      m->setLineBreak(lineBreak());
      m->setPageBreak(pageBreak());
      m->setSectionBreak(sectionBreak() ? new LayoutBreak(*sectionBreakElement()) : 0);

      m->setHeader(header()); m->setTrailer(trailer());

      int tracks = sc->nstaves() * VOICES;
      TupletMap tupletMap;

      for (Segment* oseg = first(); oseg; oseg = oseg->next()) {
            Segment* s = new Segment(m, oseg->segmentType(), oseg->rtick());
            s->setEnabled(oseg->enabled()); s->setVisible(oseg->visible());
            s->setHeader(oseg->header()); s->setTrailer(oseg->trailer());

            m->_segments.push_back(s);
            for (int track = 0; track < tracks; ++track) {
                  Element* oe = oseg->element(track);
                  for (Element* e : oseg->annotations()) {
                        if (e->generated() || e->track() != track)
                              continue;
                        Element* ne = e->clone();
                        ne->setTrack(track);
                        ne->setOffset(e->offset());
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
                                    nt->setParent(m);
                                    nt->setTick(m->tick() + ot->rtick());
                                    tupletMap.add(ot, nt);
                                    }
                              ncr->setTuplet(nt);
                              nt->add(ncr);
                              }
                        if (oe->isChord()) {
                              Chord* och = toChord(ocr);
                              Chord* nch = toChord(ncr);
                              size_t n = och->notes().size();
                              for (size_t i = 0; i < n; ++i) {
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
                  ne->setOffset(oe->offset());
                  ne->setScore(sc);
                  s->add(ne);
                  }
            }
      for (Element* e : el()) {
            Element* ne = e->clone();
            ne->setScore(sc);
            ne->setOffset(e->offset());
            m->add(ne);
            }
      return m;
      }

//---------------------------------------------------------
//   snap
//---------------------------------------------------------

Fraction Measure::snap(const Fraction& tick, const QPointF p) const
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

Fraction Measure::snapNote(const Fraction& /*tick*/, const QPointF p, int staff) const
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
//   searchSegment
///   Finds a segment which x position is most close to the
///   given \p x.
///   \param x The x coordinate in measure coordinates.
///   \param st Type of segments to search.
///   \param strack start of track range (strack included)
///   in which the found segment should contain elements.
///   \param etrack end of track range (etrack excluded)
///   in which the found segment should contain elements.
///   \param preferredSegment If not nullptr, will give
///   more space to the given segment when searching it by
///   coordinate.
///   \returns The segment that was found.
//---------------------------------------------------------

Segment* Measure::searchSegment(qreal x, SegmentType st, int strack, int etrack, const Segment* preferredSegment, qreal spacingFactor) const
      {
      const int lastTrack = etrack - 1;
      for (Segment* segment = first(st); segment; segment = segment->next(st)) {
            if (!segment->hasElements(strack, lastTrack))
                  continue;
            Segment* ns = segment->next(st);
            for (; ns; ns = ns->next(st)) {
                  if (ns->hasElements(strack, lastTrack))
                        break;
                  }
            if (!ns)
                  return segment;
            if (preferredSegment == segment) {
                  if (x < (segment->x() + (ns->x() - segment->x())))
                        return segment;
                  }
            else if (preferredSegment == ns) {
                  if (x <= segment->x())
                        return segment;
                  }
            else {
                  if (x < (segment->x() + (ns->x() - segment->x()) * spacingFactor))
                        return segment;
                  }
            }
      return nullptr;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Measure::getProperty(Pid propertyId) const
      {
      switch(propertyId) {
            case Pid::TIMESIG_NOMINAL:
                  return QVariant::fromValue(_timesig);
            case Pid::TIMESIG_ACTUAL:
                  return QVariant::fromValue(_len);
            case Pid::MEASURE_NUMBER_MODE:
                  return int(measureNumberMode());
            case Pid::BREAK_MMR:
                  return breakMultiMeasureRest();
            case Pid::REPEAT_COUNT:
                  return repeatCount();
            case Pid::USER_STRETCH:
                  return userStretch();
            default:
                  return MeasureBase::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Measure::setProperty(Pid propertyId, const QVariant& value)
      {
      switch (propertyId) {
            case Pid::TIMESIG_NOMINAL:
                  _timesig = value.value<Fraction>();
                  break;
            case Pid::TIMESIG_ACTUAL:
                  _len = value.value<Fraction>();
                  break;
            case Pid::MEASURE_NUMBER_MODE:
                  setMeasureNumberMode(MeasureNumberMode(value.toInt()));
                  break;
            case Pid::BREAK_MMR:
                  setBreakMultiMeasureRest(value.toBool());
                  break;
            case Pid::REPEAT_COUNT:
                  setRepeatCount(value.toInt());
                  break;
            case Pid::USER_STRETCH:
                  setUserStretch(value.toDouble());
                  break;
            default:
                  return MeasureBase::setProperty(propertyId, value);
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Measure::propertyDefault(Pid propertyId) const
      {
      switch(propertyId) {
            case Pid::TIMESIG_NOMINAL:
            case Pid::TIMESIG_ACTUAL:
                  return QVariant();
            case Pid::MEASURE_NUMBER_MODE:
                  return int(MeasureNumberMode::AUTO);
            case Pid::BREAK_MMR:
                  return false;
            case Pid::REPEAT_COUNT:
                  return 2;
            case Pid::USER_STRETCH:
                  return 1.0;
            case Pid::NO_OFFSET:
                  return 0;
            case Pid::IRREGULAR:
                  return false;
            default:
                  break;
            }
      return MeasureBase::propertyDefault(propertyId);
      }

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void Measure::undoChangeProperty(Pid id, const QVariant& v)
      {
      undoChangeProperty(id, v, propertyFlags(id));
      }

void Measure::undoChangeProperty(Pid id, const QVariant& v, PropertyFlags ps)
      {
      if ((getProperty(id) == v) && (propertyFlags(id) == ps))
            return;
      if (id == Pid::REPEAT_START || id == Pid::REPEAT_END) {
            // change also coresponding mmRestMeasure
            Measure* m = const_cast<Measure*>(toMeasure(this)->coveringMMRestOrThis());
            if (m != this)
                  m->undoChangeProperty(id, v);
            }
      ScoreElement::undoChangeProperty(id, v, ps);
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
//   coveringMMRestOrThis
//    if multi-measure rests are enabled,
//        and this measure is covered by an MMRest,
//            return that MMRest.
//    otherwise, return the measure itself.
//---------------------------------------------------------

const Measure* Measure::coveringMMRestOrThis() const
      {
      if (!score()->styleB(Sid::createMultiMeasureRests))
            return this;

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

      // handle measure elements
      if (e->parent() == this) {
            auto i = std::find(el().begin(), el().end(), e);
            if (i != el().end()) {
                  if (++i != el().end()) {
                        Element* resElement = *i;
                        if (resElement)
                              return resElement;
                        }
                  }
            }

      for (; e && e->type() != ElementType::SEGMENT; e = e->parent()) {
            ;
      }
      Segment* seg = toSegment(e);
      Segment* nextSegment = seg ? seg->next() : first();
      Element* next = seg->firstElementOfSegment(nextSegment, staff);
      if (next)
            return next;

      return score()->lastElement();
      }

//---------------------------------------------------------
//   prevElementStaff
//---------------------------------------------------------

Element* Measure::prevElementStaff(int staff)
      {
      Element* e = score()->selection().element();
      if (!e && !score()->selection().elements().isEmpty())
            e = score()->selection().elements().first();

      // handle measure elements
      if (e->parent() == this) {
            auto i = std::find(el().rbegin(), el().rend(), e);
            if (i != el().rend()) {
                  if (++i != el().rend()) {
                        Element* resElement = *i;
                        if (resElement)
                              return resElement;
                        }
                  }
            }

      Measure* prevM = prevMeasureMM();
      if (prevM) {
            Segment* seg = prevM->last();
            if (seg)
                  return seg->lastElement(staff);
            }
      return score()->firstElement();
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Measure::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo(), QString::number(no() + 1));
      }

//-----------------------------------------------------------------------------
//    stretchMeasure
//    resize width of measure to targetWidth
//-----------------------------------------------------------------------------

void Measure::stretchMeasure(qreal targetWidth)
      {
      bbox().setWidth(targetWidth);

      Fraction minTick = computeTicks();

      //---------------------------------------------------
      //    compute stretch
      //---------------------------------------------------

      std::multimap<qreal, Segment*> springs;

      Segment* seg = first();
      while (seg && (!seg->enabled() || seg->allElementsInvisible()))
            seg = seg->next();
      qreal minimumWidth = seg ? seg->x() : 0.0;
      for (Segment& s : _segments) {
            if (!s.enabled() || !s.visible() || s.allElementsInvisible())
                  continue;
            Fraction t = s.ticks();
            if (t.isNotZero()) {
                  qreal str = 1.0 + 0.865617 * log(qreal(t.ticks()) / qreal(minTick.ticks())); // .6 * log(t / minTick.ticks()) / log(2);
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
            for (auto i = springs.begin(); i != springs.end();) {
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
            while (s && (!s->enabled() || s->allElementsInvisible()))
                  s = s->next();
            qreal x = s ? s->pos().x() : 0.0;
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
                        for (s1 = s.prevActive(); s1 && s1->allElementsInvisible(); s1 = s1->prevActive())
                              ;
                        Segment* s2;
                        for (s2 = s.nextActive(); s2; s2 = s2->nextActive()) {
                              if (!s2->isChordRestType() && s2->element(staffIdx * VOICES))
                                    break;
                              }
                        qreal x1 = s1 ? s1->x() + s1->minRight() : 0;
                        qreal x2 = s2 ? s2->x() - s2->minLeft() : targetWidth;

                        if (isMMRest()) {
                              Rest* rest = toRest(e);
                              //
                              // center multi measure rest
                              //
                              qreal d = score()->styleP(Sid::multiMeasureRestMargin);
                              qreal w = x2 - x1 - 2 * d;

                              rest->layoutMMRest(w);
                              e->setPos(x1 - s.x() + d, e->staff()->height() * .5);   // center vertically in measure
                              s.createShape(staffIdx);
                              }
                        else { // if (rest->isFullMeasureRest()) {
                              //
                              // center full measure rest
                              //
                              e->rxpos() = (x2 - x1 - e->width()) * .5 + x1 - s.x() - e->bbox().x();
                              s.createShape(staffIdx);  // DEBUG
                              }
                        }
                  else if (t == ElementType::REST)
                        e->rxpos() = 0;
                  else if (t == ElementType::CHORD) {
                        Chord* c = toChord(e);
                        c->layout2();
                        if (c->tremolo()) {
                              Tremolo* tr = c->tremolo();
                              Chord* c1 = tr->chord1();
                              Chord* c2 = tr->chord2();
                              if (!tr->twoNotes() || (c1 && !c1->staffMove() && c2 && !c2->staffMove()))
                                    tr->layout();
                              }
                        }
                  else if (t == ElementType::BAR_LINE) {
                        e->rypos() = 0.0;
                        // for end barlines, x position was set in createEndBarLines
                        if (s.segmentType() != SegmentType::EndBarLine)
                              e->rxpos() = 0.0;
                        }
                  }
            }
      }

//---------------------------------------------------
//    computeTicks
//    set ticks for all segments
//       return minTick
//---------------------------------------------------

Fraction Measure::computeTicks()
      {
      Fraction minTick = ticks();
      if (minTick <= Fraction(0,1)) {
            qDebug("=====minTick %d measure %p", minTick.ticks(), this);
            }
      Q_ASSERT(minTick > Fraction(0,1));

      Segment* ns = first();
      while (ns && !ns->enabled())
            ns = ns->next();
      while (ns) {
            Segment* s = ns;
            ns         = s->nextActive();
            Fraction nticks = (ns ? ns->rtick() : ticks()) - s->rtick();
            if (nticks.isNotZero()) {
                  if (nticks < minTick)
                        minTick = nticks;
                  }
            s->setTicks(nticks);
            }
      return minTick;
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
//    Assume all barlines have same visibility if there is more
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
      if (prev() || next()) // avoid triggering layout before getting added to a score
            score()->setLayout(tick(), endTick(), 0, score()->nstaves() - 1, this);
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
            Part* part = score()->staff(track / VOICES)->part();
            // by default, barlines for multi-staff parts should span across staves
            if (part && part->nstaves() > 1) {
                bl->setSpanStaff(true);
            }
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
      qreal blw    = 0.0;

#if 0
#ifndef NDEBUG
      computeMinWidth();
#endif
#endif
      qreal oldWidth = width();

      if (nm && nm->repeatStart() && !repeatEnd() && !isLastMeasureInSystem && next() == nm) {
            // we may skip barline at end of a measure immediately before a start repeat:
            // next measure is repeat start, this measure is not a repeat end,
            // this is not last measure of system, no intervening frame
            if (!seg)
                  return 0.0;
            seg->setEnabled(false);
            }
      else {
            BarLineType t = nm ? BarLineType::NORMAL : BarLineType::END;
            if (!seg)
                  seg = getSegmentR(SegmentType::EndBarLine, ticks());
            seg->setEnabled(true);
            //
            //  Set flag "hasCourtesyKeySig" if this measure needs a courtesy key sig.
            //  This flag is later used to set a double end bar line and to actually
            //  create the courtesy key sig.
            //

            bool show = score()->styleB(Sid::genCourtesyKeysig) && !sectionBreak() && nm;

            setHasCourtesyKeySig(false);

            if (isLastMeasureInSystem && show) {
                  Fraction tick = endTick();
                  for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                        Staff* staff     = score()->staff(staffIdx);
                        KeySigEvent key1 = staff->keySigEvent(tick - Fraction::fromTicks(1));
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
//                  force = true;
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
                                          bl->undoChangeProperty(Pid::BARLINE_TYPE, QVariant::fromValue(t));
                                          bl->setGenerated(true);
                                          }
                                    }
                              }
                        }
                  bl->layout();
                  blw = qMax(blw, bl->width());
                  }
            // right align within segment
            for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                  int track   = staffIdx * VOICES;
                  BarLine* bl = toBarLine(seg->element(track));
                  if (bl)
                        bl->rxpos() += blw - bl->width();
                  }
            seg->createShapes();
            }

      // set relative position of end barline and clef
      // if end repeat, clef goes after, otherwise clef goes before
      Segment* clefSeg = findSegmentR(SegmentType::Clef, ticks());
      if (clefSeg) {
            bool wasVisible = clefSeg->visible();
            int visibleInt = 0;
            for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                  if (!score()->staff(staffIdx)->show())
                        continue;
                  int track    = staffIdx * VOICES;
                  Clef* clef = toClef(clefSeg->element(track));
                  if (clef) {
                        bool showCourtesy = score()->genCourtesyClef() && clef->showCourtesy(); // normally show a courtesy clef
                        // check if the measure is the last measure of the system or the last measure before a frame
                        bool lastMeasure = isLastMeasureInSystem || (nm ? !(next() == nm) : true);
                        if (!nm || isFinalMeasureOfSection() || (lastMeasure && !showCourtesy)) {
                              // hide the courtesy clef in the final measure of a section, or if the measure is the final measure of a system
                              // and the score style or the clef style is set to "not show courtesy clef",
                              // or if the clef is at the end of the very last measure of the score
                              clef->clear();
                              clefSeg->createShape(staffIdx);
                              if (visibleInt == 0)
                                    visibleInt = 1;
                              }
                        else {
                              clef->layout();
                              clefSeg->createShape(staffIdx);
                              visibleInt = 2;
                              }
                        }
                  }
            if (visibleInt == 2)       // there is at least one visible clef in the clef segment
                  clefSeg->setVisible(true);
            else if (visibleInt == 1)  // all (courtesy) clefs in the clef segment are not visible
                  clefSeg->setVisible(false);
            else // should never happen
                  qDebug("Clef Segment without Clef elements at tick %d/%d",clefSeg->tick().numerator(),clefSeg->tick().denominator());
            if ((wasVisible != clefSeg->visible()) && system()) // recompute the width only if necessary
                  computeMinWidth();
            if (seg) {
                  Segment* s1;
                  Segment* s2;
                  if (repeatEnd()) {
                        s1 = seg;
                        s2 = clefSeg;
                        }
                  else {
                        s1 = clefSeg;
                        s2 = seg;
                        }
                  if (s1->next() != s2) {
                        _segments.remove(s1);
                        _segments.insert(s1, s2);
                        }
                  }
            }

      // fix segment layout
      Segment* s = seg->prevActive();
      if (s) {
            qreal x    = s->rxpos();
            computeMinWidth(s, x, false);
            }

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
      qreal stretch = userStretch() * score()->styleD(Sid::measureSpacing);
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
      qreal minMeasureWidth = score()->styleP(Sid::minMeasureWidth);
      if (w < minMeasureWidth)
            w = minMeasureWidth;
      return w;
      }

//---------------------------------------------------------
//   layoutWeight
//---------------------------------------------------------

int Measure::layoutWeight(int maxMMRestLength) const
      {
      int w = ticks().ticks();
      // reduce weight of mmrests
      // so the nominal width is not directly proportional to duration (still linear, just not 1:1)
      // and they are not so "greedy" in taking up available space on a system
      if (isMMRest()) {
            int timesigTicks = timesig().ticks();
            // TODO: style setting
            if (maxMMRestLength) {
                  int maxW = timesigTicks * maxMMRestLength;
                  w = qMin(w, maxW);
                  }
            w -= timesigTicks;
            w = timesigTicks + w / 32;
            }
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
      Segment* kSegment = findFirstR(SegmentType::KeySig, Fraction(0,1));
      Segment* cSegment = findFirstR(SegmentType::HeaderClef, Fraction(0,1));

      for (const Staff* staff : score()->staves()) {
            const int track = staffIdx * VOICES;

            if (isFirstSystem || score()->styleB(Sid::genClef)) {
                  // find the clef type at the previous tick
                  ClefTypeList cl = staff->clefType(tick() - Fraction::fromTicks(1));
                  Segment* s = nullptr;
                  if (prevMeasure())
                        // look for a clef change at the end of the previous measure
                        s = prevMeasure()->findSegment(SegmentType::Clef, tick());
                  else if (isMMRest())
                        // look for a header clef at the beginning of the first underlying measure
                        s = mmRestFirst()->findFirstR(SegmentType::HeaderClef, Fraction(0,1));
                  if (s) {
                        Clef* c = toClef(s->element(track));
                        if (c)
                              cl = c->clefTypeList();
                        }
                  Clef* clef;
                  if (!cSegment) {
                        cSegment = new Segment(this, SegmentType::HeaderClef, Fraction(0,1));
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
                  //cSegment->createShape(staffIdx);
                  cSegment->setEnabled(true);
                  }
            else {
                  if (cSegment)
                        cSegment->setEnabled(false);
                  }

            // keep key sigs in TABs: TABs themselves should hide them
            bool needKeysig = isFirstSystem || score()->styleB(Sid::genKeysig);

            // If we need a Key::C KeySig (which would be invisible) and there is
            // a courtesy key sig, don’t create it and switch generated flags.
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
                        kSegment = new Segment(this, SegmentType::KeySig, Fraction(0,1));
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
                  //kSegment->createShape(staffIdx);
                  kSegment->setEnabled(true);
                  }
            else {
                  if (kSegment && staff->isPitchedStaff(tick())) {
                        // do not disable user modified keysigs
                        bool disable = true;
                        for (int i = 0; i < score()->nstaves(); ++i) {
                              Element* e = kSegment->element(i * VOICES);
                              Key key = score()->staff(i)->key(tick());
                              if ((e && !e->generated()) || (key != keyIdx.key())) {
                                    disable = false;
                                    }
                              else if (e && e->generated() && key == keyIdx.key() && keyIdx.key() == Key::C){
                                    // If a key sig segment is disabled, it may be re-enabled if there is
                                    // a transposing instrument using a different key sig.
                                    // To prevent this from making the wrong key sig display, remove any key
                                    // sigs on staves where the key in this measure is C.
                                    kSegment->remove(e);
                                    }
                              }

                        if (disable)
                              kSegment->setEnabled(false);
                        else {
                              Element* e = kSegment->element(track);
                              if (e && e->isKeySig()) {
                                    KeySig* keysig = toKeySig(e);
                                    keysig->layout();
                                    }
                              }
                        }
                  }

            ++staffIdx;
            }
      if (cSegment)
            cSegment->createShapes();
      if (kSegment)
            kSegment->createShapes();

      //
      // create systemic barline
      //
      Segment* s  = findSegment(SegmentType::BeginBarLine, tick());
      int n       = score()->nstaves();
      if ((n > 1 && score()->styleB(Sid::startBarlineMultiple)) || (n == 1 && score()->styleB(Sid::startBarlineSingle))) {
            if (!s) {
                  s = new Segment(this, SegmentType::BeginBarLine, Fraction(0,1));
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
                        }
                  }
            s->createShapes();
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
      Fraction _rtick = ticks();
      bool isFinalMeasure = isFinalMeasureOfSection();

      // locate a time sig. in the next measure and, if found,
      // check if it has court. sig. turned off
      TimeSig* ts = nullptr;
      bool showCourtesySig = false;
      Segment* s = findSegmentR(SegmentType::TimeSigAnnounce, _rtick);
      if (nm && score()->genCourtesyTimesig() && !isFinalMeasure && !score()->floatMode()) {
            Segment* tss = nm->findSegmentR(SegmentType::TimeSig, Fraction(0,1));
            if (tss) {
                  int nstaves = score()->nstaves();
                  for (int track = 0; track < nstaves * VOICES; track += VOICES) {
                        ts = toTimeSig(tss->element(track));
                        if (ts)
                              break;
                        }
                  if (ts && ts->showCourtesySig()) {
                        showCourtesySig = true;
                        // if due, create a new courtesy time signature for each staff
                        if (!s) {
                              s  = new Segment(this, SegmentType::TimeSigAnnounce, _rtick);
                              s->setTrailer(true);
                              add(s);
                              }
                        s->setEnabled(true);
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
                              //s->createShape(track / VOICES);
                              }
                        s->createShapes();
                        }
                  }
            }
      if (!showCourtesySig && s) {
            // remove any existing time signatures
            s->setEnabled(false);
            }

      // courtesy key signatures, clefs

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
                  //s->createShape(track / VOICES);
                  s->setEnabled(true);
                  }
            else {
                  // remove any existent courtesy key signature
                  if (s)
                        s->setEnabled(false);
                  }
            if (clefSegment) {
                  Clef* clef = toClef(clefSegment->element(track));
                  if (clef)
                         clef->setSmall(true);
                  }
            }
      if (s)
            s->createShapes();
      if (clefSegment)
            clefSegment->createShapes();

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
      if (system() && changed)
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
      qreal minWidth = isMMRest() ? score()->styleP(Sid::minMMRestWidth) : score()->styleP(Sid::minMeasureWidth);
      if (w < minWidth)
            w = minWidth;

      qreal stretchableWidth = 0.0;
      for (Segment* s = first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            if (!s->enabled())
                  continue;
            stretchableWidth += s->width();
            }
      const int maxMMRestLength = 32;     // TODO: style
      int weight = layoutWeight(maxMMRestLength);
      w += stretchableWidth * (basicStretch() - 1.0) * weight / 1920.0;

      setWidth(w);
      }

//---------------------------------------------------------
//   hasAccidental
//---------------------------------------------------------

static bool hasAccidental(Segment* s)
      {
      Score* score = s->score();
      for (int track = 0; track < s->score()->ntracks(); ++track) {
            Staff* staff = score->staff(track2staff(track));
            if (!staff->show())
                  continue;
            Element* e = s->element(track);
            if (!e || !e->isChord())
                  continue;
            Chord* c = toChord(e);
            for (Note* n : c->notes()) {
                  if (n->accidental() && n->accidental()->addToSkyline())
                        return true;
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   computeMinWidth
//    sets the minimum stretched width of segment list s
//    set the width and x position for all segments
//---------------------------------------------------------

void Measure::computeMinWidth(Segment* s, qreal x, bool isSystemHeader)
      {
      Segment* fs = firstEnabled();
      if (!fs->visible())           // first enabled could be a clef change on invisible staff
            fs = fs->nextActive();
      bool first  = isFirstInSystem();
      const Shape ls(first ? QRectF(0.0, -DBL_MAX, 0.0, DBL_MAX) : QRectF(0.0, 0.0, 0.0, spatium() * 4));

      if (isMMRest()) {
            // Reset MM rest to initial size and position
            Segment* seg = findSegmentR(SegmentType::ChordRest, Fraction(0,1));
            const int nstaves = score()->nstaves();
            for (int st = 0; st < nstaves; ++st) {
                  Rest* mmRest = toRest(seg->element(staff2track(st)));
                  if (mmRest) {
                        mmRest->rxpos() = 0;
                        mmRest->layoutMMRest(score()->styleP(Sid::minMMRestWidth) * mag());
                        mmRest->segment()->createShapes();
                        }
                  }
            }

      while (s) {
            s->rxpos() = x;
            // segments with all elements invisible are skipped, though these are already
            // skipped in computeMinWidth() -- the only way this would be an issue here is
            // if this method was called specifically with the invisible segment specified
            // which I'm pretty sure doesn't happen at this point. still...
            if (!s->enabled() || !s->visible() || s->allElementsInvisible()) {
                  s->setWidth(0);
                  s = s->next();
                  continue;
                  }
            Segment* ns = s->nextActive();
            while (ns && ns->allElementsInvisible())
                  ns = ns->nextActive();
            // end barline might be disabled
            // but still consider it for spacing of previous segment
            if (!ns)
                  ns = s->next(SegmentType::BarLineType);
            qreal w;

            if (ns) {
                  if (isSystemHeader && (ns->isChordRestType() || (ns->isClefType() && !ns->header()))) {
                        // this is the system header gap
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

                  if (s == fs) // don't let the second segment cross measure start (not covered by the loop below)
                        w = std::max(w, ns->minLeft(ls) - s->x());

                  int n = 1;
                  for (Segment* ps = s; ps != fs;) {
                        qreal ww;
                        ps = ps->prevActive();

                        Q_ASSERT(ps); // ps should never be nullptr but better be safe.
                        if (!ps)
                              break;

                        if (ps->isChordRestType())
                              ++n;
                        ww = ps->minHorizontalCollidingDistance(ns) - (s->x() - ps->x());

                        if (ps == fs)
                              ww = std::max(ww, ns->minLeft(ls) - s->x());
                        if (ww > w) {
                              // overlap !
                              // distribute extra space between segments ps - ss;
                              // only ChordRest segments get more space
                              // TODO: is there a special case n == 0 ?

                              qreal d = (ww - w) / n;
                              qreal xx = ps->x();
                              for (Segment* ss = ps; ss != s;) {
                                    Segment* ns1 = ss->nextActive();
                                    qreal ww1    = ss->width();
                                    if (ss->isChordRestType()) {
                                          ww1 += d;
                                          ss->setWidth(ww1);
                                          }
                                    xx += ww1;
                                    ns1->rxpos() = xx;
                                    ss = ns1;
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

void Measure::computeMinWidth()
      {
      Segment* s;

      // skip disabled segment
      for (s = first(); s && (!s->enabled() || s->allElementsInvisible()); s = s->next()) {
            s->rxpos() = computeFirstSegmentXPosition(s);  // this is where placement of hidden key/time sigs is set
            s->setWidth(0);                                // it shouldn't affect the width of the bar no matter what it is
            }
      if (!s) {
            setWidth(0.0);
            return;
            }
      qreal x;
      bool first = isFirstInSystem();

      // left barriere:
      //    Make sure no elements crosses the left boarder if first measure in a system.
      //
      Shape ls(first ? QRectF(0.0, -DBL_MAX, 0.0, DBL_MAX) : QRectF(0.0, 0.0, 0.0, spatium() * 4));

      x = s->minLeft(ls);

      if (s->isStartRepeatBarLineType()) {
            System*  sys = system();
            MeasureBase* pmb = prev();
            if (pmb->isMeasure() && pmb->system() == sys && pmb->repeatEnd()) {
                  Segment* seg = toMeasure(pmb)->last();
                  // overlap end repeat barline with start repeat barline
                  if (seg->isEndBarLineType())
                        x -= score()->styleP(Sid::endBarWidth) * mag();
                  }
            }

      if (s->isChordRestType())
            x += score()->styleP(hasAccidental(s) ? Sid::barAccidentalDistance : Sid::barNoteDistance);
      else if (s->isClefType() || s->isHeaderClefType())
            x += score()->styleP(Sid::clefLeftMargin);
      else if (s->isKeySigType())
            x = qMax(x, score()->styleP(Sid::keysigLeftMargin));
      else if (s->isTimeSigType())
            x = qMax(x, score()->styleP(Sid::timesigLeftMargin));
      x += s->extraLeadingSpace().val() * spatium();
      bool isSystemHeader = s->header();

      computeMinWidth(s, x, isSystemHeader);
      }

qreal Measure::computeFirstSegmentXPosition(Segment* segment)
      {
      qreal x = 0;

      Shape ls(QRectF(0.0, 0.0, 0.0, spatium() * 4));

      x = segment->minLeft(ls);

      if (segment->isChordRestType())
            x += score()->styleP(hasAccidental(segment) ? Sid::barAccidentalDistance : Sid::barNoteDistance);
      else if (segment->isClefType() || segment->isHeaderClefType())
            x += score()->styleP(Sid::clefLeftMargin);
      else if (segment->isKeySigType())
            x = qMax(x, score()->styleP(Sid::keysigLeftMargin));
      else if (segment->isTimeSigType())
            x = qMax(x, score()->styleP(Sid::timesigLeftMargin));

      x += segment->extraLeadingSpace().val() * spatium();

      return x;
      }

}
