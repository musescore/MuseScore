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
///   available rewind modes.
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
//   prev
///   Move the cursor to the previous segment.
///   \return \p false if the beginning of the score is
///   reached, \p true otherwise.
///   \since MuseScore 3.3.4
//---------------------------------------------------------

bool Cursor::prev()
      {
      if (!_segment)
            return false;
      prevInTrack();
      _score->inputState().setTrack(_track);
      _score->inputState().setSegment(_segment);
      return _segment != 0;
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
      Ms::Element* s = wrapped ? wrapped->element() : nullptr;
      if (!_segment || !s)
            return;

      // Ensure that the object has the expected ownership
      if (wrapped->ownership() == Ownership::SCORE) {
            qWarning("Cursor::add: Cannot add this element. The element is already part of the score.");
            return;        // Don't allow operation.
            }

      wrapped->setOwnership(Ownership::SCORE);
      s->setScore(_score);
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
      else {
            switch (s->type()) {
                  // To be added at measure level
                  case ElementType::MEASURE_NUMBER:
                  case ElementType::SPACER:
                  case ElementType::JUMP:
                  case ElementType::MARKER:
                  case ElementType::HBOX:
                  case ElementType::STAFFTYPE_CHANGE:
                  case ElementType::LAYOUT_BREAK: {
                        Ms::Measure* m = _segment->measure();
                        s->setParent(m);
                        _score->undoAddElement(s);
                        break;
                        }

                  // To be added at chord level
                  case ElementType::NOTE:
                  case ElementType::ARPEGGIO:
                  case ElementType::TREMOLO:
                  case ElementType::CHORDLINE:
                  case ElementType::ARTICULATION: {
                        Ms::Element* curElement = currentElement();
                        if (curElement->isChord()) {
                              // call Chord::addInternal() (i.e. do the same as a call to Chord.add())
                              Chord::addInternal(toChord(curElement), s);
                              }
                        break;
                        break;
                        }

                  // To be added at chord/rest level
                  case ElementType::LYRICS: {
                        Ms::Element* curElement = currentElement();
                        if (curElement->isChordRest()) {
                              s->setParent(curElement);
                              _score->undoAddElement(s);
                              }
                        break;
                        }

                  // To be added to a note
                  case ElementType::SYMBOL:
                  case ElementType::FINGERING:
                  case ElementType::BEND:
                  case ElementType::NOTEHEAD: {
                        Ms::Element* curElement = currentElement();
                        if (curElement->isChord()) {
                              Ms::Chord* chord = toChord(curElement);
                              Ms::Note* note = nullptr;
                              if (chord->notes().size() > 0) {
                                    // Get first note from chord to add element
                                    note = chord->notes().front();
                                    }
                              if (note) {
                                    Note::addInternal(note, s);
                                    }
                              }
                        break;
                        }

                  // To be added to a segment (clef subtype)
                  case ElementType::CLEF:
                  case ElementType::AMBITUS: {
                        Ms::Element* parent = nullptr;
                        // Find backwards first measure containing a clef
                        for (Ms::Measure* m = _segment->measure(); m != 0; m = m->prevMeasure()) {
                              Ms::Segment* seg = m->findSegment(SegmentType::Clef | SegmentType::HeaderClef, m->tick());
                                    if (seg != 0) {
                                          parent = m->undoGetSegmentR(s->isAmbitus() ? SegmentType::Ambitus : seg->segmentType(), Fraction(0,1));
                                          break;
                                          }
                              }
                        if (parent && parent->isSegment()) {
                              if (s->isClef()) {
                                    Ms::Clef* clef = toClef(s);
                                    if (clef->clefType() == Ms::ClefType::INVALID) {
                                          clef->setClefType(Ms::ClefType::G);
                                          }
                                    }
                              s->setParent(parent);
                              s->setTrack(_track);
                              _score->undoAddElement(s);
                              }
                        break;
                        }

                  default: // All others will be added to the current segment
                        _score->undoAddElement(s);
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   addNote
///   \brief Adds a note to the current cursor position.
///   \details The duration of the added note equals to
///   what has been set by the previous setDuration() call.
///   \param pitch MIDI pitch of the added note.
///   \param addToChord add note to the current chord
///   instead of replacing it. This parameter is available
///   since MuseScore 3.3.4.
//---------------------------------------------------------

void Cursor::addNote(int pitch, bool addToChord)
      {
      if (!pitchIsValid(pitch)) {
            qWarning("Cursor::addNote: invalid pitch: %d", pitch);
            return;
            }
      if (!_score->inputState().duration().isValid())
            setDuration(1, 4);
      NoteVal nval(pitch);
      _score->addPitch(nval, addToChord);
      _segment = _score->inputState().segment();
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
//   currentElement
//---------------------------------------------------------

Ms::Element* Cursor::currentElement() const
      {
      return _segment && _segment->element(_track) ? _segment->element(_track) : nullptr;
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
      Ms::Element* e = currentElement();
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
//   prevInTrack
//    go to first segment before _segment which has notes / rests in _track
//---------------------------------------------------------

void Cursor::prevInTrack()
      {
      if (_segment)
            _segment = _segment->prev1(_filter);
      while (_segment && !_segment->element(_track))
            _segment = _segment->prev1(_filter);
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
