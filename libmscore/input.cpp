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
#include "measure.h"

namespace Ms {

class DrumSet;

//---------------------------------------------------------
//   drumset
//---------------------------------------------------------

const Drumset* InputState::drumset() const
      {
      if (_segment == 0 || _track == -1)
            return 0;
      return _segment->score()->staff(_track/VOICES)->part()->instrument(_segment->tick())->drumset();
      }

//---------------------------------------------------------
//   staffGroup
//---------------------------------------------------------

StaffGroup InputState::staffGroup() const
      {
      if (_segment == 0 || _track == -1)
            return StaffGroup::STANDARD;
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
//   update
//---------------------------------------------------------

void InputState::update(Element* e)
      {
      if (e == 0)
            return;
      if (e && e->type() == Element::Type::CHORD)
            e = static_cast<Chord*>(e)->upNote();

      setDrumNote(-1);
      if (e->type() == Element::Type::NOTE) {
            Note* note    = static_cast<Note*>(e);
            Chord* chord  = note->chord();
            setDuration(chord->durationType());
            setRest(false);
            setTrack(note->track());
            setNoteType(note->noteType());
            setBeamMode(chord->beamMode());
            }
      else if (e->type() == Element::Type::REST) {
            Rest* rest   = static_cast<Rest*>(e);
            if (rest->durationType().type() == TDuration::DurationType::V_MEASURE)
                  setDuration(TDuration::DurationType::V_QUARTER);
            else
                  setDuration(rest->durationType());
            setRest(true);
            setTrack(rest->track());
            setBeamMode(rest->beamMode());
            setNoteType(NoteType::NORMAL);
            }
      if (e->type() == Element::Type::NOTE || e->type() == Element::Type::REST) {
            const Instrument* instr = e->part()->instrument();
            if (instr->useDrumset()) {
                  if (e->type() == Element::Type::NOTE)
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
      if (e->isChordRest1())
            s = toChordRest(e)->segment();
      else
            s = toSegment(e);

      if (s->isSegment()) {
            if (s->measure()->isMMRest()) {
                  Measure* m = s->measure()->mmRestFirst();
                  s = m->findSegment(Segment::Type::ChordRest, m->tick());
                  }
            _lastSegment = _segment;
            _segment = s;
            }
      }

//---------------------------------------------------------
//   setSegment
//---------------------------------------------------------

void InputState::setSegment(Segment* s)
      {
      if (s && s->measure()->isMMRest()) {
            Measure* m = s->measure()->mmRestFirst();
            s = m->findSegment(Segment::Type::ChordRest, m->tick());
            }
      _segment = s;
      _lastSegment = s;
      }

//---------------------------------------------------------
//   nextInputPos
//---------------------------------------------------------

Segment* InputState::nextInputPos() const
      {
      Measure* m = _segment->measure();
      Segment* s = _segment->next1(Segment::Type::ChordRest);
      for (; s; s = s->next1(Segment::Type::ChordRest)) {
            if (s->element(_track) || s->measure() != m) {
                  if (s->element(_track)) {
                        if (s->element(_track)->isRest() && toRest(s->element(_track))->isGap())
                              continue;
                        }
                  return s;
                  }
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
      if (s)
            _segment = s;
      }

//---------------------------------------------------------
//   endOfScore
//---------------------------------------------------------

bool InputState::endOfScore() const
      {
      return (_lastSegment == _segment) && !nextInputPos();
      }


}

