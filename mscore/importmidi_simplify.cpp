#include "importmidi_simplify.h"
#include "importmidi_chord.h"
#include "importmidi_inner.h"
#include "importmidi_meter.h"
#include "importmidi_tuplet.h"
#include "importmidi_quant.h"
#include "preferences.h"
#include "libmscore/sig.h"
#include "libmscore/durationtype.h"
#include "importmidi_tuplet_voice.h"


namespace Ms {
namespace Simplify {

ReducedFraction findBarStart(const ReducedFraction &time, const TimeSigMap *sigmap)
      {
      int barIndex, beat, tick;
      sigmap->tickValues(time.ticks(), &barIndex, &beat, &tick);
      return ReducedFraction::fromTicks(sigmap->bar2tick(barIndex, 0));
      }

double durationCount(const QList<std::pair<ReducedFraction, TDuration> > &durations)
      {
      double count = durations.size();
      for (const auto &d: durations) {
            if (d.second.dots())
                  count += 0.5;
            }
      return count;
      }

bool hasComplexBeamedDurations(const QList<std::pair<ReducedFraction, TDuration> > &list)
      {
      for (const auto &d: list) {
            if (d.second == TDuration::DurationType::V_16TH
                        || d.second == TDuration::DurationType::V_32ND
                        || d.second == TDuration::DurationType::V_64TH
                        || d.second == TDuration::DurationType::V_128TH) {
                  return true;
                  }
            }
      return false;
      }


#ifdef QT_DEBUG

bool areDurationsEqual(
            const QList<std::pair<ReducedFraction, TDuration> > &durations,
            const ReducedFraction &desiredLen)
      {
      ReducedFraction sum(0, 1);
      for (const auto &d: durations)
            sum += ReducedFraction(d.second.fraction()) / d.first;

      return desiredLen == desiredLen;
      }

bool areNotesSortedByPitchInAscOrder(const QList<MidiNote>& notes)
      {
      for (int i = 0; i != notes.size() - 1; ++i) {
            if (notes[i].pitch > notes[i + 1].pitch)
                  return false;
            }
      return true;
      }

bool areNotesSortedByOffTimeInAscOrder(
            const QList<MidiNote>& notes,
            const std::vector<int> &groupOfIndexes)
      {
      for (int i = 0; i != (int)groupOfIndexes.size() - 1; ++i) {
            if (notes[groupOfIndexes[i]].offTime > notes[groupOfIndexes[i + 1]].offTime)
                  return false;
            }
      return true;
      }

#endif


void lengthenNote(
            MidiNote &note,
            int voice,
            const ReducedFraction &noteOnTime,
            const ReducedFraction &durationStart,
            const ReducedFraction &endTime,
            const ReducedFraction &barStart,
            const ReducedFraction &barFraction,
            const std::multimap<ReducedFraction, MidiTuplet::TupletData> &tuplets)
      {
      if (endTime <= note.offTime)
            return;

      const auto &opers = preferences.midiImportOperations.currentTrackOperations();
      const bool useDots = opers.useDots;
      const auto tupletsForDuration = MidiTuplet::findTupletsInBarForDuration(
                              voice, barStart, note.offTime, endTime - note.offTime, tuplets);

      Q_ASSERT_X(note.quant != ReducedFraction(-1, 1),
                 "Simplify::lengthenNote", "Note quant value was not set");

      const auto origNoteDurations = Meter::toDurationList(
                              durationStart - barStart, note.offTime - barStart, barFraction,
                              tupletsForDuration, Meter::DurationType::NOTE, useDots, false);

      const auto origRestDurations = Meter::toDurationList(
                              note.offTime - barStart, endTime - barStart, barFraction,
                              tupletsForDuration, Meter::DurationType::REST, useDots, false);

      Q_ASSERT_X(areDurationsEqual(origNoteDurations, note.offTime - durationStart),
                 "Simplify::lengthenNote", "Too short note durations remaining");
      Q_ASSERT_X(areDurationsEqual(origRestDurations, endTime - note.offTime),
                 "Simplify::lengthenNote", "Too short rest durations remaining");

                  // double - because can be + 0.5 for dots
      double minNoteDurationCount = durationCount(origNoteDurations);
      double minRestDurationCount = durationCount(origRestDurations);

      ReducedFraction bestOffTime(-1, 1);

      for (ReducedFraction offTime = note.offTime + note.quant;
                           offTime <= endTime; offTime += note.quant) {
            double noteDurationCount = 0;
            double restDurationCount = 0;

            const auto noteDurations = Meter::toDurationList(
                              durationStart - barStart, offTime - barStart, barFraction,
                              tupletsForDuration, Meter::DurationType::NOTE, useDots, false);

            Q_ASSERT_X(areDurationsEqual(noteDurations, offTime - durationStart),
                       "Simplify::lengthenNote", "Too short note durations remaining");

            noteDurationCount += durationCount(noteDurations);

            if (offTime < endTime) {
                  const auto restDurations = Meter::toDurationList(
                              offTime - barStart, endTime - barStart, barFraction,
                              tupletsForDuration, Meter::DurationType::REST, useDots, false);

                  Q_ASSERT_X(areDurationsEqual(restDurations, endTime - offTime),
                             "Simplify::lengthenNote", "Too short rest durations remaining");

                  restDurationCount += durationCount(restDurations);
                  }

            if (noteDurationCount + restDurationCount
                              < minNoteDurationCount + minRestDurationCount) {
                  if (opers.quantize.humanPerformance || noteDurationCount == 1) {
                        minNoteDurationCount = noteDurationCount;
                        minRestDurationCount = restDurationCount;
                        bestOffTime = offTime;
                        }
                  }
            }

      if (bestOffTime == ReducedFraction(-1, 1))
            return;

      // check for staccato:
      //    don't apply staccato if note is tied
      //    (case noteOnTime != durationStart - another bar, for example)

      bool hasLossOfAccuracy = false;
      const double addedPart = ((bestOffTime - note.offTime)
                                / (bestOffTime - durationStart)).toDouble();
      const double STACCATO_TOL = 0.3;

      if (addedPart >= STACCATO_TOL) {
            if (noteOnTime == durationStart && minNoteDurationCount == 1)
                  note.staccato = true;
            else
                  hasLossOfAccuracy = true;
            }

      // if the difference is only in one note/rest and there is some loss of accuracy -
      // discard change because it silently reduces duration accuracy
      // without significant improvement of readability

      if (!opers.quantize.humanPerformance
                  && (origNoteDurations.size() + origRestDurations.size())
                       - (minNoteDurationCount + minRestDurationCount) <= 1
                  && !hasComplexBeamedDurations(origNoteDurations)
                  && !hasComplexBeamedDurations(origRestDurations)
                  && hasLossOfAccuracy) {
            return;
            }

      note.offTime = bestOffTime;
      }

void minimizeNumberOfRests(
            std::multimap<ReducedFraction, MidiChord> &chords,
            const TimeSigMap *sigmap,
            const std::multimap<ReducedFraction, MidiTuplet::TupletData> &tuplets)
      {
      for (auto it = chords.begin(); it != chords.end(); ++it) {
            for (MidiNote &note: it->second.notes) {
                  const auto barStart = findBarStart(note.offTime, sigmap);
                  const auto barFraction = ReducedFraction(
                                                sigmap->timesig(barStart.ticks()).timesig());
                  auto durationStart = (it->first > barStart) ? it->first : barStart;
                  if (it->second.isInTuplet) {
                        const auto &tuplet = it->second.tuplet->second;
                        if (note.offTime >= tuplet.onTime + tuplet.len)
                              durationStart = tuplet.onTime + tuplet.len;
                        }
                  auto endTime = (barStart == note.offTime)
                                    ? barStart : barStart + barFraction;
                  if (note.isInTuplet) {
                        const auto &tuplet = note.tuplet->second;
                        if (note.offTime == tuplet.onTime + tuplet.len)
                              continue;
                        endTime = barStart + Quantize::quantizeToLarge(
                                    note.offTime - barStart, tuplet.len / tuplet.tupletNumber);
                        }

                  const auto beatLen = Meter::beatLength(barFraction);
                  const auto beatTime = barStart + Quantize::quantizeToLarge(
                                                      note.offTime - barStart, beatLen);
                  if (endTime > beatTime)
                        endTime = beatTime;

                  auto next = std::next(it);
                  while (next != chords.end()
                              && (next->second.voice != it->second.voice
                                  || next->first < note.offTime)) {
                        ++next;
                        }
                  if (next != chords.end()) {
                        if (next->first < endTime)
                              endTime = next->first;
                        if (next->second.isInTuplet
                                    && !note.isInTuplet && next->second.tuplet == note.tuplet) {
                              const auto &tuplet = next->second.tuplet->second;
                              if (tuplet.onTime < endTime)
                                    endTime = tuplet.onTime;
                              }
                        }

                  lengthenNote(note, it->second.voice, it->first, durationStart, endTime,
                               barStart, barFraction, tuplets);
                  }
            }
      }

// -------------------- voice separation --------------------------------------

bool allNotesHaveEqualLength(const QList<MidiNote> &notes)
      {
      const auto &offTime = notes[0].offTime;
      for (int i = 1; i != notes.size(); ++i) {
            if (notes[i].offTime != offTime)
                  return false;
            }
      return true;
      }

int findDurationCountInGroup(
            const ReducedFraction &chordOnTime,
            const QList<MidiNote> &notes,
            int voice,
            const std::vector<int> &groupOfIndexes,
            const TimeSigMap *sigmap,
            const std::multimap<ReducedFraction, MidiTuplet::TupletData> &tuplets)
      {
      Q_ASSERT_X(areNotesSortedByOffTimeInAscOrder(notes, groupOfIndexes),
                 "Simplify::findDurationCountInGroup",
                 "Notes are not sorted by off time in ascending order");

      const auto &opers = preferences.midiImportOperations.currentTrackOperations();
      const bool useDots = opers.useDots;

      int count = 0;
      auto onTime = chordOnTime;
      auto onTimeBarStart = findBarStart(onTime, sigmap);
      auto onTimeBarFraction = ReducedFraction(
                        sigmap->timesig(onTimeBarStart.ticks()).timesig());

      for (int i: groupOfIndexes) {
            const auto &offTime = notes[i].offTime;
            if (offTime == onTime)
                  continue;
            const auto offTimeBarStart = findBarStart(offTime, sigmap);

            if (offTimeBarStart != onTimeBarStart) {
                  const auto offTimeBarFraction = ReducedFraction(
                              sigmap->timesig(offTimeBarStart.ticks()).timesig());

                  const auto tupletsForDuration = MidiTuplet::findTupletsInBarForDuration(
                              voice, onTimeBarStart, onTime, offTimeBarStart - onTime, tuplets);

                              // additional durations on measure boundary
                  const auto durations = Meter::toDurationList(
                              onTime - onTimeBarStart, offTimeBarStart - onTimeBarStart,
                              offTimeBarFraction, tupletsForDuration, Meter::DurationType::NOTE,
                              useDots, false);

                  count += durationCount(durations);

                  onTime = offTimeBarStart;
                  onTimeBarStart = offTimeBarStart;
                  onTimeBarFraction = offTimeBarFraction;
                  }

            const auto tupletsForDuration = MidiTuplet::findTupletsInBarForDuration(
                              voice, onTimeBarStart, onTime, offTime - onTime, tuplets);

            const auto durations = Meter::toDurationList(
                              onTime - onTimeBarStart, offTime - onTimeBarStart, onTimeBarFraction,
                              tupletsForDuration, Meter::DurationType::NOTE, useDots, false);

            count += durationCount(durations);

            onTime = offTime;
            }
      return count;
      }

// count of resulting durations in music notation

int findDurationCount(
            const QList<MidiNote> &notes,
            int voice,
            int splitPoint,
            const ReducedFraction &chordOnTime,
            const TimeSigMap *sigmap,
            const std::multimap<ReducedFraction, MidiTuplet::TupletData> &tuplets)
      {
      std::vector<int> lowGroup;
      std::vector<int> highGroup;

      for (int i = 0; i != splitPoint; ++i)
            lowGroup.push_back(i);
      for (int i = splitPoint; i != notes.size(); ++i)
            highGroup.push_back(i);

      std::sort(lowGroup.begin(), lowGroup.end(),
                [&](int i1, int i2) { return notes[i1].offTime < notes[i2].offTime; });
      std::sort(highGroup.begin(), highGroup.end(),
                [&](int i1, int i2) { return notes[i1].offTime < notes[i2].offTime; });

      return findDurationCountInGroup(chordOnTime, notes, voice, lowGroup, sigmap, tuplets)
             + findDurationCountInGroup(chordOnTime, notes, voice, highGroup, sigmap, tuplets);
      }

int findOptimalSplitPoint(
            const QList<MidiNote>& notes,
            int voice,
            const ReducedFraction &chordOnTime,
            const TimeSigMap *sigmap,
            const std::multimap<ReducedFraction, MidiTuplet::TupletData> &tuplets)
      {
      Q_ASSERT_X(!notes.isEmpty(),
                 "Simplify::findOptimalSplitPoint", "Notes are empty");
      Q_ASSERT_X(areNotesSortedByPitchInAscOrder(notes),
                 "Simplify::findOptimalSplitPoint",
                 "Notes are not sorted by pitch in ascending order");

      if (allNotesHaveEqualLength(notes))
            return -1;

      int minNoteCount = std::numeric_limits<int>::max();
      int minSplit;

      for (int splitPoint = 1; splitPoint != notes.size(); ++splitPoint) {
            int noteCount = findDurationCount(notes, voice, splitPoint,
                                              chordOnTime, sigmap, tuplets);
            if (noteCount < minNoteCount) {
                  minNoteCount = noteCount;
                  minSplit = splitPoint;
                  }
            }

      return minSplit;
      }

// which part of chord - low notes or high notes - should be shifted to another voice

enum class ShiftedPitchGroup {
      LOW,
      HIGH
      };

struct VoiceSplit {
      ShiftedPitchGroup group;
      int voice = -1;
      };

// it's an optimization function: we can don't check chords
// with (on time + max chord len) < given time moment
// because chord cannot be longer than found max length
// result: <voice, max chord length>

std::map<int, ReducedFraction> findMaxChordLengths(
            const std::multimap<ReducedFraction, MidiChord> &chords)
      {
      std::map<int, ReducedFraction> maxLengths;

      for (const auto &chord: chords) {
            const auto offTime = MChord::maxNoteOffTime(chord.second.notes);
            if (offTime - chord.first > maxLengths[chord.second.voice])
                  maxLengths[chord.second.voice] = offTime - chord.first;
            }
      return maxLengths;
      }

bool hasIntersectionWithTuplets(
            int voice,
            const ReducedFraction &onTime,
            const ReducedFraction &offTime,
            const std::multimap<ReducedFraction, MidiTuplet::TupletData> &tuplets)
      {
      auto it = MidiTuplet::findTupletForTimeRange(voice, onTime, offTime - onTime, tuplets);
      return it != tuplets.end();
      }

bool hasIntersectionWithChords(
            int voice,
            const ReducedFraction &onTime,
            const ReducedFraction &offTime,
            const std::multimap<ReducedFraction, MidiChord> &chords,
            std::map<int, ReducedFraction> &maxChordLengths)
      {
      auto it = chords.lower_bound(offTime);

      if (it == chords.begin())
            return true;
      if (it == chords.end())
            return false;

      bool hasIntersection = false;
      while (it->first + maxChordLengths[voice] > onTime) {
            const MidiChord &chord = it->second;
            const auto chordInterval = std::make_pair(
                              it->first, MChord::maxNoteOffTime(chord.notes));
            const auto durationInterval = std::make_pair(onTime, offTime);
            if (chord.voice == voice && MidiTuplet::haveIntersection(
                        chordInterval, durationInterval)) {
                  hasIntersection = true;
                  break;
                  }
            if (it == chords.begin())
                  break;
            --it;
            }

      return hasIntersection;
      }

void addGroupSplits(
            std::vector<VoiceSplit> &splits,
            std::map<int, ReducedFraction> &maxChordLengths,
            const std::multimap<ReducedFraction, MidiChord> &chords,
            const std::multimap<ReducedFraction, MidiTuplet::TupletData> &tuplets,
            const ReducedFraction &onTime,
            const ReducedFraction &groupOffTime,
            int origVoice,
            ShiftedPitchGroup groupType)
      {
      const int voiceLimit = MidiTuplet::voiceLimit();
      for (int voice = 0; voice != voiceLimit; ++voice) {
            if (voice == origVoice)
                  continue;
            if (hasIntersectionWithTuplets(voice, onTime, groupOffTime, tuplets))
                  continue;
            if (hasIntersectionWithChords(voice, onTime, groupOffTime, chords, maxChordLengths))
                  continue;

            VoiceSplit split;
            split.group = groupType;
            split.voice = voice;
            splits.push_back(split);
            }
      }

std::vector<VoiceSplit> findPossibleVoiceSplits(
            int origVoice,
            const QList<MidiNote> &notes,
            const ReducedFraction &onTime,
            int splitPoint,
            const std::multimap<ReducedFraction, MidiChord> &chords,
            const std::multimap<ReducedFraction, MidiTuplet::TupletData> &tuplets)
      {
      std::vector<VoiceSplit> splits;

      ReducedFraction lowGroupOffTime(0, 1);
      ReducedFraction highGroupOffTime(0, 1);

      for (int i = 0; i != splitPoint; ++i) {
            if (notes[i].offTime > lowGroupOffTime)
                  lowGroupOffTime = notes[i].offTime;
            }
      for (int i = splitPoint; i != notes.size(); ++i) {
            if (notes[i].offTime > highGroupOffTime)
                  highGroupOffTime = notes[i].offTime;
            }

      std::map<int, ReducedFraction> maxChordLengths = findMaxChordLengths(chords);

      addGroupSplits(splits, maxChordLengths, chords, tuplets, onTime, lowGroupOffTime,
                     origVoice, ShiftedPitchGroup::LOW);
      addGroupSplits(splits, maxChordLengths, chords, tuplets, onTime, highGroupOffTime,
                     origVoice, ShiftedPitchGroup::HIGH);

      return splits;
      }

const int MAX_PITCH_DIST = 1000;

int findPrevPitchDist(
            const std::multimap<ReducedFraction, MidiChord>::iterator &chordIt,
            int averagePitch,
            const std::multimap<ReducedFraction, MidiChord> &chords,
            int voice)
      {
      auto it = chordIt;
      while (it != chords.begin()) {
            --it;
            if (it->second.voice == voice) {
                  return qAbs(MChord::chordAveragePitch(it->second.notes) - averagePitch);
                  }
            }
      return MAX_PITCH_DIST;
      }

int findNextPitchDist(
            const std::multimap<ReducedFraction, MidiChord>::iterator &chordIt,
            int averagePitch,
            const std::multimap<ReducedFraction, MidiChord> &chords,
            int voice)
      {
      auto it = (chordIt == chords.end()) ? chordIt : std::next(chordIt);
      while (it != chords.end()) {
            if (it->second.voice == voice) {
                  return qAbs(MChord::chordAveragePitch(it->second.notes) - averagePitch);
                  }
            ++it;
            }
      return MAX_PITCH_DIST;
      }

int findMinPitchDist(
            int averagePitch,
            const int voice,
            const std::multimap<ReducedFraction, MidiChord>::iterator &chordIt,
            const std::multimap<ReducedFraction, MidiChord> &chords)
      {
      const int OCTAVE = 12;
      const int prevPitchDist = findPrevPitchDist(chordIt, averagePitch, chords, voice);
      const int nextPitchDist = findNextPitchDist(chordIt, averagePitch, chords, voice);

      int pitchDist = MAX_PITCH_DIST;

      if (prevPitchDist < nextPitchDist && prevPitchDist <= OCTAVE)
            pitchDist = prevPitchDist;
      else if (nextPitchDist <= prevPitchDist && nextPitchDist <= OCTAVE)
            pitchDist = nextPitchDist;

      return pitchDist;
      }

int findAverageLowPitch(const QList<MidiNote> &notes, int splitPoint)
      {
      int averageLowPitch = 0;
      for (int j = 0; j != splitPoint; ++j)
            averageLowPitch += notes[j].pitch;
      averageLowPitch = qRound(averageLowPitch * 1.0 / splitPoint);

      return averageLowPitch;
      }

int findAverageHighPitch(const QList<MidiNote> &notes, int splitPoint)
      {
      int averageHighPitch = 0;
      for (int j = splitPoint; j != notes.size(); ++j)
            averageHighPitch += notes[j].pitch;
      averageHighPitch = qRound(averageHighPitch * 1.0 / (notes.size() - splitPoint));

      return averageHighPitch;
      }

VoiceSplit findBestSplit(
            const std::multimap<ReducedFraction, MidiChord>::iterator &chordIt,
            const std::multimap<ReducedFraction, MidiChord> &chords,
            const std::vector<VoiceSplit> &possibleSplits,
            int splitPoint)
      {
      const int maxInvalidVoice = 100;
      std::pair<int, int> minError{MAX_PITCH_DIST, maxInvalidVoice};    // to minimize <pitch distance, voice>
      int bestSplit = -1;

      for (int i = 0; i != (int)possibleSplits.size(); ++i) {
            const int voice = possibleSplits[i].voice;
            const auto &notes = chordIt->second.notes;

            const int averageLowPitch = findAverageLowPitch(notes, splitPoint);
            const int averageHighPitch = findAverageHighPitch(notes, splitPoint);

            const int lowVoice = (possibleSplits[i].group == ShiftedPitchGroup::LOW)
                                    ? voice : chordIt->second.voice;
            const int highVoice = (possibleSplits[i].group == ShiftedPitchGroup::HIGH)
                                    ? voice : chordIt->second.voice;

            const int totalPitchDist = findMinPitchDist(averageLowPitch, lowVoice, chordIt, chords);
                                     + findMinPitchDist(averageHighPitch, highVoice, chordIt, chords);

            const std::pair<int, int> error{totalPitchDist, voice};

            if (error < minError) {
                  minError = error;
                  bestSplit = i;
                  }
            }

      return possibleSplits[bestSplit];
      }

std::pair<int, int>
findIndexRange(ShiftedPitchGroup splitGroup, int splitPoint, int noteCount)
      {
      int beg = 0;
      int end = noteCount;

      switch (splitGroup) {
            case ShiftedPitchGroup::LOW:
                  end = splitPoint;
                  break;
            case ShiftedPitchGroup::HIGH:
                  beg = splitPoint;
                  break;
            }
      return {beg, end};
      }

void separateVoices(
            std::multimap<ReducedFraction, MidiChord> &chords,
            const TimeSigMap *sigmap,
            const std::multimap<ReducedFraction, MidiTuplet::TupletData> &tuplets)
      {
      MChord::sortNotesByPitch(chords);

      for (auto it = chords.begin(); it != chords.end(); ++it) {
            const ReducedFraction onTime = it->first;
            MidiChord &chord = it->second;
            auto &notes = chord.notes;

            const int splitPoint = findOptimalSplitPoint(
                                    notes, chord.voice, onTime, sigmap, tuplets);
            if (splitPoint == -1)
                  continue;
            const auto possibleSplits = findPossibleVoiceSplits(
                                          chord.voice, notes, onTime,
                                          splitPoint, chords, tuplets);

            const VoiceSplit bestSplit = findBestSplit(it, chords, possibleSplits, splitPoint);

            MidiChord newChord(chord);
            const auto indexRange = findIndexRange(bestSplit.group, splitPoint,
                                                   chord.notes.size());
            newChord.notes.clear();
            for (int i = indexRange.first; i != indexRange.second; ++i)
                  newChord.notes.append(chord.notes[i]);
            for (int i = indexRange.first; i != indexRange.second; ++i)
                  chord.notes.removeAt(i);
            newChord.voice = bestSplit.voice;

            it = chords.insert({onTime, newChord});
            }
      }

int findBarIndexForOffTime(const ReducedFraction &offTime, const TimeSigMap *sigmap)
      {
      int barIndex, beat, tick;
      sigmap->tickValues(offTime.ticks(), &barIndex, &beat, &tick);
      if (beat == 0 && tick == 0)
            --barIndex;
      return barIndex;
      }

int averagePitchOfChords(
            const std::vector<std::multimap<ReducedFraction, MidiChord>::iterator> &chords)
      {
      if (chords.empty())
            return -1;

      int sumPitch = 0;
      int noteCounter = 0;
      for (const auto &chord: chords) {
            const auto &midiNotes = chord->second.notes;
            for (const auto &midiNote: midiNotes) {
                  sumPitch += midiNote.pitch;
                  ++noteCounter;
                  }
            }

      return qRound(sumPitch * 1.0 / noteCounter);
      }

void sortVoicesByPitch(std::map<int, std::vector<
                              std::multimap<ReducedFraction, MidiChord>::iterator>> &voiceChords)
      {
            // <average pitch, old voice>
      std::vector<std::pair<int, int>> pitches;
      for (const auto &v: voiceChords)
            pitches.push_back({averagePitchOfChords(v.second), v.first});

      std::sort(pitches.begin(), pitches.end(), std::greater<std::pair<int, int>>());

      for (int newVoice = 0; newVoice != (int)pitches.size(); ++newVoice) {
            const int oldVoice = pitches[newVoice].second;
            for (auto &chord: voiceChords[oldVoice])
                  chord->second.voice = newVoice;
            }
      }

void sortVoices(
            std::multimap<ReducedFraction, MidiChord> &chords,
            const TimeSigMap *sigmap)
      {
                  // <voice, chords>
      std::map<int, std::vector<std::multimap<ReducedFraction, MidiChord>::iterator>> voiceChords;
      int minBarIndex = 0;
      int maxBarIndex = 0;

      for (auto it = chords.begin(); it != chords.end(); ++it) {
            const auto chord = it->second;

            if (chord.barIndex >= minBarIndex && chord.barIndex <= maxBarIndex) {
                  voiceChords[chord.voice].push_back(it);
                  const int barIndex = findBarIndexForOffTime(
                                          MChord::maxNoteOffTime(chord.notes), sigmap);
                  if (barIndex > maxBarIndex)
                        maxBarIndex = barIndex;
                  }

            if (std::next(it) == chords.end()
                        || chord.barIndex < minBarIndex || chord.barIndex > maxBarIndex) {
                  sortVoicesByPitch(voiceChords);
                  voiceChords.clear();
                  minBarIndex = ++maxBarIndex;
                  }
            }
      }

void simplifyNotation(std::multimap<int, MTrack> &tracks, const TimeSigMap *sigmap)
      {
      auto &opers = preferences.midiImportOperations;

      for (auto &track: tracks) {
            MTrack &mtrack = track.second;
            if (mtrack.mtrack->drumTrack())
                  continue;
            auto &chords = track.second.chords;
            if (chords.empty())
                  continue;
                        // pass current track index through MidiImportOperations
                        // for further usage
            opers.setCurrentTrack(mtrack.indexOfOperation);

            if (opers.currentTrackOperations().simplifyNotation) {
                  if (MidiTuplet::voiceLimit() > 1) {
                        separateVoices(chords, sigmap, mtrack.tuplets);
                        sortVoices(chords, sigmap);
                        }
                  minimizeNumberOfRests(chords, sigmap, mtrack.tuplets);
                  }
            }
      }

} // Simplify
} // Ms
