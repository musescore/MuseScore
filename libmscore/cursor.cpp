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
                  _segment = m->first(_filter);
                  nextInTrack();
                  }
            }
      else if (type == 1) {
            _segment  = _score->selection().startSegment();
            _track    = _score->selection().staffStart() * VOICES;
            nextInTrack();
            }
      else if (type == 2) {
            _segment  = _score->selection().endSegment();
            _track    = (_score->selection().staffEnd() * VOICES) - 1;  // be sure _track exists
            }
      _score->inputState().setTrack(_track);
      _score->inputState().setSegment(_segment);
      }

//---------------------------------------------------------
//   scoreStart
//    move cursor's segment to the first segment of the score, on current track;
//    obeys _filter.
//
//    Sets null _segment if current track has no segments of types in _filter
//---------------------------------------------------------

void Cursor::scoreStart()
      {
      _segment = 0;
      Measure* m = _score->firstMeasure();
      if (m) {
            _segment = m->first(_filter);
            nextInTrack();
            }
      _score->inputState().setTrack(_track);
      _score->inputState().setSegment(_segment);
      }

//---------------------------------------------------------
//   scoreEnd
//    move cursor to the end of the score, on current track;
//    obeys _filter.
//
//    Sets null _segment if current track has no segments of types in _filter
//---------------------------------------------------------

void Cursor::scoreEnd()
      {
      _segment = 0;
      Measure* m = _score->lastMeasure();
      if (m) {
            _segment = m->last();
            prevInTrack();
            }
      _score->inputState().setTrack(_track);
      _score->inputState().setSegment(_segment);
      }

//---------------------------------------------------------
//   selectionStart
//    move cursor's segment to the first segment of the selection, on current track;
//    obeys _filter.
//
//    Sets null _segment if current track is outside selection OR
//    has no segments of types in _filter
//---------------------------------------------------------

void Cursor::selectionStart()
      {
      int fromTrack     = _score->qmlSelectionFirstTrack();
      int toTrack       = _score->qmlSelectionLastTrack();
      // if current track is outside selection, set _segment to null
      if (_track < fromTrack || _track > toTrack)
            _segment = nullptr;
      else {
            _segment          = _score->selection().startSegment();
            // if no selection, return score start
            if (_segment == nullptr) {
                  scoreStart();
                  return;
                  }
            // if some selection, return first segment found of required type(s)
            else
                  nextInTrack();
            }
      _score->inputState().setTrack(_track);
      _score->inputState().setSegment(_segment);
      }

//---------------------------------------------------------
//   selectionEnd
//    move cursor to the last segment of the selection, on current track;
//    obeys _filter.
//
//    Sets null _segment if current track is outside selection OR
//    has no segments of types in _filter
//---------------------------------------------------------

void Cursor::selectionEnd()
      {
      int fromTrack     = _score->qmlSelectionFirstTrack();
      int toTrack       = _score->qmlSelectionLastTrack();
      // if current track is outside selection, set _segment to null
      if (_track < fromTrack || _track > toTrack)
            _segment = nullptr;
      else {
            _segment          = _score->selection().endSegment();
            // if no selection, return score end
            // (_segment may also be null if there is a selection and it extends up to score last segment)
            if (_segment == nullptr)
                  scoreEnd();
            else {
                  // returned end segment is right beyond actual selection end
                  _segment = _segment->prev1();
                  // if some selection, return last/previous segment found of required type(s)
                  prevInTrack();
                  }
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
      _segment = _segment->next1(_filter);
      nextInTrack();
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
      _segment = m->first(_filter);
//      while (seg && seg->element(_track) == 0)
//            seg = seg->next1(_filter);
      nextInTrack();
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
            _segment = m->first(_filter);
            nextInTrack();
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
//   prevInTrack
//    go to first segment with type included in _filter at or before _segment
//---------------------------------------------------------

void Cursor::prevInTrack()
      {
      while (_segment && (_segment->element(_track) == 0 || (_segment->segmentType() & _filter) == 0) )
            _segment = _segment->prev1(_filter);
      }

//---------------------------------------------------------
//   nextInTrack
//    go to first segment with type included in _filter at or after _segment
//---------------------------------------------------------

void Cursor::nextInTrack()
      {
      while (_segment && (_segment->element(_track) == 0 || (_segment->segmentType() & _filter) == 0) )
            _segment = _segment->next1(_filter);
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
}

