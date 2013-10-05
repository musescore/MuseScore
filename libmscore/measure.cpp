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
#include "segment.h"
#include "note.h"
#include "rest.h"
#include "chord.h"
#include "xml.h"
#include "score.h"
#include "clef.h"
#include "key.h"
#include "dynamic.h"
#include "slur.h"
#include "tie.h"
#include "sig.h"
#include "beam.h"
#include "tuplet.h"
#include "system.h"
#include "undo.h"
#include "hairpin.h"
#include "text.h"
#include "select.h"
#include "staff.h"
#include "part.h"
#include "style.h"
#include "bracket.h"
#include "ottava.h"
#include "trill.h"
#include "pedal.h"
#include "timesig.h"
#include "barline.h"
#include "layoutbreak.h"
#include "page.h"
#include "lyrics.h"
#include "volta.h"
#include "image.h"
#include "hook.h"
#include "beam.h"
#include "pitchspelling.h"
#include "keysig.h"
#include "breath.h"
#include "tremolo.h"
#include "drumset.h"
#include "repeat.h"
#include "box.h"
#include "harmony.h"
#include "tempotext.h"
#include "sym.h"
#include "stafftext.h"
#include "utils.h"
#include "glissando.h"
#include "articulation.h"
#include "spacer.h"
#include "duration.h"
#include "fret.h"
#include "stafftype.h"
#include "tablature.h"
#include "tiemap.h"
#include "tupletmap.h"
#include "accidental.h"
#include "layout.h"
#include "icon.h"

namespace Ms {

//---------------------------------------------------------
//   MStaff
//---------------------------------------------------------

MStaff::MStaff()
      {
      _noText      = 0;
      distanceUp   = .0;
      distanceDown = .0;
      lines        = 0;
      hasVoices    = false;
      _vspacerUp   = 0;
      _vspacerDown = 0;
      _visible     = true;
      _slashStyle  = false;
      }

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
      distanceUp   = m.distanceUp;
      distanceDown = m.distanceDown;
      lines        = m.lines;
      hasVoices    = m.hasVoices;
      _vspacerUp   = 0;
      _vspacerDown = 0;
      _visible     = m._visible;
      _slashStyle  = m._slashStyle;
      }

//---------------------------------------------------------
//   Measure
//---------------------------------------------------------

Measure::Measure(Score* s)
   : MeasureBase(s),
     _timesig(4,4), _len(4,4)
      {
      _repeatCount           = 2;
      _repeatFlags           = 0;

      int n = _score->nstaves();
      staves.reserve(n);
      for (int staffIdx = 0; staffIdx < n; ++staffIdx) {
            MStaff* s    = new MStaff;
            Staff* staff = score()->staff(staffIdx);
            s->lines     = new StaffLines(score());
            s->lines->setTrack(staffIdx * VOICES);
            s->lines->setParent(this);
            s->lines->setVisible(!staff->invisible());
            staves.push_back(s);
            }

      _minWidth1             = 0.0;
      _minWidth2             = 0.0;

      _no                    = 0;
      _noOffset              = 0;
      _noMode                = MeasureNumberMode::AUTO;
      _userStretch           = 1.0;     // ::style->measureSpacing;
      _irregular             = false;
      _breakMultiMeasureRest = false;
      _breakMMRest           = false;
      _endBarLineGenerated   = true;
      _endBarLineVisible     = true;
      _endBarLineType        = NORMAL_BAR;
      _mmRest                = 0;
      _mmRestCount           = 0;
      setFlag(ELEMENT_MOVABLE, true);
      }

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

Measure::Measure(const Measure& m)
   : MeasureBase(m)
      {
      _segments              = m._segments.clone();
      _timesig               = m._timesig;
      _len                   = m._len;
      _repeatCount           = m._repeatCount;
      _repeatFlags           = m._repeatFlags;

      staves.reserve(m.staves.size());
      foreach(MStaff* ms, m.staves)
            staves.append(new MStaff(*ms));

      _minWidth1             = m._minWidth1;
      _minWidth2             = m._minWidth2;

      _no                    = m._no;
      _noOffset              = m._noOffset;
      _userStretch           = m._userStretch;

      _irregular             = m._irregular;
      _breakMultiMeasureRest = m._breakMultiMeasureRest;
      _breakMMRest           = m._breakMMRest;
      _endBarLineGenerated   = m._endBarLineGenerated;
      _endBarLineVisible     = m._endBarLineVisible;
      _endBarLineType        = m._endBarLineType;
      _mmRest                = m._mmRest;
      _mmRestCount           = m._mmRestCount;
      _playbackCount         = m._playbackCount;
      _endBarLineColor       = m._endBarLineColor;
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
      if (lines)
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
      qDeleteAll(staves);
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

/**
 Debug only.
*/

void Measure::dump() const
      {
      qDebug("dump measure:");
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Measure::remove(Segment* el)
      {
#ifndef NDEBUG
      Q_ASSERT(!score()->undoRedo());
      Q_ASSERT(el->type() == SEGMENT);
      if (el->prev()) {
            Q_ASSERT(el->prev()->next() == el);
            }
      else {
            Q_ASSERT(el == _segments.first());
            }

      if (el->next()) {
            Q_ASSERT(el->next()->prev() == el);
            }
      else {
            Q_ASSERT(el == _segments.last());
            }
#endif
      int tracks = staves.size() * VOICES;
      for (int track = 0; track < tracks; track += VOICES) {
            if (!el->element(track))
                  continue;
            if (el->segmentType() == Segment::SegKeySig)
                  score()->staff(track/VOICES)->setUpdateKeymap(true);
            }
      _segments.remove(el);
      setDirty();
      }

//---------------------------------------------------------
//   AcEl
//---------------------------------------------------------

struct AcEl {
      Note* note;
      qreal x;
      };

//---------------------------------------------------------
//   layoutChords0
//---------------------------------------------------------

void Measure::layoutChords0(Segment* segment, int startTrack)
      {
      int staffIdx     = startTrack/VOICES;
      Staff* staff     = score()->staff(staffIdx);
      qreal staffMag  = staff->mag();

      int endTrack = startTrack + VOICES;
      for (int track = startTrack; track < endTrack; ++track) {
            ChordRest* cr = static_cast<ChordRest*>(segment->element(track));
            if (!cr)
                 continue;
            layoutCR0(cr, staffMag);
            }
      }

//---------------------------------------------------------
//   layoutCR0
//---------------------------------------------------------

void Measure::layoutCR0(ChordRest* cr, qreal mm)
      {
      Drumset* drumset = 0;
      if (cr->staff()->part()->instr()->useDrumset())
            drumset = cr->staff()->part()->instr()->drumset();

      qreal m = mm;
      if (cr->small())
            m *= score()->styleD(ST_smallNoteMag);

      if (cr->type() == CHORD) {
            Chord* chord = static_cast<Chord*>(cr);
            for (Chord* c : chord->graceNotes())
                  layoutCR0(c, mm);

            if (chord->noteType() != NOTE_NORMAL)
                  m *= score()->styleD(ST_graceNoteMag);
            if (drumset) {
                  foreach(Note* note, chord->notes()) {
                        int pitch = note->pitch();
                        if (!drumset->isValid(pitch)) {
                              // qDebug("unmapped drum note %d", pitch);
                              }
                        else {
                              note->setHeadGroup(drumset->noteHead(pitch));
                              note->setLine(drumset->line(pitch));
                              continue;
                              }
                        }
                  }
            chord->computeUp();
            chord->layoutStem1();
            }
      if (m != mag()) {
            cr->setMag(m);
            setDirty();
            }
      }

//---------------------------------------------------------
//   layoutChords10
//    computes note lines and accidentals
//---------------------------------------------------------

void Measure::layoutChords10(Segment* segment, int startTrack, AccidentalState* as)
      {
      int endTrack = startTrack + VOICES;
      for (int track = startTrack; track < endTrack; ++track) {
            Element* e = segment->element(track);
            if (!e || e->type() != CHORD)
                 continue;
            Chord* chord = static_cast<Chord*>(e);
            chord->layout10(as);
            }
      }

//---------------------------------------------------------
//   findAccidental
///   return current accidental value at note position
//---------------------------------------------------------

AccidentalVal Measure::findAccidental(Note* note) const
      {
      AccidentalState tversatz;  // state of already set accidentals for this measure
      tversatz.init(note->chord()->staff()->keymap()->key(tick()));

      Segment::SegmentTypes st = Segment::SegChordRest;
      for (Segment* segment = first(st); segment; segment = segment->next(st)) {
            int startTrack = note->staffIdx() * VOICES;
            int endTrack   = startTrack + VOICES;
            for (int track = startTrack; track < endTrack; ++track) {
                  Element* e = segment->element(track);
                  if (!e || e->type() != CHORD)
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
                              int line = absStep(tpc, note1->pitch());

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
                        int line = absStep(tpc, note1->pitch());

                        if (note == note1)
                              return tversatz.accidentalVal(line);
                        tversatz.setAccidentalVal(line, tpc2alter(tpc));
                        }
                  }
            }
      qDebug("Measure::findAccidental: note not found");
      return NATURAL;
      }

//---------------------------------------------------------
//   findAccidental
///   Compute accidental state at segment/staffIdx for
///   relative staff line.
//---------------------------------------------------------

AccidentalVal Measure::findAccidental(Segment* s, int staffIdx, int line) const
      {
      AccidentalState tversatz;  // state of already set accidentals for this measure
      Staff* staff = score()->staff(staffIdx);
      tversatz.init(staff->keymap()->key(tick()));

      Segment::SegmentTypes st = Segment::SegChordRest;
      int startTrack           = staffIdx * VOICES;
      int endTrack             = startTrack + VOICES;
      for (Segment* segment = first(st); segment; segment = segment->next(st)) {
            if (segment == s) {
                  ClefType clef = staff->clef(s->tick());
                  int l = relStep(line, clef);
                  return tversatz.accidentalVal(l);
                  }
            for (int track = startTrack; track < endTrack; ++track) {
                  Element* e = segment->element(track);
                  if (!e || e->type() != CHORD)
                        continue;
                  Chord* chord = static_cast<Chord*>(e);
                  for (Chord* chord1 : chord->graceNotes()) {
                        for (Note* note : chord1->notes()) {
                              if (note->tieBack())
                                    continue;
                              int tpc  = note->tpc();
                              int l    = absStep(tpc, note->pitch());
                              tversatz.setAccidentalVal(l, tpc2alter(tpc));
                              }
                        }

                  for (Note* note : chord->notes()) {
                        if (note->tieBack())
                              continue;
                        int tpc    = note->tpc();
                        int l      = absStep(tpc, note->pitch());
                        tversatz.setAccidentalVal(l, tpc2alter(tpc));
                        }
                  }
            }
      qDebug("segment not found");
      return NATURAL;
      }

//---------------------------------------------------------
//   Measure::layout
///   Layout measure; must fit into  \a width.
///
///   Note: minWidth = width - stretch
//---------------------------------------------------------

void Measure::layout(qreal width)
      {
      int nstaves = _score->nstaves();
      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            staves[staffIdx]->distanceUp   = 0.0;
            staves[staffIdx]->distanceDown = 0.0;
            StaffLines* sl = staves[staffIdx]->lines;
            if (sl)
                  sl->setMag(score()->staff(staffIdx)->mag());
            staves[staffIdx]->lines->layout();
            }

      // height of boundingRect will be set in system->layout2()
      // keep old value for relayout

      bbox().setRect(0.0, 0.0, width, height());
      layoutX(width);
      }

//---------------------------------------------------------
//   tick2pos
//    return x position for tick relative to System
//---------------------------------------------------------

