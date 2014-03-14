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
#include "importmidi_tie.h"
#include "preferences.h"

#include <set>


namespace Ms {

extern Preferences preferences;

namespace MidiClef {


class AveragePitch
      {
   public:
      AveragePitch() : sumPitch_(0), count_(0) {}
      AveragePitch(int sumPitch, int count) : sumPitch_(sumPitch), count_(count) {}

      int pitch() const { return qRound(sumPitch_ * 1.0 / count_); }
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


int clefMidPitch()
      {
      static const int midPitch = 60;
      return midPitch;
      }

ClefType clefTypeFromAveragePitch(int averagePitch)
      {
      return averagePitch < clefMidPitch() ? ClefType::F : ClefType::G;
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


#ifdef QT_DEBUG

bool doesClefBreakTie(const Staff *staff)
      {
      const int strack = staff->idx() * VOICES;

      for (int voice = 0; voice < VOICES; ++voice) {
            bool currentTie = false;
            for (Segment *seg = staff->score()->firstSegment(); seg; seg = seg->next1()) {
                  if (seg->segmentType() == Segment::SegChordRest) {
                        if (MidiTie::isTiedBack(seg, strack, voice))
                              currentTie = false;
                        if (MidiTie::isTiedFor(seg, strack, voice))
                              currentTie = true;
                        }
                  else if (seg->segmentType() == Segment::SegClef && seg->element(strack)) {
                        if (currentTie) {
                              qDebug() << "Clef breaks tie; measure number (from 1):"
                                       << seg->measure()->no() + 1
                                       << ", staff index (from 0):" << staff->idx();
                              return true;
                              }
                        }
                  }
            }
      return false;
      }

#endif

// clef: 0 - treble, 1 - bass

size_t findPitchPenaltyForClef(int pitch, int clefIndex)
      {
      static const size_t farPitchPenalty = 10000;
      static const size_t approxPitchPenalty = 1;
      static const int dx = 5;

      static const int midPitch = clefMidPitch();    // all notes equal or upper - better in G clef
      static const int highPitch = midPitch + dx;    // all notes equal or upper - in G clef
      static const int lowPitch = midPitch - dx;     // all notes lower - in F clef

      switch (clefIndex) {
      case 0:
            if (pitch < lowPitch)
                  return farPitchPenalty;
            else if (pitch < midPitch)
                  return approxPitchPenalty;
            break;
      case 1:
            if (pitch >= highPitch)
                  return farPitchPenalty;
            else if (pitch >= midPitch)
                  return approxPitchPenalty;
            break;
      default:
            Q_ASSERT_X(false, "MidiClef::pitchPenalty", "Unknown clef type");
            break;
            }
      return 0;
      }

size_t findClefPenalty(int pos,
                       int clefIndex,
                       const std::vector<std::vector<int>> &trebleBassPath)
      {
      static const size_t clefChangePenalty = 1000;
      static const int notesBetweenClefs = 5;       // should be >= 2

      if (pos == 0)
            return 0;
      if (pos - 1 == 0)
            return clefChangePenalty;

      for (int j = pos - 1; j != pos - notesBetweenClefs; --j) {
            if (j == 0 || trebleBassPath[clefIndex][j] != clefIndex)
                  return clefChangePenalty;
            }
      return 0;
      }

ClefType clefFromIndex(int index)
      {
      return (index == 0) ? ClefType::G : ClefType::F;
      }

size_t findTiePenalty(MidiTie::TieStateMachine::State tieState)
      {
      static const size_t tieBreakagePenalty = 10000000;
      return (tieState == MidiTie::TieStateMachine::State::TIED_BACK
              || tieState == MidiTie::TieStateMachine::State::TIED_BOTH)
             ? tieBreakagePenalty : 0;
      }

void makeDynamicProgrammingStep(std::vector<std::vector<size_t>> &penalties,
                                std::vector<std::vector<int>> &optimalPaths,
                                int pos,
                                MidiTie::TieStateMachine::State tieState,
                                int averagePitch)
      {
      for (int clefIndex = 0; clefIndex != 2; ++clefIndex) {
            penalties[clefIndex].resize(pos + 1);
            optimalPaths[clefIndex].resize(pos + 1);
            }
      const size_t tiePenalty = findTiePenalty(tieState);

      for (int clefIndex = 0; clefIndex != 2; ++clefIndex) {
            const size_t pitchPenalty = findPitchPenaltyForClef(averagePitch, clefIndex);

            const size_t prevSameClefPenalty = (pos == 0)
                    ? 0 : penalties[clefIndex][pos - 1];
            const size_t sumPenaltySameClef = pitchPenalty + prevSameClefPenalty;

            const size_t prevDiffClefPenalty = (pos == 0)
                    ? 0 : penalties[1 - clefIndex][pos - 1];
            const size_t clefPenalty = findClefPenalty(pos, 1 - clefIndex, optimalPaths);
            const size_t sumPenaltyDiffClef
                    = tiePenalty + pitchPenalty + prevDiffClefPenalty + clefPenalty;

            if (sumPenaltySameClef <= sumPenaltyDiffClef) {
                  penalties[clefIndex][pos] = sumPenaltySameClef;
                  if (pos > 0)
                        optimalPaths[clefIndex][pos] = clefIndex;
                  }
            else {
                  penalties[clefIndex][pos] = sumPenaltyDiffClef;
                  if (pos > 0)
                        optimalPaths[clefIndex][pos] = 1 - clefIndex;
                  }
            }
      }

void createClefs(Staff *staff,
                 const std::vector<std::vector<size_t>> &penalties,
                 const std::vector<std::vector<int>> &optimalPaths,
                 std::vector<Segment *> segments,
                 ClefType *mainClef)
      {
      const size_t chordCount = penalties[0].size();
      if (chordCount != 0) {
                     // create clefs
            int currentClef = 0;
            if (penalties[1][chordCount - 1] < penalties[0][chordCount - 1])
                  currentClef = 1;
            for (size_t i = chordCount - 1; i; --i) {
                  const int prevClef = optimalPaths[currentClef][i];
                  if (prevClef != currentClef) {
                        createSmallClef(clefFromIndex(currentClef), segments[i], staff);
                        currentClef = prevClef;
                        }
                  if (i == 1)
                        *mainClef = clefFromIndex(prevClef);
                  }
            }
      }

void createClefs(Staff *staff, int indexOfOperation, bool isDrumTrack)
      {
      if (isDrumTrack) {
            createClef(ClefType::PERC, staff, 0);
            return;
            }

      ClefType mainClef = staff->clefTypeList(0)._concertClef;
      const int strack = staff->idx() * VOICES;
      const auto trackOpers = preferences.midiImportOperations.trackOperations(indexOfOperation);

      if (trackOpers.changeClef) {
            MidiTie::TieStateMachine tieTracker;
            AveragePitch allAveragePitch;

                        // find optimal clef changes by dynamic programming
            std::vector<std::vector<size_t>> penalties(2);          // 0 - treble, 1 - bass
            std::vector<std::vector<int>> optimalPaths(2);          // first col is unused
            std::vector<Segment *> segments;

            int pos = 0;
            for (Segment *seg = staff->score()->firstSegment(Segment::SegChordRest); seg;
                              seg = seg->next1(Segment::SegChordRest)) {

                  const auto averagePitch = findAverageSegPitch(seg, strack);
                  if (averagePitch.count() == 0)                    // no chords
                        continue;
                  tieTracker.addSeg(seg, strack);
                  segments.push_back(seg);

                  makeDynamicProgrammingStep(penalties, optimalPaths, pos,
                                             tieTracker.state(), averagePitch.pitch());
                  allAveragePitch += averagePitch;
                  ++pos;
                  }
                        // get the optimal clef changes found by dynamic programming
            createClefs(staff, penalties, optimalPaths, segments, &mainClef);
            }
      else {
            AveragePitch allAveragePitch;
            for (Segment *seg = staff->score()->firstSegment(Segment::SegChordRest); seg;
                          seg = seg->next1(Segment::SegChordRest)) {
                  allAveragePitch += findAverageSegPitch(seg, strack);
                  }
            mainClef = MidiClef::clefTypeFromAveragePitch(allAveragePitch.pitch());
            }

      staff->setClef(0, mainClef);      // set main clef
      createClef(mainClef, staff, 0);

      Q_ASSERT_X(!doesClefBreakTie(staff), "MidiClef::createClefs", "Clef breaks the tie");
      }

} // namespace MidiClef
} // namespace Ms

