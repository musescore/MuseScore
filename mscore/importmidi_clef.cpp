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
#include "libmscore/element.h"
#include "importmidi_tie.h"
#include "importmidi_meter.h"
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
      void addPitch(int pitch)
            {
            sumPitch_ += pitch;
            ++count_;
            }
      AveragePitch& operator+=(const AveragePitch &other)
            {
            sumPitch_ += other.sumPitch_;
            count_ += other.count_;
            return *this;
            }
   private:
      int sumPitch_;
      int count_;
      };

class MinMaxPitch
      {
   public:
      MinMaxPitch() : minPitch_(std::numeric_limits<int>::max()), maxPitch_(-1) {}
      MinMaxPitch(int minPitch, int maxPitch) : minPitch_(minPitch), maxPitch_(maxPitch) {}

      int minPitch() const { return minPitch_; }
      int maxPitch() const { return maxPitch_; }
      int empty() const { return minPitch_ == std::numeric_limits<int>::max() || maxPitch_ == -1; }
      void addPitch(int pitch)
            {
            if (pitch < minPitch_)
                  minPitch_ = pitch;
            if (pitch > maxPitch_)
                  maxPitch_ = pitch;
            }
   private:
      int minPitch_;
      int maxPitch_;
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
      AveragePitch averagePitch;
      for (int voice = 0; voice < VOICES; ++voice) {
            ChordRest *cr = static_cast<ChordRest *>(seg->element(strack + voice));
            if (cr && cr->type() == Element::CHORD) {
                  Chord *chord = static_cast<Chord *>(cr);
                  const auto &notes = chord->notes();
                  for (const Note *note: notes)
                        averagePitch.addPitch(note->pitch());
                  }
            }
      return averagePitch;
      }

