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
#include "stem.h"
#include "hook.h"
#include "score.h"
#include "chord.h"
#include "rest.h"
#include "measure.h"
#include "accidental.h"
#include "durationtype.h"
#include "select.h"

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

      StaffGroup staffGroup = _segment->score()->staff(_track/VOICES)->staffType(_segment->tick())->group();
      Instrument* instrument = _segment->score()->staff(_track/VOICES)->part()->instrument(_segment->tick());

      // if not tab, pitched/unpitched input depends on instrument, not staff (override StaffGroup)
      if ( staffGroup != StaffGroup::TAB)
            staffGroup = instrument->useDrumset() ? StaffGroup::PERCUSSION : StaffGroup::STANDARD;
      return staffGroup;
      }

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

Fraction InputState::tick() const
      {
      return _segment ? _segment->tick() : Fraction(0,1);
      }

//---------------------------------------------------------
//   cr
//---------------------------------------------------------

ChordRest* InputState::cr() const
      {
      // _track could potentially be -1, for instance after navigation through a frame
      return _segment && _track >= 0 ? toChordRest(_segment->element(_track)) : 0;
      }

//---------------------------------------------------------
//   setDots
//---------------------------------------------------------

void InputState::setDots(int n)
      {
      if (n && (!_duration.isValid() || _duration.isZero() || _duration.isMeasure()))
            _duration = TDuration::DurationType::V_QUARTER;
      _duration.setDots(n);
      }

//---------------------------------------------------------
//   note
//---------------------------------------------------------

Note* InputState::note(Element* e)
      {
      return e && e->isNote() ? toNote(e) : nullptr;
      }

//---------------------------------------------------------
//   chordRest
//---------------------------------------------------------

ChordRest* InputState::chordRest(Element* e)
      {
      if (!e)
            return nullptr;
      if (e->isChordRest())
            return toChordRest(e);
      if (e->isNote())
            return toNote(e)->chord();
      if (e->isStem())
            return toStem(e)->chord();
      if (e->isHook())
            return toHook(e)->chord();
      return nullptr;
      }

//---------------------------------------------------------
//   update
//---------------------------------------------------------

void InputState::update(Selection& selection)
      {
      setDuration(TDuration::DurationType::V_INVALID);
      setRest(false);
      setAccidentalType(AccidentalType::NONE);
      Note* n1 = nullptr;
      ChordRest* cr1 = nullptr;
      bool differentAccidentals = false;
      bool differentDurations = false;
      bool chordsAndRests = false;
      for (Element* e : selection.elements()) {
            if (Note* n = note(e)) {
                  if (n1) {
                        if (n->accidentalType() != n1->accidentalType()) {
                              setAccidentalType(AccidentalType::NONE);
                              differentAccidentals = true;
                              }
                        }
                  else {
                        setAccidentalType(n->accidentalType());
                        n1 = n;
                        }
                  }

            if (ChordRest* cr = chordRest(e)) {
                  if (cr1) {
                        if (cr->durationType() != cr1->durationType()) {
                              setDuration(TDuration::DurationType::V_INVALID);
                              differentDurations = true;
                              }
                        if ((cr->isRest() && !cr1->isRest()) || (!cr->isRest() && cr1->isRest())) {
                              setRest(false);
                              chordsAndRests = true;
                              }
                        }
                  else {
                        setDuration(cr->durationType());
                        setRest(cr->isRest());
                        cr1 = cr;
                        }
                  }

            if (differentAccidentals && differentDurations && chordsAndRests)
                  break;
            }

      Element* e = selection.element();
      if (e == 0)
            return;

      ChordRest* cr = chordRest(e);
      Note* n = note(e);
      if (!n && cr && cr->isChord())
            n = toChord(cr)->upNote();

      if (cr) {
            setTrack(cr->track());
            setNoteType(n ? n->noteType() : NoteType::NORMAL);
            setBeamMode(cr->beamMode());
            }

      setDrumNote(-1);
      if (n) {
            const Instrument* instr = n->part()->instrument(n->tick());
            if (instr->useDrumset())
                  setDrumNote(n->pitch());
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
            s = toChordRest(e)->segment();
      else
            s = toSegment(e);

      if (s->isSegment()) {
            if (s->measure()->isMMRest()) {
                  Measure* m = s->measure()->mmRestFirst();
                  s = m->findSegment(SegmentType::ChordRest, m->tick());
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
            s = m->findSegment(SegmentType::ChordRest, m->tick());
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
      Segment* s = _segment->next1(SegmentType::ChordRest);
      for (; s; s = s->next1(SegmentType::ChordRest)) {
            if (s->element(_track)) {
                  if (s->element(_track)->isRest() && toRest(s->element(_track))->isGap())
                        m = s->measure();
                  else
                        return s;
                  }
            else if (s->measure() != m)
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

