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
                  for (const auto &note: chordEventIt->second.notes) {
                        if ((note.len - tupletInfo.len / num).absValue() > tupletInfo.regularQuant)
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
                  if (note.len >= tupletNoteLen / 2)
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

ReducedFraction findQuantizationError(const ReducedFraction &value,
                                      const ReducedFraction &raster)
      {
      const auto quantizedValue = Quantize::quantizeValue(value, raster);
      return (value - quantizedValue).absValue();
      }

// return: <chordIter, minChordError>

std::pair<std::multimap<ReducedFraction, MidiChord>::iterator, ReducedFraction>
findBestChordForTupletNote(const ReducedFraction &tupletNotePos,
                           const ReducedFraction &quantValue,
                           const std::multimap<ReducedFraction, MidiChord>::iterator &startChordIt,
                           const std::multimap<ReducedFraction, MidiChord>::iterator &endChordIt)
      {
                  // choose the best chord, if any, for this tuplet note
      std::pair<std::multimap<ReducedFraction, MidiChord>::iterator, ReducedFraction> bestChord;
      bestChord.first = endChordIt;
                  // check chords - whether they can be in tuplet without large error
      bool firstLoop = true;
      for (auto chordIt = startChordIt; chordIt != endChordIt; ++chordIt) {
            auto tupletError = (chordIt->first - tupletNotePos).absValue();
            if (tupletError > quantValue)
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
findTupletsInBarForDuration(int voice,
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
findTupletForTimeRange(int voice,
                       const ReducedFraction &onTime,
                       const ReducedFraction &len,
                       const std::multimap<ReducedFraction, TupletData> &tupletEvents)
      {
      if (tupletEvents.empty() || len < ReducedFraction(0, 1))
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
findTupletContainsTime(int voice,
                       const ReducedFraction &time,
                       const std::multimap<ReducedFraction, TupletData> &tupletEvents)
      {
      return findTupletForTimeRange(voice, time, ReducedFraction(0, 1), tupletEvents);
      }

// find sum length of gaps between successive chords
// less is better

ReducedFraction findSumLengthOfRests(const TupletInfo &tupletInfo)
      {
      auto beg = tupletInfo.onTime;
      const auto tupletEndTime = tupletInfo.onTime + tupletInfo.len;
      const auto tupletNoteLen = tupletInfo.len / tupletInfo.tupletNumber;
      ReducedFraction sumLen = {0, 1};

      for (const auto &chord: tupletInfo.chords) {
            const auto staccatoIt = tupletInfo.staccatoChords.find(chord.first);
            const MidiChord &midiChord = chord.second->second;
            const auto &chordOnTime = Quantize::quantizeValue(chord.second->first,
                                                              tupletInfo.tupletQuant);
            if (beg < chordOnTime)
                  sumLen += (chordOnTime - beg);
            ReducedFraction maxLen(0, 1);
            for (int i = 0; i != midiChord.notes.size(); ++i) {
                  auto noteLen = midiChord.notes[i].len;
                  if (staccatoIt != tupletInfo.staccatoChords.end() && i == staccatoIt->second)
                        noteLen = tupletNoteLen;
                  if (noteLen > maxLen)
                        maxLen = noteLen;
                  }
            beg = chordOnTime + Quantize::quantizeValue(maxLen, tupletInfo.tupletQuant);
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
            const ReducedFraction &regularRaster,
            const ReducedFraction &startTupletTime,
            const std::multimap<ReducedFraction, MidiChord>::iterator &startChordIt,
            const std::multimap<ReducedFraction, MidiChord>::iterator &endChordIt)
      {
      TupletInfo tupletInfo;
      tupletInfo.tupletNumber = tupletNumber;
      tupletInfo.onTime = startTupletTime;
      tupletInfo.len = tupletLen;
      tupletInfo.tupletQuant = tupletLen / tupletNumber;
      while (tupletInfo.tupletQuant / 2 >= regularRaster)
            tupletInfo.tupletQuant /= 2;
      tupletInfo.regularQuant = regularRaster;

      auto startTupletChordIt = startChordIt;
      for (int k = 0; k != tupletNumber; ++k) {
            auto tupletNotePos = startTupletTime + tupletLen / tupletNumber * k;
                        // choose the best chord, if any, for this tuplet note
            auto bestChord = findBestChordForTupletNote(tupletNotePos, regularRaster,
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
            auto regularError = findQuantizationError(bestChord.first->first, regularRaster);
            tupletInfo.regularSumError += regularError;
            }

      return tupletInfo;
      }

bool isMoreTupletVoicesAllowed(int voicesInUse, int availableVoices)
      {
      return !(voicesInUse >= availableVoices || voicesInUse >= tupletVoiceLimit());
      }

void markChordsAsUsed(const TupletInfo &tuplet)
      {
      auto it = tuplet.chords.begin();
      if (tuplet.firstChordIndex == 0) {
            ++(it->second->second.usedVoices);
            ++it;
            }
      for ( ; it != tuplet.chords.end(); ++it)
            it->second->second.usedVoices = VOICES;  // lock other chords
      }

bool areTupletChordsInUse(const TupletInfo &tuplet)
      {
      auto it = tuplet.chords.begin();
      if (tuplet.firstChordIndex == 0) {
            const MidiChord &chord = it->second->second;
                        // check are first tuplet notes all in use (1 note - 1 voice)
            if (!isMoreTupletVoicesAllowed(chord.usedVoices, chord.notes.size()))
                  return true;
            ++it;
            }
      for ( ; it != tuplet.chords.end(); ++it) {
            if (it->second->second.usedVoices != 0)
                  return true;
            }
      return false;
      }

std::pair<ReducedFraction, ReducedFraction>
tupletInterval(const TupletInfo &tuplet)
      {
      ReducedFraction tupletEnd = tuplet.onTime + tuplet.len;

      for (const auto &chord: tuplet.chords) {
            auto offTime = chord.first + MChord::maxNoteLen(chord.second->second.notes);
            offTime = Quantize::quantizeValue(offTime, tuplet.regularQuant);
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

bool haveIntersection(const std::pair<ReducedFraction, ReducedFraction> &interval,
                      const std::vector<std::pair<ReducedFraction, ReducedFraction>> &intervals)
      {
      for (const auto &i: intervals) {
            if (haveIntersection(i, interval))
                  return true;
            }
      return false;
      }

void findValidTuplets(
            std::list<int> &indexes,
            size_t &voiceCount,
            const std::vector<TupletInfo> &tuplets,
            const std::vector<std::pair<ReducedFraction, ReducedFraction>> &tupletIntevals)
      {
                  // <voice, intervals>
      std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction>>> voiceIntervals;
      const int voiceLimit = tupletVoiceLimit();
                  // select tuplets with min average error
      for (auto it = indexes.begin(); it != indexes.end(); ) {
            const auto &tuplet = tuplets[*it];

            Q_ASSERT_X(!tuplet.chords.empty(),
                       "MIDI tuplets: validateTuplets", "Tuplet has no chords but it should");

            int voice = 0;
            for ( ; voice < voiceLimit; ++voice) {
                  if (!haveIntersection(tupletIntevals[*it], voiceIntervals[voice]))
                        break;
                  }
                        // check for chord notes that are already in use in another tuplets
            if (voice == voiceLimit
                        || (voice > 0 && (int)tuplet.chords.size()
                            < tupletLimits(tuplet.tupletNumber).minNoteCountAddVoice)
                        || areTupletChordsInUse(tuplet))
                  {
                  it = indexes.erase(it);
                  continue;
                  }
                        // we can use this tuplet
            markChordsAsUsed(tuplet);
            voiceIntervals[voice].push_back(tupletIntevals[*it]);
            ++it;
            }

      voiceCount = voiceIntervals.size();
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

TupletErrorResult findTupletError(const std::list<int> &indexes,
                                  const std::vector<TupletInfo> &tuplets,
                                  size_t voiceCount)
      {
      ReducedFraction sumError;
      ReducedFraction sumLengthOfRests;
      int sumChordCount = 0;
      int sumChordPlaces = 0;

      std::vector<char> usedIndexes(tuplets.size(), 0);
      for (int i: indexes) {
            sumError += tuplets[i].tupletSumError;
            sumLengthOfRests += tuplets[i].sumLengthOfRests;
            sumChordCount += tuplets[i].chords.size();
            sumChordPlaces += tuplets[i].tupletNumber;
            usedIndexes[i] = 1;
            }

      for (size_t i = 0; i != usedIndexes.size(); ++i) {
            if (usedIndexes[i])
                  continue;
            const auto &tuplet = tuplets[i];
                        // add quant error of all chords excluded from tuplets
            for (const auto &chord: tuplet.chords) {
                                    // in tuplet chord would have usedVoices >= 1
                  if (chord.second->second.usedVoices == 0) {
                        sumError += findQuantizationError(chord.first, tuplet.regularQuant);
                        chord.second->second.usedVoices = 1;  // just to mark as checked
                        }
                  }
            }

      return TupletErrorResult{
                  sumError.numerator() * 1.0 / (sumError.denominator() * sumChordCount),
                  sumChordCount * 1.0 / sumChordPlaces,
                  sumLengthOfRests,
                  voiceCount,
                  indexes.size()
                  };
      }

TupletErrorResult
validateTuplets(std::list<int> &indexes,
                const std::vector<TupletInfo> &tuplets,
                const std::vector<std::pair<ReducedFraction, ReducedFraction>> &tupletIntevals)
      {
      if (tuplets.empty())
            return TupletErrorResult();

      for (auto &tuplet: tuplets) {
            for (auto &chord: tuplet.chords) {
                  MidiChord &midiChord = chord.second->second;
                  midiChord.usedVoices = 0;
                  }
            }
      size_t voiceCount = 0;
      findValidTuplets(indexes, voiceCount, tuplets, tupletIntevals);
      return findTupletError(indexes, tuplets, voiceCount);
      }


#ifdef QT_DEBUG

bool validateSelectedTuplets(const std::list<int> &bestIndexes,
                             const std::vector<TupletInfo> &tuplets)
      {
                  // <chord address, used voices>
      std::map<std::pair<const ReducedFraction, MidiChord> *, int> usedChords;
      for (int i: bestIndexes) {
            const auto &chords = tuplets[i].chords;
            for (auto it = chords.begin(); it != chords.end(); ++it) {
                  bool isFirstChord = (tuplets[i].firstChordIndex == 0
                                       && it == tuplets[i].chords.begin());
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


void TupletCommonIndexes::add(const std::vector<int> &commonIndexes)
      {

      Q_ASSERT_X(!commonIndexes.empty(), "TupletCommonIndexes::add", "Added indexes are empty");

      indexes.push_back(commonIndexes);
      maxCount *= commonIndexes.size();
      current.resize(indexes.size());
      if (counter) {
            for (auto &index: current)
                  index = 0;
            counter = 0;
            }
      }

std::pair<std::vector<int>, bool> TupletCommonIndexes::generateNext()
      {
      std::vector<int> newIndexes(current.size());
      for (int i = 0; i != (int)current.size(); ++i)
            newIndexes[i] = indexes[i][current[i]];
      const bool isLastCombination = (counter == maxCount - 1);

      ++counter;
      if (counter == maxCount)
            counter = 0;

      for (int i = 0; i != (int)current.size(); ++i) {
            ++current[i];
            if (current[i] < (int)indexes[i].size())
                  break;
            current[i] = 0;
            if (i == (int)current.size() - 1) {
                  for (int j = 0; j != i; ++j)
                        current[j] = 0;
                  }
            }

      return {newIndexes, isLastCombination};
      }

// Try different permutations of tuplets (as indexes) to minimize quantization error.
// Because one tuplet can use the same chord as another tuplet -
// the tuplet order is important: once we choose the tuplet,
// we mark all its chords as "used", so next tuplets that use these chords
// will be discarded

std::pair<std::vector<int>, TupletErrorResult>
minimizeQuantError(const std::vector<std::vector<int>> &indexGroups,
                   const std::vector<TupletInfo> &tuplets,
                   const std::vector<std::pair<ReducedFraction, ReducedFraction>> &tupletIntevals)
      {
      TupletErrorResult minResult;
      std::vector<int> iIndexGroups;  // indexes of elements in indexGroups
      for (int i = 0; i != (int)indexGroups.size(); ++i)
            iIndexGroups.push_back(i);
      std::list<int> bestIndexes;
                  // number of permutations grows as n!
                  // 8! = 40320 - quite many; 9! = 362880 - many
                  // so set reasonable max limit to prevent the hang of our program
      const int PERMUTATION_LIMIT = 40320;
      int counter = 0;
      do {
                        // create a list of all tuplet indexes,
                        // the order of them is set by the current permutation
            std::list<int> indexesToValidate;
            for (const auto &i: iIndexGroups) {
                  const auto &group = indexGroups[i];
                  for (const auto &ii: group)
                        indexesToValidate.push_back(ii);
                  }
                        // note: redundant indexes of indexesToValidate are erased here!
            const auto result = validateTuplets(indexesToValidate, tuplets, tupletIntevals);
            if (counter == 0 || result < minResult) {
                  minResult = result;
                  bestIndexes = indexesToValidate;
                  }
            ++counter;
            } while (counter < PERMUTATION_LIMIT &&
                     std::next_permutation(iIndexGroups.begin(), iIndexGroups.end()));

      if (counter >= PERMUTATION_LIMIT)
            qDebug() << "MidiTuplet, minimizeQuantError: permutation limit was exceeded";

      Q_ASSERT_X(validateSelectedTuplets(bestIndexes, tuplets),
                 "MIDI tuplets: minimizeQuantError", "Tuplets have common chords but they shouldn't");

      std::vector<int> resultIndexes;
      for (int i: bestIndexes)
            resultIndexes.push_back(i);

      return {resultIndexes, minResult};
      }

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

std::vector<int> findLongestUncommonGroup(const std::vector<TupletInfo> &tuplets)
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

      std::vector<int> indexes;
      int lastSelected = 0;
      for (int i = 0; i != (int)info.size(); ++i) {
            if (i > 0 && info[i].onTime < info[lastSelected].offTime)
                  continue;
            lastSelected = i;
            indexes.push_back(info[i].index);
            }

      return indexes;
      }

std::vector<int> forceFindUncommonTupletPair(const std::vector<TupletInfo> &tuplets)
      {
      std::vector<int> indexes;

      for (int i = 0; i != (int)tuplets.size() - 1; ++i) {
            for (int j = i + 1; j != (int)tuplets.size(); ++j) {
                  if (!haveCommonChords(i, j, tuplets)) {
                        indexes = {i, j};
                        return indexes;
                        }
                  }
            }

      return indexes;
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

// result can be empty

std::vector<int> findLongestCommonGroup(
            const std::vector<char> &usedIndexes,
            const std::vector<TupletInfo> &tuplets)
      {
                  // <chord address, vector of tuplet indexes>
      std::map<std::pair<const ReducedFraction, MidiChord> *, std::vector<int>> usedChords;
      for (size_t i = 0; i != usedIndexes.size(); ++i) {
            if (usedIndexes[i])
                  continue;
            const auto &tuplet = tuplets[i];
            auto it = tuplet.chords.begin();
            if (tuplet.firstChordIndex == 0
                        && it->second->second.notes.size() > 1
                        && usedChords.find(&*it->second) != usedChords.end())
                  {
                        ++it;
                  }
            for ( ; it != tuplet.chords.end(); ++it)
                  usedChords[&*it->second].push_back(i);
            }
      struct {
            bool operator()(const std::pair<std::pair<const ReducedFraction, MidiChord> *,
                                            std::vector<int>> &p1,
                            const std::pair<std::pair<const ReducedFraction, MidiChord> *,
                                            std::vector<int>> &p2)
                  {
                  return p1.second.size() < p2.second.size();
                  }
            } comparator;

      if (!usedChords.empty())
            return std::max_element(usedChords.begin(), usedChords.end(), comparator)->second;
      return std::vector<int>();
      }

bool haveValidFirstCommonChord(const TupletInfo &t1, const TupletInfo &t2)
      {
      const auto opers = preferences.midiImportOperations.currentTrackOperations();
      return (opers.useMultipleVoices
              && t1.firstChordIndex == 0 && t2.firstChordIndex == 0
              && (&*(t1.chords.begin()->second) == (&*(t2.chords.begin()->second)))
              && t1.chords.begin()->second->second.notes.size() > 1);
      }

bool canBeTogether(int i, int j, const std::vector<TupletInfo> &tuplets)
      {
      std::set<std::pair<const ReducedFraction, MidiChord> *> chordSet;
      auto start1 = tuplets[i].chords.begin();
      auto start2 = tuplets[j].chords.begin();
      if (haveValidFirstCommonChord(tuplets[i], tuplets[j])) {
            ++start1;
            ++start2;
            }
      for (; start1 != tuplets[i].chords.end(); ++start1) {
            chordSet.insert(&*start1->second);
            }
      for (; start2 != tuplets[j].chords.end(); ++start2) {
            if (chordSet.find(&*start2->second) != chordSet.end())
                  return false;
            }
      return true;
      }

void collectRemainingCommonIndexes(
            const std::vector<char> &usedIndexes,
            const std::vector<TupletInfo> &tuplets,
            TupletCommonIndexes &commonIndexes)
      {
      std::vector<int> indexes;
      for (size_t i = 0; i != usedIndexes.size(); ++i) {
            if (usedIndexes[i] == 0)
                  indexes.push_back(i);
            }

      for (int i = 0; i < (int)indexes.size(); ++i) {       // not till size() - 1
            for (int j = i + 1; j < (int)indexes.size(); ++j) {
                  if (!canBeTogether(indexes[i], indexes[j], tuplets)) {
                        commonIndexes.add(std::vector<int>({indexes[i], indexes[j]}));
                        if (i + 1 != j)
                              std::swap(indexes[i + 1], indexes[j]);
                        i += 2;
                        j = i;
                        }
                  }
            if (i < (int)indexes.size())        // check because we can set i == j == size()
                  commonIndexes.add(std::vector<int>({indexes[i]}));
            }
      }

std::vector<int> findUncommonGroup(const std::vector<TupletInfo> &tuplets)
      {
      auto uncommonGroup = findLongestUncommonGroup(tuplets);
            // check: maybe tuplets intersect each other but still don't have common chords
      if (uncommonGroup.size() == 1)
            uncommonGroup = forceFindUncommonTupletPair(tuplets);   // empty if not succeed

      return uncommonGroup;
      }

TupletCommonIndexes findCommonIndexes(std::vector<char> &usedIndexes,
                                      const std::vector<TupletInfo> &tuplets)
      {
      TupletCommonIndexes commonIndexes;
      while (true) {
                        // empty if not succeed
            auto commonGroup = findLongestCommonGroup(usedIndexes, tuplets);
            if (commonGroup.size() <= 2)
                  break;
            commonIndexes.add(commonGroup);
            for (int i: commonGroup)
                  usedIndexes[i] = 1;
            }
      collectRemainingCommonIndexes(usedIndexes, tuplets, commonIndexes);

      return commonIndexes;
      }

std::vector<std::pair<ReducedFraction, ReducedFraction> >
findTupletIntervals(const std::vector<TupletInfo> &tuplets)
      {
      std::vector<std::pair<ReducedFraction, ReducedFraction>> tupletIntervals;
      for (const auto &tuplet: tuplets)
            tupletIntervals.push_back(tupletInterval(tuplet));

      return tupletIntervals;
      }

// first chord in tuplet may belong to other tuplet at the same time
// in the case if there are enough notes in this first chord
// to be splitted into different voices

void filterTuplets(std::vector<TupletInfo> &tuplets)
      {
      if (tuplets.empty())
            return;

      removeUselessTuplets(tuplets);
      auto uncommonGroup = findUncommonGroup(tuplets);

      std::vector<char> usedIndexes(tuplets.size(), 0);
      for (int i: uncommonGroup)
            usedIndexes[i] = 1;

      TupletCommonIndexes commonIndexes = findCommonIndexes(usedIndexes, tuplets);
      const auto tupletIntervals = findTupletIntervals(tuplets);

      std::vector<int> bestIndexes;
      TupletErrorResult minError;
      bool first = true;
      while (true) {
            const auto result = commonIndexes.generateNext();
            std::vector<std::vector<int>> tupletGroupsToTest;
            for (int i: result.first)
                  tupletGroupsToTest.push_back(std::vector<int>({i}));
            if (!uncommonGroup.empty())
                  tupletGroupsToTest.push_back(uncommonGroup);

            const auto indexesAndError = minimizeQuantError(tupletGroupsToTest, tuplets,
                                                            tupletIntervals);
            if (first) {
                  first = false;
                  minError = indexesAndError.second;
                  bestIndexes = indexesAndError.first;
                  }
            else if (indexesAndError.second < minError) {
                  minError = indexesAndError.second;
                  bestIndexes = indexesAndError.first;
                  }

            if (result.second)
                  break;
            }

      std::vector<TupletInfo> newTuplets;
      for (int i: bestIndexes)
            newTuplets.push_back(tuplets[i]);
      std::swap(tuplets, newTuplets);
      }

ReducedFraction noteLenQuantError(const ReducedFraction &noteOnTime,
                                  const ReducedFraction &noteLen,
                                  const ReducedFraction &raster)
      {
      const auto offTime = Quantize::quantizeValue(noteOnTime + noteLen, raster);
      const auto quantizedLen = offTime - noteOnTime;
      return (noteLen - quantizedLen).absValue();
      }

int averagePitch(const std::map<ReducedFraction, std::multimap<ReducedFraction, MidiChord>::iterator> &chords)
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

int averagePitch(const std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &chords)
      {
      if (chords.empty())
            return -1;
      int sumPitch = 0;
      int noteCounter = 0;
      for (const auto &it: chords) {
            const auto &midiNotes = it->second.notes;
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

bool hasIntersectionWithChord(const ReducedFraction &startTick,
                              const ReducedFraction &endTick,
                              const ReducedFraction &regularRaster,
                              const std::multimap<ReducedFraction, MidiChord>::iterator &chord)
      {
      const auto onTimeRaster = Quantize::reduceRasterIfDottedNote(
                                  MChord::maxNoteLen(chord->second.notes), regularRaster);
      const auto onTime = Quantize::quantizeValue(chord->first, onTimeRaster);
      auto testChord = chord->second;
      for (auto &note: testChord.notes) {
            auto offTimeRaster = Quantize::reduceRasterIfDottedNote(note.len, regularRaster);
            note.len = Quantize::quantizeValue(note.len, offTimeRaster);
            }
      return (endTick > onTime && startTick < onTime + MChord::maxNoteLen(testChord.notes));
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

void setTupletVoice(std::map<ReducedFraction, std::multimap<ReducedFraction, MidiChord>::iterator> &tupletChords,
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
                     const std::multimap<ReducedFraction, MidiChord>::iterator &chordIt)
      {
      const int firstChordIndex = tuplet.firstChordIndex;
      if (firstChordIndex <= 0)
            return false;

      const auto tupletFreeEnd = tuplet.onTime
                  + tuplet.len / tuplet.tupletNumber * firstChordIndex;
      const auto maxLen = MChord::maxNoteLen(chordIt->second.notes);
      const auto chordOffTime = chordIt->first + maxLen;
                  // if chord offTime is outside tuplet - discard chord
      if (chordIt->first + tuplet.tupletQuant >= tuplet.onTime
                  || chordOffTime <= tuplet.onTime
                  || chordOffTime >= tupletFreeEnd + tuplet.tupletQuant)
            return false;
      const auto chordError = findQuantizationError(chordOffTime, tuplet.regularQuant);
      const auto tupletError = findQuantizationError(chordOffTime, tuplet.tupletQuant);

      if (tupletError < chordError)
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
findBackTiedTuplets(const std::multimap<ReducedFraction, MidiChord> &chords,
                    const std::vector<TupletInfo> &tuplets,
                    const ReducedFraction &prevBarStart)
      {
      std::vector<TiedTuplet> tiedTuplets;
      std::set<int> usedVoices;
      const auto tupletChords = findMappedTupletChords(tuplets);

      for (int i = 0; i != (int)tuplets.size(); ++i) {
            if (tuplets[i].chords.empty())
                  continue;
            auto chordIt = tuplets[i].chords.begin()->second;
            while (chordIt != chords.begin() && chordIt->first >= prevBarStart) {
                  --chordIt;
                  int voice = chordIt->second.voice;
                  if (usedVoices.find(voice) != usedVoices.end())
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
                  if (canTupletBeTied(tuplets[i], chordIt)) {
                        tiedTuplets.push_back({i, voice, &*chordIt});
                        usedVoices.insert(voice);
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

void minimizeOffTimeError(std::vector<TupletInfo> &tuplets,
                          std::multimap<ReducedFraction, MidiChord> &chords,
                          std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets)
      {
      for (auto it = tuplets.begin(); it != tuplets.end(); ) {
            TupletInfo &tupletInfo = *it;
            const auto firstChord = tupletInfo.chords.begin();
            if (firstChord == tupletInfo.chords.end() || tupletInfo.firstChordIndex != 0) {
                  ++it;
                  continue;
                  }
            const auto &onTime = firstChord->second->first;
            MidiChord &midiChord = firstChord->second->second;
            auto &notes = midiChord.notes;

            std::vector<int> removedIndexes;
            std::vector<int> leavedIndexes;
            for (int i = 0; i != notes.size(); ++i) {
                  const auto &note = notes[i];
                  if (note.len <= tupletInfo.len) {
                        if ((tupletInfo.chords.size() == 1
                                    && notes.size() > (int)removedIndexes.size())
                                 || (tupletInfo.chords.size() > 1
                                    && notes.size() > (int)removedIndexes.size() + 1))
                              {
                              auto tupletError = noteLenQuantError(
                                                    onTime, note.len, tupletInfo.tupletQuant);
                              auto regularError = noteLenQuantError(
                                                    onTime, note.len, tupletInfo.regularQuant);
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
                  std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets)
      {
      for (TupletInfo &tuplet: tuplets) {
            for (auto it = nonTuplets.begin(); it != nonTuplets.end(); ) {
                  const auto &chordIt = *it;
                  const auto &onTime = chordIt->first;
                  if (onTime > tuplet.onTime
                              && hasIntersectionWithChord(tuplet.onTime, tuplet.onTime + tuplet.len,
                                                          tuplet.regularQuant, chordIt)) {
                        auto regularError = findQuantizationError(onTime, tuplet.regularQuant);
                        auto tupletError = findQuantizationError(onTime, tuplet.tupletQuant);
                        const auto offTime = MChord::maxNoteLen(chordIt->second.notes);

                        if (offTime < tuplet.onTime + tuplet.len) {
                              regularError += findQuantizationError(offTime, tuplet.regularQuant);
                              tupletError += findQuantizationError(offTime, tuplet.tupletQuant);
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
chordInterval(const std::pair<const ReducedFraction, MidiChord> *chord,
              const ReducedFraction &regularRaster)
      {
      auto onTime = Quantize::quantizeValue(chord->first, regularRaster);
      auto offTime = chord->first + MChord::maxNoteLen(chord->second.notes);
      offTime = Quantize::quantizeValue(offTime, regularRaster);
      return std::make_pair(onTime, offTime);
      }

void setTupletVoices(std::vector<TupletInfo> &tuplets,
                     std::set<int> &pendingTuplets,
                     std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction>>> &tupletIntervals)
      {
      int limit = tupletVoiceLimit();
      int voice = 0;
      while (!pendingTuplets.empty() && voice < limit) {
            for (auto it = pendingTuplets.begin(); it != pendingTuplets.end(); ) {
                  int i = *it;
                  const auto interval = tupletInterval(tuplets[i]);
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

void setNonTupletVoices(std::set<std::pair<const ReducedFraction, MidiChord> *> &pendingNonTuplets,
                        const std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction>>> &tupletIntervals,
                        const ReducedFraction &regularRaster)
      {
      const int limit = voiceLimit();
      int voice = 0;
      while (!pendingNonTuplets.empty() && voice < limit) {
            for (auto it = pendingNonTuplets.begin(); it != pendingNonTuplets.end(); ) {
                  auto chord = *it;
                  const auto interval = chordInterval(chord, regularRaster);
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

void removeUnusedTuplets(std::vector<TupletInfo> &tuplets,
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

bool haveOverlappingVoices(const std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
                           const std::vector<TupletInfo> &tuplets,
                           const ReducedFraction &regularRaster,
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
            const auto interval = chordInterval(&*chord, regularRaster);
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
findNonTupletIntervals(const std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
                       const ReducedFraction &regularRaster)
      {
      std::vector<std::pair<ReducedFraction, ReducedFraction>> nonTupletIntervals;
      for (const auto &nonTuplet: nonTuplets) {
            nonTupletIntervals.push_back(chordInterval(&*nonTuplet, regularRaster));
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
findPendingNonTuplets(const std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets)
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
findForTiedTuplets(const std::vector<TupletInfo> &tuplets,
                   const std::vector<TiedTuplet> &tiedTuplets,
                   const std::set<std::pair<const ReducedFraction, MidiChord> *> &pendingNonTuplets,
                   const ReducedFraction &startBarTick)
      {
      std::vector<std::pair<int, int>> forTiedTuplets;  // <tuplet index, voice to assign>

      for (const TiedTuplet &tuplet: tiedTuplets) {
            if (tuplet.chord->first < startBarTick)
                  continue;               // only for chords in the current bar
            if (pendingNonTuplets.find(tuplet.chord) == pendingNonTuplets.end()) {
                  const int i = findTupletWithChord(tuplet.chord->second, tuplets);
                  if (i != -1)
                        forTiedTuplets.push_back({i, tuplet.voice});
                  }
            }
      return forTiedTuplets;
      }


#ifdef QT_DEBUG

bool areAllElementsUnique(const std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets)
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

size_t chordCount(const std::vector<TupletInfo> &tuplets,
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
            const ReducedFraction &regularRaster)
      {
                  // remove overlapping tuplets
      size_t sz = tuplets.size();
      if (sz == 0)
            return;
      while (true) {
            bool change = false;
            for (size_t i = 0; i < sz - 1; ++i) {
                  const auto interval1 = tupletInterval(tuplets[i]);
                  for (size_t j = i + 1; j < sz; ++j) {
                        const auto interval2 = tupletInterval(tuplets[j]);
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
            const auto nonTupletIntervals = findNonTupletIntervals(nonTuplets, regularRaster);

            for (size_t i = 0; i < sz; ) {
                  const auto interval = tupletInterval(tuplets[i]);
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

void assignVoices(std::multimap<ReducedFraction, MidiChord> &chords,
                  std::vector<TupletInfo> &tuplets,
                  std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
                  const ReducedFraction &startBarTick,
                  const ReducedFraction &endBarTick,
                  const ReducedFraction &regularRaster)
      {
#ifdef QT_DEBUG
      size_t oldChordCount = chordCount(tuplets, nonTuplets);
#endif
      Q_ASSERT_X(!haveTupletsEmptyChords(tuplets),
                 "MIDI tuplets: assignVoices", "Empty tuplet chords");

      auto pendingTuplets = findPendingTuplets(tuplets);
      auto pendingNonTuplets = findPendingNonTuplets(nonTuplets);
      const auto prevBarStart = findPrevBarStart(startBarTick, endBarTick - startBarTick);
      const auto backTiedTuplets = findBackTiedTuplets(chords, tuplets, prevBarStart);
      const auto forTiedTuplets = findForTiedTuplets(tuplets, backTiedTuplets, pendingNonTuplets,
                                                     startBarTick);
      std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction>>> tupletIntervals;

      for (const auto &t: forTiedTuplets) {
            const int i = t.first;
            const int voice = t.second;
            setTupletVoice(tuplets[i].chords, voice);
                        // remove tuplets with already set voices
            pendingTuplets.erase(i);
            tupletIntervals[voice].push_back(tupletInterval(tuplets[i]));
            }

      for (const TiedTuplet &tuplet: backTiedTuplets) {
            setTupletVoice(tuplets[tuplet.tupletIndex].chords, tuplet.voice);
            pendingTuplets.erase(tuplet.tupletIndex);
            tupletIntervals[tuplet.voice].push_back(
                              tupletInterval(tuplets[tuplet.tupletIndex]));
                        // set for-tied chords
                        // some chords can be the same as in forTiedTuplets
            if (tuplet.chord->first < startBarTick)
                  continue;               // only for chords in the current bar
            tuplet.chord->second.voice = tuplet.voice;
                        // remove chords with already set voices
            pendingNonTuplets.erase(tuplet.chord);
            }

      {
      setTupletVoices(tuplets, pendingTuplets, tupletIntervals);

      Q_ASSERT_X((voiceLimit() == 1) ? pendingTuplets.empty() : true,
                 "MIDI tuplets: assignVoices", "Unused tuplets for the case !useMultipleVoices");

      removeUnusedTuplets(tuplets, nonTuplets, pendingTuplets, pendingNonTuplets);
      setNonTupletVoices(pendingNonTuplets, tupletIntervals, regularRaster);
      }

      Q_ASSERT_X(pendingNonTuplets.empty(),
                 "MIDI tuplets: assignVoices", "Unused non-tuplets");
      Q_ASSERT_X(!haveTupletsEmptyChords(tuplets),
                 "MIDI tuplets: assignVoices", "Empty tuplet chords");
      Q_ASSERT_X(doTupletChordsHaveSameVoice(tuplets),
                 "MIDI tuplets: assignVoices", "Tuplet chords have different voices");
      Q_ASSERT_X(!haveOverlappingVoices(nonTuplets, tuplets, regularRaster, backTiedTuplets),
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
                                                 tupletInfo.tupletQuant,
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
                        if (midiChord.notes[i].len < tupletNoteLen / 2) {
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
                  if ((pos - it->first).absValue() < tuplet.regularQuant)
                        return true;
                  }
            }
      return false;
      }

// decrease tuplet error by enlarging staccato notes:
// make note.len = tuplet note length

void applyStaccatoForTuplets(std::vector<TupletInfo> &tuplets)
      {
      for (auto &tuplet: tuplets) {
            const auto tupletNoteLen = tuplet.len / tuplet.tupletNumber;
            for (auto &chord: tuplet.chords) {
                  MidiChord &midiChord = chord.second->second;
                  for (auto &note: midiChord.notes) {
                        if (note.staccato)
                              note.len = tupletNoteLen;
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

std::vector<TupletData> findTuplets(const ReducedFraction &startBarTick,
                                    const ReducedFraction &endBarTick,
                                    const ReducedFraction &barFraction,
                                    std::multimap<ReducedFraction, MidiChord> &chords)
      {
      if (chords.empty() || startBarTick >= endBarTick)     // invalid cases
            return std::vector<TupletData>();
      const auto operations = preferences.midiImportOperations.currentTrackOperations();
      if (!operations.tuplets.doSearch)
            return std::vector<TupletData>();
      const auto tol = Quantize::fixedQuantRaster() / 2;
      auto startBarChordIt = MChord::findFirstChordInRange(startBarTick - tol, endBarTick,
                                                           chords.begin(), chords.end());
      startBarChordIt = findTupletFreeChord(startBarChordIt, chords.end(), startBarTick);
      if (startBarChordIt == chords.end())      // no chords in this bar
            return std::vector<TupletData>();

      const auto endBarChordIt = MChord::findEndChordInRange(endBarTick,
                                                             startBarChordIt, chords.end());
      const auto regularRaster = Quantize::findRegularQuantRaster(startBarChordIt, endBarChordIt,
                                                                  endBarTick);
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

                              // end iterator, as usual, will point to the next - invalid chord
                  const auto endDivChordIt = MChord::findEndChordInRange(
                                 endDivTime, startDivChordIt, endBarChordIt);
                              // try different tuplets, nested tuplets are not allowed
                  for (const auto &tupletNumber: tupletNumbers) {
                        const auto tupletNoteLen = divLen / tupletNumber;
                        if (tupletNoteLen < regularRaster)
                              continue;
                        auto tupletInfo = findTupletApproximation(divLen, tupletNumber,
                                             regularRaster, startDivTime, startDivChordIt, endDivChordIt);

                        if (!haveChordsInTheMiddleBetweenTupletChords(
                                          startDivChordIt, endDivChordIt, tupletInfo)) {
                              detectStaccato(tupletInfo);
                              }
                        tupletInfo.sumLengthOfRests = findSumLengthOfRests(tupletInfo);

                        if (!isTupletAllowed(tupletInfo))
                              continue;
                        tuplets.push_back(tupletInfo);   // tuplet found
                        }      // next tuplet type
                  }
            }

      filterTuplets(tuplets);

            // later notes will be sorted and their indexes become invalid
            // so assign staccato information to notes now
      markStaccatoTupletNotes(tuplets);

      auto nonTuplets = findNonTupletChords(tuplets, startBarChordIt, endBarChordIt);
      if (tupletVoiceLimit() == 1)
            excludeExtraVoiceTuplets(tuplets, nonTuplets, regularRaster);

      resetTupletVoices(tuplets);  // because of tol some chords may have non-zero voices
      addChordsBetweenTupletNotes(tuplets, nonTuplets);
      sortNotesByPitch(startBarChordIt, endBarChordIt);
      sortTupletsByAveragePitch(tuplets);

      if (operations.useMultipleVoices) {
            splitFirstTupletChords(tuplets, chords);
            minimizeOffTimeError(tuplets, chords, nonTuplets);
            }
      cleanStaccatoOfNonTuplets(nonTuplets);
      applyStaccatoForTuplets(tuplets);

      Q_ASSERT_X(!doTupletsHaveCommonChords(tuplets),
                 "MIDI tuplets: findTuplets", "Tuplets have common chords but they shouldn't");
      Q_ASSERT_X((voiceLimit() == 1)
                        ? !haveOverlappingVoices(nonTuplets, tuplets, regularRaster)
                        : true,
                 "MIDI tuplets: findTuplets",
                 "Overlapping tuplet and non-tuplet voices for the case !useMultipleVoices");

      assignVoices(chords, tuplets, nonTuplets, startBarTick, endBarTick, regularRaster);

      return convertToData(tuplets);
      }

std::multimap<ReducedFraction, TupletData>
findAllTuplets(std::multimap<ReducedFraction, MidiChord> &chords,
               const TimeSigMap *sigmap,
               const ReducedFraction &lastTick)
      {
      std::multimap<ReducedFraction, TupletData> tupletEvents;
      ReducedFraction startBarTick = {0, 1};

      for (int i = 1;; ++i) {       // iterate over all measures by indexes
            const auto endBarTick = ReducedFraction::fromTicks(sigmap->bar2tick(i, 0));
            const auto barFraction = ReducedFraction(sigmap->timesig(startBarTick.ticks()).timesig());
            const auto tuplets = findTuplets(startBarTick, endBarTick, barFraction, chords);
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
                                    if (note.len < tupletData.len) {
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
