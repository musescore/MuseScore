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

#include "cursor.h"
#include "elements.h"
#include "score.h"
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

namespace Ms {
namespace PluginAPI {

//---------------------------------------------------------
//   Cursor
//---------------------------------------------------------

Cursor::Cursor(Ms::Score* s)
   : QObject(0), _filter(Ms::SegmentType::ChordRest)
      {
      setScore(s);
      }

//---------------------------------------------------------
//   score
//---------------------------------------------------------

Score* Cursor::score() const
      {
      return wrap<Score>(_score, Ownership::SCORE);
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Cursor::setScore(Ms::Score* s)
      {
      _score = s;
      if (_score) {
            _score->inputState().setTrack(_track);
            _score->inputState().setSegment(_segment);
            }
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Cursor::setScore(Score* s)
      {
      setScore(s ? s->score() : nullptr);
      }

//---------------------------------------------------------
//   rewind
///   Rewind cursor to a certain position.
///   \param mode Determines the position where to move
///   this cursor. See Cursor::RewindMode to see the list of
///   avaliable rewind modes.
///   \note In MuseScore 2.X, this function took an integer
///   value (0, 1 or 2) as its parameter. For compatibility
///   reasons, the old values are still working, but it is
///   recommended to use RewindMode enumerators instead.
//---------------------------------------------------------

void Cursor::rewind(RewindMode mode)
      {
      //
      // rewind to start of score
      //
      if (mode == SCORE_START) {
            _segment = nullptr;
            Ms::Measure* m = _score->firstMeasure();
            if (m) {
                  _segment = m->first(_filter);
                  nextInTrack();
                  }
            }
      //
      // rewind to start of selection
      //
      else if (mode == SELECTION_START) {
            if (!_score->selection().isRange())
                  return;
            _segment  = _score->selection().startSegment();
            _track    = _score->selection().staffStart() * VOICES;
            nextInTrack();
            }
      //
      // rewind to end of selection
      //
      else if (mode == SELECTION_END) {
            if (!_score->selection().isRange())
                  return;
            _segment  = _score->selection().endSegment();
            _track    = (_score->selection().staffEnd() * VOICES) - 1;  // be sure _track exists
            }
      _score->inputState().setTrack(_track);
      _score->inputState().setSegment(_segment);
      }

//---------------------------------------------------------
//   next
///   Move the cursor to the next segment.
///   \return \p false if the end of the score is reached,
///   \p true otherwise.
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
///   Move the cursor to the first segment of the next
///   measure.
///   \return \p false if the end of the score is reached,
///   \p true otherwise.
//---------------------------------------------------------

bool Cursor::nextMeasure()
      {
      if (_segment == 0)
            return false;
      Ms::Measure* m = _segment->measure()->nextMeasure();
      if (m == 0) {
            _segment = 0;
            return false;
            }
      _segment = m->first(_filter);
      nextInTrack();
      return _segment != 0;
      }

//---------------------------------------------------------
//   add
///   Adds the given element to a score at this cursor's
///   position.
//---------------------------------------------------------

void Cursor::add(Element* wrapped)
      {
      Ms::Element* s = wrapped->element();
      if (!_segment || !s)
            return;

      wrapped->setOwnership(Ownership::SCORE);
      s->setTrack(_track);
      s->setParent(_segment);

      if (s->isChordRest())
            s->score()->undoAddCR(toChordRest(s), _segment->measure(), _segment->tick());
      else if (s->type() == ElementType::KEYSIG) {
            Ms::Segment* ns = _segment->measure()->undoGetSegment(SegmentType::KeySig, _segment->tick());
            s->setParent(ns);
            _score->undoAddElement(s);
            }
      else if (s->type() == ElementType::TIMESIG) {
            Ms::Measure* m = _segment->measure();
            Fraction tick = m->tick();
            _score->cmdAddTimeSig(m, _track, toTimeSig(s), false);
            m = _score->tick2measure(tick);
            _segment = m->first(_filter);
            nextInTrack();
            }
      else if (s->type() == ElementType::LAYOUT_BREAK) {
            Ms::Measure* m = _segment->measure();
            s->setParent(m);
            _score->undoAddElement(s);
            }
      else {
            _score->undoAddElement(s);
            }
      _score->setLayoutAll();
      }

//---------------------------------------------------------
//   addNote
///   \brief Adds a note to the current cursor position.
///   \details The duration of the added note equals to
///   what has been set by the previous setDuration() call.
///   \param pitch MIDI pitch of the added note.
//---------------------------------------------------------

void Cursor::addNote(int pitch)
      {
      if (!pitchIsValid(pitch)) {
            qWarning("Cursor::addNote: invalid pitch: %d", pitch);
            return;
            }
      if (!_score->inputState().duration().isValid())
            setDuration(1, 4);
      NoteVal nval(pitch);
      _score->addPitch(nval, false);
      }

//---------------------------------------------------------
//   setDuration
///   Set duration of the notes added by the cursor.
///   \param z: numerator
///   \param n: denominator. If n == 0, sets duration to
///   a quarter.
///   \see addNote()
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
      return (_segment) ? _segment->tick().ticks() : 0;
      }

//---------------------------------------------------------
//   time
//---------------------------------------------------------

double Cursor::time()
      {
      return _score->utick2utime(tick()) * 1000;
      }

//---------------------------------------------------------
//   tempo
//---------------------------------------------------------

qreal Cursor::tempo()
      {
      return _score->tempo(Fraction::fromTicks(tick()));
      }

//---------------------------------------------------------
//   segment
//---------------------------------------------------------

Segment* Cursor::segment() const
      {
      return _segment ? wrap<Segment>(_segment, Ownership::SCORE) : nullptr;
      }

//---------------------------------------------------------
//   element
//---------------------------------------------------------

Element* Cursor::element() const
      {
      Ms::Element* e = _segment && _segment->element(_track) ? _segment->element(_track) : nullptr;
      if (!e)
            return nullptr;
      return wrap(e, Ownership::SCORE);
      }

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

Measure* Cursor::measure() const
      {
      return _segment ? wrap<Measure>(_segment->measure(), Ownership::SCORE) : nullptr;
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

void Cursor::nextInTrack()
      {
      while (_segment && _segment->element(_track) == 0)
            _segment = _segment->next1(_filter);
      }

//---------------------------------------------------------
//   qmlKeySignature
//   read access to key signature in current track
//   at current position
//---------------------------------------------------------

int Cursor::qmlKeySignature()
      {
      Staff* staff = _score->staves()[staffIdx()];
      return (int) staff->key(Fraction::fromTicks(tick()));
      }
}
}
