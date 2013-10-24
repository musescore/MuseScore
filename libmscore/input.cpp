//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "input.h"
#include "segment.h"
#include "part.h"
#include "staff.h"
#include "score.h"
#include "chord.h"
#include "rest.h"

namespace Ms {

class DrumSet;

//---------------------------------------------------------
//   drumset
//---------------------------------------------------------

Drumset* InputState::drumset() const
      {
      if (_segment == 0 || _track == -1)
            return 0;
      return _segment->score()->staff(_track/VOICES)->part()->instr(_segment->tick())->drumset();
      }

//---------------------------------------------------------
//   staffGroup
//---------------------------------------------------------

StaffGroup InputState::staffGroup() const
      {
      if (_segment == 0 || _track == -1)
            return STANDARD_STAFF_GROUP;
      return _segment->score()->staff(_track/VOICES)->staffType()->group();
      }

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

int InputState::tick() const
      {
      return _segment ? _segment->tick() : 0;
      }

//---------------------------------------------------------
//   cr
//---------------------------------------------------------

ChordRest* InputState::cr() const
      {
      return _segment ? static_cast<ChordRest*>(_segment->element(_track)) : 0;
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void InputState::setTrack(int v)
      {
      _track = v;
      }

//---------------------------------------------------------
//   update
//---------------------------------------------------------

void InputState::update(Element* e)
      {
      if (e == 0)
            return;
      if (e && e->type() == Element::CHORD)
            e = static_cast<Chord*>(e)->upNote();

      setDrumNote(-1);
      if (e->type() == Element::NOTE) {
            Note* note    = static_cast<Note*>(e);
            Chord* chord  = note->chord();
            setDuration(chord->durationType());
            setRest(false);
            setTrack(note->track());
            setNoteType(note->noteType());
            setBeamMode(chord->beamMode());
            }
      else if (e->type() == Element::REST) {
            Rest* rest   = static_cast<Rest*>(e);
            if (rest->durationType().type() == TDuration::V_MEASURE)
                  setDuration(TDuration::V_QUARTER);
            else
                  setDuration(rest->durationType());
            setRest(true);
            setTrack(rest->track());
            setBeamMode(rest->beamMode());
            setNoteType(NOTE_NORMAL);
            }
      if (e->type() == Element::NOTE || e->type() == Element::REST) {
            const Instrument* instr = e->staff()->part()->instr();
            if (instr->useDrumset()) {
                  if (e->type() == Element::NOTE)
                        setDrumNote(static_cast<Note*>(e)->pitch());
                  else
                        setDrumNote(-1);
                  }
            }
      }

//---------------------------------------------------------
//   moveInputPos
//---------------------------------------------------------

void InputState::moveInputPos(Element* e)
      {
      if (e == 0)
            return;
      Segment* s;
      if (e->isChordRest())
            s = static_cast<ChordRest*>(e)->segment();
      else
            s = static_cast<Segment*>(e);
      if (s->type() == Element::SEGMENT) {
            _lastSegment = _segment;
            _segment = s;
            _score->setPos(POS::CURRENT, _segment->tick());
            }
      }

//---------------------------------------------------------
//   nextInputPos
//---------------------------------------------------------

Segment* InputState::nextInputPos() const
      {
      Measure* m = _segment->measure();
      Segment* s = _segment->next1(Segment::SegChordRest);
      for (; s; s = s->next1(Segment::SegChordRest)) {
            if (s->element(_track) || s->measure() != m)
                  return s;
            }
      return 0;
      }

//---------------------------------------------------------
//   moveToNextInputPos
//   TODO: special case: note is first note of tie: goto to last note of tie
//---------------------------------------------------------

void InputState::moveToNextInputPos()
      {
      Segment* s   = nextInputPos();
      _lastSegment = _segment;
      if (s) {
            _segment     = s;
            _score->setPos(POS::CURRENT, _segment->tick());
            }
      }

//---------------------------------------------------------
//   endOfScore
//---------------------------------------------------------

bool InputState::endOfScore() const
      {
      return (_lastSegment == _segment) && !nextInputPos();
      }


}

