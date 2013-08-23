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
      return averagePitch < midPitch() ? CLEF_F : CLEF_G;
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

void createClefs(Staff *staff, int indexOfOperation)
      {
      ClefType currentClef = staff->initialClef()._concertClef;
      createClef(currentClef, staff, 0);

      const auto trackOpers = preferences.midiImportOperations.trackOperations(indexOfOperation);
      if (!trackOpers.changeClef)
            return;

      const int highPitch = 64;          // all notes upper - in treble clef
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

                  if (currentClef == CLEF_G && avgPitch.pitch() < lowPitch) {
                        currentClef = CLEF_F;
                        createSmallClef(currentClef, seg, staff);
                        }
                  else if (currentClef == CLEF_F && avgPitch.pitch() > highPitch) {
                        currentClef = CLEF_G;
                        createSmallClef(currentClef, seg, staff);
                        }
                  else if (currentClef == CLEF_G && avgPitch.pitch() >= lowPitch
                           && avgPitch.pitch() < midPitch) {
                        if (counter < counterLimit) {
                              if (counter == 0)
                                    prevSeg = seg;
                              ++counter;
                              }
                        else {
                              currentClef = CLEF_F;
                              createSmallClef(currentClef, prevSeg, staff);
                              }
                        }
                  else if (currentClef == CLEF_F && avgPitch.pitch() <= highPitch
                           && avgPitch.pitch() >= midPitch) {
                        if (counter < counterLimit){
                              if (counter == 0)
                                    prevSeg = seg;
                              ++counter;
                              }
                        else {
                              currentClef = CLEF_G;
                              createSmallClef(currentClef, prevSeg, staff);
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