qreal Measure::tick2pos(int tck) const
      {
      if (isMMRest()) {
            Segment* s = first(Segment::SegChordRest);
            qreal x1 = s->x();
            qreal w  = width() - x1;
            return x1 + (tck * w) / (ticks() * mmRestCount());
            }

      Segment* s;
      qreal x1 = 0;
      qreal x2 = 0;
      int tick1 = tick();
      int tick2 = tick1;
      for (s = first(Segment::SegChordRest); s; s = s->next(Segment::SegChordRest)) {
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
//    called after layout of all pages
//---------------------------------------------------------

void Measure::layout2()
      {
      if (parent() == 0)
            return;

      Q_ASSERT(score()->nstaves() == staves.size());

      int tracks = score()->nstaves() * VOICES;
      qreal _spatium = spatium();
      static const Segment::SegmentTypes st = Segment::SegChordRest;
      for (int track = 0; track < tracks; ++track) {
            for (Segment* s = first(st); s; s = s->next(st)) {
                  ChordRest* cr = s->cr(track);
                  if (!cr)
                        continue;
                  int n = cr->lyricsList().size();
                  for (int i = 0; i < n; ++i) {
                        Lyrics* lyrics = cr->lyricsList().at(i);
                        if (lyrics)
                              system()->layoutLyrics(lyrics, s, track/VOICES);
                        }
                  }
            if (track % VOICES == 0) {
                  int staffIdx = track / VOICES;
                  qreal y = system()->staff(staffIdx)->y();
                  Spacer* sp = staves[staffIdx]->_vspacerDown;
                  if (sp) {
                        sp->layout();
                        int n = score()->staff(staffIdx)->lines() - 1;
                        sp->setPos(_spatium * .5, y + n * _spatium);
                        }
                  sp = staves[staffIdx]->_vspacerUp;
                  if (sp) {
                        sp->layout();
                        sp->setPos(_spatium * .5, y - sp->gap());
                        }
                  }
            }
      for (MStaff* ms : staves)
            ms->lines->setWidth(width());

      MeasureBase::layout();  // layout LAYOUT_BREAK elements

      //
      //   set measure number
      //
      bool smn = false;

//      if (!_noText || _noText->generated()) {
            if (_noMode == MeasureNumberMode::SHOW)
                  smn = true;
            else if (_noMode == MeasureNumberMode::HIDE)
                  smn = false;
            else {
                  if (score()->styleB(ST_showMeasureNumber)
                     && !_irregular
                     && (_no || score()->styleB(ST_showMeasureNumberOne))) {
                        if (score()->styleB(ST_measureNumberSystem))
                              smn = system()->firstMeasure() == this;
                        else {
                              smn = (_no == 0 && score()->styleB(ST_showMeasureNumberOne)) ||
                                    ( ((_no+1) % score()->style(ST_measureNumberInterval).toInt()) == 0 );
                              }
                        }
                  }
            QString s;
            if (smn)
                  s = QString("%1").arg(_no + 1);
            int nn = 1;
            if (!score()->styleB(ST_measureNumberAllStaffs)) {
                  //find first non invisible staff
                  for (int staffIdx = 0; staffIdx < staves.size(); ++staffIdx) {
                        MStaff* ms = staves.at(staffIdx);
                        SysStaff* s  = system()->staff(staffIdx);
                        Staff* staff = score()->staff(staffIdx);
                        if (ms->visible() && staff->show() && s->show()) {
                              nn = staffIdx;
                              break;
                              }
                        }
                  }
            for (int staffIdx = 0; staffIdx < staves.size(); ++staffIdx) {
                  MStaff* ms = staves.at(staffIdx);
                  Text* t = ms->noText();
                  if (smn) {
                        if (t == 0 && (staffIdx == nn || score()->styleB(ST_measureNumberAllStaffs))) {
                              t = new Text(score());
                              t->setFlag(ELEMENT_ON_STAFF, true);
                              t->setTrack(staffIdx * VOICES);
                              t->setGenerated(true);
                              t->setTextStyleType(TEXT_STYLE_MEASURE_NUMBER);
                              t->setParent(this);
                              score()->undoAddElement(t);
                              }
                        if(t) {
                              t->setText(s);
                              t->layout();
                              smn = score()->styleB(ST_measureNumberAllStaffs);
                              }
                        }
                  else {
                        if (t)
                              score()->undoRemoveElement(t);
                        }
                  }
//            }

      //
      // slur layout needs articulation layout first
      //
      for (Segment* s = first(st); s; s = s->next(st)) {
            for (int track = 0; track < tracks; ++track) {
                  if (!score()->staff(track / VOICES)->show()) {
                        track += VOICES-1;
                        continue;
                        }
                  Element* el = s->element(track);
                  if (el) {
                        ChordRest* cr = static_cast<ChordRest*>(el);
                        if (cr->type() == CHORD) {
                              Chord* c = static_cast<Chord*>(cr);
                              foreach(const Note* note, c->notes()) {
                                    foreach (Spanner* sp, note->spannerFor())
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
                  if (el && el->type() == CHORD)
                        return static_cast<Chord*>(el);
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
      for (Segment* seg = first(); seg; seg = seg->next()) {
            if (seg->tick() > tick)
                  return 0;
            if (seg->tick() == tick) {
                  Element* el = seg->element(track);
                  if (el && (el->type() == CHORD || el->type() == REST)) {
                        return (ChordRest*)el;
                        }
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   tick2segment
//---------------------------------------------------------

Segment* Measure::tick2segment(int tick) const
      {
      for (Segment* s = first(); s; s = s->next()) {
            if (s->tick() == tick) {
                  if (s->segmentType() == Segment::SegChordRest)
                        return s;
                  }
            if (s->tick() > tick)
                  return 0;
            }
      return 0;
      }

//---------------------------------------------------------
//   findSegment
/// Search for a segment of type \a st at position \a t.
//---------------------------------------------------------

Segment* Measure::findSegment(Segment::SegmentType st, int t)
      {
      Segment* s;
      for (s = first(); s && s->tick() < t; s = s->next())
            ;

      for (; s && s->tick() == t; s = s->next()) {
            if (s->segmentType() == st)
                  return s;
            }
      return 0;
      }

//---------------------------------------------------------
//   undoGetSegment
//---------------------------------------------------------

Segment* Measure::undoGetSegment(Segment::SegmentType type, int tick)
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

Segment* Measure::getSegment(Segment::SegmentType st, int tick)
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

void Measure::add(Element* el)
      {
      setDirty();
      el->setParent(this);
      ElementType type = el->type();

//      if (MScore::debugMode)
//            qDebug("measure %p(%d): add %s %p", this, _no, el->name(), el);

      switch (type) {
            case TEXT:
                  staves[el->staffIdx()]->setNoText(static_cast<Text*>(el));
                  break;

            case TUPLET:
                  qDebug("Measure::add(Tuplet) ??");
                  break;

            case SPACER:
                  {
                  Spacer* sp = static_cast<Spacer*>(el);
                  if (sp->spacerType() == SPACER_UP)
                        staves[el->staffIdx()]->_vspacerUp = sp;
                  else if (sp->spacerType() == SPACER_DOWN)
                        staves[el->staffIdx()]->_vspacerDown = sp;
                  }
                  break;
            case SEGMENT:
                  {
                  Segment* seg = static_cast<Segment*>(el);
                  int tracks = staves.size() * VOICES;
                  if (seg->segmentType() == Segment::SegKeySig) {
                        for (int track = 0; track < tracks; track += VOICES) {
                              if (!seg->element(track))
                                    continue;
                              score()->staff(track/VOICES)->setUpdateKeymap(true);
                              }
                        }

                  // insert segment at specific position
                  if (seg->next()) {
                        _segments.insert(seg, seg->next());
                        break;
                        }
                  int t  = seg->tick();
                  Segment::SegmentType st = seg->segmentType();
                  Segment* s;
                  for (s = first(); s && s->tick() < t; s = s->next())
                        ;
                  if (s) {
                        if (st == Segment::SegChordRest) {
                              while (s && s->segmentType() != st && s->tick() == t) {
                                    if (s->segmentType() == Segment::SegEndBarLine)
                                          break;
                                    s = s->next();
                                    }
                              }
                        else {
                              while (s && s->segmentType() <= st) {
                                    if (s->next() && s->next()->tick() != t)
                                          break;
                                    s = s->next();
                                    }
                              //
                              // place breath _after_ chord
                              //
                              if (s && st == Segment::SegBreath)
                                    s = s->next();
                              }
                        }
                  seg->setParent(this);
                  _segments.insert(seg, s);
                  if ((seg->segmentType() == Segment::SegTimeSig) && seg->element(0)) {
#if 0
                        Fraction nfraction(static_cast<TimeSig*>(seg->element(0))->getSig());
                        setTimesig2(nfraction);
                        for (Measure* m = nextMeasure(); m; m = m->nextMeasure()) {
                              if (m->first(SegTimeSig))
                                    break;
                              m->setTimesig2(nfraction);
                              }
#endif
                        score()->addLayoutFlags(LAYOUT_FIX_TICKS);
                        }
                  }
                  break;

            case JUMP:
                  _repeatFlags |= RepeatJump;
                  _el.push_back(el);
                  break;

            case HBOX:
                  if (el->staff())
                        el->setMag(el->staff()->mag());     // ?!
                  _el.push_back(el);
                  break;

            default:
                  MeasureBase::add(el);
                  break;
            }

      }

//---------------------------------------------------------
//   remove
///   Remove Element \a el from Measure.
//---------------------------------------------------------

void Measure::remove(Element* el)
      {
      setDirty();
      switch(el->type()) {
            case TEXT:
                  staves[el->staffIdx()]->setNoText(static_cast<Text*>(0));
                  break;

            case SPACER:
                  if (static_cast<Spacer*>(el)->spacerType() == SPACER_DOWN)
                        staves[el->staffIdx()]->_vspacerDown = 0;
                  else if (static_cast<Spacer*>(el)->spacerType() == SPACER_UP)
                        staves[el->staffIdx()]->_vspacerUp = 0;
                  break;

            case SEGMENT:
                  remove(static_cast<Segment*>(el));
                  break;

            case JUMP:
                  _repeatFlags &= ~RepeatJump;
                  // fall through

            case HBOX:
                  if (!_el.remove(el)) {
                        qDebug("Measure(%p)::remove(%s,%p) not found",
                           this, el->name(), el);
                        }
                  break;

            case CLEF:
            case CHORD:
            case REST:
            case TIMESIG:
                  for (Segment* segment = first(); segment; segment = segment->next()) {
                        int staves = _score->nstaves();
                        int tracks = staves * VOICES;
                        for (int track = 0; track < tracks; ++track) {
                              Element* e = segment->element(track);
                              if (el == e) {
                                    segment->setElement(track, 0);
                                    return;
                                    }
                              }
                        }
                  qDebug("Measure::remove: %s %p not found", el->name(), el);
                  break;

            default:
                  MeasureBase::remove(el);
                  break;
            }
      }

//---------------------------------------------------------
//   change
//---------------------------------------------------------

void Measure::change(Element* o, Element* n)
      {
      if (o->type() == TUPLET) {
            Tuplet* t = static_cast<Tuplet*>(n);
            foreach(DurationElement* e, t->elements()) {
                  e->setTuplet(t);
                  }
            }
      else {
            remove(o);
            add(n);
            }
      }

//-------------------------------------------------------------------
//   moveTicks
//    Also adjust endBarLine if measure len has changed. For this
//    diff == 0 cannot be optimized away
//-------------------------------------------------------------------

void Measure::moveTicks(int diff)
      {
      setTick(tick() + diff);
      for (Segment* segment = first(); segment; segment = segment->next()) {
            if (segment->segmentType() & (Segment::SegEndBarLine | Segment::SegTimeSigAnnounce))
                  segment->setTick(tick() + ticks());
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
      foreach (Element* e, _el) {
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
      foreach (Element* e, _el) {
            if (e->track() == -1)
                  continue;
            int staffIdx = e->staffIdx();
            if (staffIdx >= sStaff) {
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
// qDebug("cmdRemoveStaves %d-%d", sStaff, eStaff);
      int sTrack = sStaff * VOICES;
      int eTrack = eStaff * VOICES;
      for (Segment* s = first(); s; s = s->next()) {
//            qDebug(" seg %d <%s>", s->tick(), s->subTypeName());
            for (int track = eTrack - 1; track >= sTrack; --track) {
                  Element* el = s->element(track);
//                  if (el && !el->generated()) {
                  if (el) {
//                        qDebug("  remove %s track %d", el->name(), track);
                        _score->undoRemoveElement(el);
                        }
                  }
            foreach(Element* e, s->annotations()) {
                  int staffIdx = e->staffIdx();
                  if ((staffIdx >= sStaff) && (staffIdx < eStaff)) {
                        qDebug("  remove annotation %s staffIdx %d", e->name(), staffIdx);
                        _score->undoRemoveElement(e);
                        }
                  }
            }
      foreach(Element* e, _el) {
            if (e->track() == -1)
                  continue;
            int staffIdx = e->staffIdx();
            if (staffIdx >= sStaff && staffIdx < eStaff)
                  _score->undoRemoveElement(e);
            }

      _score->undo(new RemoveStaves(this, sStaff, eStaff));

      for (int i = eStaff - 1; i >= sStaff; --i)
            _score->undo(new RemoveMStaff(this, *(staves.begin()+i), i));

      // barLine
      // TODO
      }

//---------------------------------------------------------
//   cmdAddStaves
//---------------------------------------------------------

void Measure::cmdAddStaves(int sStaff, int eStaff, bool createRest)
      {
      _score->undo(new InsertStaves(this, sStaff, eStaff));

      Segment* ts = findSegment(Segment::SegTimeSig, tick());

      for (int i = sStaff; i < eStaff; ++i) {
            Staff* staff = _score->staff(i);
            MStaff* ms   = new MStaff;
            ms->lines    = new StaffLines(score());
            ms->lines->setTrack(i * VOICES);
            // ms->lines->setLines(staff->lines());
            ms->lines->setParent(this);
            ms->lines->setVisible(!staff->invisible());

            _score->undo(new InsertMStaff(this, ms, i));

            if (createRest) {
                  Rest* rest = new Rest(score(), TDuration(TDuration::V_MEASURE));
                  rest->setTrack(i * VOICES);
                  rest->setDuration(len());
                  Segment* s = undoGetSegment(Segment::SegChordRest, tick());
                  rest->setParent(s);
                  score()->undoAddElement(rest);
                  }

            // replicate time signature
            if (ts) {
                  TimeSig* ots = 0;
                  for (int track = 0; track < staves.size() * VOICES; ++track) {
                        if (ts->element(track)) {
                              ots = (TimeSig*)ts->element(track);
                              break;
                              }
                        }
                  if (ots) {
                        TimeSig* timesig = new TimeSig(*ots);
                        timesig->setTrack(i * VOICES);
                        timesig->setSig(ots->sig(), ots->timeSigType());
                        score()->undoAddElement(timesig);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void MStaff::setTrack(int track)
      {
      if (lines)
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
      staves.insert(idx, staff);
      for (int staffIdx = 0; staffIdx < staves.size(); ++staffIdx)
            staves[staffIdx]->setTrack(staffIdx * VOICES);
      }

//---------------------------------------------------------
//   removeMStaff
//---------------------------------------------------------

void Measure::removeMStaff(MStaff* /*staff*/, int idx)
      {
      staves.removeAt(idx);
      for (int staffIdx = 0; staffIdx < staves.size(); ++staffIdx)
            staves[staffIdx]->setTrack(staffIdx * VOICES);
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Measure::insertStaff(Staff* staff, int staffIdx)
      {
qDebug("Measure::insertStaff: %d", staffIdx);
      for (Segment* s = first(); s; s = s->next())
            s->insertStaff(staffIdx);

      MStaff* ms = new MStaff;
      ms->lines  = new StaffLines(score());
      ms->lines->setParent(this);
      ms->lines->setTrack(staffIdx * VOICES);
//      ms->distanceUp   = 0.0;
//      ms->distanceDown = 0.0; // TODO point(staffIdx == 0 ? score()->styleS(ST_minSystemDistance) : score()->styleS(ST_staffDistance));
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
 at canvas relative position \a p.

 Note special handling for clefs (allow drop if left of rightmost chord or rest in this staff)
 and key- and timesig (allow drop if left of first chord or rest).
*/

bool Measure::acceptDrop(MuseScoreView* viewer, const QPointF& p, Element* e) const
      {
      int type = e->type();
      // convert p from canvas to measure relative position and take x and y coordinates
      QPointF mrp = p - canvasPos(); // pos() - system()->pos() - system()->page()->pos();
      qreal mrpx = mrp.x();
      qreal mrpy = mrp.y();

      System* s = system();
      int idx = s->y2staff(p.y());
      if (idx == -1) {
            return false;                       // staff not found
            }
      QRectF sb(s->staff(idx)->bbox());
      qreal t = sb.top();    // top of staff
      qreal b = sb.bottom(); // bottom of staff

      // compute rectangle of staff in measure
      QRectF rrr(sb.translated(s->pagePos()));
      QRectF rr(abbox());
      QRectF r(rr.x(), rrr.y(), rr.width(), rrr.height());

      Page* page = system()->page();
      r.translate(page->pos());
      rr.translate(page->pos());

      switch(type) {
            case STAFF_LIST:
                  viewer->setDropRectangle(r);
                  return true;

            case MEASURE_LIST:
            case JUMP:
            case MARKER:
            case LAYOUT_BREAK:
                  viewer->setDropRectangle(rr);
                  return true;

            case BRACKET:
            case REPEAT_MEASURE:
            case MEASURE:
            case SPACER:
            case IMAGE:
                  viewer->setDropRectangle(r);
                  return true;

            case BAR_LINE:
            case SYMBOL:
                  // accept drop only inside staff
                  if (mrpy < t || mrpy > b)
                        return false;
                  viewer->setDropRectangle(r);
                  return true;

            case CLEF:
                  {
                  // accept drop only inside staff
                  if (mrpy < t || mrpy > b)
                        return false;
                  viewer->setDropRectangle(r);
                  // search segment list backwards for segchordrest
                  for (Segment* seg = last(); seg; seg = seg->prev()) {
                        if (seg->segmentType() != Segment::SegChordRest)
                              continue;
                        // SegChordRest found, check if it contains anything in this staff
                        for (int track = idx * VOICES; track < idx * VOICES + VOICES; ++track)
                              if (seg->element(track)) {
                                    // LVIFIX: for the rest in newly created empty measures,
                                    // seg->pos().x() is incorrect
                                    return mrpx < seg->pos().x();
                                    }
                        }
                  }
                  return false;
            case ICON:
                  switch(static_cast<Icon*>(e)->iconType()) {
                        case ICON_VFRAME:
                        case ICON_HFRAME:
                        case ICON_TFRAME:
                        case ICON_FFRAME:
                        case ICON_MEASURE:
                              viewer->setDropRectangle(rr);
                              return true;
                        }
                  break;

            case KEYSIG:
            case TIMESIG:
                  // accept drop only inside staff
                  if (mrpy < t || mrpy > b)
                        return false;
                  viewer->setDropRectangle(r);
                  return true;

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
      int staffIdx;
      Segment* seg;
      _score->pos2measure(data.pos, &staffIdx, 0, &seg, 0);

      if (e->systemFlag())
            staffIdx = 0;
      QPointF mrp(data.pos - pagePos());
      Staff* staff = score()->staff(staffIdx);

      switch(e->type()) {
            case MEASURE_LIST:
qDebug("drop measureList or StaffList");
                  delete e;
                  break;

            case STAFF_LIST:
qDebug("drop staffList");
//TODO                  score()->pasteStaff(e, this, staffIdx);
                  delete e;
                  break;

            case MARKER:
            case JUMP:
                  e->setParent(seg);
                  e->setTrack(0);
                  score()->undoAddElement(e);
                  return e;

            case DYNAMIC:
            case FRET_DIAGRAM:
                  e->setParent(seg);
                  e->setTrack(staffIdx * VOICES);
                  score()->undoAddElement(e);
                  return e;

            case IMAGE:
            case SYMBOL:
                  e->setParent(seg);
                  e->setTrack(staffIdx * VOICES);
                  e->layout();
                  {
                  QPointF uo(data.pos - e->canvasPos() - data.dragOffset);
                  e->setUserOff(uo);
                  }
                  score()->undoAddElement(e);
                  return e;

            case BRACKET:
                  {
                  Bracket* b = static_cast<Bracket*>(e);
                  int level = 0;
                  int firstStaff = 0;
                  foreach (Staff* s, score()->staves()) {
                        foreach (const BracketItem& bi, s->brackets()) {
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

            case CLEF:
                  score()->undoChangeClef(staff, first(), static_cast<Clef*>(e)->clefType());
                  delete e;
                  break;

            case KEYSIG:
                  {
                  KeySig* ks    = static_cast<KeySig*>(e);
                  KeySigEvent k = ks->keySigEvent();
                  //add custom key to score if not exist
                  if (k.custom()) {
                        int customIdx = score()->customKeySigIdx(ks);
                        if (customIdx == -1) {
                              customIdx = score()->addCustomKeySig(ks);
                              k.setCustomType(customIdx);
                              }
                        else
                              delete ks;
                      }
                  else
                        delete ks;
                  if (data.modifiers & Qt::ControlModifier) {
                        // apply only to this stave
                        score()->undoChangeKeySig(staff, tick(), k);
                        }
                  else {
                        // apply to all staves:
                        foreach(Staff* s, score()->staves())
                              score()->undoChangeKeySig(s, tick(), k);
                        }

                  break;
                  }

            case TIMESIG:
                  score()->cmdAddTimeSig(this, staffIdx, static_cast<TimeSig*>(e),
                     data.modifiers & Qt::ControlModifier);
                  return 0;

            case LAYOUT_BREAK:
                  {
                  LayoutBreak* lb = static_cast<LayoutBreak*>(e);
                  if (
                        (lb->layoutBreakType() == LayoutBreak::PAGE && _pageBreak)
                     || (lb->layoutBreakType() == LayoutBreak::LINE && _lineBreak)
                     || (lb->layoutBreakType() == LayoutBreak::SECTION && _sectionBreak)
                     ) {
                        //
                        // if break already set
                        //
                        delete lb;
                        break;
                        }
                  // make sure there is only LayoutBreak::LINE or LayoutBreak::PAGE
                  if ((lb->layoutBreakType() != LayoutBreak::SECTION) && (_pageBreak || _lineBreak)) {
                        foreach(Element* le, _el) {
                              if (le->type() == LAYOUT_BREAK
                                 && (static_cast<LayoutBreak*>(le)->layoutBreakType() == LayoutBreak::LINE
                                  || static_cast<LayoutBreak*>(le)->layoutBreakType() == LayoutBreak::PAGE)) {
                                    score()->undoChangeElement(le, e);
                                    break;
                                    }
                              }
                        break;
                        }
                  lb->setTrack(-1);       // this are system elements
                  lb->setParent(this);
                  score()->undoAddElement(lb);
                  return lb;
                  }

            case SPACER:
                  {
                  Spacer* spacer = static_cast<Spacer*>(e);
                  spacer->setTrack(staffIdx * VOICES);
                  spacer->setParent(this);
                  score()->undoAddElement(spacer);
                  return spacer;
                  }

            case BAR_LINE:
                  {
                  BarLine* bl = static_cast<BarLine*>(e);
                  // if dropped bar line refers to span rather than to subtype
                  if (bl->spanFrom() != 0 && bl->spanTo() != DEFAULT_BARLINE_TO) {
                        // get existing bar line for this staff, and drop the change to it
                        Segment* seg = undoGetSegment(Segment::SegEndBarLine, tick() + ticks());
                        BarLine* cbl = static_cast<BarLine*>(seg->element(staffIdx * VOICES));
                        if (cbl)
                              cbl->drop(data);
                        }
                  // if dropped bar line refers to line subtype
                  else {
                        score()->undoChangeBarLine(this, bl->barLineType());
                        delete e;
                        }
                  break;
                  }

            case REPEAT_MEASURE:
                  {
                  delete e;
                  return cmdInsertRepeatMeasure(staffIdx);
                  }
            case ICON:
                  switch(static_cast<Icon*>(e)->iconType()) {
                        case ICON_VFRAME:
                              score()->insertMeasure(VBOX, this);
                              break;
                        case ICON_HFRAME:
                              score()->insertMeasure(HBOX, this);
                              break;
                        case ICON_TFRAME:
                              score()->insertMeasure(TBOX, this);
                              break;
                        case ICON_FFRAME:
                              score()->insertMeasure(FBOX, this);
                              break;
                        case ICON_MEASURE:
                              score()->insertMeasure(MEASURE, this);
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
//   cmdRemoveEmptySegment
//---------------------------------------------------------

void Measure::cmdRemoveEmptySegment(Segment* s)
      {
      if (s->isEmpty())
            _score->undoRemoveElement(s);
      }

//---------------------------------------------------------
//   cmdInsertRepeatMeasure
//---------------------------------------------------------
RepeatMeasure* Measure::cmdInsertRepeatMeasure(int staffIdx)
      {
      //
      // see also cmdDeleteSelection()
      //
      _score->select(0, SELECT_SINGLE, 0);
      for (Segment* s = first(); s; s = s->next()) {
            if (s->segmentType() & Segment::SegChordRest) {
                  int strack = staffIdx * VOICES;
                  int etrack = strack + VOICES;
                  for (int track = strack; track < etrack; ++track) {
                        Element* el = s->element(track);
                        if (el)
                              _score->undoRemoveElement(el);
                        }
                  if (s->isEmpty())
                        _score->undoRemoveElement(s);
                  }
            }
      //
      // add repeat measure
      //
      Segment* seg = undoGetSegment(Segment::SegChordRest, tick());
      RepeatMeasure* rm = new RepeatMeasure(_score);
      rm->setTrack(staffIdx * VOICES);
      rm->setParent(seg);
      _score->undoAddElement(rm);
      foreach(Element* el, _el) {
            if (el->type() == SLUR && el->staffIdx() == staffIdx)
                  _score->undoRemoveElement(el);
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
      int ol = len().ticks();
      int nl = nf.ticks();

      int staves = score()->nstaves();
      int diff   = nl - ol;

      if (nl > ol) {
            // move EndBarLine
            for (Segment* s = first(); s; s = s->next()) {
                  if (s->segmentType() & (Segment::SegEndBarLine|Segment::SegTimeSigAnnounce|Segment::SegKeySigAnnounce)) {
                        s->setTick(tick() + nl);
                        }
                  }
            }

      for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            int rests  = 0;
            int chords = 0;
            Rest* rest = 0;
            for (Segment* segment = first(); segment; segment = segment->next()) {
                  int strack = staffIdx * VOICES;
                  int etrack = strack + VOICES;
                  for (int track = strack; track < etrack; ++track) {
                        Element* e = segment->element(track);
                        if (e && e->type() == REST) {
                              ++rests;
                              rest = static_cast<Rest*>(e);
                              }
                        else if (e && e->type() == CHORD)
                              ++chords;
                        }
                  }
            if (rests == 1 && chords == 0 && rest->durationType().type() == TDuration::V_MEASURE) {
                  rest->setDuration(Fraction::fromTicks(nl));
                  continue;
                  }
            if ((_timesig == _len) && (rests == 1) && (chords == 0)) {
                  rest->setDurationType(TDuration::V_MEASURE);    // whole measure rest
                  }
            else {
                  int strack = staffIdx * VOICES;
                  int etrack = strack + VOICES;

                  for (int trk = strack; trk < etrack; ++trk) {
                        int n = diff;
                        bool rFlag = false;
                        if (n < 0)  {
                              for (Segment* segment = last(); segment;) {
                                    Segment* pseg = segment->prev();
                                    Element* e = segment->element(trk);
                                    if (e && e->isChordRest()) {
                                          ChordRest* cr = static_cast<ChordRest*>(e);
                                          if (cr->durationType() == TDuration::V_MEASURE)
                                                n = nl;
                                          else
                                                n += cr->actualTicks();
                                          score()->undoRemoveElement(e);
                                          if (segment->isEmpty())
                                                score()->undoRemoveElement(segment);
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
                              Segment* seg = undoGetSegment(Segment::SegChordRest, rtick);
                              TDuration d;
                              d.setVal(n);
                              rest = new Rest(score(), d);
                              rest->setDuration(d.fraction());
                              rest->setTrack(staffIdx * VOICES + voice);
                              rest->setParent(seg);
                              score()->undoAddElement(rest);
                              }
                        }
                  }
            }
      score()->undo(new ChangeMeasureLen(this, nf));
      if (diff < 0) {
            //
            //  CHECK: do not remove all slurs
            //
            foreach(Element* e, _el) {
                  if (e->type() == SLUR)
                        score()->undoRemoveElement(e);
                  }
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Measure::write(Xml& xml, int staff, bool writeSystemElements) const
      {
      int mno = _no + 1;
      if (_len != _timesig) {
            // this is an irregular measure
            xml.stag(QString("Measure number=\"%1\" len=\"%2/%3\"").arg(mno).arg(_len.numerator()).arg(_len.denominator()));
            }
      else
            xml.stag(QString("Measure number=\"%1\"").arg(mno));
      xml.curTick = tick();

      if (writeSystemElements) {
            if (_repeatFlags & RepeatStart)
                  xml.tagE("startRepeat");
            if (_repeatFlags & RepeatEnd)
                  xml.tag("endRepeat", _repeatCount);
            if (_irregular)
                  xml.tagE("irregular");
            if (_breakMultiMeasureRest)
                  xml.tagE("breakMultiMeasureRest");
            if (_userStretch != 1.0)
                  xml.tag("stretch", _userStretch);
            if (_noOffset)
                  xml.tag("noOffset", _noOffset);
#if 0
            if (_noText && !_noText->generated()) {
                  xml.stag("MeasureNumber");
                  _noText->writeProperties(xml);
                  xml.etag();
                  }
#endif
            }
      qreal _spatium = spatium();
      MStaff* mstaff = staves[staff];
      if (mstaff->noText() && !mstaff->noText()->generated()) {
            xml.stag("MeasureNumber");
            mstaff->noText()->writeProperties(xml);
            xml.etag();
            }

      if (mstaff->_vspacerUp)
            xml.tag("vspacerUp", mstaff->_vspacerUp->gap() / _spatium);
      if (mstaff->_vspacerDown)
            xml.tag("vspacerDown", mstaff->_vspacerDown->gap() / _spatium);
      if (!mstaff->_visible)
            xml.tag("visible", mstaff->_visible);
      if (mstaff->_slashStyle)
            xml.tag("slashStyle", mstaff->_slashStyle);

      int strack = staff * VOICES;
      int etrack = strack + VOICES;
      foreach (const Element* el, _el) {
            if (!el->generated() && ((el->staffIdx() == staff) || (el->systemFlag() && writeSystemElements))) {
                  el->write(xml);
                  }
            }
      Q_ASSERT(first());
      Q_ASSERT(last());
      score()->writeSegments(xml, this, strack, etrack, first(), last()->next1(), writeSystemElements, false, false);
      xml.etag();
      }

//---------------------------------------------------------
//   Measure::read
//---------------------------------------------------------

void Measure::read(XmlReader& e, int staffIdx)
      {
      Segment* segment = 0;
      qreal _spatium = spatium();

      QList<Chord*> graceNotes;

      //sort tuplet elements. needed for nested tuplets #22537
      if (score()->mscVersion() <= 114) {
            for(Tuplet* t : e.tuplets()) {
                  t->sortElements();
                  }
            }
      e.tuplets().clear();
      e.setTrack(staffIdx * VOICES);

      for (int n = staves.size(); n <= staffIdx; ++n) {
            Staff* staff = score()->staff(n);
            MStaff* s    = new MStaff;
            s->lines     = new StaffLines(score());
            s->lines->setParent(this);
            s->lines->setTrack(n * VOICES);
            s->lines->setVisible(!staff->invisible());
            staves.append(s);
            }

      // tick is obsolete
      if (e.hasAttribute("tick"))
            e.setTick(score()->fileDivision(e.intAttribute("tick")));
      // setTick(e.tick());
      // e.setTick(tick());

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

            if (tag == "tick")
                  e.setTick(e.readInt());
            else if (tag == "BarLine") {
                  BarLine* barLine = new BarLine(score());
                  barLine->setTrack(e.track());
                  barLine->read(e);
                  if ((e.tick() != tick()) && (e.tick() != (tick() + ticks()))) {
                        // this is a mid measure bar line
                        segment = getSegment(Segment::SegBarLine, e.tick());
                        }
                  else if (barLine->barLineType() == START_REPEAT)
                        segment = getSegment(Segment::SegStartRepeatBarLine, e.tick());
                  else {
                        setEndBarLineType(barLine->barLineType(), false, true);
                        if(!barLine->customSpan()) {
                              Staff* staff = score()->staff(staffIdx);
                              barLine->setSpan(staff->barLineSpan());
                              }
                        segment = getSegment(Segment::SegEndBarLine, e.tick());
                        }
                  segment->add(barLine);
                  }
            else if (tag == "Chord") {

                  Chord* chord = new Chord(score());
                  chord->setTrack(e.track());
                  chord->read(e);
                  segment = getSegment(Segment::SegChordRest, e.tick());

                  if (chord->noteType() != NOTE_NORMAL)
                        graceNotes.push_back(chord);
                  else {
                        segment->add(chord);
                        Q_ASSERT(segment->segmentType() == Segment::SegChordRest);

                        for (int i = 0; i < graceNotes.size(); ++i) {
                              Chord* gc = graceNotes[i];
                              gc->setGraceIndex(i);
                              chord->add(gc);
                              }
                        graceNotes.clear();

                        Fraction ts(timeStretch * chord->globalDuration());
                        int crticks = ts.ticks();

                        if (chord->tremolo() && chord->tremolo()->tremoloType() < TREMOLO_R8) {
                              //
                              // old style tremolo found
                              //
                              Tremolo* tremolo = chord->tremolo();
                              TremoloType st;
                              switch (tremolo->tremoloType()) {
                                    default:
                                    case OLD_TREMOLO_R8:  st = TREMOLO_R8;  break;
                                    case OLD_TREMOLO_R16: st = TREMOLO_R16; break;
                                    case OLD_TREMOLO_R32: st = TREMOLO_R32; break;
                                    case OLD_TREMOLO_C8:  st = TREMOLO_C8;  break;
                                    case OLD_TREMOLO_C16: st = TREMOLO_C16; break;
                                    case OLD_TREMOLO_C32: st = TREMOLO_C32; break;
                                    }
                              tremolo->setTremoloType(st);
                              if (tremolo->twoNotes()) {
                                    int track = chord->track();
                                    Segment* ss = 0;
                                    for (Segment* ps = first(Segment::SegChordRest); ps; ps = ps->next(Segment::SegChordRest)) {
                                          if (ps->tick() >= e.tick())
                                                break;
                                          if (ps->element(track))
                                                ss = ps;
                                          }
                                    Chord* pch = 0;       // previous chord
                                    if (ss) {
                                          ChordRest* cr = static_cast<ChordRest*>(ss->element(track));
                                          if (cr && cr->type() == CHORD)
                                                pch = static_cast<Chord*>(cr);
                                          }
                                    if (pch) {
                                          tremolo->setParent(pch);
                                          pch->setTremolo(tremolo);
                                          chord->setTremolo(0);
                                          // force duration to half
                                          Fraction pts(timeStretch * pch->globalDuration());
                                          int pcrticks = pts.ticks();
                                          pch->setDuration(pcrticks / 2);
                                          chord->setDuration(crticks / 2);
                                          }
                                    else {
                                          qDebug("tremolo: first note not found");
                                          }
                                    crticks /= 2;
                                    }
                              else {
                                    tremolo->setParent(chord);
                                    }
                              }
                        e.rtick() += crticks;
                        }
                  }
            else if (tag == "Rest") {
                  Rest* rest = new Rest(score());
                  rest->setDurationType(TDuration::V_MEASURE);
                  rest->setDuration(timesig()/timeStretch);
                  rest->setTrack(e.track());
                  rest->read(e);

                  segment = getSegment(rest, e.tick());
                  segment->add(rest);
                  if (!rest->duration().isValid())     // hack
                        rest->setDuration(timesig()/timeStretch);
                  Fraction ts(timeStretch * rest->globalDuration());

                  e.rtick() += ts.ticks();
                  }
            else if (tag == "Breath") {
                  Breath* breath = new Breath(score());
                  breath->setTrack(e.track());
                  breath->read(e);
                  segment = getSegment(Segment::SegBreath, e.tick());
                  segment->add(breath);
                  }
            else if (tag == "endSpanner") {
                  int id = e.attribute("id").toInt();
                  Spanner* spanner = score()->findSpanner(id);
                  if (spanner) {
                        spanner->setTick2(e.tick());
                        if(spanner->track2() == -1)
                              spanner->setTrack2(e.track());
                        if (spanner->type() == OTTAVA) {
                              Ottava* o = static_cast<Ottava*>(spanner);
                              o->staff()->updateOttava(o);
                              }
                        else if (spanner->type() == HAIRPIN) {
                              Hairpin* hp = static_cast<Hairpin*>(spanner);
                              score()->updateHairpin(hp);
                              }
                        }
                  else
                        qDebug("Measure::read(): cannot find spanner %d", id);
                  e.readNext();
                  }
            else if (tag == "HairPin"
               || tag == "Pedal"
               || tag == "Ottava"
               || tag == "Trill"
               || tag == "TextLine"
               || tag == "Slur"
               || tag == "Volta") {
                  Spanner* sp = static_cast<Spanner*>(Element::name2Element(tag, score()));
                  sp->setTrack(e.track());
                  sp->setTick(e.tick());
                  sp->setAnchor(Spanner::ANCHOR_SEGMENT);
                  sp->read(e);
                  score()->addSpanner(sp);
                  }
            else if (tag == "RepeatMeasure") {
                  RepeatMeasure* rm = new RepeatMeasure(score());
                  rm->setTrack(e.track());
                  rm->read(e);
                  segment = getSegment(Segment::SegChordRest, e.tick());
                  segment->add(rm);
                  e.setTick(e.tick() + ticks());
                  }
            else if (tag == "Clef") {
                  Clef* clef = new Clef(score());
                  clef->setTrack(e.track());
                  clef->read(e);
                  clef->setGenerated(false);

                  // there may be more than one clef segment for same tick position
                  if (!segment)
                        segment = getSegment(Segment::SegClef, e.tick());
                  else {
                        Segment* ns = 0;
                        if (segment->next()) {
                              ns = segment->next();
                              while (ns && ns->tick() < e.tick())
                                    ns = ns->next();
                              }
                        segment = 0;
                        for (Segment* s = ns; s && s->tick() == e.tick(); s = s->next()) {
                              if (s->segmentType() == Segment::SegClef) {
                                    segment = s;
                                    break;
                                    }
                              }
                        if (!segment) {
                              segment = new Segment(this, Segment::SegClef, e.tick());
                              _segments.insert(segment, ns);
                              }
                        }
                  segment->add(clef);
                  }
            else if (tag == "TimeSig") {
                  TimeSig* ts = new TimeSig(score());
                  ts->setTrack(e.track());
                  ts->read(e);
                  segment = getSegment(Segment::SegTimeSig, e.tick());
                  segment->add(ts);
                  timeStretch = ts->stretch().reduced();

                  _timesig = ts->sig() * timeStretch;

                  if (score()->mscVersion() > 114) {
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
                  int tick = e.tick();
                  segment = getSegment(Segment::SegKeySig, tick);
                  segment->add(ks);
                  staff->setKey(tick, ks->keySigEvent());
                  }
            else if (tag == "Lyrics") {       // obsolete, keep for compatibility with version 114
                  Element* element = Element::name2Element(tag, score());
                  element->setTrack(e.track());
                  element->read(e);
                  segment       = getSegment(Segment::SegChordRest, e.tick());
                  ChordRest* cr = static_cast<ChordRest*>(segment->element(e.track()));
                  if (!cr)
                        qDebug("Internal Error: no Chord/Rest for lyrics");
                  else
                        cr->add(element);
                  }
            else if (tag == "Text") {
                  Text* t = new Text(score());
                  t->setTrack(e.track());
                  t->read(e);
                  // previous versions stored measure number, delete it
                  if ((score()->mscVersion() <= 114) && (t->textStyleType() == TEXT_STYLE_MEASURE_NUMBER))
                        delete t;
                  else {
                        segment = getSegment(Segment::SegChordRest, e.tick());
                        segment->add(t);
                        }
                  }

            //----------------------------------------------------
            // Annotation

            else if (tag == "Dynamic") {
                  Dynamic* dyn = new Dynamic(score());
                  dyn->setTrack(e.track());
                  dyn->read(e);
                  if (score()->mscVersion() <= 114)
                        dyn->setDynamicType(dyn->text());
                  segment = getSegment(Segment::SegChordRest, e.tick());
                  segment->add(dyn);
                  }
            else if (tag == "Harmony"
               || tag == "FretDiagram"
               || tag == "Symbol"
               || tag == "Tempo"
               || tag == "StaffText"
               || tag == "RehearsalMark"
               || tag == "InstrumentChange"
               || tag == "Marker"
               || tag == "Jump"
               || tag == "StaffState"
               || tag == "FiguredBass"
               || tag == "Image"
               ) {
                  Element* el = Element::name2Element(tag, score());
                  el->setTrack(e.track());
                  el->read(e);
                  segment = getSegment(Segment::SegChordRest, e.tick());
                  segment->add(el);
                  }
            //----------------------------------------------------
            else if (tag == "stretch")
                  _userStretch = e.readDouble();
            else if (tag == "LayoutBreak") {
                  LayoutBreak* lb = new LayoutBreak(score());
                  lb->read(e);
                  add(lb);
                  }
            else if (tag == "noOffset")
                  _noOffset = e.readInt();
            else if (tag == "irregular") {
                  _irregular = true;
                  e.readNext();
                  }
            else if (tag == "breakMultiMeasureRest") {
                  _breakMultiMeasureRest = true;
                  e.readNext();
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
                  _repeatFlags |= RepeatStart;
                  e.readNext();
                  }
            else if (tag == "endRepeat") {
                  _repeatCount = e.readInt();
                  _repeatFlags |= RepeatEnd;
                  }
            else if (tag == "vspacer" || tag == "vspacerDown") {
                  if (staves[staffIdx]->_vspacerDown == 0) {
                        Spacer* spacer = new Spacer(score());
                        spacer->setSpacerType(SPACER_DOWN);
                        spacer->setTrack(staffIdx * VOICES);
                        add(spacer);
                        }
                  staves[staffIdx]->_vspacerDown->setGap(e.readDouble() * _spatium);
                  }
            else if (tag == "vspacer" || tag == "vspacerUp") {
                  if (staves[staffIdx]->_vspacerUp == 0) {
                        Spacer* spacer = new Spacer(score());
                        spacer->setSpacerType(SPACER_UP);
                        spacer->setTrack(staffIdx * VOICES);
                        add(spacer);
                        }
                  staves[staffIdx]->_vspacerUp->setGap(e.readDouble() * _spatium);
                  }
            else if (tag == "visible")
                  staves[staffIdx]->_visible = e.readInt();
            else if (tag == "slashStyle")
                  staves[staffIdx]->_slashStyle = e.readInt();
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
                  noText->setFlag(ELEMENT_ON_STAFF, true);
                  if (noText->track() == -1)
                        noText->setTrack(0);
                  noText->setParent(this);
                  staves[noText->staffIdx()]->setNoText(noText);
                  }
            else if (Element::readProperties(e))
                  ;
            else
                  e.unknown();
            }
      if (staffIdx == 0) {
            Segment* s = last();
            if (s && s->segmentType() == Segment::SegBarLine) {
                  BarLine* b = static_cast<BarLine*>(s->element(0));
                  setEndBarLineType(b->barLineType(), false, b->visible(), b->color());
                  // s->remove(b);
                  // delete b;
                  }
            }
      //
      // for compatibility with 1.22:
      //
      if (score()->mscVersion() == 122) {
            int ticks1 = 0;
            for (Segment* s = last(); s; s = s->prev()) {
                  if (s->segmentType() == Segment::SegChordRest) {
                        if (s->element(0)) {
                              ChordRest* cr = static_cast<ChordRest*>(s->element(0));
                              if (cr->type() == REPEAT_MEASURE)
                                    ticks1 = ticks();
                              else
                                    ticks1 = s->rtick() + cr->actualTicks();
                              break;
                              }
                        }
                  }
            if (ticks() != ticks1) {
                  // this is a irregular measure
                  _len = Fraction::fromTicks(ticks1);
                  _len.reduce();
                  for (Segment* s = last(); s; s = s->prev()) {
                        if (s->tick() < tick() + ticks())
                              break;
                        if (s->segmentType() == Segment::SegBarLine) {
                              qDebug("reduce BarLine to EndBarLine");
                              s->setSegmentType(Segment::SegEndBarLine);
                              }
                        }

                  }
            }
      foreach (Tuplet* tuplet, e.tuplets()) {
            if (tuplet->elements().isEmpty()) {
                  // this should not happen and is a sign of input file corruption
                  qDebug("Measure:read(): empty tuplet id %d (%p), input file corrupted?",
                     tuplet->id(), tuplet);
                  delete tuplet;
                  }
            else
                  tuplet->setParent(this);
            }
      }

//---------------------------------------------------------
//   visible
//---------------------------------------------------------

bool Measure::visible(int staffIdx) const
      {
      if (system() && (system()->staves()->isEmpty() || !system()->staff(staffIdx)->show()))
            return false;
      return score()->staff(staffIdx)->show() && staves[staffIdx]->_visible;
      }

//---------------------------------------------------------
//   slashStyle
//---------------------------------------------------------

bool Measure::slashStyle(int staffIdx) const
      {
      return score()->staff(staffIdx)->slashStyle() || staves[staffIdx]->_slashStyle || score()->staff(staffIdx)->staffType()->slashStyle();
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
            MStaff* ms = staves[staffIdx];
            if (ms->lines)
                  func(data, ms->lines);
            if (ms->_vspacerUp)
                  func(data, ms->_vspacerUp);
            if (ms->_vspacerDown)
                  func(data, ms->_vspacerDown);
            if (ms->noText())
                  func(data, ms->noText());
            }

      int tracks = nstaves * VOICES;
      for (Segment* s = first(); s; s = s->next()) {
            for (int track = 0; track < tracks; ++track) {
                  int staffIdx = track/VOICES;
                  if (!all && !(visible(staffIdx) && score()->staff(staffIdx)->show())) {
                        track += VOICES - 1;
                        continue;
                        }
                  Element* e = s->element(track);
                  if (e == 0)
                        continue;
                  e->scanElements(data, func, all);
                  }
            foreach(Element* e, s->annotations()) {
                  if (all || e->systemFlag() || visible(e->staffIdx()))
                        e->scanElements(data,  func, all);
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
            if (s->segmentType() != Segment::SegChordRest)
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

bool Measure::setStartRepeatBarLine(bool val)
      {
      bool changed = false;
      Segment* s = findSegment(Segment::SegStartRepeatBarLine, tick());

      for (int staffIdx = 0; staffIdx < score()->nstaves();) {
            int track    = staffIdx * VOICES;
            Staff* staff = score()->staff(staffIdx);
            BarLine* bl  = s ? static_cast<BarLine*>(s->element(track)) : 0;
            int span     = staff->barLineSpan();

            if (span && val && (bl == 0)) {
                  // no barline were we need one:
                  bl = new BarLine(score());
                  bl->setTrack(track);
                  bl->setBarLineType(START_REPEAT);
                  if (s == 0) {
                        if (score()->undoRedo()) {
                              return false;
                              }
                        s = undoGetSegment(Segment::SegStartRepeatBarLine, tick());
                        }
                  bl->setParent(s);
                  score()->undoAddElement(bl);
                  changed = true;
                  }
            else if (bl && !val) {
                  // barline were we do not need one:
                  score()->undoRemoveElement(bl);
                  changed = true;
                  }
            if (bl && val && span) {
                  bl->setSpan(span);
                  bl->setSpanFrom(staff->barLineFrom());
                  bl->setSpanTo(staff->barLineTo());
                  }

            ++staffIdx;
            //
            // remove any unwanted barlines:
            //
            if (s) {
                  for (int i = 1; i < span; ++i) {
                        BarLine* bl  = static_cast<BarLine*>(s->element(staffIdx * VOICES));
                        if (bl) {
                              score()->undoRemoveElement(bl);
                              changed = true;
                              }
                        ++staffIdx;
                        }
                  }
            }
      return changed;
      }

//---------------------------------------------------------
//   createEndBarLines
//    actually creates or modifies barlines
//    returns true if layout changes
//---------------------------------------------------------

bool Measure::createEndBarLines()
      {
      bool changed = false;
      int nstaves  = score()->nstaves();
      Segment* seg = undoGetSegment(Segment::SegEndBarLine, tick() + ticks());

      BarLine* bl = 0;
      int span    = 0;        // span counter
      int aspan   = 0;        // actual span
      bool mensur = false;    // keep note of mensurstrich case
      int spanTot;            // to keep track of the target span
      int spanFrom;
      int spanTo;

      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            Staff* staff = score()->staff(staffIdx);
            int track   = staffIdx * VOICES;
            // get existing bar line for this staff, if any
            BarLine* cbl = static_cast<BarLine*>(seg->element(track));
            // if span counter has been counted off, get new span values
            // and forget about any previous bar line
            if (span == 0) {
                  if(cbl && cbl->customSpan()) {      // if there is a bar line and has custom span,
                        span        = cbl->span();    // get span values from it
                        spanFrom    = cbl->spanFrom();
                        spanTo      = cbl->spanTo();
                        // if bar span values == staff span values, set bar as not custom
                        if(span == staff->barLineSpan() && spanFrom == staff->barLineFrom()
                           && spanTo == staff->barLineTo())
                              cbl->setCustomSpan(false);
                        }
                  else {                              // otherwise, get from staff
                        span        = staff->barLineSpan();
                        // if some span OR last staff (span=0) of a mensurstrich case, get From/To from staff
                        if (span || mensur) {
                              spanFrom    = staff->barLineFrom();
                              spanTo      = staff->barLineTo();
                              mensur      = false;
                              }
                        // but if staff is set to no span, a multi-staff spanning bar line
                        // has been shortened to span less staves and following staves left without bars;
                        // set bar line span values to default
                        else {
                              span        = 1;
                              spanFrom    = 0;
                              spanTo      = (staff->lines()-1)*2;
                              }
                        }
                  if ((staffIdx + span) > nstaves)
                        span = nstaves - staffIdx;
                  spanTot     = span;
                  bl = 0;
                  }
            if (staff->show() && span) {
                  //
                  // there should be a barline in this staff
                  //
                  // if we already have a bar line, keep extending this bar line down until span exhausted;
                  // if no barline yet, re-use the bar line existing in this staff if any,
                  // restarting actual span
                  if (!bl) {
                        bl = cbl;
                        aspan = 0;
                        }
                  if (!bl) {
                        // no suitable bar line: create a new one
                        bl = new BarLine(score());
                        bl->setVisible(_endBarLineVisible);
                        bl->setColor(_endBarLineColor);
                        bl->setGenerated(bl->el()->empty() && _endBarLineGenerated);
                        bl->setBarLineType(_endBarLineType);
                        bl->setParent(seg);
                        bl->setTrack(track);
                        score()->undoAddElement(bl);
                        changed = true;
                        }
                  else {
                        // a bar line is there (either existing or newly created):
                        // adjust subtype, if not fitting
                        if (bl->barLineType() != _endBarLineType && !bl->customSubtype()) {
                              score()->undoChangeProperty(bl, P_SUBTYPE, _endBarLineType);
                              bl->setGenerated(bl->el()->empty() && _endBarLineGenerated);
                              changed = true;
                              }
                        // or clear custom subtype flag if same type as measure
                        if (bl->barLineType() == _endBarLineType && bl->customSubtype())
                              bl->setCustomSubtype(false);
                        // if a bar line exists for this staff (cbl) but
                        // it is not the bar line we are dealing with (bl),
                        // we are extending down the bar line of a staff above (bl)
                        // and the bar line for this staff (cbl) is not needed:
                        // DELETE it
                        if (cbl && cbl != bl) {
                              // mensurstrich special case:
                              // if span arrives inside the end staff (spanTo>0) OR
                              //          span is not multi-staff (spanTot<=1) OR
                              //          current staff is not the last spanned staff (span!=1) OR
                              //          staff is the last score staff
                              //    remove bar line for this staff
                              // If NONE of the above conditions holds, the staff is the last staff of
                              // a mensurstrich(-like) span: keep its bar line, as it may span to next staff
                              if (spanTo > 0 || spanTot <= 1 || span != 1 || staffIdx == nstaves-1) {
                                    score()->undoRemoveElement(cbl);
                                    changed = true;
                                    }
                              }
                        }
                  }
            else {
                  //
                  // there should be no barline in this staff
                  //
                  if (cbl) {
                        score()->undoRemoveElement(cbl);
                        changed = true;
                        }
                  }
            // if span not counted off AND we have a bar line AND this staff is shown,
            // set bar line span values (this may result in extending down a bar line
            // for a previous staff, if we are counting off a span > 1)
            if (span) {
                  if (bl) {
                        ++aspan;
                        if (staff->show()) {          // update only if visible
                              bl->setSpan(aspan);
                              bl->setSpanFrom(spanFrom);
                              // if current actual span < target span, set spanTo to full staff height
                              if(aspan < spanTot)
                                    bl->setSpanTo((staff->lines()-1)*2);
                              // if we reached target span, set spanTo to intended value
                              else
                                    bl->setSpanTo(spanTo);
                              }
                        }
                  --span;
                  }
            // if just finished (span==0) a multi-staff span (spanTot>1) ending at the top of a staff (spanTo<=0)
            // scan this staff again, as it may have its own bar lines (mensurstich(-like) span)
            if (spanTot > 1 && spanTo <= 0 && span == 0) {
                  mensur = true;
                  staffIdx--;
                  }
            }
      return changed;
      }

//---------------------------------------------------------
//   setEndBarLineType
//---------------------------------------------------------

void Measure::setEndBarLineType(BarLineType val, bool g, bool visible, QColor color)
      {
      _endBarLineType      = val;
      _endBarLineGenerated = g;
      _endBarLineVisible   = visible;
      if(color.isValid())
            _endBarLineColor = color;
      else
            _endBarLineColor = curColor();
      }

//---------------------------------------------------------
//   sortStaves
//---------------------------------------------------------

void Measure::sortStaves(QList<int>& dst)
      {
      QList<MStaff*> ms;
      foreach (int idx, dst)
            ms.push_back(staves[idx]);
      staves = ms;

      for (int staffIdx = 0; staffIdx < staves.size(); ++staffIdx) {
            if (staves[staffIdx]->lines)
                  staves[staffIdx]->lines->setTrack(staffIdx * VOICES);
            }
      for (Segment* s = first(); s; s = s->next())
            s->sortStaves(dst);

      foreach(Element* e, _el) {
            if (e->track() == -1)
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

void Measure::exchangeVoice(int v1, int v2, int staffIdx1, int staffIdx2)
      {
      for (int staffIdx = staffIdx1; staffIdx < staffIdx2; ++ staffIdx) {
            for (Segment* s = first(Segment::SegChordRest); s; s = s->next(Segment::SegChordRest)) {
                  int strack = staffIdx * VOICES + v1;
                  int dtrack = staffIdx * VOICES + v2;
                  s->swapElements(strack, dtrack);
                  }
            MStaff* ms = mstaff(staffIdx);
            ms->hasVoices = true;
            }
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
      staves[staffIdx]->hasVoices = false;
      for (Segment* s = first(); s; s = s->next()) {
            if (s->segmentType() != Segment::SegChordRest)
                  continue;
            for (int track = strack; track < etrack; ++track) {
                  if (s->element(track)) {
                        staves[staffIdx]->hasVoices = true;
                        return;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   hasVoice
//---------------------------------------------------------

bool Measure::hasVoice(int track) const
      {
      for (Segment* s = first(); s; s = s->next()) {
            if (s->segmentType() != Segment::SegChordRest)
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

bool Measure::isMeasureRest(int staffIdx)
      {
      int strack;
      int etrack;
      if (staffIdx < 0) {
            strack = 0;
            etrack = score()->nstaves() * VOICES;
            }
      else {
            strack = staffIdx * VOICES;
            etrack = staffIdx * VOICES + VOICES;
            }
      for (Segment* s = first(Segment::SegChordRest); s; s = s->next(Segment::SegChordRest)) {
            for (int track = strack; track < etrack; ++track) {
                  Element* e = s->element(track);
                  if (e && e->type() != REST)
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

bool Measure::isFullMeasureRest()
      {
      int strack = 0;
      int etrack = score()->nstaves() * VOICES;

      Segment* s = first(Segment::SegChordRest);
      for (int track = strack; track < etrack; ++track) {
            Element* e = s->element(track);
            if (e) {
                  if (e->type() != REST)
                        return false;
                  Rest* rest = static_cast<Rest*>(e);
                  if (rest->durationType().type() != TDuration::V_MEASURE)
                        return false;
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   isRepeatMeasure
//---------------------------------------------------------

bool Measure::isRepeatMeasure(Part* part)
      {
      int firstStaffIdx = score()->staffIdx(part);
      int nextStaffIdx  = firstStaffIdx + part->nstaves();
      int strack        = firstStaffIdx * VOICES;
      int etrack        = nextStaffIdx * VOICES;
      Segment* s        = first(Segment::SegChordRest);

      if (s == 0)
            return false;

      for (int track = strack; track < etrack; ++track) {
            Element* e = s->element(track);
            if (e && e->type() == REPEAT_MEASURE)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   distanceDown
//---------------------------------------------------------

qreal Measure::distanceDown(int i) const
      {
      if (staves[i]->_vspacerDown)
            return qMax(staves[i]->distanceDown, staves[i]->_vspacerDown->gap());
      return staves[i]->distanceDown;
      }

//---------------------------------------------------------
//   distanceUp
//---------------------------------------------------------

qreal Measure::distanceUp(int i) const
      {
      if (staves[i]->_vspacerUp)
            return qMax(staves[i]->distanceUp, staves[i]->_vspacerUp->gap());
      return staves[i]->distanceUp;
      }

//---------------------------------------------------------
//   isEmpty
//---------------------------------------------------------

bool Measure::isEmpty() const
      {
      if (_irregular)
            return false;
      int n = 0;
      int tracks = staves.size() * VOICES;
      static const Segment::SegmentType st = Segment::SegChordRest;
      for (const Segment* s = first(st); s; s = s->next(st)) {
            bool restFound = false;
            for (int track = 0; track < tracks; ++track) {
                  if ((track % VOICES) == 0 && !score()->staff(track/VOICES)->show()) {
                        track += VOICES-1;
                        continue;
                        }
                  if (s->element(track))  {
                        if (s->element(track)->type() != REST)
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
//   Space::max
//---------------------------------------------------------

void Space::max(const Space& s)
      {
      if (s._lw > _lw)
            _lw = s._lw;
      if (s._rw > _rw)
            _rw = s._rw;
      }

//---------------------------------------------------------
//   setDirty
//---------------------------------------------------------

void Measure::setDirty()
      {
      _minWidth1 = 0.0;
      _minWidth2 = 0.0;
      }

//---------------------------------------------------------
//   systemHeader
///   return true if the measure contains a system header
//    The system header is identified by a generated Clef in
//    the first segment.
//---------------------------------------------------------

bool Measure::systemHeader() const
      {
      Segment* s = first();
      return s && (s->segmentType() == Segment::SegClef) && s->element(0) && s->element(0)->generated();
      }

//---------------------------------------------------------
//   minWidth1
///   return minimum width of measure excluding system
///   header
//---------------------------------------------------------

qreal Measure::minWidth1() const
      {
      if (_minWidth1 == 0.0) {
            Segment* s = first();
            Segment::SegmentTypes st = Segment::SegClef | Segment::SegKeySig | Segment::SegStartRepeatBarLine;
            while ((s->segmentType() & st)
               && s->next()
               && (!s->element(0) || s->element(0)->generated())
               ) {
                  s = s->next();
                  }
            _minWidth1 = score()->computeMinWidth(s);
            }
      return _minWidth1;
      }

//---------------------------------------------------------
//   minWidth2
///   return minimum width of measure
//---------------------------------------------------------

qreal Measure::minWidth2() const
      {
      if (_minWidth2 == 0.0)
            _minWidth2 = score()->computeMinWidth(first());
      return _minWidth2;
      }

//-----------------------------------------------------------------------------
//    computeStretch
///   \brief distribute stretch across a range of segments
//-----------------------------------------------------------------------------

void computeStretch(int minTick, qreal minimum, qreal stretch, int first, int last, int ticksList[], qreal xpos[], qreal width[])
      {
      SpringMap springs;
      for (int i = first; i < last; ++i) {
            qreal str = 1.0;
            qreal d;
            qreal w = width[i];

            int t = ticksList[i];
            if (t) {
                  if (minTick > 0)
                        // str += .6 * log(qreal(t) / qreal(minTick)) / log(2.0);
                        str = 1.0 + 0.865617 * log(qreal(t) / qreal(minTick));
                  d = w / str;
                  }
            else {
                  str = 0.0;              // dont stretch timeSig and key
                  d   = 100000000.0;      // CHECK
                  }
            springs.insert(std::pair<qreal, Spring>(d, Spring(i, str, w)));
            minimum += w;
            }

      //---------------------------------------------------
      //    distribute stretch to segments
      //---------------------------------------------------

      qreal force = sff(stretch, minimum, springs);

      for (iSpring i = springs.begin(); i != springs.end(); ++i) {
            qreal stretch = force * i->second.stretch;
            if (stretch < i->second.fix)
                  stretch = i->second.fix;
            width[i->second.seg] = stretch;
            }
      qreal x = xpos[first];
      for (int i = first; i < last; ++i) {
            x += width[i];
            xpos[i+1] = x;
            }
      }

//-----------------------------------------------------------------------------
//    layoutX
///   \brief main layout routine for note spacing
///   Return width of measure (in MeasureWidth), taking into account \a stretch.
//-----------------------------------------------------------------------------

void Measure::layoutX(qreal stretch)
      {
      int nstaves = _score->nstaves();

      int segs = 0;
      for (const Segment* s = first(); s; s = s->next()) {
            if (s->segmentType() == Segment::SegClef && (s != first()))
                  continue;
            ++segs;
            }

      if (nstaves == 0 || segs == 0)
            return;

      qreal _spatium           = spatium();
      int tracks               = nstaves * VOICES;
      qreal clefKeyRightMargin = score()->styleS(ST_clefKeyRightMargin).val() * _spatium;
      qreal minHarmonyDistance = score()->styleS(ST_minHarmonyDistance).val() * _spatium;

      qreal rest[nstaves];    // fixed space needed from previous segment
      memset(rest, 0, nstaves * sizeof(qreal));

      qreal hRest[nstaves];    // fixed space needed from previous harmony
      memset(hRest, 0, nstaves * sizeof(qreal));

      //--------tick table for segments
      int ticksList[segs];
      memset(ticksList, 0, segs * sizeof(int));

      qreal xpos[segs+1];
      qreal width[segs];

      int segmentIdx = 0;
      qreal x        = 0.0;
      qreal lastx    = 0.0;
      int minTick    = 100000;
      int hMinTick   = 100000;
      int hLastIdx   = -1;
      int ntick      = ticks();   // position of next measure

      if (system()->firstMeasure() == this && system()->barLine()) {
            BarLine* bl = system()->barLine();
            x += BarLine::layoutWidth(score(), bl->barLineType(), bl->magS());
            }

      qreal minNoteDistance = score()->styleS(ST_minNoteDistance).val() * _spatium;

      qreal clefWidth[nstaves];
      memset(clefWidth, 0, nstaves * sizeof(qreal));

      std::vector<QRectF> hLastBbox(nstaves);    // bbox of previous harmony to test vertical separation

      const Segment* s = first();
      const Segment* pSeg = 0;
      for (; s; s = s->next(), ++segmentIdx) {
            qreal elsp = s->extraLeadingSpace().val()  * _spatium;
            qreal etsp = s->extraTrailingSpace().val() * _spatium;
            if ((s->segmentType() == Segment::SegClef) && (s != first())) {
                  --segmentIdx;
                  for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                        if (!score()->staff(staffIdx)->show())
                              continue;
                        int track  = staffIdx * VOICES;
                        Element* e = s->element(track);
                        if (e) {
                              clefWidth[staffIdx] = e->width() + _spatium + elsp;
                              }
                        }
                  pSeg = s;
                  continue;
                  }
            bool gotHarmony = false;
            bool rest2[nstaves];
            bool hRest2[nstaves];
            Segment::SegmentType segType = s->segmentType();
            qreal segmentWidth     = 0.0;
            qreal harmonyWidth     = 0.0;
            qreal stretchDistance  = 0.0;
            int pt                 = pSeg ? pSeg->segmentType() : Segment::SegBarLine;

            for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                  if (!score()->staff(staffIdx)->show())
                        continue;
                  qreal minDistance = 0.0;
                  Space space;
                  Space hSpace;
                  QRectF hBbox;
                  int track  = staffIdx * VOICES;
                  bool found = false;
                  bool hFound = false;
                  bool eFound = false;
                  if (segType & (Segment::SegChordRest)) {
                        qreal llw = 0.0;
                        qreal rrw = 0.0;
                        Lyrics* lyrics = 0;
                        for (int voice = 0; voice < VOICES; ++voice) {
                              ChordRest* cr = static_cast<ChordRest*>(s->element(track+voice));
                              if (!cr)
                                    continue;
                              found = true;
                              if (pt & (Segment::SegStartRepeatBarLine | Segment::SegBarLine)) {
                                    // check for accidentals in chord
                                    bool accidental = false;
                                    if (cr->type() == Element::CHORD) {
                                          Chord* c = static_cast<Chord*>(cr);
                                          if (!c->graceNotes().empty())
                                                accidental = true;
                                          else {
                                                for (Note* note : c->notes()) {
                                                      if (note->accidental()) {
                                                            accidental = true;
                                                            break;
                                                            }
                                                      }
                                                }
                                          }
                                    // no distance to full measure rest
                                    if (!(cr->type() == REST && static_cast<Rest*>(cr)->durationType() == TDuration::V_MEASURE)) {
                                          StyleIdx si = accidental ? ST_barAccidentalDistance : ST_barNoteDistance;
                                          qreal sp    = score()->styleS(si).val() * _spatium;
                                          sp          += elsp;
                                          minDistance = qMax(minDistance, sp);
                                          }
                                    }
                              else if (pt & (Segment::SegChordRest)) {
                                    minDistance = qMax(minDistance, minNoteDistance);
                                    }
                              else {
                                    bool firstClef = (segmentIdx == 1) && (pt == Segment::SegClef);
                                    if ((pt & (Segment::SegKeySig | Segment::SegTimeSig)) || firstClef)
                                          minDistance = qMax(minDistance, clefKeyRightMargin);
                                    }
                              space.max(cr->space());
                              int n = cr->lyricsList().size();
                              for (int i = 0; i < n; ++i) {
                                    Lyrics* l = cr->lyricsList().at(i);
                                    if (!l || l->isEmpty())
                                          continue;
                                    lyrics = l;
                                    if (!lyrics->isMelisma()) {
                                          QRectF b(l->bbox().translated(l->pos()));
                                          llw = qMax(llw, -b.left());
                                          rrw = qMax(rrw, b.right());
                                          }
                                    }
                              }
                        if (lyrics) {
                              qreal y = lyrics->ipos().y() + score()->styleS(ST_lyricsMinBottomDistance).val() * _spatium;
                              if (y > staves[staffIdx]->distanceDown)
                                 staves[staffIdx]->distanceDown = y;
                              space.max(Space(llw, rrw));
                              }

                        // add spacing for chord symbols
                        foreach (const Element* e, s->annotations()) {
                              if (e->type() != Element::HARMONY || e->track() < track || e->track() >= track+VOICES)
                                    continue;
                              const Harmony* h = static_cast<const Harmony*>(e);
                              QRectF b(h->bboxtight().translated(h->pos()));
                              if (hFound)
                                    hBbox |= b;
                              else
                                    hBbox = b;
                              hFound = true;
                              gotHarmony = true;
                              // allow chord at the beginning of a measure to be dragged left
                              hSpace.max(Space(s->rtick()?-b.left():0.0, b.right()));
                              }
                        }
                  else {
                        Element* e = s->element(track);
                        if ((segType == Segment::SegClef) && (pt != Segment::SegChordRest))
                              minDistance = score()->styleS(ST_clefLeftMargin).val() * _spatium;
                        else if (segType == Segment::SegStartRepeatBarLine)
                              minDistance = .5 * _spatium;
                        else if ((segType == Segment::SegEndBarLine) && segmentIdx) {
                              if (pt == Segment::SegClef)
                                    minDistance = score()->styleS(ST_clefBarlineDistance).val() * _spatium;
                              else
                                    stretchDistance = score()->styleS(ST_noteBarDistance).val() * _spatium;
                              if (e == 0) {
                                    // look for barline
                                    for (int i = track - VOICES; i >= 0; i -= VOICES) {
                                          e = s->element(i);
                                          if (e)
                                                break;
                                          }
                                    }
                              }
                        if (e) {
                              eFound = true;
                              gotHarmony = true;      // to avoid closing barline
                              space.max(e->space());
                              }
                        }
                  space += Space(elsp, etsp);
                  if (isMMRest())
                        minDistance = 0;

                  if (found || eFound) {
                        space.rLw() += clefWidth[staffIdx];
                        qreal sp     = minDistance + rest[staffIdx] + qMax(space.lw(), stretchDistance);
                        rest[staffIdx]  = space.rw();
                        rest2[staffIdx] = false;
                        segmentWidth    = qMax(segmentWidth, sp);
                        }
                  else
                        rest2[staffIdx] = true;

                  // space chord symbols separately from segments
                  if (hFound || eFound) {
                        qreal sp = 0.0;

                        // space chord symbols unless they miss each other vertically
                        if (eFound || (hBbox.top() < hLastBbox[staffIdx].bottom() && hBbox.bottom() > hLastBbox[staffIdx].top()))
                              sp = hRest[staffIdx] + minHarmonyDistance + hSpace.lw();
                        hLastBbox[staffIdx] = hBbox;

                        hRest[staffIdx] = hSpace.rw();
                        hRest2[staffIdx] = false;
                        harmonyWidth = qMax(harmonyWidth, sp);
                        }
                  else
                        hRest2[staffIdx] = true;

                  clefWidth[staffIdx] = 0.0;
                  }

            // set previous seg width before adding in harmony, to allow stretching
            if (segmentIdx) {
                  width[segmentIdx-1] = segmentWidth;
                  if (pSeg)
                        pSeg->setbbox(QRectF(0.0, 0.0, segmentWidth, _spatium * 5));  //??
                  }

            // make room for harmony if needed
            segmentWidth = qMax(segmentWidth,harmonyWidth);

            x += segmentWidth;
            xpos[segmentIdx]  = x;

            for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                  if (!score()->staff(staffIdx)->show())
                        continue;
                  if (rest2[staffIdx])
                        rest[staffIdx] -= qMin(rest[staffIdx],segmentWidth);
                  if (hRest2[staffIdx])
                        hRest[staffIdx] -= qMin(hRest[staffIdx],segmentWidth);
                  }
            if ((s->segmentType() == Segment::SegChordRest)) {
                  const Segment* nseg = s;
                  for (;;) {
                        nseg = nseg->next();
                        if (nseg == 0 || nseg->segmentType() == Segment::SegChordRest)
                              break;
                        }
                  int nticks = (nseg ? nseg->rtick() : ntick) - s->rtick();
                  if (nticks == 0) {
                        // this happens for tremolo notes
                        qDebug("layoutX: empty segment(%p)%s: measure: tick %d ticks %d",
                           s, s->subTypeName(), tick(), ticks());
                        qDebug("         nticks==0 segmente %d, segmentIdx: %d, segTick: %d nsegTick(%p) %d",
                           size(), segmentIdx-1, s->tick(), nseg, ntick
                           );
                        }
                  else {
                        if (nticks < minTick)
                              minTick = nticks;
                        if (nticks < hMinTick)
                              hMinTick = nticks;
                        }
                  ticksList[segmentIdx] = nticks;
                  }
            else
                  ticksList[segmentIdx] = 0;

            // if we are on a chord symbol, stretch the notes below it if necessary
            if (gotHarmony) {
                  if (hLastIdx >= 0)
                        computeStretch(hMinTick, 0.0, x-lastx, hLastIdx, segmentIdx, ticksList, xpos, width);

                  hMinTick = 10000;
                  lastx = x;
                  hLastIdx = segmentIdx;
                  }

            //
            // set pSeg only to used segments
            //
            for (int voice = 0; voice < nstaves * VOICES; ++voice) {
                  if (!score()->staff(voice/VOICES)->show()) {
                        voice += VOICES-1;
                        continue;
                        }
                  if (s->element(voice)) {
                        pSeg = s;
                        break;
                        }
                  }
            }

      //---------------------------------------------------
      // TAB: compute distance above and below
      //---------------------------------------------------

      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            if (!score()->staff(staffIdx)->show())
                  continue;
            qreal distAbove = 0.0;
            qreal distBelow = 0.0;
            Staff * staff = _score->staff(staffIdx);
            if (staff->isTabStaff()) {
                  StaffTypeTablature* stt = static_cast<StaffTypeTablature*>(staff->staffType());
                  if (stt->slashStyle())        // if no stems
                        distAbove = stt->genDurations() ? -stt->durationBoxY() : 0.0;
                  else {                        // if stems
                        if (stt->stemsDown())
                              distBelow = (STAFFTYPE_TAB_DEFAULTSTEMLEN_UP + STAFFTYPE_TAB_DEFAULTSTEMDIST_UP)*_spatium;
                        else
                              distAbove = (STAFFTYPE_TAB_DEFAULTSTEMLEN_DN + STAFFTYPE_TAB_DEFAULTSTEMDIST_DN)*_spatium;
                        }
                  if (distAbove > staves[staffIdx]->distanceUp)
                     staves[staffIdx]->distanceUp = distAbove;
                  if (distBelow > staves[staffIdx]->distanceDown)
                     staves[staffIdx]->distanceDown = distBelow;
                  }
            }

      qreal segmentWidth = 0.0;
      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            if (!score()->staff(staffIdx)->show())
                  continue;
            segmentWidth = qMax(segmentWidth, rest[staffIdx]);
            segmentWidth = qMax(segmentWidth, hRest[staffIdx]);
            }
      if (segmentIdx)
            width[segmentIdx-1] = segmentWidth;
      xpos[segmentIdx]    = x + segmentWidth;

      //---------------------------------------------------
      // compute stretches for whole measure
      //---------------------------------------------------

      computeStretch(minTick, xpos[0], stretch, 0, segs, ticksList, xpos, width);

      //---------------------------------------------------
      //    layout individual elements
      //---------------------------------------------------

      int seg = 0;
      for (Segment* s = first(); s; s = s->next(), ++seg) {
            if ((s->segmentType() == Segment::SegClef) && (s != first())) {
                  //
                  // clefs are not in xpos[] table
                  //
                  s->setPos(xpos[seg], 0.0);
                  for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                        if (!score()->staff(staffIdx)->show())
                              continue;
                        int track  = staffIdx * VOICES;
                        Element* e = s->element(track);
                        if (e) {
                              qreal lm = 0.0;
                              if (s->next()) {
                                    for (int track = staffIdx * VOICES; track < staffIdx*VOICES+VOICES; ++track) {
                                          if (s->next()->element(track)) {
                                                qreal clm = s->next()->element(track)->space().lw();
                                                lm = qMax(lm, clm);
                                                }
                                          }
                                    }
                              e->setPos(-e->width() - lm - _spatium*.5, 0.0);
                              e->adjustReadPos();
                              }
                        }
                  --seg;
                  continue;
                  }
            s->setPos(xpos[seg], 0.0);
            }

      for (Segment* s = first(); s; s = s->next(), ++seg) {
            for (int track = 0; track < tracks; ++track) {
                  if (!score()->staff(track/VOICES)->show()) {
                        track += VOICES-1;
                        continue;
                        }
                  Element* e = s->element(track);
                  if (e == 0)
                        continue;
                  ElementType t = e->type();
                  Rest* rest = static_cast<Rest*>(e);
                  if (((track % VOICES) == 0) &&
                     (t == REPEAT_MEASURE || (t == REST && (isMMRest() || rest->isFullMeasureRest())))) {
                        //
                        // element has to be centered in free space
                        //    x1 - left measure position of free space
                        //    x2 - right measure position of free space

                        if (isMMRest()) {
                              //
                              // center multi measure rest
                              //
                              qreal x1 = 0.0, x2;
                              Segment* ss = first();
                              for (; ss->segmentType() != Segment::SegChordRest; ss = ss->next())
                                    ;
                              if (s != first()) {
                                    ss = ss->prev();
                                    for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
                                          int track = staffIdx * VOICES;
                                          Element* e = ss->element(track);
                                          if (e)
                                                x1 = qMax(x1, ss->x() + e->x() + e->width());
                                          }
                                    }
                              Segment* ns = s->next();
                              x2 = this->width();
                              if (ns) {
                                    x2 = ns->x();
                                    if (ns->segmentType() != Segment::SegEndBarLine) {
                                          for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
                                                int track = staffIdx * VOICES;
                                                Element* e = ns->element(track);
                                                if (e)
                                                      x2 = qMin(x2, ns->x() + e->x());
                                                }
                                          }
                                    }

                              qreal d  = point(score()->styleS(ST_multiMeasureRestMargin));
                              qreal w = x2 - x1 - 2 * d;

                              rest->setMMWidth(w);
                              StaffLines* sl = staves[track/VOICES]->lines;
                              qreal x = x1 - s->pos().x() + d;
                              e->setPos(x, sl->staffHeight() * .5);   // center vertically in measure
                              }
                        else { // if (rest->isFullMeasureRest()) {
                              //
                              // center full measure rest
                              //
                              qreal x1 = 0.0;
                              qreal x2 = this->width();
                              Segment* ss = first();
                              for (; ss->segmentType() != Segment::SegChordRest; ss = ss->next())
                                    ;
                              if (ss != first()) {
                                    ss = ss->prev();
                                    for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
                                          int track = staffIdx * VOICES;
                                          Element* e = ss->element(track);
                                          if (e)
                                                x1 = qMax(x1, ss->x() + e->x() + e->width());
                                          }
                                    }
                              rest->rxpos() = (x2 - x1 - e->width()) * .5 + x1 - s->x();
                              rest->adjustReadPos();
                              }
                        }
                  else if (t == REST)
                        e->rxpos() = 0;
                  else if (t == CHORD)
                        static_cast<Chord*>(e)->layout2();
                  else if (t == CLEF) {
                        if (s == first()) {
                              // clef at the beginning of measure
                              qreal gap = 0.0;
                              Segment* ps = s->prev();
                              if (ps)
                                    gap = s->x() - (ps->x() + ps->width());
                              e->rxpos() = -gap * .5;
                              e->adjustReadPos();
                              }
                        }
                  else {
                        e->setPos(-e->bbox().x(), 0.0);
                        e->adjustReadPos();
                        }
                  }
            }
      }

//---------------------------------------------------------
//   layoutStage1
//    compute multi measure rest break
//    call layoutChords0
//---------------------------------------------------------

void Measure::layoutStage1()
      {
      setDirty();

      for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
            if (score()->styleB(ST_createMultiMeasureRests)) {
                  if ((repeatFlags() & RepeatStart) || (prevMeasure() && (prevMeasure()->repeatFlags() & RepeatEnd)))
                        setBreakMMRest(true);
                  else if (!breakMultiMeasureRest()) {
                        for (Segment* s = first(); s; s = s->next()) {
                              int n = s->annotations().size();
                              for (int i = 0; i < n; ++i) {
                                    Element* e = s->annotations().at(i);
                                    if (e->type() == REHEARSAL_MARK || e->type() == TEMPO_TEXT) {
                                          setBreakMMRest(true);
                                          break;
                                          }
                                    }
                              if (breakMultiMeasureRest())      // optimize
                                    break;
                              }
                        }
                  }

            int track = staffIdx * VOICES;

            for (Segment* segment = first(); segment; segment = segment->next()) {
                  Element* e = segment->element(track);

                  if (e && !e->generated()) {
                        if (segment->segmentType() & (Segment::SegKeySig | Segment::SegStartRepeatBarLine | Segment::SegTimeSig))
                              setBreakMMRest(true);
                        else if (segment->segmentType() == Segment::SegClef) {
                              if (segment->tick() == endTick()) {
                                    Measure* m = nextMeasure();
                                    if (m)
                                          m->setBreakMMRest(true);
                                    }
                              else
                                    setBreakMMRest(true);
                              }
                        }

                  if (segment->segmentType() == Segment::SegChordRest)
                        layoutChords0(segment, staffIdx * VOICES);
                  }
            }

      if (!breakMultiMeasureRest()) {
            for (auto i = score()->spanner().lower_bound(tick()); i != score()->spanner().upper_bound(tick()); ++i) {
               Spanner* sp = i->second;
               if (sp->type() == Element::VOLTA)
                     setBreakMMRest(true);
                     break;
               }
            }
      if (!breakMultiMeasureRest() && this != score()->lastMeasure()) {
            auto i = score()->spanner().upper_bound(tick());
            std::reverse_iterator<std::map<int,Spanner*>::const_iterator> revit (i);
            for (; revit != score()->spanner().rend(); revit++) {
                  Spanner* sp = revit->second;
                  if (sp->type() == Element::VOLTA)
                        if(sp->tick2() == tick())
                              setBreakMMRest(true);
                        break;
                  }
            }

      MeasureBase* mb = prev();
      if (mb && mb->type() == Element::MEASURE) {
            Measure* pm = static_cast<Measure*>(mb);
            if (pm->endBarLineType() != NORMAL_BAR
               && pm->endBarLineType() != BROKEN_BAR && pm->endBarLineType() != DOTTED_BAR)
                  setBreakMMRest(true);
            }
      }

//---------------------------------------------------------
//   updateAccidentals
//    recompute accidentals,
///   undoable add/remove
//---------------------------------------------------------

void Measure::updateAccidentals(Segment* segment, int staffIdx, AccidentalState* tversatz)
      {
      Staff* staff            = score()->staff(staffIdx);
      int startTrack          = staffIdx * VOICES;
      int endTrack            = startTrack + VOICES;
      StaffGroup staffGroup   = staff->staffType()->group();
      const Instrument* instrument = staff->part()->instr();

      for (int track = startTrack; track < endTrack; ++track) {
            Chord* chord = static_cast<Chord*>(segment->element(track));
            if (!chord || chord->type() != CHORD)
                 continue;

            for (Chord* ch : chord->graceNotes()) {
                  for (Note* note : ch->notes())
                        note->updateAccidental(tversatz);
                  }

            // TAB_STAFF is different, as each note has to be fretted
            // in the context of the all of the chords of the whole segment

            if (staffGroup == TAB_STAFF_GROUP) {
                  instrument->stringData()->fretChords(chord);
                  continue;               // skip other staff type cases
                  }

            // PITCHED_ and PERCUSSION_STAFF can go note by note

            for (Note* note : chord->notes()) {
                  switch(staffGroup) {
                        case STANDARD_STAFF_GROUP:
                              if (note->tieBack()) {
                                    if (note->accidental() && note->tpc() == note->tieBack()->startNote()->tpc()) {
                                          // TODO: remove accidental only if note is not
                                          // on new system
                                          score()->undoRemoveElement(note->accidental());
                                          }
                                    }
                              note->updateAccidental(tversatz);
                              break;
                        case PERCUSSION_STAFF_GROUP:
                              {
                              Drumset* drumset = instrument->drumset();
                              int pitch = note->pitch();
                              if(drumset) {
                                    if (!drumset->isValid(pitch)) {
                                          // qDebug("unmapped drum note %d", pitch);
                                          }
                                    else {
                                          note->setHeadGroup(drumset->noteHead(pitch));
                                          note->setLine(drumset->line(pitch));
                                          continue;
                                          }
                                    }
                              }
                              break;
                        case TAB_STAFF_GROUP:   // to avoid compiler warning
                              break;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   stretchedLen
//---------------------------------------------------------

Fraction Measure::stretchedLen(Staff* staff) const
      {
      return len() / staff->timeStretch(tick());
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
      m->_repeatFlags = _repeatFlags;

      foreach(MStaff* ms, staves)
            m->staves.append(new MStaff(*ms));

      m->_no                    = _no;
      m->_noOffset              = _noOffset;
      m->_userStretch           = _userStretch;
      m->_irregular             = _irregular;
      m->_breakMultiMeasureRest = _breakMultiMeasureRest;
      m->_breakMMRest           = _breakMMRest;
      m->_endBarLineGenerated   = _endBarLineGenerated;
      m->_endBarLineVisible     = _endBarLineVisible;
      m->_endBarLineType        = _endBarLineType;
      m->_playbackCount         = _playbackCount;
      m->_endBarLineColor       = _endBarLineColor;

      m->_minWidth1             = _minWidth1;
      m->_minWidth2             = _minWidth2;

      m->setTick(tick());
      m->setLineBreak(lineBreak());
      m->setPageBreak(pageBreak());
      m->setSectionBreak(sectionBreak() ? new LayoutBreak(*sectionBreak()) : 0);

      int tracks = sc->nstaves() * VOICES;
      TupletMap tupletMap;

      for (Segment* oseg = first(); oseg; oseg = oseg->next()) {
            Segment* s = new Segment(m);
            s->setSegmentType(oseg->segmentType());
            s->setRtick(oseg->rtick());
            m->_segments.push_back(s);
            for (int track = 0; track < tracks; ++track) {
                  Element* oe = oseg->element(track);
                  if (oe) {
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
                                          m->add(nt);
                                          tupletMap.add(ot, nt);
                                          }
                                    ncr->setTuplet(nt);
                                    }
                              if (oe->type() == CHORD) {
                                    Chord* och = static_cast<Chord*>(ocr);
                                    Chord* nch = static_cast<Chord*>(ncr);
                                    int n = och->notes().size();
                                    for (int i = 0; i < n; ++i) {
                                          Note* on = och->notes().at(i);
                                          Note* nn = nch->notes().at(i);
                                          if (on->tieFor()) {
                                                Tie* tie = new Tie(sc);
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
                        s->add(ne);
                        }
                  foreach(Element* e, oseg->annotations()) {
                        if (e->generated() || e->track() != track)
                              continue;
                        Element* ne = e->clone();
                        ne->setTrack(track);
                        s->add(ne);
                        }
                  }
            }
      foreach(Element* e, *el()) {
            Element* ne = e->clone();
            ne->setScore(sc);
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
            case P_TIMESIG_NOMINAL:
                  return QVariant::fromValue(_timesig);
            case P_TIMESIG_ACTUAL:
                  return QVariant::fromValue(_len);
            case P_REPEAT_FLAGS:
                  return repeatFlags();
            case P_MEASURE_NUMBER_MODE:
                  return int(measureNumberMode());
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
            case P_TIMESIG_NOMINAL:
                  _timesig = value.value<Fraction>();
                  break;
            case P_TIMESIG_ACTUAL:
                  _len = value.value<Fraction>();
                  break;
            case P_REPEAT_FLAGS:
                  setRepeatFlags(value.toInt());
                  break;
            case P_MEASURE_NUMBER_MODE:
                  setMeasureNumberMode(MeasureNumberMode(value.toInt()));
                  break;
            default:
                  return MeasureBase::setProperty(propertyId, value);
            }
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Measure::propertyDefault(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_TIMESIG_NOMINAL:
            case P_TIMESIG_ACTUAL:
                  return QVariant();
            case P_REPEAT_FLAGS:
                  return 0;
            case P_MEASURE_NUMBER_MODE:
                  return int(MeasureNumberMode::AUTO);
            default:
                  break;
            }
      return MeasureBase::getProperty(propertyId);
      }

}

