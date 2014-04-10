#include "importmidi_tuplet.h"
#include "importmidi_chord.h"
#include "importmidi_meter.h"
#include "importmidi_quant.h"
#include "importmidi_inner.h"
#include "libmscore/mscore.h"
#include "libmscore/staff.h"
#include "libmscore/score.h"
#include "libmscore/fraction.h"
#include "libmscore/duration.h"
#include "libmscore/measure.h"
#include "libmscore/tuplet.h"
#include "preferences.h"


#include <set>


namespace Ms {
namespace MidiTuplet {

const std::map<int, TupletLimits>& tupletsLimits()
      {
      const static std::map<int, TupletLimits> values = {
            {2, {{2, 3}, 1, 2, 2}},
            {3, {{3, 2}, 1, 3, 3}},
            {4, {{4, 3}, 3, 4, 3}},
            {5, {{5, 4}, 3, 4, 4}},
            {7, {{7, 8}, 4, 6, 5}},
            {9, {{9, 8}, 6, 7, 7}}
            };
      return values;
      }

const TupletLimits& tupletLimits(int tupletNumber)
      {
      auto it = tupletsLimits().find(tupletNumber);

      Q_ASSERT_X(it != tupletsLimits().end(), "MidiTuplet::tupletValue", "Unknown tuplet");

      return it->second;
      }

int voiceLimit()
      {
      const auto operations = preferences.midiImportOperations.currentTrackOperations();
      return (operations.useMultipleVoices) ? VOICES : 1;
      }

int tupletVoiceLimit()
      {
      const auto operations = preferences.midiImportOperations.currentTrackOperations();
                  // for multiple voices: one voice is reserved for non-tuplet chords
      return (operations.useMultipleVoices) ? VOICES - 1 : 1;
      }

bool isTupletAllowed(const TupletInfo &tupletInfo)
      {
      {
                  // special check for duplets and triplets
      const std::vector<int> nums = {2, 3};
                  // for duplet: if note first and single - only 1/2*tupletLen duration is allowed
                  // for triplet: if note first and single - only 1/3*tupletLen duration is allowed
      for (int num: nums) {
            if (tupletInfo.tupletNumber == num
                        && tupletInfo.chords.size() == 1
                        && tupletInfo.firstChordIndex == 0) {
                  const auto &chordEventIt = tupletInfo.chords.begin()->second;
                  const auto tupletNoteLen = tupletInfo.len / num;
                  for (const auto &note: chordEventIt->second.notes) {
                        if ((note.offTime - chordEventIt->first - tupletNoteLen).absValue()
                                    > tupletNoteLen / 2)
                              return false;
                        }
                  }
            }
      }
                  // for all tuplets
      const int minAllowedNoteCount = tupletLimits(tupletInfo.tupletNumber).minNoteCount;
      if ((int)tupletInfo.chords.size() < minAllowedNoteCount)
            return false;
                  // allow duplets and quadruplets with the zero error == regular error
      if (tupletInfo.tupletNumber == 2 || tupletInfo.tupletNumber == 4) {
            if (tupletInfo.tupletSumError > tupletInfo.regularSumError)
                  return false;
            else if (tupletInfo.tupletSumError == tupletInfo.regularSumError
                     && tupletInfo.tupletSumError > ReducedFraction(0, 1))
                  return false;
            }
      else {
            if (tupletInfo.tupletSumError >= tupletInfo.regularSumError)
                  return false;
            }
                  // only notes with len >= (half tuplet note len) allowed
      const auto tupletNoteLen = tupletInfo.len / tupletInfo.tupletNumber;
      for (const auto &tupletChord: tupletInfo.chords) {
            for (const auto &note: tupletChord.second->second.notes) {
                  if (note.offTime - tupletChord.first >= tupletNoteLen / 2)
                        return true;
                  }
            }
      return false;
      }

std::vector<int> findTupletNumbers(const ReducedFraction &divLen,
                                   const ReducedFraction &barFraction)
      {
      const auto operations = preferences.midiImportOperations.currentTrackOperations();
      std::vector<int> tupletNumbers;

      if (Meter::isCompound(barFraction) && divLen == Meter::beatLength(barFraction)) {
            if (operations.tuplets.duplets)
                  tupletNumbers.push_back(2);
            if (operations.tuplets.quadruplets)
                  tupletNumbers.push_back(4);
            }
      else {
            if (operations.tuplets.triplets)
                  tupletNumbers.push_back(3);
            if (operations.tuplets.quintuplets)
                  tupletNumbers.push_back(5);
            if (operations.tuplets.septuplets)
                  tupletNumbers.push_back(7);
            if (operations.tuplets.nonuplets)
                  tupletNumbers.push_back(9);
            }

      return tupletNumbers;
      }

// return: <chordIter, minChordError>

std::pair<std::multimap<ReducedFraction, MidiChord>::iterator, ReducedFraction>
findBestChordForTupletNote(
            const ReducedFraction &tupletNotePos,
            const ReducedFraction &basicQuant,
            const std::multimap<ReducedFraction, MidiChord>::iterator &startChordIt,
            const std::multimap<ReducedFraction, MidiChord>::iterator &endChordIt)
      {
                  // choose the best chord, if any, for this tuplet note
      std::pair<std::multimap<ReducedFraction, MidiChord>::iterator, ReducedFraction> bestChord;
      bestChord.first = endChordIt;
                  // check chords - whether they can be in tuplet without large error
      bool firstLoop = true;
      for (auto chordIt = startChordIt; chordIt != endChordIt; ++chordIt) {
            const auto tupletError = (chordIt->first - tupletNotePos).absValue();
            const auto regularError = Quantize::findOnTimeQuantError(*chordIt, basicQuant);
            if (tupletError > regularError)
                  continue;
            if (firstLoop) {
                  bestChord.first = chordIt;
                  bestChord.second = tupletError;
                  firstLoop = false;
                  continue;
                  }
            if (tupletError < bestChord.second) {
                  bestChord.first = chordIt;
                  bestChord.second = tupletError;
                  }
            }
      return bestChord;
      }

// find tuplets over which duration lies

std::vector<TupletData>
findTupletsInBarForDuration(
            int voice,
            const ReducedFraction &barStartTick,
            const ReducedFraction &durationOnTime,
            const ReducedFraction &durationLen,
            const std::multimap<ReducedFraction, TupletData> &tupletEvents)
      {
      std::vector<TupletData> tupletsData;
      if (tupletEvents.empty())
            return tupletsData;
      auto tupletIt = tupletEvents.lower_bound(barStartTick);

      while (tupletIt != tupletEvents.end()
                && tupletIt->first < durationOnTime + durationLen) {
            if (tupletIt->second.voice == voice
                        && durationOnTime < tupletIt->first + tupletIt->second.len) {
                              // if tuplet and duration intersect each other
                  auto tupletData = tupletIt->second;
                              // convert tuplet onTime to local bar ticks
                  tupletData.onTime -= barStartTick;
                  tupletsData.push_back(tupletData);
                  }
            ++tupletIt;
            }
      return tupletsData;
      }

std::multimap<ReducedFraction, MidiTuplet::TupletData>::const_iterator
findTupletForTimeRange(
            int voice,
            const ReducedFraction &onTime,
            const ReducedFraction &len,
            const std::multimap<ReducedFraction, TupletData> &tupletEvents)
      {
      Q_ASSERT_X(len >= ReducedFraction(0, 1),
                 "MidiTuplet::findTupletForTimeRange", "Negative length of time range");

      if (tupletEvents.empty())
            return tupletEvents.end();

      auto it = tupletEvents.upper_bound(onTime + len);
      if (it != tupletEvents.begin())
            --it;
      while (true) {
            const auto &tupletData = it->second;
            const auto &tupletOffTime = tupletData.onTime + tupletData.len;
            if (tupletData.voice == voice
                        && onTime >= tupletData.onTime
                        && ((len > ReducedFraction(0, 1)
                             ? onTime + len <= tupletOffTime
                             : onTime < tupletOffTime)
                            )) {
                  return it;
                  }
            if (it == tupletEvents.begin())
                  break;
            --it;
            }
      return tupletEvents.end();
      }

std::multimap<ReducedFraction, MidiTuplet::TupletData>::const_iterator
findTupletContainsTime(
            int voice,
            const ReducedFraction &time,
            const std::multimap<ReducedFraction, TupletData> &tupletEvents)
      {
      return findTupletForTimeRange(voice, time, ReducedFraction(0, 1), tupletEvents);
      }

// find sum length of gaps between successive chords
// less is better

ReducedFraction findSumLengthOfRests(
            const TupletInfo &tupletInfo,
            const ReducedFraction &startBarTick,
            const ReducedFraction &basicQuant)
      {
      auto beg = tupletInfo.onTime;
      const auto tupletEndTime = tupletInfo.onTime + tupletInfo.len;
      const auto tupletNoteLen = tupletInfo.len / tupletInfo.tupletNumber;
      ReducedFraction sumLen = {0, 1};

      for (const auto &chord: tupletInfo.chords) {
            const auto staccatoIt = tupletInfo.staccatoChords.find(chord.first);
            const MidiChord &midiChord = chord.second->second;
            const auto &chordOnTime = (chord.second->first < startBarTick)
                        ? startBarTick
                        : Quantize::findQuantizedChordOnTime(*chord.second, basicQuant,
                                  tupletLimits(tupletInfo.tupletNumber).ratio, startBarTick);
            if (beg < chordOnTime)
                  sumLen += (chordOnTime - beg);
            ReducedFraction maxOffTime(0, 1);
            for (int i = 0; i != midiChord.notes.size(); ++i) {
                  auto noteOffTime = midiChord.notes[i].offTime;
                  if (staccatoIt != tupletInfo.staccatoChords.end() && i == staccatoIt->second)
                        noteOffTime = chordOnTime + tupletNoteLen;
                  if (noteOffTime > maxOffTime)
                        maxOffTime = noteOffTime;
                  }
            beg = Quantize::findQuantizedNoteOffTime(*chord.second, maxOffTime, basicQuant,
                                    tupletLimits(tupletInfo.tupletNumber).ratio, startBarTick);
            if (beg >= tupletEndTime)
                  break;
            }
      if (beg < tupletEndTime)
            sumLen += (tupletEndTime - beg);
      return sumLen;
      }

TupletInfo findTupletApproximation(
            const ReducedFraction &tupletLen,
            int tupletNumber,
            const ReducedFraction &basicQuant,
            const ReducedFraction &startTupletTime,
            const std::multimap<ReducedFraction, MidiChord>::iterator &startChordIt,
            const std::multimap<ReducedFraction, MidiChord>::iterator &endChordIt)
      {
      TupletInfo tupletInfo;
      tupletInfo.tupletNumber = tupletNumber;
      tupletInfo.onTime = startTupletTime;
      tupletInfo.len = tupletLen;

      auto startTupletChordIt = startChordIt;
      for (int k = 0; k != tupletNumber; ++k) {
            auto tupletNotePos = startTupletTime + tupletLen / tupletNumber * k;
                        // choose the best chord, if any, for this tuplet note
            auto bestChord = findBestChordForTupletNote(tupletNotePos, basicQuant,
                                                        startTupletChordIt, endChordIt);
            if (bestChord.first == endChordIt)
                  continue;   // no chord fits to this tuplet note position
                        // chord can be in tuplet
            if (tupletInfo.chords.empty())
                  tupletInfo.firstChordIndex = k;
            const auto &chordOnTime = bestChord.first->first;
            tupletInfo.chords.insert({chordOnTime, bestChord.first});
            tupletInfo.tupletSumError += bestChord.second;
                        // for next tuplet note we start from the next chord
                        // because chord for the next tuplet note cannot be earlier
            startTupletChordIt = bestChord.first;
            ++startTupletChordIt;
                        // find chord quant error for a regular grid
            tupletInfo.regularSumError += Quantize::findOnTimeQuantError(
                                                      *bestChord.first, basicQuant);
            }

      return tupletInfo;
      }

bool isMoreTupletVoicesAllowed(int voicesInUse, int availableVoices)
      {
      return !(voicesInUse >= availableVoices || voicesInUse >= tupletVoiceLimit());
      }

std::pair<ReducedFraction, ReducedFraction>
tupletInterval(const TupletInfo &tuplet,
               const ReducedFraction &basicQuant)
      {
      ReducedFraction tupletEnd = tuplet.onTime + tuplet.len;

      for (const auto &chord: tuplet.chords) {
            const auto offTime = Quantize::findMaxQuantizedOffTime(*chord.second, basicQuant);
            if (offTime > tupletEnd)
                  tupletEnd = offTime;
            }
      return std::make_pair(tuplet.onTime, tupletEnd);
      }

bool haveIntersection(const std::pair<ReducedFraction, ReducedFraction> &interval1,
                      const std::pair<ReducedFraction, ReducedFraction> &interval2)
      {
      return interval1.second > interval2.first && interval1.first < interval2.second;
      }

bool haveIntersection(
            const std::pair<ReducedFraction, ReducedFraction> &interval,
            const std::vector<std::pair<ReducedFraction, ReducedFraction>> &intervals)
      {
      for (const auto &i: intervals) {
            if (haveIntersection(i, interval))
                  return true;
            }
      return false;
      }

class TupletErrorResult
      {
   public:
      TupletErrorResult(double t = 0.0,
                        double relPlaces = 0.0,
                        const ReducedFraction &r = ReducedFraction(0, 1),
                        size_t vc = 0,
                        size_t tc = 0)
            : tupletAverageError(t)
            , relativeUsedChordPlaces(relPlaces)
            , sumLengthOfRests(r)
            , voiceCount(vc)
            , tupletCount(tc)
            {}

