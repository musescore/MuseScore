//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "importmidi_clef.h"
#include "libmscore/score.h"
#include "libmscore/staff.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/clef.h"
#include "libmscore/chordrest.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/slur.h"
#include "preferences.h"

#include <set>


namespace Ms {

extern Preferences preferences;

namespace MidiClef {


int midPitch()
      {
      static const int clefMidPitch = 60;
      return clefMidPitch;
      }

bool isTied(const Segment *seg, int strack, int voice,
            Ms::Tie*(Note::*tieFunc)() const)
      {
      ChordRest *cr = static_cast<ChordRest *>(seg->element(strack + voice));
      if (cr && cr->type() == Element::CHORD) {
            Chord *chord = static_cast<Chord *>(cr);
            const auto &notes = chord->notes();
            for (const Note *note: notes) {
                  if ((note->*tieFunc)())
                        return true;
                  }
            }
      return false;
      }

bool isTiedFor(const Segment *seg, int strack, int voice)
      {
      return isTied(seg, strack, voice, &Note::tieFor);
      }

bool isTiedBack(const Segment *seg, int strack, int voice)
      {
      return isTied(seg, strack, voice, &Note::tieBack);
      }

class AveragePitch
      {
   public:
      AveragePitch() : sumPitch_(0), count_(0) {}
      AveragePitch(int sumPitch, int count) : sumPitch_(sumPitch), count_(count) {}

      int pitch() const { return sumPitch_ / count_; }
      int sumPitch() const { return sumPitch_; }
      int count() const { return count_; }
      void addPitch(int pitch)
            {
            sumPitch_ += pitch;
            ++count_;
            }
      void reset()
            {
            sumPitch_ = 0;
            count_ = 0;
            }
      AveragePitch& operator+=(const AveragePitch &other)
            {
            sumPitch_ += other.sumPitch();
            count_ += other.count();
            return *this;
            }
   private:
      int sumPitch_;
      int count_;
      };

class TieStateMachine
      {
   public:
      enum class State
            {
            UNTIED, TIED_FOR, TIED_BOTH, TIED_BACK
            };

      void addSeg(const Segment *seg, int strack)
            {
            bool isChord = false;
            for (int voice = 0; voice < VOICES; ++voice) {
                  ChordRest *cr = static_cast<ChordRest *>(seg->element(strack + voice));
                  if (!cr || cr->type() != Element::CHORD)
                        continue;
                  if (!isChord)
                        isChord = true;

                  bool tiedFor = isTiedFor(seg, strack, voice);
                  bool tiedBack = isTiedBack(seg, strack, voice);

                  if (tiedFor && !tiedBack)
                        tiedVoices.insert(voice);
                  else if (!tiedFor && tiedBack)
                        tiedVoices.erase(voice);
                  }
            if (!isChord)
                  return;

            if (tiedVoices.empty() && (state_ == State::TIED_FOR
                                       || state_ == State::TIED_BOTH)) {
                  state_ = State::TIED_BACK;
                  }
            else if (tiedVoices.empty() && state_ == State::TIED_BACK) {
                  state_ = State::UNTIED;
                  }
            else if (!tiedVoices.empty() && (state_ == State::TIED_BACK
                                             || state_ == State::UNTIED)) {
                  state_ = State::TIED_FOR;
                  }
            else if (!tiedVoices.empty() && state_ == State::TIED_FOR) {
                  state_ = State::TIED_BOTH;
                  }
            }
      State state() const { return state_; }