MinMaxPitch findMinMaxSegPitch(const Segment *seg, int strack)
      {
      MinMaxPitch minMaxPitch;
      for (int voice = 0; voice < VOICES; ++voice) {
            ChordRest *cr = static_cast<ChordRest *>(seg->element(strack + voice));
            if (cr && cr->type() == Element::CHORD) {
                  Chord *chord = static_cast<Chord *>(cr);
                  const auto &notes = chord->notes();
                  for (const Note *note: notes)
                        minMaxPitch.addPitch(note->pitch());
                  }
            }
      return minMaxPitch;
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


// clef index: 0 - treble, 1 - bass

int findPitchPenaltyForClef(int pitch, int clefIndex)
      {
      static const int farPitchPenalty = 10000;
      static const int approxPitchPenalty = 1;
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

std::pair<Element::ElementType, ReducedFraction>
findChordRest(const Segment *seg, int strack)
      {
      Element::ElementType elType = Element::INVALID;
      ReducedFraction newRestLen(0, 1);
      for (int voice = 0; voice < VOICES; ++voice) {
            ChordRest *cr = static_cast<ChordRest *>(seg->element(strack + voice));
            if (!cr)
                  continue;
            if (cr->type() == Element::CHORD) {
                  elType = Element::CHORD;
                  break;
                  }
            else if (cr->type() == Element::REST) {
                  elType = Element::REST;
                  newRestLen = qMax(newRestLen, ReducedFraction(cr->globalDuration()));
                  }
            }
      return {elType, newRestLen};
      }

int findClefChangePenalty(
            int pos,
            int clefIndex,
            const std::vector<std::vector<int>> &trebleBassPath,
            const Segment *segment,
            const Staff *staff)
      {
      if (pos == 0)
            return 0;

      static const int clefChangePenalty = 1000;
      static const int orphanChordPenalty = 2;
      static const int notesBetweenClefs = 5;       // should be >= 2

      int j = pos;
      ReducedFraction totalRestLen(0, 1);
      int penalty = 0;
      const int strack = staff->idx() * VOICES;
      const auto barFraction = ReducedFraction(
                        staff->score()->sigmap()->timesig(segment->tick()).timesig());
      const ReducedFraction beatLen = Meter::beatLength(barFraction);

                  // find backward penalty
      for (const Segment *segPrev = segment->prev1(Segment::SegChordRest); ;
                    segPrev = segPrev->prev1(Segment::SegChordRest)) {
            if (!segPrev) {
                  penalty += clefChangePenalty;
                  break;
                  }
            const auto el = findChordRest(segPrev, strack);
            if (el.first == Element::CHORD) {
                  --j;
                  if (j == pos - notesBetweenClefs)
                        break;
                  if (j == 0 || trebleBassPath[clefIndex][j] != clefIndex) {
                        penalty += clefChangePenalty;
                        break;
                        }
                  totalRestLen = {0, 1};
                  }
            else if (el.first == Element::REST) {
                  totalRestLen += el.second;
                  if (totalRestLen >= beatLen) {
                        if (j != pos)
                              penalty += orphanChordPenalty;
                        break;
                        }
                  }
            }
                  // find forward penalty
      int chordCounter = 0;
      for (const Segment *seg = segment; ; seg = seg->next1(Segment::SegChordRest)) {
            if (!seg) {
                  penalty += clefChangePenalty;
                  break;
                  }
            const auto el = findChordRest(seg, strack);
            if (el.first == Element::CHORD) {
                  ++chordCounter;
                  if (chordCounter == notesBetweenClefs)
                        break;
                  totalRestLen = {0, 1};
                  }
            else if (el.first == Element::REST) {
                  totalRestLen += el.second;
                  if (totalRestLen >= beatLen) {
                        penalty += orphanChordPenalty;
                        break;
                        }
                  }
            }

      return penalty;
      }

ClefType clefFromIndex(int index)
      {
      return (index == 0) ? ClefType::G : ClefType::F;
      }

void makeDynamicProgrammingStep(std::vector<std::vector<int>> &penalties,
                                std::vector<std::vector<int>> &optimalPaths,
                                int pos,
                                MidiTie::TieStateMachine::State tieState,
                                const MinMaxPitch &minMaxPitch,
                                const Segment *seg,
                                const Staff *staff)
      {
      for (int clefIndex = 0; clefIndex != 2; ++clefIndex)
            optimalPaths[clefIndex].resize(pos + 1);

      for (int curClef = 0; curClef != 2; ++curClef) {
            const int significantPitch = (curClef == 0)
                                       ? minMaxPitch.minPitch() : minMaxPitch.maxPitch();
            const int pitchPenalty = findPitchPenaltyForClef(significantPitch, curClef);
            int minPenalty = std::numeric_limits<int>::max();
            int minIndex;
            for (int prevClef = 0; prevClef != 2; ++prevClef) {
                  int penalty = pitchPenalty;
                  if (prevClef != curClef) {
                        if (tieState == MidiTie::TieStateMachine::State::TIED_BACK
                                || tieState == MidiTie::TieStateMachine::State::TIED_BOTH) {
                              continue;   // there is a tie breakage that is incorrect
                              }
                        penalty += findClefChangePenalty(pos, prevClef, optimalPaths, seg, staff);
                        }
                  penalty += (pos > 0) ? penalties[prevClef][(pos + 1) % 2] : 0;
                  if ((prevClef != curClef && penalty < minPenalty)
                              || (prevClef == curClef && penalty <= minPenalty)) {
                        minPenalty = penalty;
                        minIndex = prevClef;
                        }
                  }

            penalties[curClef][pos % 2] = minPenalty;
            if (pos > 0)
                  optimalPaths[curClef][pos] = minIndex;
            }
      }

void createClefs(Staff *staff,
                 const std::vector<std::vector<int>> &optimalPaths,
                 int lastClef,
                 std::vector<Segment *> segments,
                 ClefType *mainClef)
      {
      int currentClef = lastClef;
      for (size_t i = optimalPaths[0].size() - 1; i; --i) {
            const int prevClef = optimalPaths[currentClef][i];
            if (prevClef != currentClef) {
                  createSmallClef(clefFromIndex(currentClef), segments[i], staff);
                  currentClef = prevClef;
                  }
            }
      *mainClef = clefFromIndex(currentClef);
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

                        // find optimal clef changes via dynamic programming
            std::vector<std::vector<int>> penalties(2);         // 0 - treble, 1 - bass
            for (size_t i = 0; i != penalties.size(); ++i)
                  penalties[i].resize(2);                       // 2 = current + prev
            std::vector<std::vector<int>> optimalPaths(2);      // first col is unused
            std::vector<Segment *> segments;

            int pos = 0;
            for (Segment *seg = staff->score()->firstSegment(Segment::SegChordRest); seg;
                              seg = seg->next1(Segment::SegChordRest)) {

                  const auto minMaxPitch = findMinMaxSegPitch(seg, strack);
                  if (minMaxPitch.empty())                      // no chords
                        continue;
                  tieTracker.addSeg(seg, strack);
                  segments.push_back(seg);

                  makeDynamicProgrammingStep(penalties, optimalPaths, pos,
                                             tieTracker.state(), minMaxPitch, seg, staff);
                  ++pos;
                  }

            if (!optimalPaths[0].empty()) {
                  int lastClef = (penalties[1][(pos - 1) % 2] < penalties[0][(pos - 1) % 2])
                              ? 1 : 0;
                        // get the optimal clef changes found via dynamic programming
                  createClefs(staff, optimalPaths, lastClef, segments, &mainClef);
                  }
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