      bool isInitialized() const { return tupletCount != 0; }

      bool operator<(const TupletErrorResult &er) const
            {
            double value = div(tupletAverageError, er.tupletAverageError)
                         - div(relativeUsedChordPlaces, er.relativeUsedChordPlaces)
                         + div(sumLengthOfRests.numerator() * 1.0 / sumLengthOfRests.denominator(),
                               er.sumLengthOfRests.numerator() * 1.0 / er.sumLengthOfRests.denominator());
            if (value == 0) {
                   value = div(voiceCount, er.voiceCount)
                         + div(tupletCount, er.tupletCount);
                  }
            return value < 0;
            }

   private:
      static double div(double val1, double val2)
            {
            if (val1 == val2)
                  return 0;
            return (val1 - val2) / qMax(val1, val2);
            }

      double tupletAverageError;
      double relativeUsedChordPlaces;
      ReducedFraction sumLengthOfRests;
      size_t voiceCount;
      size_t tupletCount;
      };

bool haveCommonChords(int i, int j, const std::vector<TupletInfo> &tuplets)
      {
      if (tuplets.empty())
            return false;
      std::set<std::pair<const ReducedFraction, MidiChord> *> chordsI;
      for (const auto &chord: tuplets[i].chords)
            chordsI.insert(&*chord.second);
      for (const auto &chord: tuplets[j].chords)
            if (chordsI.find(&*chord.second) != chordsI.end())
                  return true;
      return false;
      }

// remove overlapping tuplets with the same tuplet number
// when tuplet with bigger length contains the same notes

void removeUselessTuplets(std::vector<TupletInfo> &tuplets)
      {
      struct {
            bool operator()(const TupletInfo &t1, const TupletInfo &t2) {
                  if (t1.tupletNumber < t2.tupletNumber)
                        return true;
                  if (t1.tupletNumber == t2.tupletNumber)
                        return t1.len < t2.len;
                  return false;
                  }
            } comparator;
      std::sort(tuplets.begin(), tuplets.end(), comparator);

      size_t beg = 0;
      while (beg < tuplets.size()) {
            size_t end = beg + 1;
            while (tuplets.size() > end && tuplets[end].tupletNumber == tuplets[beg].tupletNumber)
                  ++end;
            for (size_t i = beg; i < end - 1; ++i) {
                  const auto &t1 = tuplets[i];
                  for (size_t j = i + 1; j < end; ++j) {
                        const auto &t2 = tuplets[j];
                        if (t1.onTime >= t2.onTime && t1.onTime + t1.len <= t2.onTime + t2.len) {
                                    // check onTimes
                              if (t2.chords.rbegin()->first < t1.onTime + t1.len
                                          && t2.chords.begin()->first >= t1.onTime)
                                    {
                                          // remove larger tuplet
                                    tuplets.erase(tuplets.begin() + j);
                                    --j;
                                    --end;
                                    }
                              }
                        }
                  }
            beg = end;
            }
      }

std::vector<std::pair<ReducedFraction, ReducedFraction> >
findTupletIntervals(const std::vector<TupletInfo> &tuplets,
                    const ReducedFraction &basicQuant)
      {
      std::vector<std::pair<ReducedFraction, ReducedFraction>> tupletIntervals;
      for (const auto &tuplet: tuplets)
            tupletIntervals.push_back(tupletInterval(tuplet, basicQuant));

      return tupletIntervals;
      }

std::set<int> findLongestUncommonGroup(const std::vector<TupletInfo> &tuplets)
      {
      struct TInfo
            {
            bool operator<(const TInfo &other) const
                  {
                  if (offTime < other.offTime)
                        return true;
                  else if (offTime > other.offTime)
                        return false;
                  else
                        return onTime >= other.onTime;
                  }
            bool operator==(const TInfo &other) const
                  {
                  return offTime == other.offTime;
                  }

            ReducedFraction onTime;
            ReducedFraction offTime;
            int index;
            };

      std::vector<TInfo> info;
      for (int i = 0; i != (int)tuplets.size(); ++i) {
            const auto &tuplet = tuplets[i];
            info.push_back({tuplet.onTime, tuplet.onTime + tuplet.len, i});
            }

      std::sort(info.begin(), info.end());
      info.erase(std::unique(info.begin(), info.end()), info.end());

      // check for overlapping tuplets
      // and check for rare case: because of tol when detecting tuplets
      // non-overlapping tuplets can have common chords

      std::set<int> indexes;
      int lastSelected = 0;
      for (int i = 0; i != (int)info.size(); ++i) {
            if (i > 0 && info[i].onTime < info[lastSelected].offTime)
                  continue;
            if (haveCommonChords(info[lastSelected].index, info[i].index, tuplets))
                  continue;
            lastSelected = i;
            indexes.insert(info[i].index);
            }

      return indexes;
      }


struct TupletCommon
      {
            // indexes of tuplets that have common chords with the tuplet with tupletIndex
      std::set<int> commonIndexes;
      };


std::vector<TupletCommon> findTupletCommons(const std::vector<TupletInfo> &tuplets)
      {
                 // <chord address, tuplet indexes>
      std::map<std::pair<const ReducedFraction, MidiChord> *, std::set<int>> usedChords;
      const int voiceLimit = tupletVoiceLimit();

      for (size_t i = 0; i != tuplets.size(); ++i) {
            const auto &tuplet = tuplets[i];
            auto it = tuplet.chords.begin();
            if (tuplet.firstChordIndex == 0
                        && voiceLimit > 1
                        && it->second->second.notes.size() > 1
                        && usedChords.find(&*it->second) != usedChords.end()) {
                  ++it;
                  }
            for ( ; it != tuplet.chords.end(); ++it)
                  usedChords[&*it->second].insert(i);
            }

      std::vector<TupletCommon> tupletCommons(tuplets.size());

      for (size_t i = 0; i != tuplets.size(); ++i) {
            for (const auto &chord: tuplets[i].chords) {
                  auto &indexes = usedChords[&*chord.second];
                  indexes.erase(i);
                  tupletCommons[i].commonIndexes.insert(indexes.begin(), indexes.end());
                  }
            }

      return tupletCommons;
      }

bool isInCommonIndexes(
            int indexToCheck,
            const std::vector<int> &selectedTuplets,
            const std::vector<TupletCommon> &tupletCommons)
      {
      for (size_t i = 0; i != selectedTuplets.size() - 1; ++i) {
            const auto &indexes = tupletCommons[selectedTuplets[i]].commonIndexes;
            if (indexes.find(indexToCheck) != indexes.end())
                  return true;
            }
      return false;
      }

TupletErrorResult findTupletError(
            const std::vector<int> &tupletIndexes,
            const std::vector<TupletInfo> &tuplets,
            size_t voiceCount,
            const ReducedFraction &basicQuant)
      {
      ReducedFraction sumError{0, 1};
      ReducedFraction sumLengthOfRests{0, 1};
      int sumChordCount = 0;
      int sumChordPlaces = 0;
      std::set<std::pair<const ReducedFraction, MidiChord> *> usedChords;
      std::vector<char> usedIndexes(tuplets.size(), 0);

      for (int i: tupletIndexes) {
            const auto &tuplet = tuplets[i];

            sumError += tuplet.tupletSumError;
            sumLengthOfRests += tuplet.sumLengthOfRests;
            sumChordCount += tuplet.chords.size();
            sumChordPlaces += tuplet.tupletNumber;

            usedIndexes[i] = 1;
            for (const auto &chord: tuplet.chords)
                  usedChords.insert(&*chord.second);
            }
                  // add quant error of all chords excluded from tuplets
      for (size_t i = 0; i != tuplets.size(); ++i) {
            if (usedIndexes[i])
                  continue;
            const auto &tuplet = tuplets[i];
            for (const auto &chord: tuplet.chords) {
                  if (usedChords.find(&*chord.second) != usedChords.end())
                        continue;
                  sumError += Quantize::findOnTimeQuantError(*chord.second, basicQuant);
                  }
            }

      return TupletErrorResult{
                  sumError.numerator() * 1.0 / (sumError.denominator() * sumChordCount),
                  sumChordCount * 1.0 / sumChordPlaces,
                  sumLengthOfRests,
                  voiceCount,
                  tupletIndexes.size()
            };
      }


#ifdef QT_DEBUG

bool areCommonsDifferent(const std::vector<int> &selectedCommons)
      {
      std::set<int> commons;
      for (int i: selectedCommons) {
            if (commons.find(i) != commons.end())
                  return false;
            commons.insert(i);
            }
      return true;
      }

bool areCommonsUncommon(const std::vector<int> &selectedCommons,
                        const std::vector<TupletCommon> &tupletCommons)
      {
      std::set<int> commons;
      for (int i: selectedCommons) {
            for (int j: tupletCommons[i].commonIndexes)
                  commons.insert(j);
            }
      for (int i: selectedCommons) {
            if (commons.find(i) != commons.end())
                  return false;
            }
      return true;
      }

#endif


int findAvailableVoice(
            size_t tupletIndex,
            const std::vector<std::pair<ReducedFraction, ReducedFraction>> &tupletIntervals,
            const std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction>>> &voiceIntervals)
      {
      int voice = 0;
      while (true) {
            const auto it = voiceIntervals.find(voice);
            if (it != voiceIntervals.end()
                        && haveIntersection(tupletIntervals[tupletIndex], it->second)) {
                  ++voice;
                  continue;
                  }
            break;
            }

      return voice;
      }

std::map<std::pair<const ReducedFraction, MidiChord> *, int>
prepareUsedFirstChords(const std::vector<int> &selectedTuplets,
                       const std::vector<TupletInfo> &tuplets)
      {
      std::map<std::pair<const ReducedFraction, MidiChord> *, int> usedFirstChords;
      for (int i: selectedTuplets) {
            if (tuplets[i].firstChordIndex != 0)
                  continue;
            const auto firstChord = tuplets[i].chords.begin();
            const auto it = usedFirstChords.find(&*firstChord->second);
            if (it != usedFirstChords.end())
                  ++(it->second);
            else
                  usedFirstChords.insert({&*firstChord->second, 1});
            }

      return usedFirstChords;
      }

std::vector<int> findUnusedIndexes(const std::vector<int> &selectedTuplets)
      {
      std::vector<int> unusedIndexes;
      int k = 0;
      for (int i = 0; i != selectedTuplets.back(); ++i) {
            if (i == selectedTuplets[k]) {
                  ++k;
                  continue;
                  }
            unusedIndexes.push_back(i);
            }
      return unusedIndexes;
      }

bool canUseIndex(
            int indexToCheck,
            const std::vector<TupletInfo> &tuplets,
            const std::vector<std::pair<ReducedFraction, ReducedFraction> > &tupletIntervals,
            const std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction>>> &voiceIntervals,
            const std::map<std::pair<const ReducedFraction, MidiChord> *, int> &usedFirstChords)
      {
      const auto &tuplet = tuplets[indexToCheck];
                  // check tuplets for common 1st chord
      if (tuplet.firstChordIndex == 0) {
            const auto firstChord = tuplet.chords.begin();
            const auto it = usedFirstChords.find(&*firstChord->second);
            if (it != usedFirstChords.end() && !isMoreTupletVoicesAllowed(
                              it->second, it->first->second.notes.size())) {
                  return false;
                  }
            }
                  // check tuplets for resulting voice count
      const int voice = findAvailableVoice(indexToCheck, tupletIntervals, voiceIntervals);
      const int voiceCount = qMax((int)voiceIntervals.size(), voice + 1);
      if (voiceCount > tupletVoiceLimit() || (voiceCount > 1 && (int)tuplet.chords.size()
                        < tupletLimits(tuplet.tupletNumber).minNoteCountAddVoice)) {
            return false;
            }
      return true;
      }

void tryUpdateBestIndexes(
            std::vector<int> &bestTupletIndexes,
            TupletErrorResult &minCurrentError,
            const std::vector<int> &selectedTuplets,
            const std::vector<TupletInfo> &tuplets,
            const std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction>>> &voiceIntervals,
            const ReducedFraction &basicQuant)
      {
      const size_t voiceCount = voiceIntervals.size();
      const auto error = findTupletError(selectedTuplets, tuplets,
                                         voiceCount, basicQuant);
      if (!minCurrentError.isInitialized() || error < minCurrentError) {
            minCurrentError = error;
            bestTupletIndexes = selectedTuplets;
            }
      }

std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction> > >
prepareVoiceIntervals(
            const std::vector<int> &selectedTuplets,
            const std::vector<std::pair<ReducedFraction, ReducedFraction> > &tupletIntervals)
      {
                  // <voice, intervals>
      std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction>>> voiceIntervals;
      for (int i: selectedTuplets) {
            int voice = findAvailableVoice(i, tupletIntervals, voiceIntervals);
            voiceIntervals[voice].push_back(tupletIntervals[i]);
            }
      return voiceIntervals;
      }

class ValidTuplets
      {
   public:
      ValidTuplets(int tupletsSize)
            : indexes_(tupletsSize)
            , first_(0)
            {
            for (int i = 0; i != (int)indexes_.size(); ++i)
                  indexes_[i] = {i - 1, i + 1};
            }

