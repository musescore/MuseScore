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
//   ElementW
//---------------------------------------------------------

QString ElementW::name() const
      {
      return QString(e->name());
      }

int ElementW::type() const
      {
      return int(e->type());
      }

int ElementW::tick() const
      {
      return ((Element*)e)->tick();
      }

QVariant ElementW::get(const QString& s) const
      {
      QVariant val;
      if (e) {
            Pid pid = propertyIdQml(s);
            val = e->getProperty(pid);
            if (propertyType(pid) == P_TYPE::FRACTION) {
                  Fraction f(val.value<Fraction>());
                  FractionWrapper*  fw = new FractionWrapper(f);
                  return QVariant::fromValue(fw);
                  }
            }
      return val;
      }

//---------------------------------------------------------
//   Cursor
//---------------------------------------------------------

Cursor::Cursor(Score* s)
   : QObject(0)
      {
      _track   = 0;
      _segment = 0;
      _filter  = SegmentType::ChordRest;
      setScore(s);
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Cursor::setScore(Score* s)
      {
      _score = s;
      if (score()) {
            score()->inputState().setTrack(_track);
            score()->inputState().setSegment(_segment);
            }
      }

//---------------------------------------------------------
//   rewind
//---------------------------------------------------------

void Cursor::rewind(int type)
      {
      //
      // rewind to start of score
      //
      if (type == 0) {
            _segment = 0;
            Measure* m = score()->firstMeasure();
            if (m) {
                  _segment = m->first(_filter);
                  nextInTrack();
                  }
            }
      //
      // rewind to start of selection
      //
      else if (type == 1) {
            _segment  = score()->selection().startSegment();
            _track    = score()->selection().staffStart() * VOICES;
            nextInTrack();
            }
      //
      // rewind to end of selection
      //
      else if (type == 2) {
            _segment  = score()->selection().endSegment();
            _track    = (score()->selection().staffEnd() * VOICES) - 1;  // be sure _track exists
            }
      score()->inputState().setTrack(_track);
      score()->inputState().setSegment(_segment);
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
      score()->inputState().setTrack(_track);
      score()->inputState().setSegment(_segment);
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
            s->score()->undoAddCR(toChordRest(s), _segment->measure(), _segment->tick());
      else if (s->type() == ElementType::KEYSIG) {
            Segment* ns = _segment->measure()->undoGetSegment(SegmentType::KeySig, _segment->tick());
            s->setParent(ns);
            score()->undoAddElement(s);
            }
      else if (s->type() == ElementType::TIMESIG) {
            Measure* m = _segment->measure();
            int tick = m->tick();
            score()->cmdAddTimeSig(m, _track, toTimeSig(s), false);
            m = score()->tick2measure(tick);
            _segment = m->first(_filter);
            nextInTrack();
            }
      else if (s->type() == ElementType::LAYOUT_BREAK) {
            Measure* m = _segment->measure();
            s->setParent(m);
            score()->undoAddElement(s);
            }
      else {
            score()->undoAddElement(s);
            }
      score()->setLayoutAll();
      }

//---------------------------------------------------------
//   addNote
//---------------------------------------------------------

void Cursor::addNote(int pitch)
      {
      NoteVal nval(pitch);
      score()->addPitch(nval, false);
      }

//---------------------------------------------------------
//   setDuration
//---------------------------------------------------------

void Cursor::setDuration(int z, int n)
      {
      TDuration d(Fraction(z, n));
      if (!d.isValid())
            d = TDuration(TDuration::DurationType::V_QUARTER);
      score()->inputState().setDuration(d);
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
      return score()->utick2utime(tick()) * 1000;
      }

//---------------------------------------------------------
//   tempo
//---------------------------------------------------------

qreal Cursor::tempo()
      {
      return score()->tempo(tick());
      }

//---------------------------------------------------------
//   segment
//---------------------------------------------------------

ElementW* Cursor::segment() const
      {
      return _segment ? new ElementW(_segment) : 0;
      }

//---------------------------------------------------------
//   element
//---------------------------------------------------------

ElementW* Cursor::element() const
      {
      return _segment && _segment->element(_track) ? new ElementW(_segment->element(_track)) : 0;
      }

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

ElementW* Cursor::measure() const
      {
      return _segment ? new ElementW(_segment->measure()) : 0;
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Cursor::setTrack(int v)
      {
      _track = v;
      int tracks = score()->nstaves() * VOICES;
      if (_track < 0)
            _track = 0;
      else if (_track >= tracks)
            _track = tracks - 1;
      score()->inputState().setTrack(_track);
      }

//---------------------------------------------------------
//   setStaffIdx
//---------------------------------------------------------

void Cursor::setStaffIdx(int v)
      {
      _track = v * VOICES + _track % VOICES;
      int tracks = score()->nstaves() * VOICES;
      if (_track < 0)
            _track = 0;
      else if (_track >= tracks)
            _track = tracks - 1;
      score()->inputState().setTrack(_track);
      }

//---------------------------------------------------------
//   setVoice
//---------------------------------------------------------

void Cursor::setVoice(int v)
      {
      _track = (_track / VOICES) * VOICES + v;
      int tracks = score()->nstaves() * VOICES;
      if (_track < 0)
            _track = 0;
      else if (_track >= tracks)
            _track = tracks - 1;
      score()->inputState().setTrack(_track);
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
      Staff *staff = score()->staves()[staffIdx()];
      return (int) staff->key(tick());
      }
}