   private:
      std::set<int> tiedVoices;
      State state_ = State::UNTIED;
      };


ClefType clefTypeFromAveragePitch(int averagePitch)
      {
      return averagePitch < midPitch() ? ClefType::F : ClefType::G;
      }

void createClef(ClefType clefType, Staff* staff, int tick, bool isSmall = false)
      {
      Clef* clef = new Clef(staff->score());
      clef->setClefType(clefType);
      const int track = staff->idx() * VOICES;
      clef->setTrack(track);
      clef->setGenerated(false);
      clef->setMag(staff->mag());
      clef->setSmall(isSmall);
      Measure* m = staff->score()->tick2measure(tick);
      Segment* seg = m->getSegment(clef, tick);
      seg->add(clef);
      }

void createSmallClef(ClefType clefType, Segment *chordRestSeg, Staff *staff)
      {
      const int strack = staff->idx() * VOICES;
      const int tick = chordRestSeg->tick();
      Segment *clefSeg = chordRestSeg->measure()->findSegment(Segment::SegClef, tick);
                  // remove clef if it is not the staff clef
      if (clefSeg && clefSeg != chordRestSeg->score()->firstSegment(Segment::SegClef)) {
            Clef *c = static_cast<Clef *>(clefSeg->element(strack));   // voice == 0 for clefs
            if (c) {
                  clefSeg->remove(c);
                  delete c;
                  if (clefSeg->isEmpty()) {
                        chordRestSeg->measure()->remove(clefSeg);
                        delete clefSeg;
                        }
                  return;
                  }
            }
      createClef(clefType, staff, tick, true);
      }

AveragePitch findAverageSegPitch(const Segment *seg, int strack)
      {
      AveragePitch avgPitch;
      for (int voice = 0; voice < VOICES; ++voice) {
            ChordRest *cr = static_cast<ChordRest *>(seg->element(strack + voice));
            if (cr && cr->type() == Element::CHORD) {
                  Chord *chord = static_cast<Chord *>(cr);
                  const auto &notes = chord->notes();
                  for (const Note *note: notes)
                        avgPitch.addPitch(note->pitch());
                  }
            }
      return avgPitch;
      }

Segment* enlargeSegToPrev(Segment *s, int strack, int counterLimit, int lPitch, int hPitch)
      {
      int count = 0;
      auto newSeg = s;
      for (Segment *segPrev = s->prev1(Segment::SegChordRest); segPrev;
                              segPrev = segPrev->prev1(Segment::SegChordRest)) {
            auto pitch = findAverageSegPitch(segPrev, strack);
            if (pitch.count() > 0) {
                  if (pitch.pitch() >= lPitch && pitch.pitch() < hPitch)
                        newSeg = segPrev;
                  else
                        break;
                  }
            else {                  // it's a rest - should be at the end
                  s = newSeg;
                  break;
                  }
            ++count;
            if (count == counterLimit)
                  break;
            }
      return s;
      }

void createClefs(Staff *staff, int indexOfOperation, bool isDrumTrack)
      {
      ClefType currentClef = staff->clefTypeList(0)._concertClef;
      createClef(currentClef, staff, 0);

      const auto trackOpers = preferences.midiImportOperations.trackOperations(indexOfOperation);
      if (!trackOpers.changeClef || isDrumTrack)
            return;

      const int highPitch = 65;          // all notes equal or upper - in treble clef
      const int midPitch = 60;
      const int lowPitch = 55;           // all notes lower - in bass clef
      const int counterLimit = 3;
      int counter = 0;
      Segment *prevSeg = nullptr;
      Segment *tiedSeg = nullptr;
      const int strack = staff->idx() * VOICES;
      AveragePitch avgGroupPitch;
      TieStateMachine tieTracker;

      for (Segment *seg = staff->score()->firstSegment(Segment::SegChordRest); seg;
                        seg = seg->next1(Segment::SegChordRest)) {
            const auto avgPitch = findAverageSegPitch(seg, strack);
            if (avgPitch.count() == 0)    // no chords
                  continue;
            tieTracker.addSeg(seg, strack);
            auto tieState = tieTracker.state();

            if (tieState != TieStateMachine::State::UNTIED)
                  avgGroupPitch += avgPitch;
            if (tieState == TieStateMachine::State::TIED_FOR)
                  tiedSeg = seg;
            else if (tieState == TieStateMachine::State::TIED_BACK) {
                  ClefType clef = clefTypeFromAveragePitch(avgGroupPitch.pitch());
                  if (clef != currentClef) {
                        currentClef = clef;
                        if (tiedSeg)
                              createSmallClef(currentClef, tiedSeg, staff);
                        else {
                              qDebug("createClefs: empty tied segment, tick = %d, that should not occur",
                                     seg->tick());
                              }
                        }
                  avgGroupPitch.reset();
                  tiedSeg = nullptr;
                  }

            int oldCounter = counter;
            if (tieState != TieStateMachine::State::TIED_BOTH
                        && tieState != TieStateMachine::State::TIED_BACK) {

                  if (avgPitch.pitch() < lowPitch) {
                        if (currentClef == ClefType::G) {
                              Segment *s = (counter > 0) ? prevSeg : seg;
                              currentClef = ClefType::F;
                              s = enlargeSegToPrev(s, strack, counterLimit, midPitch, highPitch);
                              createSmallClef(currentClef, s, staff);
                              }
                        }
                  else if (avgPitch.pitch() >= lowPitch && avgPitch.pitch() < midPitch) {
                        if (currentClef == ClefType::G) {
                              if (counter < counterLimit) {
                                    if (counter == 0)
                                          prevSeg = seg;
                                    ++counter;
                                    }
                              else {
                                    currentClef = ClefType::F;
                                    auto s = enlargeSegToPrev(prevSeg, strack, counterLimit, midPitch, highPitch);
                                    createSmallClef(currentClef, s, staff);
                                    }
                              }
                        }
                  else if (avgPitch.pitch() >= midPitch && avgPitch.pitch() < highPitch) {
                        if (currentClef == ClefType::F) {
                              if (counter < counterLimit){
                                    if (counter == 0)
                                          prevSeg = seg;
                                    ++counter;
                                    }
                              else {
                                    currentClef = ClefType::G;
                                    auto s = enlargeSegToPrev(prevSeg, strack, counterLimit, lowPitch, midPitch);
                                    createSmallClef(currentClef, s, staff);
                                    }
                              }
                        }
                  else if (avgPitch.pitch() >= highPitch) {
                        if (currentClef == ClefType::F) {
                              Segment *s = (counter > 0) ? prevSeg : seg;
                              currentClef = ClefType::G;
                              s = enlargeSegToPrev(s, strack, counterLimit, lowPitch, midPitch);
                              createSmallClef(currentClef, s, staff);
                              }
                        }
                  }
            if (counter > 0 && counter == oldCounter) {
                  counter = 0;
                  prevSeg = nullptr;
                  }
            }
      }

} // namespace MidiClef
} // namespace Ms