      int first() const
            {
            return first_;
            }

      bool isValid(int index) const
            {
            return index >= first_ && index < (int)indexes_.size();
            }

      int next(int index) const
            {
            return indexes_[index].second;
            }

      bool empty() const
            {
            return first_ >= (int)indexes_.size();
            }

      int exclude(int index)
            {
            if (index < first_)
                  return index;
            int prev = indexes_[index].first;
            int next = indexes_[index].second;
            indexes_[index].first = -1;
            indexes_[index].second = indexes_.size();
            if (prev >= first_)
                  indexes_[prev].second = next;
            if (next < (int)indexes_.size())
                  indexes_[next].first = prev;
            if (index == first_)
                  first_ = next;
            return next;
            }

      std::vector<std::pair<int, int>> save()
            {
            std::vector<std::pair<int, int>> indexes(indexes_.size() - first_);
            for (int i = first_; i != (int)indexes_.size(); ++i)
                  indexes[i - first_] = indexes_[i];
            return indexes;
            }

      void restore(const std::vector<std::pair<int, int>> &indexes)
            {
            first_ = indexes_.size() - indexes.size();
            for (int i = 0; i != (int)indexes.size(); ++i)
                  indexes_[i + first_] = indexes[i];
            }

   private:
      std::vector<std::pair<int, int>> indexes_;      // pair<prev, next>
      int first_;
      };


void findNextTuplet(
            std::vector<int> &selectedTuplets,
            ValidTuplets &validTuplets,
            std::vector<int> &bestTupletIndexes,
            TupletErrorResult &minCurrentError,
            const std::vector<TupletCommon> &tupletCommons,
            const std::vector<TupletInfo> &tuplets,
            const std::vector<std::pair<ReducedFraction, ReducedFraction> > &tupletIntervals,
            size_t commonsSize,
            const ReducedFraction &basicQuant)
      {
      while (!validTuplets.empty()) {
            size_t index = validTuplets.first();

            bool isCommonGroupBegins = (selectedTuplets.empty() && index == commonsSize);
            if (isCommonGroupBegins) {      // first level
                  for (size_t i = index; i < tuplets.size(); ++i)
                        selectedTuplets.push_back(i);
                  }
            else {
                  selectedTuplets.push_back(index);
                  }
            const auto voiceIntervals = prepareVoiceIntervals(selectedTuplets, tupletIntervals);
            const auto usedFirstChords = prepareUsedFirstChords(selectedTuplets, tuplets);

            Q_ASSERT_X(areCommonsDifferent(selectedTuplets), "MidiTuplet::findNextTuplet",
                       "There are duplicates in selected commons");
            Q_ASSERT_X(areCommonsUncommon(selectedTuplets, tupletCommons),
                       "MidiTuplet::findNextTuplet", "Incompatible selected commons");

            if (isCommonGroupBegins) {
                  bool canAddMoreIndexes = false;
                  for (size_t i = 0; i != commonsSize; ++i) {
                        if (!isInCommonIndexes(i, selectedTuplets, tupletCommons)
                                    && canUseIndex(i, tuplets, tupletIntervals,
                                                   voiceIntervals, usedFirstChords)) {
                              canAddMoreIndexes = true;
                              break;
                              }
                        }
                  if (!canAddMoreIndexes) {
                        tryUpdateBestIndexes(bestTupletIndexes, minCurrentError,
                                             selectedTuplets, tuplets, voiceIntervals, basicQuant);
                        }
                  return;
                  }

            validTuplets.exclude(index);
            const auto savedTuplets = validTuplets.save();
                        // check tuplets for compatibility
            if (!validTuplets.empty()) {
                  for (int i: tupletCommons[index].commonIndexes) {
                        validTuplets.exclude(i);
                        if (validTuplets.empty())
                              break;
                        }
                  }
            for (int i = validTuplets.first(); validTuplets.isValid(i); ) {
                  if (!canUseIndex(i, tuplets, tupletIntervals,
                                   voiceIntervals, usedFirstChords)) {
                        i = validTuplets.exclude(i);
                        continue;
                        }
                  i = validTuplets.next(i);
                  }
            if (validTuplets.empty()) {
                  const auto unusedIndexes = findUnusedIndexes(selectedTuplets);
                  bool canAddMoreIndexes = false;
                  for (int i: unusedIndexes) {
                        if (!isInCommonIndexes(i, selectedTuplets, tupletCommons)
                                    && canUseIndex(i, tuplets, tupletIntervals,
                                                   voiceIntervals, usedFirstChords)) {
                              canAddMoreIndexes = true;
                              break;
                              }
                        }
                  if (!canAddMoreIndexes) {
                        tryUpdateBestIndexes(bestTupletIndexes, minCurrentError,
                                             selectedTuplets, tuplets, voiceIntervals, basicQuant);
                        }
                  }
            else {
                  findNextTuplet(selectedTuplets, validTuplets, bestTupletIndexes, minCurrentError,
                                 tupletCommons, tuplets, tupletIntervals, commonsSize, basicQuant);
                  }

            selectedTuplets.pop_back();
            validTuplets.restore(savedTuplets);
            }
      }

void moveUncommonTupletsToEnd(std::vector<TupletInfo> &tuplets, std::set<int> &uncommons)
      {
      int swapWith = tuplets.size() - 1;
      for (int i = swapWith; i >= 0; --i) {
            auto it = uncommons.find(i);
            if (it != uncommons.end()) {
                  if (i != swapWith)
                        std::swap(tuplets[i], tuplets[swapWith]);
                  --swapWith;
                  uncommons.erase(it);
                  }
            }

      Q_ASSERT_X(uncommons.empty(), "MidiTuplet::moveUncommonTupletsToEnd",
                 "Untested uncommon tuplets remaining");
      }

std::vector<int> findBestTuplets(
            const std::vector<TupletCommon> &tupletCommons,
            const std::vector<TupletInfo> &tuplets,
            size_t commonsSize,
            const ReducedFraction &basicQuant)
      {
      std::vector<int> bestTupletIndexes;
      std::vector<int> selectedTuplets;
      TupletErrorResult minCurrentError;
      const auto tupletIntervals = findTupletIntervals(tuplets, basicQuant);

      ValidTuplets validTuplets(tuplets.size());

      findNextTuplet(selectedTuplets, validTuplets, bestTupletIndexes, minCurrentError,
                     tupletCommons, tuplets, tupletIntervals, commonsSize, basicQuant);

      return bestTupletIndexes;
      }


#ifdef QT_DEBUG

bool areTupletChordsEmpty(const std::vector<TupletInfo> &tuplets)
      {
      for (const auto &tuplet: tuplets) {
            if (tuplet.chords.empty())
                  return true;
            }
      return false;
      }

template<typename Iter>
bool validateSelectedTuplets(Iter beginIt,
                             Iter endIt,
                             const std::vector<TupletInfo> &tuplets)
      {
                  // <chord address, used voices>
      std::map<std::pair<const ReducedFraction, MidiChord> *, int> usedChords;
      for (auto it = beginIt; it != endIt; ++it) {
            const auto &tuplet = tuplets[*it];
            const auto &chords = tuplet.chords;
            for (auto it = chords.begin(); it != chords.end(); ++it) {
                  bool isFirstChord = (tuplet.firstChordIndex == 0 && it == tuplet.chords.begin());
                  const auto fit = usedChords.find(&*(it->second));
                  if (fit == usedChords.end()) {
                        usedChords.insert({&*(it->second), isFirstChord ? 1 : VOICES});
                        }
                  else {
                        if (!isFirstChord)
                              return false;
                        if (!isMoreTupletVoicesAllowed(fit->second, it->second->second.notes.size()))
                              return false;
                        ++(fit->second);
                        }
                  }
            }
      return true;
      }

#endif

void removeExtraTuplets(std::vector<TupletInfo> &tuplets)
      {
      const size_t MAX_TUPLETS = 23;         // found empirically

      if (tuplets.size() <= MAX_TUPLETS)
            return;

      std::map<TupletErrorResult, size_t> errors;
      for (size_t i = 0; i != tuplets.size(); ++i) {
            auto tupletError = TupletErrorResult{
                        tuplets[i].tupletSumError.numerator() * 1.0
                              / (tuplets[i].tupletSumError.denominator() * tuplets[i].chords.size()),
                        tuplets[i].chords.size() * 1.0 / tuplets[i].tupletNumber,
                        tuplets[i].sumLengthOfRests,
                        1,
                        1
                  };
            errors.insert({tupletError, i});
            }
      std::vector<TupletInfo> newTuplets;
      size_t count = 0;
      for (const auto &e: errors) {
            ++count;
            newTuplets.push_back(tuplets[e.second]);
            if (count == MAX_TUPLETS)
                  break;
            }

      std::swap(tuplets, newTuplets);
      }


// first chord in tuplet may belong to other tuplet at the same time
// in the case if there are enough notes in this first chord
// to be splitted into different voices

void filterTuplets(std::vector<TupletInfo> &tuplets,
                   const ReducedFraction &basicQuant)
      {
      if (tuplets.empty())
            return;

      Q_ASSERT_X(!areTupletChordsEmpty(tuplets),
                 "MIDI tuplets: filterTuplets", "Tuplet has no chords but it should");

      removeUselessTuplets(tuplets);
      removeExtraTuplets(tuplets);

      std::set<int> uncommons = findLongestUncommonGroup(tuplets);

      Q_ASSERT_X(validateSelectedTuplets(uncommons.begin(), uncommons.end(), tuplets),
                 "MIDI tuplets: filterTuplets",
                 "Uncommon tuplets have common chords but they shouldn't");

      size_t commonsSize = tuplets.size();
      if (uncommons.size() > 1) {
            commonsSize -= uncommons.size();
            moveUncommonTupletsToEnd(tuplets, uncommons);
            }
      const auto tupletCommons = findTupletCommons(tuplets);

      const std::vector<int> bestIndexes = findBestTuplets(tupletCommons, tuplets,
                                                           commonsSize, basicQuant);

      Q_ASSERT_X(validateSelectedTuplets(bestIndexes.begin(), bestIndexes.end(), tuplets),
                 "MIDI tuplets: filterTuplets", "Tuplets have common chords but they shouldn't");

      std::vector<TupletInfo> newTuplets;
      for (int i: bestIndexes)
            newTuplets.push_back(tuplets[i]);

      std::swap(tuplets, newTuplets);
      }

int averagePitch(const std::map<ReducedFraction,
                                std::multimap<ReducedFraction, MidiChord>::iterator> &chords)
      {
      if (chords.empty())
            return -1;
      int sumPitch = 0;
      int noteCounter = 0;
      for (const auto &chord: chords) {
            const auto &midiNotes = chord.second->second.notes;
            for (const auto &midiNote: midiNotes) {
                  sumPitch += midiNote.pitch;
                  ++noteCounter;
                  }
            }
      return sumPitch / noteCounter;
      }

void sortNotesByPitch(const std::multimap<ReducedFraction, MidiChord>::iterator &startBarChordIt,
                      const std::multimap<ReducedFraction, MidiChord>::iterator &endBarChordIt)
      {
      struct {
            bool operator()(const MidiNote &n1, const MidiNote &n2)
                  {
                  return (n1.pitch > n2.pitch);
                  }
            } pitchComparator;

      for (auto it = startBarChordIt; it != endBarChordIt; ++it) {
            auto &midiNotes = it->second.notes;
            std::sort(midiNotes.begin(), midiNotes.end(), pitchComparator);
            }
      }

void sortTupletsByAveragePitch(std::vector<TupletInfo> &tuplets)
      {
      struct {
            bool operator()(const TupletInfo &t1, const TupletInfo &t2)
                  {
                  return (averagePitch(t1.chords) > averagePitch(t2.chords));
                  }
            } averagePitchComparator;
      std::sort(tuplets.begin(), tuplets.end(), averagePitchComparator);
      }

std::set<std::pair<const ReducedFraction, MidiChord> *>
findTupletChords(const std::vector<TupletInfo> &tuplets)
      {
      std::set<std::pair<const ReducedFraction, MidiChord> *> tupletChords;
      for (const auto &tupletInfo: tuplets) {
            for (const auto &tupletChord: tupletInfo.chords) {
                  auto tupletIt = tupletChord.second;
                  tupletChords.insert(&*tupletIt);
                  }
            }
      return tupletChords;
      }

std::map<std::pair<const ReducedFraction, MidiChord> *, int>
findMappedTupletChords(const std::vector<TupletInfo> &tuplets)
      {
                  // <chord address, tupletIndex>
      std::map<std::pair<const ReducedFraction, MidiChord> *, int> tupletChords;
      for (int i = 0; i != (int)tuplets.size(); ++i) {
            for (const auto &tupletChord: tuplets[i].chords) {
                  auto tupletIt = tupletChord.second;
                  tupletChords.insert({&*tupletIt, i});
                  }
            }
      return tupletChords;
      }

std::list<std::multimap<ReducedFraction, MidiChord>::iterator>
findNonTupletChords(const std::vector<TupletInfo> &tuplets,
                    const std::multimap<ReducedFraction, MidiChord>::iterator &startBarChordIt,
                    const std::multimap<ReducedFraction, MidiChord>::iterator &endBarChordIt)
      {
      const auto tupletChords = findTupletChords(tuplets);
      std::list<std::multimap<ReducedFraction, MidiChord>::iterator> nonTuplets;
      for (auto it = startBarChordIt; it != endBarChordIt; ++it) {
            if (tupletChords.find(&*it) == tupletChords.end())
                  nonTuplets.push_back(it);
            }

      return nonTuplets;
      }

// do quantization before checking

bool hasIntersectionWithChord(
            const ReducedFraction &startTick,
            const ReducedFraction &endTick,
            const ReducedFraction &basicQuant,
            const std::multimap<ReducedFraction, MidiChord>::iterator &chord)
      {
      const auto onTime = Quantize::findQuantizedChordOnTime(*chord, basicQuant);
      const auto offTime = Quantize::findMaxQuantizedOffTime(*chord, basicQuant);
      return (endTick > onTime && startTick < offTime);
      }

// split first tuplet chord, that belong to 2 tuplets, into 2 chords

void splitTupletChord(const std::vector<TupletInfo>::iterator &lastMatch,
                      std::multimap<ReducedFraction, MidiChord> &chords)
      {
      auto &chordEvent = lastMatch->chords.begin()->second;
      MidiChord &prevChord = chordEvent->second;
      const auto onTime = chordEvent->first;
      MidiChord newChord = prevChord;
                        // erase all notes except the first one
      auto beg = newChord.notes.begin();
      newChord.notes.erase(++beg, newChord.notes.end());
                        // erase the first note
      prevChord.notes.erase(prevChord.notes.begin());
      chordEvent = chords.insert({onTime, newChord});
      if (prevChord.notes.isEmpty()) {
                        // normally this should not happen at all because of filtering of tuplets
            qDebug("Tuplets were not filtered correctly: same notes in different tuplets");
            }
      }

void setTupletVoice(
            std::map<ReducedFraction,
                     std::multimap<ReducedFraction, MidiChord>::iterator> &tupletChords,
            int voice)
      {
      for (auto &tupletChord: tupletChords) {
            MidiChord &midiChord = tupletChord.second->second;
            midiChord.voice = voice;
            midiChord.isInTuplet = true;
            }
      }

void splitFirstTupletChords(std::vector<TupletInfo> &tuplets,
                            std::multimap<ReducedFraction, MidiChord> &chords)
      {
      for (auto now = tuplets.begin(); now != tuplets.end(); ++now) {
            auto lastMatch = tuplets.end();
            const auto nowChordIt = now->chords.begin();
            for (auto prev = tuplets.begin(); prev != now; ++prev) {
                  auto prevChordIt = prev->chords.begin();
                  if (now->firstChordIndex == 0
                              && prev->firstChordIndex == 0
                              && nowChordIt->second == prevChordIt->second) {
                        lastMatch = prev;
                        }
                  }
            if (lastMatch != tuplets.end())
                  splitTupletChord(lastMatch, chords);
            }
      }

bool canTupletBeTied(const TupletInfo &tuplet,
                     const std::multimap<ReducedFraction, MidiChord>::iterator &chordIt,
                     const ReducedFraction &startBarTick,
                     const ReducedFraction &basicQuant)
      {
      const auto chordOffTime = MChord::maxNoteOffTime(chordIt->second.notes);
      const auto onTime = Quantize::findQuantizedChordOnTime(*chordIt, basicQuant);
      if (onTime >= tuplet.onTime)
            return false;

      const auto tupletOffTime = Quantize::findMaxQuantizedOffTime(
                                    *chordIt, basicQuant,
                                    tupletLimits(tuplet.tupletNumber).ratio, startBarTick);
                  // if chord offTime is outside this bar or tuplet - discard chord
      if (tupletOffTime < startBarTick
                  || tupletOffTime <= tuplet.onTime
                  || tupletOffTime >= tuplet.onTime + tuplet.len)
            return false;

      const auto offTime = Quantize::findMaxQuantizedOffTime(*chordIt, basicQuant);
      const auto regularError = (chordOffTime - offTime).absValue();
      const auto tupletError = (chordOffTime - tupletOffTime).absValue();

      if (tupletError <= regularError)
            return true;

      return false;
      }

ReducedFraction findPrevBarStart(const ReducedFraction &barStart,
                                 const ReducedFraction &barLen)
      {
      auto prevBarStart = barStart - barLen;
      if (prevBarStart < ReducedFraction(0, 1))
            prevBarStart = ReducedFraction(0, 1);
      return prevBarStart;
      }

struct TiedTuplet
      {
      int tupletIndex;
      int voice;
      std::pair<const ReducedFraction, MidiChord> *chord;  // chord the tuplet is tied with
      };

std::vector<TiedTuplet>
findBackTiedTuplets(
            const std::multimap<ReducedFraction, MidiChord> &chords,
            const std::vector<TupletInfo> &tuplets,
            const ReducedFraction &prevBarStart,
            const ReducedFraction &startBarTick,
            const ReducedFraction &basicQuant)
      {
      std::vector<TiedTuplet> tiedTuplets;
      const auto tupletChords = findMappedTupletChords(tuplets);
      const auto tupletIntervals = findTupletIntervals(tuplets, basicQuant);

      for (int i = 0; i != (int)tuplets.size(); ++i) {

            Q_ASSERT_X(!tuplets[i].chords.empty(),
                       "MidiTuplets::findBackTiedTuplets", "Tuplet chords are empty");

            auto chordIt = tuplets[i].chords.begin()->second;
            while (chordIt != chords.begin() && chordIt->first >= prevBarStart) {
                  --chordIt;
                  int voice = chordIt->second.voice;  // voice can be from prev bar also
                  bool used = false;
                              // check: if tuplet 'i' already tied then voices must match
                  for (const auto &t: tiedTuplets) {
                        if (t.tupletIndex == i && t.voice != voice) {
                              used = true;
                              break;
                              }
                        if (t.tupletIndex != i && t.voice == voice) {
                              if (haveIntersection(tupletIntervals[i],
                                                   tupletIntervals[t.tupletIndex])) {
                                    used = true;
                                    break;
                                    }
                              }
                        }
                  if (used)
                        continue;
                              // don't make back tie to the chord in overlapping tuplet
                  const auto tupletIt = tupletChords.find(&*chordIt);
                  if (tupletIt != tupletChords.end()) {
                        const auto onTime1 = tuplets[tupletIt->second].onTime;
                        const auto endTime1 = onTime1 + tuplets[tupletIt->second].len;
                        const auto onTime2 = tuplets[i].onTime;
                        const auto endTime2 = onTime1 + tuplets[i].len;
                        if (endTime1 > onTime2 && onTime1 < endTime2) // tuplet intersection
                              continue;
                        }
                  if (canTupletBeTied(tuplets[i], chordIt, startBarTick, basicQuant)) {
                        tiedTuplets.push_back({i, voice, &*chordIt});
                        break;
                        }
                  }
            }
      return tiedTuplets;
      }

// first tuplet notes with offTime quantization error,
// that is greater for tuplet quant rather than for regular quant,
// are removed from tuplet, except that was the last note

// if noteLen <= tupletLen
//     remove notes with big offTime quant error;
//     and here is a tuning for clearer tuplet processing:
//         if tuplet has only one (first) chord -
//             we can remove all notes and erase tuplet;
//         if tuplet has multiple chords -
//             we should leave at least one note in the first chord

void minimizeOffTimeError(
            std::vector<TupletInfo> &tuplets,
            std::multimap<ReducedFraction, MidiChord> &chords,
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            const ReducedFraction &startBarTick,
            const ReducedFraction &basicQuant)
      {
      for (auto it = tuplets.begin(); it != tuplets.end(); ) {
            TupletInfo &tupletInfo = *it;
            const auto firstChord = tupletInfo.chords.begin();
            if (firstChord == tupletInfo.chords.end() || tupletInfo.firstChordIndex != 0) {
                  ++it;
                  continue;
                  }
            auto onTime = firstChord->second->first;
                        // because of tol onTime can be less than start bar tick
            if (onTime < startBarTick)
                  onTime = startBarTick;
            MidiChord &midiChord = firstChord->second->second;
            auto &notes = midiChord.notes;

            std::vector<int> removedIndexes;
            std::vector<int> leavedIndexes;
            for (int i = 0; i != notes.size(); ++i) {
                  const auto &note = notes[i];
                  if (note.offTime - onTime <= tupletInfo.len) {
                        if ((tupletInfo.chords.size() == 1
                                    && notes.size() > (int)removedIndexes.size())
                                 || (tupletInfo.chords.size() > 1
                                    && notes.size() > (int)removedIndexes.size() + 1))
                              {
                              const auto tupletError = Quantize::findOffTimeQuantError(
                                                *firstChord->second, note.offTime, basicQuant,
                                                tupletLimits(tupletInfo.tupletNumber).ratio, startBarTick);
                              const auto regularError = Quantize::findOffTimeQuantError(
                                                *firstChord->second, note.offTime, basicQuant);

                              if (tupletError > regularError) {
                                    removedIndexes.push_back(i);
                                    continue;
                                    }
                              }
                        }
                  leavedIndexes.push_back(i);
                  }
            if (!removedIndexes.empty()) {
                  MidiChord newTupletChord;
                  for (int i: leavedIndexes)
                        newTupletChord.notes.push_back(notes[i]);

                  QList<MidiNote> newNotes;
                  for (int i: removedIndexes)
                        newNotes.push_back(notes[i]);
                  notes = newNotes;
                  nonTuplets.push_back(firstChord->second);
                  if (!newTupletChord.notes.empty())
                        firstChord->second = chords.insert({onTime, newTupletChord});
                  else {
                        tupletInfo.chords.erase(tupletInfo.chords.begin());
                        if (tupletInfo.chords.empty()) {
                              it = tuplets.erase(it);   // remove tuplet without chords
                              continue;
                              }
                        }
                  }
            ++it;
            }
      }

void addChordsBetweenTupletNotes(
            std::vector<TupletInfo> &tuplets,
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            const ReducedFraction &startBarTick,
            const ReducedFraction &basicQuant)
      {
      for (TupletInfo &tuplet: tuplets) {
            for (auto it = nonTuplets.begin(); it != nonTuplets.end(); ) {
                  const auto &chordIt = *it;
                  const auto &onTime = chordIt->first;
                  if (onTime > tuplet.onTime
                              && hasIntersectionWithChord(tuplet.onTime, tuplet.onTime + tuplet.len,
                                                          basicQuant, chordIt)) {
                        const auto tupletRatio = tupletLimits(tuplet.tupletNumber).ratio;

                        auto tupletError = Quantize::findOnTimeQuantError(
                                                  *chordIt, basicQuant, tupletRatio, startBarTick);
                        auto regularError = Quantize::findOnTimeQuantError(*chordIt, basicQuant);

                        const auto offTime = MChord::maxNoteOffTime(chordIt->second.notes);
                        if (offTime < tuplet.onTime + tuplet.len) {
                              tupletError += Quantize::findOffTimeQuantError(
                                                *chordIt, offTime, basicQuant,
                                                tupletRatio, startBarTick);
                              regularError += Quantize::findOffTimeQuantError(
                                                *chordIt, offTime, basicQuant);
                              }
                        if (tupletError < regularError) {
                              tuplet.chords.insert({onTime, chordIt});
                              it = nonTuplets.erase(it);
                              continue;
                              }
                        }
                  ++it;
                  }
            }
      }

std::pair<ReducedFraction, ReducedFraction>
chordInterval(const std::pair<const ReducedFraction, MidiChord> &chord,
              const ReducedFraction &basicQuant)
      {
      const auto onTime = Quantize::findMinQuantizedOnTime(chord, basicQuant);
      const auto offTime = Quantize::findMaxQuantizedOffTime(chord, basicQuant);
      return std::make_pair(onTime, offTime);
      }

void setTupletVoices(
            std::vector<TupletInfo> &tuplets,
            std::set<int> &pendingTuplets,
            std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction>>> &tupletIntervals,
            const ReducedFraction &basicQuant)
      {
      int limit = tupletVoiceLimit();
      int voice = 0;
      while (!pendingTuplets.empty() && voice < limit) {
            for (auto it = pendingTuplets.begin(); it != pendingTuplets.end(); ) {
                  int i = *it;
                  const auto interval = tupletInterval(tuplets[i], basicQuant);
                  if (!haveIntersection(interval, tupletIntervals[voice])) {
                        setTupletVoice(tuplets[i].chords, voice);
                        tupletIntervals[voice].push_back(interval);
                        it = pendingTuplets.erase(it);
                        continue;
                        }
                  ++it;
                  }
            ++voice;
            }
      }

void setNonTupletVoices(
            std::set<std::pair<const ReducedFraction, MidiChord> *> &pendingNonTuplets,
            const std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction>>> &tupletIntervals,
            const ReducedFraction &basicQuant)
      {
      const int limit = voiceLimit();
      int voice = 0;
      while (!pendingNonTuplets.empty() && voice < limit) {
            for (auto it = pendingNonTuplets.begin(); it != pendingNonTuplets.end(); ) {
                  auto chord = *it;
                  const auto interval = chordInterval(*chord, basicQuant);
                  const auto fit = tupletIntervals.find(voice);
                  if (fit == tupletIntervals.end() || !haveIntersection(interval, fit->second)) {
                        chord->second.voice = voice;
                        it = pendingNonTuplets.erase(it);
                                    // don't insert chord interval here
                        continue;
                        }
                  ++it;
                  }
            ++voice;
            }
      }

void removeUnusedTuplets(
            std::vector<TupletInfo> &tuplets,
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            std::set<int> &pendingTuplets,
            std::set<std::pair<const ReducedFraction, MidiChord> *> &pendingNonTuplets)
      {
      if (pendingTuplets.empty())
            return;

      std::vector<TupletInfo> newTuplets;
      for (int i = 0; i != (int)tuplets.size(); ++i) {
            if (pendingTuplets.find(i) == pendingTuplets.end()) {
                  newTuplets.push_back(tuplets[i]);
                  }
            else {
                  for (const auto &chord: tuplets[i].chords) {
                        nonTuplets.push_back(chord.second);
                        pendingNonTuplets.insert(&*chord.second);
                        }
                  }
            }
      pendingTuplets.clear();
      std::swap(tuplets, newTuplets);
      }


#ifdef QT_DEBUG

bool haveTupletsEmptyChords(const std::vector<TupletInfo> &tuplets)
      {
      for (const auto &tuplet: tuplets) {
            if (tuplet.chords.empty())
                  return true;
            }
      return false;
      }

bool doTupletChordsHaveSameVoice(const std::vector<TupletInfo> &tuplets)
      {
      for (const auto &tuplet: tuplets) {
            auto it = tuplet.chords.cbegin();
            const int voice = it->second->second.voice;
            ++it;
            for ( ; it != tuplet.chords.cend(); ++it) {
                  if (it->second->second.voice != voice)
                        return false;
                  }
            }
      return true;
      }

// back tied tuplets are not checked here

bool haveOverlappingVoices(
            const std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            const std::vector<TupletInfo> &tuplets,
            const ReducedFraction &basicQuant,
            const std::vector<TiedTuplet> &backTiedTuplets = std::vector<TiedTuplet>())
      {
                  // <voice, intervals>
      std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction>>> intervals;

      for (const auto &tuplet: tuplets) {
            const int voice = tuplet.chords.begin()->second->second.voice;
            const auto interval = std::make_pair(tuplet.onTime, tuplet.onTime + tuplet.len);
            if (haveIntersection(interval, intervals[voice]))
                  return true;
            else
                  intervals[voice].push_back(interval);
            }

      for (const auto &chord: nonTuplets) {
            const int voice = chord->second.voice;
            const auto interval = chordInterval(*chord, basicQuant);
            if (haveIntersection(interval, intervals[voice])) {
                  bool flag = false;      // if chord is tied then it can intersect tuplet
                  for (const TiedTuplet &tiedTuplet: backTiedTuplets) {
                        if (tiedTuplet.chord == (&*chord) && tiedTuplet.voice == voice) {
                              flag = true;
                              break;
                              }
                        }
                  if (!flag)
                        return true;
                  }
            }

      return false;
      }

bool doTupletsHaveCommonChords(const std::vector<TupletInfo> &tuplets)
      {
      if (tuplets.empty())
            return false;
      std::set<std::pair<const ReducedFraction, MidiChord> *> chordsI;
      for (const auto &tuplet: tuplets) {
            for (const auto &chord: tuplet.chords) {
                  if (chordsI.find(&*chord.second) != chordsI.end())
                        return true;
                  chordsI.insert(&*chord.second);
                  }
            }
      return false;
      }

#endif


std::vector<std::pair<ReducedFraction, ReducedFraction> >
findNonTupletIntervals(
            const std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            const ReducedFraction &basicQuant)
      {
      std::vector<std::pair<ReducedFraction, ReducedFraction>> nonTupletIntervals;
      for (const auto &nonTuplet: nonTuplets) {
            nonTupletIntervals.push_back(chordInterval(*nonTuplet, basicQuant));
            }
      return nonTupletIntervals;
      }

std::set<int> findPendingTuplets(const std::vector<TupletInfo> &tuplets)
      {
      std::set<int> pendingTuplets;       // tuplet indexes
      for (int i = 0; i != (int)tuplets.size(); ++i) {
            pendingTuplets.insert(i);
            }
      return pendingTuplets;
      }

std::set<std::pair<const ReducedFraction, MidiChord> *>
findPendingNonTuplets(
            const std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets)
      {
      std::set<std::pair<const ReducedFraction, MidiChord> *> pendingNonTuplets;
      for (const auto &c: nonTuplets) {
            pendingNonTuplets.insert(&*c);
            }
      return pendingNonTuplets;
      }

int findTupletWithChord(const MidiChord &midiChord,
                        const std::vector<TupletInfo> &tuplets)
      {
      for (int i = 0; i != (int)tuplets.size(); ++i) {
            for (const auto &chord: tuplets[i].chords) {
                  if (&(chord.second->second) == &midiChord)
                        return i;
                  }
            }
      return -1;
      }

std::vector<std::pair<int, int> >
findForTiedTuplets(
            const std::vector<TupletInfo> &tuplets,
            const std::vector<TiedTuplet> &tiedTuplets,
            const std::set<std::pair<const ReducedFraction, MidiChord> *> &pendingNonTuplets,
            const ReducedFraction &startBarTick)
      {
      std::vector<std::pair<int, int>> forTiedTuplets;  // <tuplet index, voice to assign>

      for (const TiedTuplet &tuplet: tiedTuplets) {
                        // only for chords in the current bar (because of tol some can be outside)
            if (tuplet.chord->first < startBarTick)
                  continue;
            if (pendingNonTuplets.find(tuplet.chord) == pendingNonTuplets.end()) {
                  const int i = findTupletWithChord(tuplet.chord->second, tuplets);
                  if (i != -1)
                        forTiedTuplets.push_back({i, tuplet.voice});
                  }
            }
      return forTiedTuplets;
      }


#ifdef QT_DEBUG

bool areAllElementsUnique(
            const std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets)
      {
      std::set<std::pair<const ReducedFraction, MidiChord> *> chords;
      for (const auto &chord: nonTuplets) {
            if (chords.find(&*chord) == chords.end())
                  chords.insert(&*chord);
            else
                  return false;
            }
      return true;
      }

size_t chordCount(
            const std::vector<TupletInfo> &tuplets,
            const std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets)
      {
      size_t sum = nonTuplets.size();
      for (const auto &tuplet: tuplets) {
            sum += tuplet.chords.size();
            }
      return sum;
      }

#endif


// for the case !useMultipleVoices

void excludeExtraVoiceTuplets(
            std::vector<TupletInfo> &tuplets,
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            const ReducedFraction &basicQuant)
      {
                  // remove overlapping tuplets
      size_t sz = tuplets.size();
      if (sz == 0)
            return;
      while (true) {
            bool change = false;
            for (size_t i = 0; i < sz - 1; ++i) {
                  const auto interval1 = tupletInterval(tuplets[i], basicQuant);
                  for (size_t j = i + 1; j < sz; ++j) {
                        const auto interval2 = tupletInterval(tuplets[j], basicQuant);
                        if (haveIntersection(interval1, interval2)) {
                              --sz;
                              if (j < sz)
                                    tuplets[j] = tuplets[sz];
                              --sz;
                              if (i < sz)
                                    tuplets[i] = tuplets[sz];
                              change = true;
                              break;
                              }
                        }
                  if (change)
                        break;
                  }
            if (!change || sz == 0)
                  break;
            }

      if (sz > 0) {     // remove tuplets that are overlapped with non-tuplets
            const auto nonTupletIntervals = findNonTupletIntervals(nonTuplets, basicQuant);

            for (size_t i = 0; i < sz; ) {
                  const auto interval = tupletInterval(tuplets[i], basicQuant);
                  if (haveIntersection(interval, nonTupletIntervals)) {
                        for (const auto &chord: tuplets[i].chords)
                              nonTuplets.push_back(chord.second);
                        --sz;
                        if (i < sz) {
                              tuplets[i] = tuplets[sz];
                              continue;
                              }
                        }
                  ++i;
                  }
            }

      Q_ASSERT_X(areAllElementsUnique(nonTuplets),
                 "MIDI tuplets: excludeExtraVoiceTuplets", "non unique chords in non-tuplets");

      tuplets.resize(sz);
      }

void assignVoices(
            std::vector<TupletInfo> &tuplets,
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            const std::vector<TiedTuplet> &backTiedTuplets,
            const ReducedFraction &startBarTick,
            const ReducedFraction &basicQuant)
      {
#ifdef QT_DEBUG
      size_t oldChordCount = chordCount(tuplets, nonTuplets);
#endif
      Q_ASSERT_X(!haveTupletsEmptyChords(tuplets),
                 "MIDI tuplets: assignVoices", "Empty tuplet chords");

      auto pendingTuplets = findPendingTuplets(tuplets);
      auto pendingNonTuplets = findPendingNonTuplets(nonTuplets);
      const auto forTiedTuplets = findForTiedTuplets(tuplets, backTiedTuplets,
                                                     pendingNonTuplets, startBarTick);
      std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction>>> tupletIntervals;

      for (const auto &t: forTiedTuplets) {
            const int i = t.first;
            const int voice = t.second;
            setTupletVoice(tuplets[i].chords, voice);
                        // remove tuplets with already set voices
            pendingTuplets.erase(i);
            tupletIntervals[voice].push_back(tupletInterval(tuplets[i], basicQuant));
            }

      for (const TiedTuplet &tuplet: backTiedTuplets) {
            setTupletVoice(tuplets[tuplet.tupletIndex].chords, tuplet.voice);
            pendingTuplets.erase(tuplet.tupletIndex);
            tupletIntervals[tuplet.voice].push_back(
                              tupletInterval(tuplets[tuplet.tupletIndex], basicQuant));
                        // set for-tied chords
                        // some chords can be the same as in forTiedTuplets

                  // only for chords in the current bar (because of tol some can be outside)
            if (tuplet.chord->first < startBarTick)
                  continue;
            tuplet.chord->second.voice = tuplet.voice;
                        // remove chords with already set voices
            pendingNonTuplets.erase(tuplet.chord);
            }

      {
      setTupletVoices(tuplets, pendingTuplets, tupletIntervals, basicQuant);

      Q_ASSERT_X((voiceLimit() == 1) ? pendingTuplets.empty() : true,
                 "MIDI tuplets: assignVoices", "Unused tuplets for the case !useMultipleVoices");

      removeUnusedTuplets(tuplets, nonTuplets, pendingTuplets, pendingNonTuplets);
      setNonTupletVoices(pendingNonTuplets, tupletIntervals, basicQuant);
      }

      Q_ASSERT_X(pendingNonTuplets.empty(),
                 "MIDI tuplets: assignVoices", "Unused non-tuplets");
      Q_ASSERT_X(!haveTupletsEmptyChords(tuplets),
                 "MIDI tuplets: assignVoices", "Empty tuplet chords");
      Q_ASSERT_X(doTupletChordsHaveSameVoice(tuplets),
                 "MIDI tuplets: assignVoices", "Tuplet chords have different voices");
      Q_ASSERT_X(!haveOverlappingVoices(nonTuplets, tuplets, basicQuant, backTiedTuplets),
                 "MIDI tuplets: assignVoices", "Overlapping tuplets of the same voice");
#ifdef QT_DEBUG
      size_t newChordCount = chordCount(tuplets, nonTuplets);
#endif
      Q_ASSERT_X(oldChordCount == newChordCount,
                 "MIDI tuplets: assignVoices", "Chord count is not preserved");
      }

std::vector<TupletData> convertToData(const std::vector<TupletInfo> &tuplets)
      {
      std::vector<TupletData> tupletsData;
      for (const auto &tupletInfo: tuplets) {
            MidiTuplet::TupletData tupletData = {tupletInfo.chords.begin()->second->second.voice,
                                                 tupletInfo.onTime,
                                                 tupletInfo.len,
                                                 tupletInfo.tupletNumber,
                                                 {}};
            tupletsData.push_back(tupletData);
            }
      return tupletsData;
      }

// check is the chord already in tuplet in prev bar or division
// it's possible because we use (startDivTick - tol) as a start tick

template <typename Iter>
Iter findTupletFreeChord(const Iter &startChordIt,
                         const Iter &endChordIt,
                         const ReducedFraction &startDivTick)
      {
      auto result = startChordIt;
      for (auto it = startChordIt; it != endChordIt && it->first < startDivTick; ++it) {
            if (it->second.isInTuplet)
                  result = std::next(it);
            }
      return result;
      }

void resetTupletVoices(std::vector<TupletInfo> &tuplets)
      {
      for (auto &tuplet: tuplets) {
            for (auto &chord: tuplet.chords) {
                  MidiChord &midiChord = chord.second->second;
                  midiChord.voice = 0;
                  }
            }
      }

// detect staccato notes; later sum length of rests of this notes
// will be reduced by enlarging the length of notes to the tuplet note length

void detectStaccato(TupletInfo &tuplet)
      {
      if ((int)tuplet.chords.size() >= tupletLimits(tuplet.tupletNumber).minNoteCountStaccato) {
            const auto tupletNoteLen = tuplet.len / tuplet.tupletNumber;
            for (auto &chord: tuplet.chords) {
                  MidiChord &midiChord = chord.second->second;
                  for (int i = 0; i != midiChord.notes.size(); ++i) {
                        if (midiChord.notes[i].offTime - chord.first < tupletNoteLen / 2) {
                                    // later if chord have one or more notes
                                    // with staccato -> entire chord is staccato

                                    // don't mark note as staccato here, only remember it
                                    // because different tuplets may contain this note,
                                    // it will be resolved after tuplet filtering
                              tuplet.staccatoChords.insert({chord.first, i});
                              }
                        }
                  }
            }
      }

// this function is needed because if there are additional chords
// that can be in the middle between tuplet chords,
// and tuplet chords are staccato, i.e. have short length,
// then such a long tuplet with lots of short chords
// would be not pretty-looked converted to notation

template <typename Iter>
bool haveChordsInTheMiddleBetweenTupletChords(
            const Iter &startDivChordIt,
            const Iter &endDivChordIt,
            const TupletInfo &tuplet)
      {
      const auto tupletNoteLen = tuplet.len / tuplet.tupletNumber;
      for (auto it = startDivChordIt; it != endDivChordIt; ++it) {
            for (int i = 0; i != tuplet.tupletNumber; ++i) {
                  const auto pos = tuplet.onTime + tupletNoteLen * i + tupletNoteLen / 2;
                  if ((pos - it->first).absValue() < tupletNoteLen / 2)
                        return true;
                  }
            }
      return false;
      }

void findTupletQuantizedOffTime(
            std::vector<TupletInfo> &tuplets,
            const ReducedFraction &barStart,
            const ReducedFraction &barEnd,
            const ReducedFraction &basicQuant)
      {
      for (auto &tuplet: tuplets) {
            const auto tupletNoteLen = tuplet.len / tuplet.tupletNumber;
            const auto tupletRatio = tupletLimits(tuplet.tupletNumber).ratio;
            for (auto it = tuplet.chords.begin(); it != tuplet.chords.end(); ++it) {
                  MidiChord &midiChord = it->second->second;
                  for (auto &note: midiChord.notes) {
                        if (note.staccato) {
                                    // decrease tuplet error by enlarging staccato notes:
                                    // make note.len = tuplet note length
                              auto offTime = Quantize::findQuantizedNoteOffTime(
                                          *it->second, it->first + tupletNoteLen, basicQuant,
                                          tupletRatio, barStart);
                              auto next = std::next(it);
                              if (next != tuplet.chords.end()) {
                                    const auto nextOnTime = next->second->second.quantizedOnTime;

                                    Q_ASSERT_X(nextOnTime != ReducedFraction(-1, 1),
                                         "MidiTuplet::findTupletQuantizedOffTime",
                                         "Tuplet onTime is not quantized but it should at this time");

                                    if (offTime > nextOnTime)
                                          offTime = nextOnTime;
                                    }
                              note.quantizedOffTime = offTime;
                              }
                        else {
                              if (note.offTime <= tuplet.onTime + tuplet.len) {
                                    note.quantizedOffTime = Quantize::findQuantizedNoteOffTime(
                                                *it->second, note.offTime, basicQuant,
                                                tupletRatio, barStart);
                                    }
                              else if (note.offTime == ReducedFraction(-1, 1) // unset yet
                                              && note.offTime <= barEnd) {    // belongs to this bar
                                    note.quantizedOffTime = Quantize::findQuantizedNoteOffTime(
                                                       *it->second, note.offTime, basicQuant);
                                    }
                              // outside bar quantization cases will be considered later
                              }
                        }
                  }
            }
      }

void markStaccatoTupletNotes(std::vector<TupletInfo> &tuplets)
      {
      for (auto &tuplet: tuplets) {
            for (const auto &staccato: tuplet.staccatoChords) {
                  const auto it = tuplet.chords.find(staccato.first);
                  if (it != tuplet.chords.end()) {
                        MidiChord &midiChord = it->second->second;
                        midiChord.notes[staccato.second].staccato = true;
                        }
                  }
            tuplet.staccatoChords.clear();
            }
      }

// if notes with staccato were in tuplets previously - remove staccato

void cleanStaccatoOfNonTuplets(
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets)
      {
      for (auto &nonTuplet: nonTuplets) {
            for (auto &note: nonTuplet->second.notes) {
                  if (note.staccato)
                        note.staccato = false;
                  }
            }
      }

void findTupletQuantizedOnTime(
            std::vector<TupletInfo> &tuplets,
            const ReducedFraction &startBarTick,
            const ReducedFraction &basicQuant)
      {
      for (auto &tuplet: tuplets) {
            for (auto &chord: tuplet.chords) {
                  if (chord.first < startBarTick) {
                        chord.second->second.quantizedOnTime = startBarTick;
                        }
                  else {
                        chord.second->second.quantizedOnTime = Quantize::findQuantizedChordOnTime(
                                          *chord.second, basicQuant,
                                          tupletLimits(tuplet.tupletNumber).ratio, startBarTick);
                        }

                  Q_ASSERT_X(chord.second->second.quantizedOnTime >= tuplet.onTime,
                             "MidiTuplet::findTupletQuantizedOnTime",
                             "Chord onTime value is less than tuplet begin time");
                  }
            }
      }

void findNonTupletQuantizedOnTime(
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            const ReducedFraction &basicQuant)
      {
      for (auto &nonTuplet: nonTuplets) {
            nonTuplet->second.quantizedOnTime = Quantize::findQuantizedChordOnTime(
                                                      *nonTuplet, basicQuant);
            }
      }

void findNonTupletQuantizedOffTime(
            std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
            const ReducedFraction &basicQuant,
            const ReducedFraction &endBarTick)
      {
      for (auto &nonTuplet: nonTuplets) {
            for (auto &note: nonTuplet->second.notes) {
                        // if offTime is already set or belongs to another bar - go to next note
                  if (note.quantizedOffTime == ReducedFraction(-1, 1)
                              && note.offTime <= endBarTick) {
                        note.quantizedOffTime = Quantize::findQuantizedNoteOffTime(
                                          *nonTuplet, note.offTime, basicQuant);
                        }
                  // outside bar quantization cases will be considered later
                  }
            }
      }

void findTiedQuantizedOffTime(const std::vector<TiedTuplet> &backTiedTuplets,
                              const std::vector<TupletInfo> &tuplets,
                              const ReducedFraction &startBarTick,
                              const ReducedFraction &basicQuant)
      {
      for (const TiedTuplet &tuplet: backTiedTuplets) {
            for (auto &note: tuplet.chord->second.notes) {
                  if (note.offTime > tuplets[tuplet.tupletIndex].onTime) {

                        Q_ASSERT_X(note.quantizedOffTime == ReducedFraction(-1, 1),
                                   "MidiTuplet::findTiedQuantizedOffTime",
                                   "Note quantized off time is already set");

                        note.quantizedOffTime = Quantize::findQuantizedNoteOffTime(
                                    *tuplet.chord, note.offTime, basicQuant,
                                    tupletLimits(tuplets[tuplet.tupletIndex].tupletNumber).ratio,
                                    startBarTick);
                        }
                  }
            }
      }

bool isTupletLenAllowed(
            const ReducedFraction &tupletLen,
            int tupletNumber,
            const std::multimap<ReducedFraction, MidiChord>::const_iterator beg,
            const std::multimap<ReducedFraction, MidiChord>::const_iterator end,
            const ReducedFraction &basicQuant)
      {
      const auto tupletNoteLen = tupletLen / tupletNumber;
      const auto regularQuant = Quantize::findQuantForRange(beg, end, basicQuant, {1, 1});
      return tupletNoteLen >= regularQuant;
      }

std::vector<TupletData> findTuplets(
            const ReducedFraction &startBarTick,
            const ReducedFraction &endBarTick,
            const ReducedFraction &barFraction,
            std::multimap<ReducedFraction, MidiChord> &chords,
            const ReducedFraction &basicQuant)
      {
      if (chords.empty() || startBarTick >= endBarTick)     // invalid cases
            return std::vector<TupletData>();
      const auto operations = preferences.midiImportOperations.currentTrackOperations();
      if (!operations.tuplets.doSearch)
            return std::vector<TupletData>();
      auto startBarChordIt = MChord::findFirstChordInRange(chords, startBarTick, endBarTick);
      startBarChordIt = findTupletFreeChord(startBarChordIt, chords.end(), startBarTick);
      if (startBarChordIt == chords.end())      // no chords in this bar
            return std::vector<TupletData>();

      const auto endBarChordIt = chords.lower_bound(endBarTick);
      const auto tol = basicQuant / 2;
                  // update start chord: use chords with onTime >= (start bar tick - tol)
      startBarChordIt = MChord::findFirstChordInRange(chords, startBarTick - tol, endBarTick);
      const auto divLengths = Meter::divisionsOfBarForTuplets(barFraction);

      std::vector<TupletInfo> tuplets;
      for (const auto &divLen: divLengths) {
            const auto tupletNumbers = findTupletNumbers(divLen, barFraction);
            const auto div = barFraction / divLen;
            const int divCount = div.numerator() / div.denominator();

            for (int i = 0; i != divCount; ++i) {
                  const auto startDivTime = startBarTick + divLen * i;
                  const auto endDivTime = startBarTick + divLen * (i + 1);
                              // check which chords can be inside tuplet period
                              // [startDivTime - tol, endDivTime]
                  auto startDivChordIt = MChord::findFirstChordInRange(startDivTime - tol, endDivTime,
                                                                       startBarChordIt, endBarChordIt);
                  startDivChordIt = findTupletFreeChord(startDivChordIt, endBarChordIt, startDivTime);
                  if (startDivChordIt == endBarChordIt)
                        continue;

                  Q_ASSERT_X(startDivChordIt->second.isInTuplet == false,
                             "MIDI tuplets: findTuplets", "Voice of the chord has been already set");

                              // end iterator, as usual, point to the next - invalid chord
                  const auto endDivChordIt = chords.lower_bound(endDivTime);
                              // try different tuplets, nested tuplets are not allowed
                  for (const auto &tupletNumber: tupletNumbers) {
                        if (!isTupletLenAllowed(divLen, tupletNumber, startDivChordIt, endDivChordIt,
                                                basicQuant)) {
                              continue;
                              }
                        auto tupletInfo = findTupletApproximation(divLen, tupletNumber,
                                             basicQuant, startDivTime, startDivChordIt, endDivChordIt);

                        if (!haveChordsInTheMiddleBetweenTupletChords(
                                          startDivChordIt, endDivChordIt, tupletInfo)) {
                              detectStaccato(tupletInfo);
                              }
                        tupletInfo.sumLengthOfRests = findSumLengthOfRests(
                                                            tupletInfo, startBarTick, basicQuant);

                        if (!isTupletAllowed(tupletInfo))
                              continue;
                        tuplets.push_back(tupletInfo);   // tuplet found
                        }      // next tuplet type
                  }
            }

      filterTuplets(tuplets, basicQuant);

            // later notes will be sorted and their indexes become invalid
            // so assign staccato information to notes now
      markStaccatoTupletNotes(tuplets);

            // because of tol for non-tuplets we should use only chords with onTime >= bar start
      auto startNonTupletChordIt = startBarChordIt;
      while (startNonTupletChordIt->first < startBarTick)
            ++startNonTupletChordIt;
      auto nonTuplets = findNonTupletChords(tuplets, startNonTupletChordIt, endBarChordIt);
      if (tupletVoiceLimit() == 1)
            excludeExtraVoiceTuplets(tuplets, nonTuplets, basicQuant);

      resetTupletVoices(tuplets);  // because of tol some chords may have non-zero voices
      addChordsBetweenTupletNotes(tuplets, nonTuplets, startBarTick, basicQuant);
      sortNotesByPitch(startBarChordIt, endBarChordIt);
      sortTupletsByAveragePitch(tuplets);

      if (operations.useMultipleVoices) {
            splitFirstTupletChords(tuplets, chords);
            minimizeOffTimeError(tuplets, chords, nonTuplets, startBarTick, basicQuant);
            }

      cleanStaccatoOfNonTuplets(nonTuplets);

      Q_ASSERT_X(!doTupletsHaveCommonChords(tuplets),
                 "MIDI tuplets: findTuplets", "Tuplets have common chords but they shouldn't");
      Q_ASSERT_X((voiceLimit() == 1)
                        ? !haveOverlappingVoices(nonTuplets, tuplets, basicQuant)
                        : true,
                 "MIDI tuplets: findTuplets",
                 "Overlapping tuplet and non-tuplet voices for the case !useMultipleVoices");

      const auto prevBarStart = findPrevBarStart(startBarTick, endBarTick - startBarTick);
      const auto backTiedTuplets = findBackTiedTuplets(chords, tuplets, prevBarStart,
                                                       startBarTick, basicQuant);
      assignVoices(tuplets, nonTuplets, backTiedTuplets, startBarTick, basicQuant);

      findTiedQuantizedOffTime(backTiedTuplets, tuplets, startBarTick, basicQuant);
      findTupletQuantizedOnTime(tuplets, startBarTick, basicQuant);
      findTupletQuantizedOffTime(tuplets, startBarTick, endBarTick, basicQuant);
      findNonTupletQuantizedOnTime(nonTuplets, basicQuant);
      findNonTupletQuantizedOffTime(nonTuplets, basicQuant, endBarTick);

      return convertToData(tuplets);
      }

std::multimap<ReducedFraction, TupletData>
findAllTuplets(std::multimap<ReducedFraction, MidiChord> &chords,
               const TimeSigMap *sigmap,
               const ReducedFraction &lastTick,
               const ReducedFraction &basicQuant)
      {
      std::multimap<ReducedFraction, TupletData> tupletEvents;
      ReducedFraction startBarTick = {0, 1};

      for (int i = 1;; ++i) {       // iterate over all measures by indexes
            const auto endBarTick = ReducedFraction::fromTicks(sigmap->bar2tick(i, 0));
            const auto barFraction = ReducedFraction(sigmap->timesig(startBarTick.ticks()).timesig());
            const auto tuplets = findTuplets(startBarTick, endBarTick, barFraction, chords, basicQuant);
            for (const auto &tupletData: tuplets)
                  tupletEvents.insert({tupletData.onTime, tupletData});
            if (endBarTick > lastTick)
                  break;
            startBarTick = endBarTick;
            }
      return tupletEvents;
      }

// tuplets with no chords are removed
// tuplets with single chord with chord.onTime = tuplet.onTime
//    and chord.len = tuplet.len are removed as well

void removeEmptyTuplets(MTrack &track)
      {
      if (track.tuplets.empty())
            return;
      for (auto it = track.tuplets.begin(); it != track.tuplets.end(); ) {
            const auto &tupletData = it->second;
            bool ok = false;
            for (const auto &chord: track.chords) {
                  if (chord.first >= tupletData.onTime + tupletData.len)
                        break;
                  if (tupletData.voice != chord.second.voice)
                        continue;
                  const ReducedFraction &onTime = chord.first;
                  if (onTime >= tupletData.onTime
                              && onTime < tupletData.onTime + tupletData.len) {
                                    // tuplet contains at least one chord
                                    // check now for notes with len == tupletData.len
                        if (onTime == tupletData.onTime) {
                              for (const auto &note: chord.second.notes) {
                                    if (note.offTime - onTime < tupletData.len) {
                                          ok = true;
                                          break;
                                          }
                                    }
                              }
                        else {
                              ok = true;
                              break;
                              }
                        }
                  }
            if (!ok) {
                  it = track.tuplets.erase(it);
                  continue;
                  }
            ++it;
            }
      }

void addElementToTuplet(int voice,
                        const ReducedFraction &onTime,
                        const ReducedFraction &len,
                        DurationElement *el,
                        std::multimap<ReducedFraction, TupletData> &tuplets)
      {
      const auto it = findTupletForTimeRange(voice, onTime, len, tuplets);
      if (it != tuplets.end()) {
            auto &tuplet = const_cast<TupletData &>(it->second);
            tuplet.elements.push_back(el);       // add chord/rest to the tuplet
            }
      }

void createTuplets(Staff *staff,
                   const std::multimap<ReducedFraction, TupletData> &tuplets)
      {
      Score* score = staff->score();
      const int track = staff->idx() * VOICES;

      for (const auto &tupletEvent: tuplets) {
            const auto &tupletData = tupletEvent.second;
            if (tupletData.elements.empty())
                  continue;

            Tuplet* tuplet = new Tuplet(score);
            const auto &tupletRatio = tupletLimits(tupletData.tupletNumber).ratio;
            tuplet->setRatio(tupletRatio.fraction());

            tuplet->setDuration(tupletData.len.fraction());
            const TDuration baseLen = tupletData.len.fraction() / tupletRatio.denominator();
            tuplet->setBaseLen(baseLen);

            tuplet->setTrack(track);
            tuplet->setTick(tupletData.onTime.ticks());
            Measure* measure = score->tick2measure(tupletData.onTime.ticks());
            tuplet->setParent(measure);

            for (DurationElement *el: tupletData.elements) {
                  tuplet->add(el);
                  el->setTuplet(tuplet);
                  }
            }
      }

} // namespace MidiTuplet
} // namespace Ms
