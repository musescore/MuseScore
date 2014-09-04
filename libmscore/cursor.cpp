//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "libmscore/score.h"
#include "libmscore/chordrest.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/note.h"
#include "libmscore/stafftext.h"
#include "libmscore/measure.h"
#include "libmscore/repeatlist.h"
#include "libmscore/page.h"
#include "libmscore/system.h"
#include "libmscore/segment.h"
#include "libmscore/timesig.h"
#include "cursor.h"

namespace Ms {

//---------------------------------------------------------
//   Cursor
//---------------------------------------------------------

Cursor::Cursor(Score* s)
   : QObject(0)
      {
      _track   = 0;
      _segment = 0;
      setScore(s);
      }

void Cursor::setScore(Score* s)
      {
      _score = s;
      if (_score) {
            _score->inputState().setTrack(_track);
            _score->inputState().setSegment(_segment);
            }
      }

//---------------------------------------------------------
//   rewind
//---------------------------------------------------------

void Cursor::rewind(int type)
      {
      if (type == 0) {
            _segment = 0;
            Measure* m = _score->firstMeasure();
            if (m) {
                  _segment = m->first(Segment::Type::ChordRest);
                  firstChordRestInTrack();
                  }
            }
      else if (type == 1) {
            _segment  = _score->selection().startSegment();
            _track    = _score->selection().staffStart() * VOICES;
            firstChordRestInTrack();
            }
      else if (type == 2) {
            _segment  = _score->selection().endSegment();
            _track    = (_score->selection().staffEnd() * VOICES) - 1;  // be sure _track exists
            }
      _score->inputState().setTrack(_track);
      _score->inputState().setSegment(_segment);
      }

//---------------------------------------------------------
//   next
//    go to next segment
//    return false if end of score is reached
//---------------------------------------------------------

bool Cursor::next()
      {
      if (!_segment)
            return false;
      _segment = _segment->next1(Segment::Type::ChordRest);
      firstChordRestInTrack();
      _score->inputState().setTrack(_track);
      _score->inputState().setSegment(_segment);
      return _segment != 0;
      }

//---------------------------------------------------------
//   nextMeasure
//    go to first segment of next measure
//    return false if end of score is reached
//---------------------------------------------------------

bool Cursor::nextMeasure()
      {
      if (_segment == 0)
            return false;
      Measure* m = _segment->measure()->nextMeasure();
      if (m == 0) {
            _segment = 0;
            return false;
            }
      _segment = m->first(Segment::Type::ChordRest);
//      while (seg && seg->element(_track) == 0)
//            seg = seg->next1(SegChordRest);
      firstChordRestInTrack();
      return _segment != 0;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Cursor::add(Element* s)
      {
      if (!_segment)
            return;

      s->setTrack(_track);
      s->setParent(_segment);

      if (s->isChordRest())
            s->score()->undoAddCR(static_cast<ChordRest*>(s), _segment->measure(), _segment->tick());
      else if (s->type() == Element::Type::KEYSIG) {
            Segment* ns = _segment->measure()->undoGetSegment(Segment::Type::KeySig, _segment->tick());
            s->setParent(ns);
            _score->undoAddElement(s);
            }
      else if (s->type() == Element::Type::TIMESIG) {
            Measure* m = _segment->measure();
            int tick = m->tick();
            _score->cmdAddTimeSig(m, _track, static_cast<TimeSig*>(s), false);
            m = _score->tick2measure(tick);
            _segment = m->first(Segment::Type::ChordRest);
            firstChordRestInTrack();
            }
      else if (s->type() == Element::Type::LAYOUT_BREAK) {
            Measure* m = _segment->measure();
            s->setParent(m);
            _score->undoAddElement(s);
            }
      else {
            _score->undoAddElement(s);
            }
      _score->setLayoutAll(true);
      }

//---------------------------------------------------------
//   addNote
//---------------------------------------------------------

void Cursor::addNote(int pitch)
      {
      NoteVal nval(pitch);
      _score->addPitch(nval, false);
      }

//---------------------------------------------------------
//   setDuration
//---------------------------------------------------------

void Cursor::setDuration(int z, int n)
      {
      TDuration d(Fraction(z, n));
      if (!d.isValid())
            d = TDuration(TDuration::DurationType::V_QUARTER);
      _score->inputState().setDuration(d);
      }

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

int Cursor::tick()
      {
      return (_segment) ? _segment->tick() : 0;
      }

//---------------------------------------------------------
//   time
//---------------------------------------------------------

double Cursor::time()
      {
      return _score->utick2utime(tick()) * 1000;
      }

//---------------------------------------------------------
//   element
//---------------------------------------------------------

Element* Cursor::element() const
      {
      return _segment ? _segment->element(_track) : 0;
      }

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

Measure* Cursor::measure() const
      {
      return _segment ? _segment->measure() : 0;
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Cursor::setTrack(int v)
      {
      _track = v;
      int tracks = _score->nstaves() * VOICES;
      if (_track < 0)
            _track = 0;
      else if (_track >= tracks)
            _track = tracks - 1;
      _score->inputState().setTrack(_track);
      }

//---------------------------------------------------------
//   setStaffIdx
//---------------------------------------------------------

void Cursor::setStaffIdx(int v)
      {
      _track = v * VOICES + _track % VOICES;
      int tracks = _score->nstaves() * VOICES;
      if (_track < 0)
            _track = 0;
      else if (_track >= tracks)
            _track = tracks - 1;
      _score->inputState().setTrack(_track);
      }

//---------------------------------------------------------
//   setVoice
//---------------------------------------------------------

void Cursor::setVoice(int v)
      {
      _track = (_track / VOICES) * VOICES + v;
      int tracks = _score->nstaves() * VOICES;
      if (_track < 0)
            _track = 0;
      else if (_track >= tracks)
            _track = tracks - 1;
      _score->inputState().setTrack(_track);
      }

//---------------------------------------------------------
//   staffIdx
//---------------------------------------------------------

int Cursor::staffIdx() const
      {
      return _track / VOICES;
      }

//---------------------------------------------------------
//   voice
//---------------------------------------------------------

int Cursor::voice() const
      {
      return _track % VOICES;
      }

//---------------------------------------------------------
//   nextInTrack
//    go to first segment at or after _segment which has notes / rests in _track
//---------------------------------------------------------

inline void Cursor::firstChordRestInTrack()
      {
      while (_segment && _segment->element(_track) == 0)
            _segment = _segment->next1(Segment::Type::ChordRest);
      }

//---------------------------------------------------------
//   qmlKeySignature
//   read access to key signature in current track
//   at current position
//---------------------------------------------------------

int Cursor::qmlKeySignature()
      {
	    Staff *staff = _score->staves()[staffIdx()];
            return (int) staff->key(tick());
      }

//---------------------------------------------------------
//   inSelection
//---------------------------------------------------------

bool Cursor::inSelection()
      {
      if (!_segment)
            return false;

      // check if after start of selection
      Segment* seg = _score->selection().startSegment();
      int track = _score->selection().staffStart() * VOICES;

      if (!seg)
            return(false); // don't have selection
      if (seg->tick() > tick())
            return false; // selection starts later
      if (track > _track)
            return false; // our track is not included in the selection

      // check if before end of selection
      seg = _score->selection().endSegment();
      track = (_score->selection().staffEnd() * VOICES) - 1;

      if (_track > track)
            return false; // our track is not included in the selection
      if (!seg)
            return true; // selection contains last measure
      if (tick() < seg->tick())
            return true;
      else
            return false;
      }

//---------------------------------------------------------
//   hasSelection
//---------------------------------------------------------

bool Cursor::hasSelection()
      {
      if (_score->selection().startSegment())
            return true;
      else
            return false;
      }

//---------------------------------------------------------
//   iterate()
//
//   reset iterator
//---------------------------------------------------------

void Cursor::iterate(bool type)
      {
      switch (type) {
            case false: // iterate over whole score
                  _iterationType = 0;
                  setTrack(0);
                  rewind(0);
                  break;
            case true:  // iterate over selection
                  if (!hasSelection())
                        _iterationType = -1;
                  else {
                        _iterationType = 1;
                        rewind(1);
                        }
                  break;
            }
      }

//---------------------------------------------------------
//   iterating()
//---------------------------------------------------------

bool Cursor::iterating()
      {
      switch (_iterationType) {
            case 0:
            case 1:
                  return true;
            case -1:
                  return false;
            }
      }

//---------------------------------------------------------
//   nextTrack()
//---------------------------------------------------------

void Cursor::nextTrack()
      {
      int tracks = _score->nstaves() * VOICES;
      if ((_track + 1) >= tracks) {
            _iterationType = -1; // done iterating
            return;
            }

      switch (_iterationType) {
            case 0:
                  setTrack(_track + 1);
                  rewind(0);
                  break;
            case 1:
                  int nextTrack = _track + 1;
                  if ((_track + 1) >= (_score->selection().staffEnd() * VOICES))
                        _iterationType = -1; // done iterating
                  else {
                        rewind(1);
                        setTrack(nextTrack);
                        }
                  break;
            }
      }

//---------------------------------------------------------
//   nextStaff()
//---------------------------------------------------------

void Cursor::nextStaff()
      {
      int tracks = _score->nstaves() * VOICES;
      int nextTrack = (staffIdx() + 1) * VOICES;
      if (nextTrack >= tracks) {
            _iterationType = -1; // done iterating
            return;
            }

      switch (_iterationType) {
            case 0:
                  setTrack(nextTrack);
                  rewind(0);
                  break;
            case 1:
                  if (nextTrack >= (_score->selection().staffEnd() * VOICES))
                        _iterationType = -1; // done iterating
                  else {
                        rewind(1);
                        setTrack(nextTrack);
                        }
                  break;
            }
      }

}

