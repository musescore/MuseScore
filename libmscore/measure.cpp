//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: measure.cpp 5656 2012-05-21 15:36:47Z wschweer $
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
#include "slurmap.h"
#include "tiemap.h"
#include "tupletmap.h"
#include "spannermap.h"
#include "accidental.h"
#include "layout.h"
#include "icon.h"

//---------------------------------------------------------
//   MStaff
//---------------------------------------------------------

MStaff::MStaff()
      {
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
      delete lines;
      delete _vspacerUp;
      delete _vspacerDown;
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
      _noText                = 0;
      _userStretch           = 1.0;     // ::style->measureSpacing;
      _irregular             = false;
      _breakMultiMeasureRest = false;
      _breakMMRest           = false;
      _endBarLineGenerated   = true;
      _endBarLineVisible     = true;
      _endBarLineType        = NORMAL_BAR;
      _mmEndBarLineType      = NORMAL_BAR;
      _multiMeasure          = 0;
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

      foreach(MStaff* ms, m.staves)
            staves.append(new MStaff(*ms));

      _minWidth1             = m._minWidth1;
      _minWidth2             = m._minWidth2;

      _no                    = m._no;
      _noOffset              = m._noOffset;
      _noText                = 0;
      _userStretch           = m._userStretch;

      _irregular             = m._irregular;
      _breakMultiMeasureRest = m._breakMultiMeasureRest;
      _breakMMRest           = m._breakMMRest;
      _endBarLineGenerated   = m._endBarLineGenerated;
      _endBarLineVisible     = m._endBarLineVisible;
      _endBarLineType        = m._endBarLineType;
      _mmEndBarLineType      = m._mmEndBarLineType;
      _multiMeasure          = m._multiMeasure;
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
      foreach(Spanner* s, _spannerFor)
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
      foreach(MStaff* m, staves)
            delete m;
      delete _noText;
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
            if (el->subtype() == Segment::SegKeySig)
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
      Drumset* drumset = 0;

      if (staff->part()->instr()->useDrumset())
            drumset = staff->part()->instr()->drumset();

      int endTrack = startTrack + VOICES;
      for (int track = startTrack; track < endTrack; ++track) {
            ChordRest* cr = static_cast<ChordRest*>(segment->element(track));
            if (!cr)
                 continue;
            qreal m = staffMag;
            if (cr->small())
                  m *= score()->styleD(ST_smallNoteMag);

            if (cr->type() == CHORD) {
                  Chord* chord = static_cast<Chord*>(cr);

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
      }

//---------------------------------------------------------
//   layoutChords10
//    computes note lines and accidentals
//---------------------------------------------------------

void Measure::layoutChords10(Segment* segment, int startTrack, AccidentalState* as)
      {
      int staffIdx     = startTrack/VOICES;
      Staff* staff     = score()->staff(staffIdx);
      Drumset* drumset = 0;

      if (staff->part()->instr()->useDrumset())
            drumset = staff->part()->instr()->drumset();

      int endTrack = startTrack + VOICES;
      for (int track = startTrack; track < endTrack; ++track) {
            Element* e = segment->element(track);
            if (!e || e->type() != CHORD)
                 continue;
            Chord* chord = static_cast<Chord*>(e);
            int n = chord->notes().size();
            for (int i = 0; i < n; ++i) {
                  Note* note = chord->notes().at(i);
                  if (drumset) {
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
                  note->layout10(as);
                  }
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

      Segment::SegmentTypes st = Segment::SegChordRestGrace;
      for (Segment* segment = first(st); segment; segment = segment->next(st)) {
            int startTrack = note->staffIdx() * VOICES;
            int endTrack   = startTrack + VOICES;
            for (int track = startTrack; track < endTrack; ++track) {
                  Element* e = segment->element(track);
                  if (!e || e->type() != CHORD)
                        continue;
                  Chord* chord = static_cast<Chord*>(e);

                  int n = chord->notes().size();
                  for (int i = 0; i < n; ++i) {
                        Note* note1 = chord->notes().at(i);

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
      qDebug("note not found");
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

      Segment::SegmentTypes st = Segment::SegChordRestGrace;
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

                  int n = chord->notes().size();
                  for (int i = 0; i < n; ++i) {
                        Note* note = chord->notes().at(i);
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
//---------------------------------------------------------

qreal Measure::tick2pos(int tck) const
      {
      if (multiMeasure() > 0) {
            Segment* s = first(Segment::SegChordRest);
            qreal x1 = s->x();
            qreal w  = width() - x1;
            return x1 + (tck * w) / (ticks() * multiMeasure());
            }

      Segment* s;
      qreal x1 = 0;
      qreal x2 = 0;
      int tick1 = tick();
      int tick2 = tick1;
      for (s = first(); s; s = s->next()) {
            if (s->subtype() != Segment::SegChordRest)
                  continue;
            x2 = s->x();
            tick2 = s->tick();
            if (tck <= tick2) {
                  if (tck == tick2)
                        x1 = x2;
                  break;
                  }
            x1    = x2;
            tick1 = tick2;
            }
      if (s == 0) {
            x2    = width();
            tick2 = tick() + ticks();
            }
      qreal x = 0;
      if (tick2 > tick1) {
            qreal dx = x2 - x1;
            int dt    = tick2 - tick1;
            if (dt == 0)
                  x = 0.0;
            else
                  x = dx * (tck - tick1) / dt;
            }
      return x1 + x;
      }

//---------------------------------------------------------
//   layout2
//    called after layout of all pages
//---------------------------------------------------------

void Measure::layout2()
      {
      if (parent() == 0)
            return;

      qreal _spatium = spatium();
      int tracks = staves.size() * VOICES;
      static const Segment::SegmentTypes st = Segment::SegChordRestGrace;
      for (int track = 0; track < tracks; ++track) {
            for (Segment* s = first(st); s; s = s->next(st)) {
                  ChordRest* cr = static_cast<ChordRest*>(s->element(track));
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
      int n = staves.size();
      for (int i = 0; i < n; ++i)
            staves.at(i)->lines->setWidth(width());


      MeasureBase::layout();  // layout LAYOUT_BREAK elements

      //
      //   set measure number
      //
      bool smn = false;

      if (!_noText || _noText->generated()) {
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
            if (smn) {
                  QString s(QString("%1").arg(_no + 1));
                  if (_noText == 0) {
                        _noText = new Text(score());
                        _noText->setGenerated(true);
                        _noText->setTextStyleType(TEXT_STYLE_MEASURE_NUMBER);
                        _noText->setParent(this);
                        score()->undoAddElement(_noText);
                        }
                  _noText->setText(s);
                  }
            else {
                  if (_noText)
                        score()->undoRemoveElement(_noText);
                  }
            }
      if (_noText)
            _noText->layout();

      //
      // slur layout needs articulation layout first
      //
      for (Segment* s = first(st); s; s = s->next(st)) {
            for (int track = 0; track < tracks; ++track) {
                  if (!score()->staff(track / VOICES)->show()) {
                        track += 4;
                        continue;
                        }
                  Element* el = s->element(track);
                  if (el) {
                        ChordRest* cr = static_cast<ChordRest*>(el);
                        foreach(Spanner* sp, static_cast<ChordRest*>(el)->spannerFor())
                              sp->layout();
                        if (cr->type() == CHORD) {
                              Chord* c = static_cast<Chord*>(cr);
                              foreach(const Note* note, c->notes()) {
                                    foreach(Spanner* sp, note->spannerFor())
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
      n = spannerFor().size();
      for (int i = 0; i < n; ++i)
            spannerFor().at(i)->layout();
      }

//---------------------------------------------------------
//   findChord
//---------------------------------------------------------

/**
 Search for chord at position \a tick in \a track at grace level \a gl.
 Grace level is 0 for a normal chord, 1 for the grace note closest
 to the normal chord, etc.
*/

Chord* Measure::findChord(int tick, int track, int gl)
      {
      int graces = 0;
      for (Segment* seg = last(); seg; seg = seg->prev()) {
            if (seg->tick() < tick)
                  return 0;
            if (seg->tick() == tick) {
                  if (seg->subtype() == Segment::SegGrace)
                        graces++;
                  Element* el = seg->element(track);
                  if (el && el->type() == CHORD && graces == gl)
                        return static_cast<Chord*>(el);
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   findChordRest
//---------------------------------------------------------

/**
 Search for chord or rest at position \a tick at \a staff in \a voice.
*/

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

Segment* Measure::tick2segment(int tick, bool grace) const
      {
      for (Segment* s = first(); s; s = s->next()) {
            if (s->tick() == tick) {
                  if (grace && (s->subtype() == Segment::SegGrace))
                        return s;
                  if (s->subtype() == Segment::SegChordRest)
                        return s;
                  }
            if (s->tick() > tick)
                  return 0;
            }
      return 0;
      }

//---------------------------------------------------------
//   findSegment
//---------------------------------------------------------

/**
 Search for a segment of type \a st at position \a t.
*/

Segment* Measure::findSegment(Segment::SegmentType st, int t)
      {
      Segment* s;
      for (s = first(); s && s->tick() < t; s = s->next())
            ;

      for (Segment* ss = s; ss && ss->tick() == t; ss = ss->next()) {
            if (ss->subtype() == st)
                  return ss;
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
      Segment::SegmentType st;
      if ((e->type() == CHORD) && (((Chord*)e)->noteType() != NOTE_NORMAL)) {
            Segment* s = findSegment(Segment::SegGrace, tick);
            if (s) {
                  if (s->element(e->track())) {
                        s = s->next();
                        if (s && s->subtype() == Segment::SegGrace && !s->element(e->track()))
                              return s;
                        }
                  else
                        return s;
                  }
            s = new Segment(this, Segment::SegGrace, tick);
            add(s);
            return s;
            }
      st = Segment::segmentType(e->type());
      return getSegment(st, tick);
      }

//---------------------------------------------------------
//   getSegment
///   Get a segment of type \a st at tick position \a t.
///   If the segment does not exist, it is created.
//---------------------------------------------------------

Segment* Measure::getSegment(Segment::SegmentType st, int t)
      {
      Segment* s = findSegment(st, t);
      if (!s) {
            s = new Segment(this, st, t);
            add(s);
            }
      return s;
      }

//---------------------------------------------------------
//   getSegment
//---------------------------------------------------------

/**
 Get a grace segment at tick position \a t and grace level \a gl.
 Grace level is 0 for a normal chord, 1 for the grace note closest
 to the normal chord, etc.
 If the segment does not exist, it is created.
*/

// when looking for a SegChordRest, return the first one found at t
// when looking for a SegGrace, first search for a SegChordRest at t,
// then search backwards for gl SegGraces

Segment* Measure::getGraceSegment(int t, int gl)
      {
      Segment* s = 0;

      // find the first segment at tick == t and create it if necessary
      for (Segment* ss = first(Segment::SegChordRest); ss && ss->tick() <= t; ss = ss->next(Segment::SegChordRest)) {
            if(ss->tick() == t) {
                  s = ss;
                  break;
                  }
            }
      if (s == 0) {
            s = new Segment(this, Segment::SegChordRest, t);
            add(s);
            }

      int nGraces  = 0;
      Segment* sCr = s;
      // count SegGrace segments
      for (Segment* ss = sCr; ss && ss->tick() == t; ss = ss->prev()) {
            if ((ss->subtype() == Segment::SegGrace) && (ss->tick() == t))
                  nGraces++;
            }
 	    //create grace segment if necessary
      for (; nGraces < gl; ++nGraces) {
            Segment* ps = new Segment(this, Segment::SegGrace, t);
            _segments.insert(ps, s);
            s = ps;
            setDirty();
            }

      int graces = 0;
      for (Segment* ss = sCr; ss && ss->tick() == t; ss = ss->prev()) {
            if ((ss->subtype() == Segment::SegGrace) && (ss->tick() == t))
                  graces++;
            if (gl == graces)
                  return ss;
            }
      return 0; // should not be reached
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
                  _noText = static_cast<Text*>(el);
                  break;

            case TUPLET:
                  qDebug("Measure::add(Tuplet) ??");
                  break;

            case SPACER:
                  {
                  Spacer* sp = static_cast<Spacer*>(el);
                  if (sp->subtype() == SPACER_UP)
                        staves[el->staffIdx()]->_vspacerUp = sp;
                  else if (sp->subtype() == SPACER_DOWN)
                        staves[el->staffIdx()]->_vspacerDown = sp;
                  }
                  break;
            case SEGMENT:
                  {
                  Segment* seg = static_cast<Segment*>(el);
                  int tracks = staves.size() * VOICES;
                  if (seg->subtype() == Segment::SegKeySig) {
                        for (int track = 0; track < tracks; track += VOICES) {
                              if (!seg->element(track))
                                    continue;
                              score()->staff(track/VOICES)->setUpdateKeymap(true);
                              }
                        }
                  int t  = seg->tick();
                  Segment::SegmentType st = seg->subtype();
                  if (seg->prev() || seg->next()) {
                        //
                        // undo operation
                        //
                        _segments.insert(seg);
                        }
                  else {
                        Segment* s;
                        if (st == Segment::SegGrace) {
                              for (s = first(); s && s->tick() < t; s = s->next())
                                    ;
                              if (s && (s->tick() > t)) {
                                    seg->setParent(this);
                                    _segments.insert(seg, s);
                                    break;
                                    }
                              if (s && s->subtype() != Segment::SegChordRest) {
                                    for (; s && s->subtype() != Segment::SegEndBarLine
                                       && s->subtype() != Segment::SegChordRest; s = s->next())
                                          ;
                                    }
                              }
                        else {
                              for (s = first(); s && s->tick() < t; s = s->next())
                                    ;
                              if (s) {
                                    if (st == Segment::SegChordRest) {
                                          while (s && s->subtype() != st && s->tick() == t) {
                                                if (s->subtype() == Segment::SegEndBarLine)
                                                      break;
                                                s = s->next();
                                                }
                                          }
                                    else {
                                          while (s && s->subtype() <= st) {
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
                              }
                        seg->setParent(this);
                        _segments.insert(seg, s);
                        }
                  if ((seg->subtype() == Segment::SegTimeSig) && seg->element(0)) {
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
                  _el.append(el);
                  break;

            case HBOX:
                  if (el->staff())
                        el->setMag(el->staff()->mag());     // ?!
                  _el.append(el);
                  break;

            case VOLTA:
                  {
                  Volta* volta = static_cast<Volta*>(el);
                  Measure* m = volta->endMeasure();
                  if (m)
                        m->addSpannerBack(volta);
                  _spannerFor.append(volta);
                  foreach(SpannerSegment* ss, volta->spannerSegments()) {
                        if (ss->system())
                              ss->system()->add(ss);
                        }
                  }
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
                  Q_ASSERT(el == _noText);
                  _noText = 0;
                  break;

            case SPACER:
                  if (static_cast<Spacer*>(el)->subtype() == SPACER_DOWN)
                        staves[el->staffIdx()]->_vspacerDown = 0;
                  else if (static_cast<Spacer*>(el)->subtype() == SPACER_UP)
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

            case VOLTA:
                  {
                  Volta* volta = static_cast<Volta*>(el);
                  Measure* m = volta->endMeasure();
                  m->removeSpannerBack(volta);
                  if (!_spannerFor.removeOne(volta)) {
                        qDebug("Measure:remove: %s not found", volta->name());
                        Q_ASSERT(volta->score() == score());
                        }
                  foreach(SpannerSegment* ss, volta->spannerSegments()) {
                        if (ss->system())
                              ss->system()->remove(ss);
                        }
                  }
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
            if (segment->subtype() & (Segment::SegEndBarLine | Segment::SegTimeSigAnnounce))
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
      foreach(Element* e, _el) {
            if (e->track() == -1)
                  continue;
            int voice = e->voice();
            int staffIdx = e->staffIdx();
            if (staffIdx >= eStaff) {
                  staffIdx -= eStaff - sStaff;
                  e->setTrack(staffIdx * VOICES + voice);
                  }
            }
      for (int i = 0; i < staves.size(); ++i)
            staves[i]->setTrack(i * VOICES);
      }

//---------------------------------------------------------
//   insertStaves
//---------------------------------------------------------

void Measure::insertStaves(int sStaff, int eStaff)
      {
      foreach(Element* e, _el) {
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
      for (int i = 0; i < staves.size(); ++i)
            staves[i]->setTrack(i * VOICES);
      }

//---------------------------------------------------------
//   cmdRemoveStaves
//---------------------------------------------------------

void Measure::cmdRemoveStaves(int sStaff, int eStaff)
      {
qDebug("cmdRemoveStaves %d-%d", sStaff, eStaff);
      int sTrack = sStaff * VOICES;
      int eTrack = eStaff * VOICES;
      for (Segment* s = first(); s; s = s->next()) {
            qDebug(" seg %d <%s>", s->tick(), s->subTypeName());
            for (int track = eTrack - 1; track >= sTrack; --track) {
                  Element* el = s->element(track);
                  if (el && !el->generated()) {
                        qDebug("  remove %s track %d", el->name(), track);
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

      for (int i = 0; i < staves.size(); ++i)
            staves[i]->lines->setTrack(i * VOICES);

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
      if (MScore::debugMode)
            qDebug("     Measure::insertMStaff %d -> n:%d", idx, staves.size());
      }

//---------------------------------------------------------
//   removeMStaff
//---------------------------------------------------------

void Measure::removeMStaff(MStaff* /*staff*/, int idx)
      {
      if (MScore::debugMode)
            qDebug("     Measure::removeMStaff %d", idx);

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
                        if (seg->subtype() != Segment::SegChordRest)
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
                  switch(static_cast<Icon*>(e)->subtype()) {
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
                  for (Segment* seg = first(); seg; seg = seg->next()) {
                        if (seg->subtype() == Segment::SegChordRest) {
                              if (mrpx < seg->pos().x())
                                    return true;
                              }
                        }
                  // fall through if no chordrest segment found

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
                  e->setTrack(staffIdx * VOICES);
                  e->setParent(system());
                  static_cast<Bracket*>(e)->setLevel(-1);  // add bracket
                  score()->undoAddElement(e);
                  return e;

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
                        (lb->subtype() == LAYOUT_BREAK_PAGE && _pageBreak)
                     || (lb->subtype() == LAYOUT_BREAK_LINE && _lineBreak)
                     || (lb->subtype() == LAYOUT_BREAK_SECTION && _sectionBreak)
                     ) {
                        //
                        // if break already set
                        //
                        delete lb;
                        break;
                        }
                  // make sure there is only LAYOUT_BREAK_LINE or LAYOUT_BREAK_PAGE
                  if ((lb->subtype() != LAYOUT_BREAK_SECTION) && (_pageBreak || _lineBreak)) {
                        foreach(Element* le, _el) {
                              if (le->type() == LAYOUT_BREAK
                                 && (static_cast<LayoutBreak*>(le)->subtype() == LAYOUT_BREAK_LINE
                                  || static_cast<LayoutBreak*>(le)->subtype() == LAYOUT_BREAK_PAGE)) {
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
                        score()->undoChangeBarLine(this, bl->subtype());
                        delete e;
                        }
                  break;
                  }

            case REPEAT_MEASURE:
                  {
                  delete e;
                  //
                  // see also cmdDeleteSelection()
                  //
                  _score->select(0, SELECT_SINGLE, 0);
                  for (Segment* s = first(); s; s = s->next()) {
                        if (s->subtype() & Segment::SegChordRestGrace) {
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

            case ICON:
                  switch(static_cast<Icon*>(e)->subtype()) {
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
                  if (s->subtype() & (Segment::SegEndBarLine|Segment::SegTimeSigAnnounce|Segment::SegKeySigAnnounce)) {
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
            if (_noText && !_noText->generated()) {
                  xml.stag("MeasureNumber");
                  _noText->writeProperties(xml);
                  xml.etag();
                  }
            }
      qreal _spatium = spatium();
      MStaff* mstaff = staves[staff];
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
      foreach(Spanner* e, _spannerFor) {
            if (e->track() >= strack && e->track() < etrack && !e->generated()) {
                  e->setId(++xml.spannerId);
                  e->write(xml);
                  }
            }
      foreach(Spanner* e, _spannerBack) {
            if (e->track() >= strack && e->track() < etrack && !e->generated()) {
                  xml.tagE(QString("endSpanner id=\"%1\"").arg(e->id()));
                  }
            }
      foreach (const Element* el, _el) {
            if (!el->generated() && ((el->staffIdx() == staff) || (el->systemFlag() && writeSystemElements))) {
                  el->write(xml);
                  }
            }

      int track = staff * VOICES;
      score()->writeSegments(xml, this, track, track+VOICES, first(), last()->next1(), writeSystemElements);
      xml.etag();
      }

//---------------------------------------------------------
//   Measure::read
//---------------------------------------------------------

void Measure::read(const QDomElement& de, int staffIdx)
      {
      QList<Tuplet*> tuplets;

      Segment* segment = 0;
      qreal _spatium = spatium();
      bool irregular;

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
      if (de.hasAttribute("tick"))
            score()->curTick = score()->fileDivision(de.attribute("tick").toInt());
      setTick(score()->curTick);

      const SigEvent& sigEvent = score()->sigmap()->timesig(tick());
      _timesig  = sigEvent.nominal();
      if (de.hasAttribute("len")) {
            QStringList sl = de.attribute("len").split('/');
            if (sl.size() == 2)
                  _len = Fraction(sl[0].toInt(), sl[1].toInt());
            else
                  qDebug("illegal measure size <%s>", qPrintable(de.attribute("len")));
            irregular = true;
            score()->sigmap()->add(tick(), SigEvent(_len, _timesig));
            score()->sigmap()->add(tick() + ticks(), SigEvent(_timesig));
            }
      else {
            if (staffIdx == 0)
                  _len = sigEvent.timesig();
            irregular = false;
            }
      Staff* staff = score()->staff(staffIdx);
      Fraction timeStretch(staff->timeStretch(tick()));

      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());

            if (tag == "tick") {
                  score()->curTick = val.toInt();
                  }
            else if (tag == "BarLine") {
                  BarLine* barLine = new BarLine(score());
                  barLine->setTrack(score()->curTrack);
                  barLine->read(e);
                  if ((score()->curTick != tick()) && (score()->curTick != (tick() + ticks()))) {
                        // this is a mid measure bar line
                        segment = getSegment(Segment::SegBarLine, score()->curTick);
                        }
                  else if (barLine->subtype() == START_REPEAT)
                        segment = getSegment(Segment::SegStartRepeatBarLine, score()->curTick);
                  else {
                        // setEndBarLineType(barLine->barLineType(), false, barLine->visible(), barLine->color());
                        setEndBarLineType(barLine->subtype(), false, true, Qt::black);
                        if(!barLine->customSpan()) {
                              Staff* staff = score()->staff(staffIdx);
                              barLine->setSpan(staff->barLineSpan());
                              }
                        segment = getSegment(Segment::SegEndBarLine, score()->curTick);
                        }
                  segment->add(barLine);
                  }
            else if (tag == "Chord") {
                  Chord* chord = new Chord(score());
                  chord->setTrack(score()->curTrack);
                  chord->read(e, &tuplets, &score()->spanner);

                  if (chord->noteType() != NOTE_NORMAL
                     && segment
                     && segment->subtype() == Segment::SegChordRest
                     && segment->tick() == score()->curTick
                     && segment->element(score()->curTrack)
                     && segment->element(score()->curTrack)->type() == CHORD
                     )
                        {
                        //
                        // handle grace note after chord
                        //
                        Segment* s = new Segment(this);
                        s->setSubtype(Segment::SegChordRest);
                        s->setTick(segment->tick());
                        s->setPrev(segment);
                        s->setNext(segment->next());
                        add(s);
                        segment = s;
                        }
                  else
                        segment = getSegment(chord, score()->curTick);
                  Fraction ts(timeStretch * chord->globalDuration());
                  int crticks = chord->noteType() != NOTE_NORMAL ? 0 : ts.ticks();
                  if (chord->tremolo() && chord->tremolo()->subtype() < 6) {
                        //
                        // old style tremolo found
                        //
                        Tremolo* tremolo = chord->tremolo();
                        TremoloType st;
                        switch (tremolo->subtype()) {
                              default:
                              case OLD_TREMOLO_R8:  st = TREMOLO_R8;  break;
                              case OLD_TREMOLO_R16: st = TREMOLO_R16; break;
                              case OLD_TREMOLO_R32: st = TREMOLO_R32; break;
                              case OLD_TREMOLO_C8:  st = TREMOLO_C8;  break;
                              case OLD_TREMOLO_C16: st = TREMOLO_C16; break;
                              case OLD_TREMOLO_C32: st = TREMOLO_C32; break;
                              }
                        tremolo->setSubtype(st);
                        if (tremolo->twoNotes()) {
                              int track = chord->track();
                              Segment* ss = 0;
                              for (Segment* ps = first(Segment::SegChordRest); ps; ps = ps->next(Segment::SegChordRest)) {
                                    if (ps->tick() >= score()->curTick)
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
                                    }
                              else {
                                    qDebug("tremolo: first note not found");
                                    }
                              score()->curTick += crticks / 2;
                              }
                        else {
                              tremolo->setParent(chord);
                              score()->curTick += crticks;
                              }
                        }
                  else {
                        score()->curTick += crticks;
                        }
                  segment->add(chord);
                  }
            else if (tag == "Rest") {
                  Rest* rest = new Rest(score());
                  rest->setDurationType(TDuration::V_MEASURE);
                  rest->setDuration(timesig()/timeStretch);
                  rest->setTrack(score()->curTrack);
                  rest->read(e, &tuplets, &score()->spanner);

                  segment = getSegment(rest, score()->curTick);
                  segment->add(rest);
                  if (!rest->duration().isValid())     // hack
                        rest->setDuration(timesig()/timeStretch);
                  Fraction ts(timeStretch * rest->globalDuration());

                  score()->curTick += ts.ticks();
                  }
            else if (tag == "Note") {                 // obsolete
                  Chord* chord = new Chord(score());
                  chord->setTrack(score()->curTrack);
                  chord->readNote(e, &tuplets, &score()->spanner);
                  segment = getSegment(chord, score()->curTick);
                  segment->add(chord);

                  Fraction ts(timeStretch * chord->globalDuration());
                  score()->curTick += ts.ticks();
                  }
            else if (tag == "Breath") {
                  Breath* breath = new Breath(score());
                  breath->setTrack(score()->curTrack);
                  breath->read(e);
                  segment = getSegment(Segment::SegBreath, score()->curTick);
                  segment->add(breath);
                  }
            else if (tag == "endSpanner") {
                  int id = e.attribute("id").toInt();
                  Spanner* e = score()->findSpanner(id);
                  if (e) {
                        if (e->anchor() == Spanner::ANCHOR_MEASURE) {
                              e->setEndElement(this);
                              addSpannerBack(e);
                              }
                        else {
                              segment = getSegment(Segment::SegChordRest, score()->curTick);
                              e->setEndElement(segment);
                              segment->addSpannerBack(e);
                              }
                        if (e->type() == OTTAVA) {
                              Ottava* o = static_cast<Ottava*>(e);
                              int shift = o->pitchShift();
                              Staff* st = o->staff();
                              int tick1 = static_cast<Segment*>(o->startElement())->tick();
                              st->pitchOffsets().setPitchOffset(tick1, shift);
                              st->pitchOffsets().setPitchOffset(segment->tick(), 0);
                              }
                        else if (e->type() == HAIRPIN) {
                              Hairpin* hp = static_cast<Hairpin*>(e);
                              score()->updateHairpin(hp);
                              }
                        }
                  else
                        qDebug("Measure::read(): cannot find spanner %d", id);
                  }
            else if (tag == "HairPin"
               || tag == "Pedal"
               || tag == "Ottava"
               || tag == "Trill"
               || tag == "TextLine"
               || tag == "Volta") {
                  Spanner* sp = static_cast<Spanner*>(Element::name2Element(tag, score()));
                  score()->spanner.append(sp);
                  sp->setTrack(staffIdx * VOICES);
                  sp->read(e);
                  segment = getSegment(Segment::SegChordRest, score()->curTick);
                  if (sp->anchor() == Spanner::ANCHOR_SEGMENT) {
                        sp->setStartElement(segment);
                        segment->add(sp);
                        }
                  else {
                        sp->setStartElement(this);
                        add(sp);
                        }
                  }
            else if (tag == "RepeatMeasure") {
                  RepeatMeasure* rm = new RepeatMeasure(score());
                  rm->setTrack(score()->curTrack);
                  rm->read(e, &tuplets, &score()->spanner);
                  segment = getSegment(Segment::SegChordRest, score()->curTick);
                  segment->add(rm);
                  score()->curTick += ticks();
                  }
            else if (tag == "Clef") {
                  Clef* clef = new Clef(score());
                  clef->setTrack(score()->curTrack);
                  clef->read(e);
                  clef->setGenerated(false);
                  if (segment && segment->next() && segment->next()->subtype() == Segment::SegClef) {
                        segment = segment->next();
                        }
                  else if (segment && segment != first()) {
                        Segment* ns;
                        if (score()->curTick == tick())
                              ns = first();
                        else {
                              // ??
                              // Segment* ns = segment->next();
                              ns = segment;
                              while (ns && ns->tick() < score()->curTick)
                                    ns = ns->next();
                              }
                        segment = new Segment(this, Segment::SegClef, score()->curTick);
                        _segments.insert(segment, ns);
                        }
                  else {
                        segment = getSegment(Segment::SegClef, score()->curTick);
                        }
                  segment->add(clef);
                  }
            else if (tag == "TimeSig") {
                  TimeSig* ts = new TimeSig(score());
                  ts->setTrack(score()->curTrack);
                  ts->read(e);
                  segment = getSegment(Segment::SegTimeSig, score()->curTick);
                  segment->add(ts);

                  if (score()->mscVersion() > 114) {
                        // in <= 114 sigmap is part of the mscx file
                        timeStretch = ts->stretch();
                        _timesig    = ts->sig();
                        if (timeStretch == Fraction(1,1)) {
                              // if len is not irregular, set _len to time sig and store new time sig
                              if (!irregular) {
                                    _len = _timesig * timeStretch;
                                    score()->sigmap()->add(tick(), SigEvent(_timesig));
                                    }
                              // if len is irregular, update stored time sigs with new nominal value
                              else {
                                    score()->sigmap()->add(tick(), SigEvent(_len, _timesig));
                                    score()->sigmap()->add(tick() + ticks(), SigEvent(_timesig));
                                    }
                              }
                        }
                  }
            else if (tag == "KeySig") {
                  KeySig* ks = new KeySig(score());
                  ks->setTrack(score()->curTrack);
                  ks->read(e);
                  int tick = score()->curTick;
                  segment = getSegment(Segment::SegKeySig, tick);
                  segment->add(ks);
                  staff->setKey(tick, ks->keySigEvent());
                  }
            else if (tag == "Lyrics") {                           // obsolete
                  Lyrics* lyrics = new Lyrics(score());
                  lyrics->setTrack(score()->curTrack);
                  lyrics->read(e);
                  segment       = getSegment(Segment::SegChordRest, score()->curTick);
                  ChordRest* cr = static_cast<ChordRest*>(segment->element(lyrics->track()));
                  if (!cr)
                        qDebug("Internal Error: no Chord/Rest for lyrics");
                  else
                        cr->add(lyrics);
                  }
            else if (tag == "Text") {
                  Text* t = new Text(score());
                  t->setTrack(score()->curTrack);
                  t->read(e);
                  //previous versions stored measure number, delete it
                  if(t->textStyleType() == TEXT_STYLE_MEASURE_NUMBER) {
                        delete t;
                      }
                  else {
                        segment = getSegment(Segment::SegChordRest, score()->curTick);
                        segment->add(t);
                        }
                  }

            //----------------------------------------------------
            // Annotation

            else if (tag == "Dynamic") {
                  Dynamic* dyn = new Dynamic(score());
                  dyn->setTrack(score()->curTrack);
                  dyn->read(e);
                  dyn->resetType(); // for backward compatibility
                  segment = getSegment(Segment::SegChordRest, score()->curTick);
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
               ) {
                  Element* el = Element::name2Element(tag, score());
                  el->setTrack(score()->curTrack);
                  el->read(e);
                  segment = getSegment(Segment::SegChordRest, score()->curTick);
                  segment->add(el);
                  }
            else if (tag == "Image") {
                  // look ahead for image type
                  QString path;
                  QDomElement ee = e.firstChildElement("path");
                  if (!ee.isNull())
                        path = ee.text();
                  Image* image = 0;
                  QString s(path.toLower());
                  if (s.endsWith(".svg"))
                        image = new SvgImage(score());
                  else
                        if (s.endsWith(".jpg")
                     || s.endsWith(".png")
                     || s.endsWith(".gif")
                     || s.endsWith(".xpm")
                        ) {
                        image = new RasterImage(score());
                        }
                  else {
                        qDebug("unknown image format <%s>", path.toLatin1().data());
                        }
                  if (image) {
                        image->setTrack(score()->curTrack);
                        image->read(e);
                        Segment* s = getSegment(Segment::SegChordRest, score()->curTick);
                        s->add(image);
                        }
                  }

            //----------------------------------------------------
            else if (tag == "stretch")
                  _userStretch = val.toDouble();
            else if (tag == "LayoutBreak") {
                  LayoutBreak* lb = new LayoutBreak(score());
                  lb->read(e);
                  add(lb);
                  }
            else if (tag == "noOffset")
                  _noOffset = val.toInt();
            else if (tag == "irregular")
                  _irregular = true;
            else if (tag == "breakMultiMeasureRest")
                  _breakMultiMeasureRest = true;
            else if (tag == "Tuplet") {
                  Tuplet* tuplet = new Tuplet(score());
                  tuplet->setTrack(score()->curTrack);
                  tuplet->setTick(score()->curTick);
                  tuplet->setParent(this);
                  tuplet->read(e, &tuplets, &score()->spanner);
                  tuplets.append(tuplet);
                  }
            else if (tag == "startRepeat")
                  _repeatFlags |= RepeatStart;
            else if (tag == "endRepeat") {
                  _repeatCount = val.toInt();
                  _repeatFlags |= RepeatEnd;
                  }
            else if (tag == "Slur") {
                  Slur* slur = new Slur(score());
                  slur->setTrack(score()->curTrack);
                  slur->read(e);
                  score()->spanner.append(slur);
                  }
            else if (tag == "vspacer" || tag == "vspacerDown") {
                  if (staves[staffIdx]->_vspacerDown == 0) {
                        Spacer* spacer = new Spacer(score());
                        spacer->setSubtype(SPACER_DOWN);
                        spacer->setTrack(staffIdx * VOICES);
                        add(spacer);
                        }
                  staves[staffIdx]->_vspacerDown->setGap(val.toDouble() * _spatium);
                  }
            else if (tag == "vspacer" || tag == "vspacerUp") {
                  if (staves[staffIdx]->_vspacerUp == 0) {
                        Spacer* spacer = new Spacer(score());
                        spacer->setSubtype(SPACER_UP);
                        spacer->setTrack(staffIdx * VOICES);
                        add(spacer);
                        }
                  staves[staffIdx]->_vspacerUp->setGap(val.toDouble() * _spatium);
                  }
            else if (tag == "visible")
                  staves[staffIdx]->_visible = val.toInt();
            else if (tag == "slashStyle")
                  staves[staffIdx]->_slashStyle = val.toInt();
            else if (tag == "Beam") {
                  Beam* beam = new Beam(score());
                  beam->setTrack(score()->curTrack);
                  beam->read(e);
                  beam->setParent(0);
                  score()->beams.prepend(beam);
                  }
            else if (tag == "Segment")
                  segment->read(e);
            else if (tag == "MeasureNumber") {
                  _noText = new Text(score());
                  _noText->read(e);
                  _noText->setParent(this);
                  }
            else
                  domError(e);
            }
      if (staffIdx == 0) {
            Segment* s = last();
            if (s && s->subtype() == Segment::SegBarLine) {
                  BarLine* b = static_cast<BarLine*>(s->element(0));
                  setEndBarLineType(b->subtype(), false, b->visible(), b->color());
                  // s->remove(b);
                  // delete b;
                  }
            }
      //
      // for compatibility with 1.22:
      //
      int ticks1 = 0;
      for (Segment* s = last(); s; s = s->prev()) {
            if (s->subtype() == Segment::SegChordRest) {
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
                  if (s->subtype() == Segment::SegBarLine) {
                        printf("reduce BarLine to EndBarLine\n");
                        s->setSubtype(Segment::SegEndBarLine);
                        }
                  }

            }
      foreach (Tuplet* tuplet, tuplets) {
            if (tuplet->elements().isEmpty()) {
                  // this should not happen and is a sign of input file corruption
                  qDebug("Measure:read(): empty tuplet, input file corrupted?");
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
      return score()->staff(staffIdx)->slashStyle() || staves[staffIdx]->_slashStyle;
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
                  if (all || visible(e->staffIdx()))
                        e->scanElements(data,  func, all);
                  }
            }
      if (noText())
            func(data, noText());
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
            if (s->subtype() != Segment::SegChordRest)
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
                  bl->setSubtype(START_REPEAT);
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
            if (bl && val && span)
                  bl->setSpan(span);

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
//    actually create or modify barlines
//    return true if layout changes
//---------------------------------------------------------

bool Measure::createEndBarLines()
      {
      bool changed = false;
      int nstaves  = score()->nstaves();
      Segment* seg = undoGetSegment(Segment::SegEndBarLine, tick() + ticks());

      BarLine* bl = 0;
      int span    = 0;        // span counter
      int aspan   = 0;        // actual span
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
                        if (span) {
                              spanFrom    = staff->barLineFrom();
                              spanTo      = staff->barLineTo();
                              }
                        // but if staff is set to no span, a multi-staff spanning bar line
                        // has been shortened to span less staves and following staves left without bars;
                        // set bar line span values to default
                        else {
                              span        = 1;
                              spanFrom    = 0;
                              spanTot     = (staff->lines()-1)*2;
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
                  BarLineType et = _multiMeasure > 0 ? _mmEndBarLineType : _endBarLineType;
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
                        bl->setGenerated(bl->el()->isEmpty() && _endBarLineGenerated);
                        bl->setSubtype(et);
                        bl->setParent(seg);
                        bl->setTrack(track);
                        score()->undoAddElement(bl);
                        changed = true;
                        }
                  else {
                        // a bar line is there (either existing or newly created):
                        // adjust subtype, if not fitting
                        if (bl->subtype() != et && !bl->customSubtype()) {
                              score()->undoChangeProperty(bl, P_SUBTYPE, et);
                              bl->setGenerated(bl->el()->isEmpty() && _endBarLineGenerated);
                              changed = true;
                              }
                        // or clear custom subtype flag if same type as measure
                        if (bl->subtype() == et && bl->customSubtype())
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
            if(spanTot > 1 && spanTo <= 0 && span == 0)
                  staffIdx--;
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
      _endBarLineColor     = color;
      }

//---------------------------------------------------------
//   setRepeatFlags
//---------------------------------------------------------

void Measure::setRepeatFlags(int val)
      {
      _repeatFlags = val;
      }

//---------------------------------------------------------
//   sortStaves
//---------------------------------------------------------

void Measure::sortStaves(QList<int>& dst)
      {
      QList<MStaff*> ms;
      foreach(int idx, dst)
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
            if (s->subtype() != Segment::SegChordRest)
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
            if (s->subtype() != Segment::SegChordRest)
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
      return s && (s->subtype() == Segment::SegClef) && s->element(0) && s->element(0)->generated();
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
            for (;;) {
                  if ((s->subtype() & st)
                     && s->next()
                     && (!s->element(0) || s->element(0)->generated())
                     ) {
                        s = s->next();
                        }
                  else
                        break;
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
//    layoutX
///   \brief main layout routine for note spacing
///   Return width of measure (in MeasureWidth), taking into account \a stretch.
//-----------------------------------------------------------------------------

void Measure::layoutX(qreal stretch)
      {
      int nstaves = _score->nstaves();

      int segs = 0;
      for (const Segment* s = first(); s; s = s->next()) {
            if (s->subtype() == Segment::SegClef && (s != first()))
                  continue;
            ++segs;
            }

      if (nstaves == 0 || segs == 0)
            return;

      qreal _spatium           = spatium();
      int tracks               = nstaves * VOICES;
      qreal clefKeyRightMargin = score()->styleS(ST_clefKeyRightMargin).val() * _spatium;

      qreal rest[nstaves];    // fixed space needed from previous segment
      memset(rest, 0, nstaves * sizeof(qreal));

      //--------tick table for segments
      int ticksList[segs];
      memset(ticksList, 0, segs * sizeof(int));

      qreal xpos[segs+1];
      Segment::SegmentType types[segs];
      qreal width[segs];

      int segmentIdx = 0;
      qreal x        = 0.0;
      if (system()->firstMeasure() == this && system()->barLine()) {
            x += BarLine::layoutWidth(score(), system()->barLine()->subtype(), system()->barLine()->magS());
            }
      int minTick    = 100000;
      int ntick      = tick() + ticks();   // position of next measure

      qreal minNoteDistance = score()->styleS(ST_minNoteDistance).val() * _spatium;

      qreal clefWidth[nstaves];
      memset(clefWidth, 0, nstaves * sizeof(qreal));

      const Segment* s = first();
      const Segment* pSeg = 0;
      for (; s; s = s->next(), ++segmentIdx) {
            qreal elsp = s->extraLeadingSpace().val()  * _spatium;
            qreal etsp = s->extraTrailingSpace().val() * _spatium;
            if ((s->subtype() == Segment::SegClef) && (s != first())) {
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
            bool rest2[nstaves+1];
            Segment::SegmentType segType    = s->subtype();
            types[segmentIdx]      = segType;
            qreal segmentWidth     = 0.0;
            qreal stretchDistance  = 0.0;
            int pt                 = pSeg ? pSeg->subtype() : Segment::SegBarLine;

            for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                  if (!score()->staff(staffIdx)->show())
                        continue;
                  qreal minDistance = 0.0;
                  Space space;
                  int track  = staffIdx * VOICES;
                  bool found = false;
                  if (segType & (Segment::SegChordRestGrace)) {
                        qreal llw = 0.0;
                        qreal rrw = 0.0;
                        Lyrics* lyrics = 0;
                        for (int voice = 0; voice < VOICES; ++voice) {
                              ChordRest* cr = static_cast<ChordRest*>(s->element(track+voice));
                              if (!cr)
                                    continue;
                              found = true;
                              if (pt & (Segment::SegStartRepeatBarLine | Segment::SegBarLine)) {
                                    qreal sp        = score()->styleS(ST_barNoteDistance).val() * _spatium;
                                    sp             += elsp;
                                    minDistance     = qMax(minDistance, sp);
                                    stretchDistance = sp * .7;
                                    }
                              else if (pt & (Segment::SegChordRestGrace)) {
                                    minDistance = qMax(minDistance, minNoteDistance);
                                    }
                              else {
                                    // if (pt & (Segment::SegKeySig | Segment::SegClef))
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
                        }
                  else {
                        Element* e = s->element(track);
                        if ((segType == Segment::SegClef) && (pt != Segment::SegChordRest))
                              minDistance = score()->styleP(ST_clefLeftMargin);
                        else if (segType == Segment::SegStartRepeatBarLine)
                              minDistance = .5 * _spatium;
                        else if ((segType == Segment::SegEndBarLine) && segmentIdx) {
                              if (pt == Segment::SegClef)
                                    minDistance = score()->styleP(ST_clefBarlineDistance);
                              else
                                    stretchDistance = score()->styleP(ST_noteBarDistance);
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
                              found = true;
                              space.max(e->space());
                              }
                        }
                  space += Space(elsp, etsp);
                  if (found) {
                        space.rLw() += clefWidth[staffIdx];
                        qreal sp     = minDistance + rest[staffIdx] + stretchDistance;
                        if (space.lw() > stretchDistance)
                              sp += (space.lw() - stretchDistance);
                        rest[staffIdx]  = space.rw();
                        rest2[staffIdx] = false;
                        segmentWidth    = qMax(segmentWidth, sp);
                        }
                  else
                        rest2[staffIdx] = true;
                  clefWidth[staffIdx] = 0.0;
                  }

            x += segmentWidth;
            xpos[segmentIdx]  = x;

            if (segmentIdx) {
                  width[segmentIdx-1] = segmentWidth;
                  if (pSeg)
                        pSeg->setbbox(QRectF(0.0, 0.0, segmentWidth, _spatium * 5));  //??
                  }

            for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                  if (!score()->staff(staffIdx)->show())
                        continue;
                  if (rest2[staffIdx])
                        rest[staffIdx] -= segmentWidth;
                  }
            if ((s->subtype() == Segment::SegChordRest)) {
                  const Segment* nseg = s;
                  for (;;) {
                        nseg = nseg->next();
                        if (nseg == 0 || nseg->subtype() == Segment::SegChordRest)
                              break;
                        }
                  int nticks = (nseg ? nseg->tick() : ntick) - s->tick();
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
                        }
                  ticksList[segmentIdx] = nticks;
                  }
            else
                  ticksList[segmentIdx] = 0;
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
            }
      xpos[segmentIdx]    = x + segmentWidth;
      width[segmentIdx-1] = segmentWidth;

      //---------------------------------------------------
      // compute stretches
      //---------------------------------------------------

      SpringMap springs;

      qreal minimum = xpos[0];
      for (int i = 0; i < segs; ++i) {
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
      x = xpos[0];
      for (int i = 1; i <= segs; ++i) {
            x += width[i-1];
            xpos[i] = x;
            }

      //---------------------------------------------------
      //    layout individual elements
      //---------------------------------------------------

      int seg = 0;
      for (Segment* s = first(); s; s = s->next(), ++seg) {
            if ((s->subtype() == Segment::SegClef) && (s != first())) {
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

            for (int track = 0; track < tracks; ++track) {
                  if (!score()->staff(track/VOICES)->show()) {
                        track += VOICES-1;
                        continue;
                        }
                  Element* e = s->element(track);
                  if (e == 0)
                        continue;
                  ElementType t = e->type();
                  if (t == REST) {
                        Rest* rest = static_cast<Rest*>(e);
                        if (_multiMeasure > 0) {
                              if ((track % VOICES) == 0) {
                                    Segment* ls = last();
                                    qreal eblw = 0.0;
                                    int t = (track / VOICES) * VOICES;
                                    if (ls->subtype() == Segment::SegEndBarLine) {
                                          Element* e = ls->element(t);
                                          if (!e)
                                                e = ls->element(0);
                                          eblw = e ? e->width() : 0.0;
                                          }
                                    if (seg == 1)
                                          rest->setMMWidth(xpos[segs] - 2 * s->x() - eblw);
                                    else
                                          rest->setMMWidth(xpos[segs] - s->x() - point(score()->styleS(ST_barNoteDistance)) - eblw);
                                    e->rxpos() = 0.0;
                                    }
                              }
                        else if (rest->durationType() == TDuration::V_MEASURE) {
                              qreal x1 = seg == 0 ? 0.0 : xpos[seg] - clefKeyRightMargin;
                              qreal w;
                              if ((segs > 2) && types[segs-2] == Segment::SegClef)
                                    w  = xpos[segs-2] - x1;
                              else
                                    w  = xpos[segs-1] - x1;
                              e->rxpos() = (w - e->width()) * .5 + x1 - s->x();
                              }
                        e->adjustReadPos();
                        }
                  else if (t == REPEAT_MEASURE) {
                        qreal x1 = seg == 0 ? 0.0 : xpos[seg] - clefKeyRightMargin;
                        qreal w  = xpos[segs-1] - x1;
                        e->rxpos() = (w - e->width()) * .5 + x1 - s->x();
                        }
                  else if (t == CHORD)
                        static_cast<Chord*>(e)->layout2();
                  else if (t == CLEF) {
                        qreal gap = 0.0;
                        Segment* ps = s->prev();
                        if (ps)
                              gap = s->x() - (ps->x() + ps->width());
                        e->rxpos() = -gap * .5;
                        e->adjustReadPos();
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
//---------------------------------------------------------

void Measure::layoutStage1()
      {
//      (systemHeader() ? _minWidth2 : _minWidth1) = 0.0;
      setDirty();

      for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
            setBreakMMRest(false);
            if (score()->styleB(ST_createMultiMeasureRests)) {
                  if ((repeatFlags() & RepeatStart) || (prevMeasure() && (prevMeasure()->repeatFlags() & RepeatEnd)))
                        setBreakMMRest(true);
                  else if (!breakMMRest()) {
                        for (Segment* s = first(); s; s = s->next()) {
                              int n = s->annotations().size();
                              for (int i = 0; i < n; ++i) {
                                    Element* e = s->annotations().at(i);
                                    if (e->type() == REHEARSAL_MARK || e->type() == TEMPO_TEXT) {
                                          setBreakMMRest(true);
                                          break;
                                          }
                                    }
                              n = s->spannerFor().size();
                              for (int i = 0; i < n; ++i) {
                                    Spanner* sp = s->spannerFor().at(i);
                                    if (sp->type() == VOLTA) {
                                          setBreakMMRest(true);
                                          break;
                                          }
                                    }
                              n = s->spannerBack().size();
                              for (int i = 0; i < n; ++i) {
                                    Spanner* sp = s->spannerBack().at(i);
                                    if (sp->type() == VOLTA) {
                                          setBreakMMRest(true);
                                          break;
                                          }
                                    }
                              if (breakMMRest())      // optimize
                                    break;
                              }
                        }
                  }

            int track = staffIdx * VOICES;

            for (Segment* segment = first(); segment; segment = segment->next()) {
                  Element* e = segment->element(track);

                  if (segment->subtype() & (Segment::SegKeySig | Segment::SegStartRepeatBarLine | Segment::SegTimeSig)) {
                        if (e && !e->generated())
                              setBreakMMRest(true);
                        }

                  if (segment->subtype() & Segment::SegChordRestGrace)
                        layoutChords0(segment, staffIdx * VOICES);
                  }
            }
      int n = _spannerFor.size();
      for (int i = 0; i < n; ++i) {
            Spanner* spanner = _spannerFor.at(i);
            if (spanner->type() == Element::VOLTA)
                  setBreakMMRest(true);
            }
      MeasureBase* mb = prev();
      if (mb && mb->type() == Element::MEASURE) {
            Measure* pm = static_cast<Measure*>(mb);
            if (pm->endBarLineType() != NORMAL_BAR
               && pm->endBarLineType() != BROKEN_BAR && pm->endBarLineType() != DOTTED_BAR)
                  setBreakMMRest(true);
            int n = pm->_spannerBack.size();
            for (int i = 0; i < n; ++i) {
                  Spanner* spanner = pm->_spannerBack.at(i);
                  if (spanner->type() == Element::VOLTA)
                        setBreakMMRest(true);
                  }
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

            // TAB_STAFF is different, as each note has to be fretted
            // in the context of the whole chord

            if (staffGroup == TAB_STAFF) {
                  instrument->tablature()->fretChord(chord);
                  continue;               // skip other staff type cases
                  }

            // PITCHED_ and PERCUSSION_STAFF can go note by note

            int n = chord->notes().size();
            for (int i = 0; i < n; ++i) {
                  Note* note = chord->notes().at(i);
                  switch(staffGroup) {
                        case PITCHED_STAFF:
                              if (note->tieBack()) {
                                    int line = note->tieBack()->startNote()->line();
                                    note->setLine(line);
                                    if (note->accidental()) {
                                          // TODO: remove accidental only if note is not
                                          // on new system
                                          score()->undoRemoveElement(note->accidental());
                                          }
                                    }
                              else
                                    note->updateAccidental(tversatz);
                              break;
                        case PERCUSSION_STAFF:
                              {
                              Drumset* drumset = instrument->drumset();
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
                              break;
                        case TAB_STAFF:   // to avoid compiler warning
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

Measure* Measure::cloneMeasure(Score* sc, TieMap* tieMap, SpannerMap* spannerMap)
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
      m->_noText                = 0;
      m->_userStretch           = _userStretch;
      m->_irregular             = _irregular;
      m->_breakMultiMeasureRest = _breakMultiMeasureRest;
      m->_breakMMRest           = _breakMMRest;
      m->_endBarLineGenerated   = _endBarLineGenerated;
      m->_endBarLineVisible     = _endBarLineVisible;
      m->_endBarLineType        = _endBarLineType;
      m->_mmEndBarLineType      = _mmEndBarLineType;
      m->_multiMeasure          = _multiMeasure;
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
            s->setSubtype(oseg->subtype());
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
                              foreach(Spanner* sp, ocr->spannerFor()) {
                                    if (sp->type() != SLUR)
                                          continue;
                                    Slur* s = static_cast<Slur*>(sp);
                                    Slur* slur = new Slur(*s);
                                    slur->setScore(sc);
                                    slur->setStartElement(ncr);
                                    ncr->addSlurFor(slur);
                                    spannerMap->add(s, slur);
                                    }
                              foreach(Spanner* sp, ocr->spannerBack()) {
                                    if (sp->type() != SLUR)
                                          continue;
                                    Slur* s = static_cast<Slur*>(sp);
                                    Slur* slur = static_cast<Slur*>(spannerMap->findNew(s));
                                    if (slur) {
                                          slur->setEndElement(ncr);
                                          ncr->addSlurBack(slur);
                                          }
                                    else {
                                          qDebug("cloneMeasure(%d): cannot find slur, track %d", tick(), track);
                                          int tracks = score()->nstaves() * VOICES;
                                          for (int i = 0; i < tracks; ++i) {
                                                Slur* sl = static_cast<Slur*>(spannerMap->findNew(s));
                                                if (sl) {
                                                      qDebug("    found in track %d", i);
                                                      break;
                                                      }
                                                }
                                          }
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
                  foreach(Spanner* spanner, oseg->spannerFor()) {
                        if (spanner->track() != track)
                              continue;
                        Spanner* nsp = static_cast<Spanner*>(spanner->clone());
                        nsp->setScore(sc);
                        s->addSpannerFor(nsp);
                        spannerMap->add(spanner, nsp);
                        nsp->setStartElement(s);
                        }
                  foreach(Spanner* osp, oseg->spannerBack()) {
                        if (osp->track() != track)
                              continue;
                        Spanner* spanner = spannerMap->findNew(osp);
                        if (spanner) {
                              s->addSpannerBack(spanner);
                              spanner->setEndElement(s);
                              }
                        else
                              qDebug("cloneMeasure: cannot find spanner %p", osp);
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
                  break;
            case P_TIMESIG_ACTUAL:
                  return QVariant::fromValue(_len);
                  break;
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
            default:
                  return MeasureBase::setProperty(propertyId, value);
            }
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Measure::propertyDefault(P_ID) const
      {
      return QVariant();
      }
