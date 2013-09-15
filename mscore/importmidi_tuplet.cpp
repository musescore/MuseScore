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

const std::map<int, ReducedFraction>& tupletRatios()
      {
      const static std::map<int, ReducedFraction> ratios = {
            {2, {2, 3}},
            {3, {3, 2}},
            {4, {4, 3}},
            {5, {5, 4}},
            {7, {7, 8}},
            {9, {9, 8}}
            };
      return ratios;
      }

bool isTupletAllowed(const TupletInfo &tupletInfo)
      {
                  // special check for duplets and triplets
      const std::vector<int> nums = {2, 3};
                  // for duplet: if note first and single - only 1/2*tupletLen duration is allowed
                  // for triplet: if note first and single - only 1/3*tupletLen duration is allowed
      for (auto num: nums) {
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
                  // for all tuplets
      const int minAllowedNoteCount = tupletInfo.tupletNumber / 2 + tupletInfo.tupletNumber / 4;
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
//

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

ReducedFraction findSumLengthOfRests(const TupletInfo &tupletInfo)
      {
      auto beg = tupletInfo.onTime;
      const auto tupletEndTime = (tupletInfo.onTime + tupletInfo.len);
      ReducedFraction sumLen = {0, 1};

      for (const auto &chord: tupletInfo.chords) {
            const MidiChord &midiChord = chord.second->second;
            const auto &chordOnTime = chord.second->first;
                        // approximate length of gaps between successive chords,
                        // quantization is not taken into account
            if (beg < chordOnTime)
                  sumLen += (chordOnTime - beg);
            beg = chordOnTime + MChord::maxNoteLen(midiChord.notes);
            if (beg >= tupletInfo.onTime + tupletInfo.len)
                  break;
            }
      if (beg < tupletEndTime)
            sumLen += (tupletEndTime - beg);
      return sumLen;
      }

TupletInfo findTupletApproximation(const ReducedFraction &tupletLen,
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

void markChordsAsUsed(std::map<std::pair<const ReducedFraction, MidiChord> *, int> &usedFirstTupletNotes,
                      std::set<std::pair<const ReducedFraction, MidiChord> *> &usedChords,
                      const std::map<ReducedFraction, std::multimap<ReducedFraction, MidiChord>::iterator> &tupletChords,
                      int firstChordIndex)
      {
      if (tupletChords.empty())
            return;

      auto i = tupletChords.begin();
      if (firstChordIndex == 0) {
                        // check is the note of the first tuplet chord in use
            auto ii = usedFirstTupletNotes.find(&*(i->second));
            if (ii == usedFirstTupletNotes.end())
                  ii = usedFirstTupletNotes.insert({&*(i->second), 1}).first;
            else
                  ++(ii->second);         // increase chord note counter
            ++i;              // start from the second chord
            }
      for ( ; i != tupletChords.end(); ++i) {
                        // mark the chord as used
            usedChords.insert(&*(i->second));
            }
      }

bool areTupletChordsInUse(
            const std::map<std::pair<const ReducedFraction, MidiChord> *, int> &usedFirstTupletNotes,
            const std::set<std::pair<const ReducedFraction, MidiChord> *> &usedChords,
            const std::map<ReducedFraction, std::multimap<ReducedFraction, MidiChord>::iterator> &tupletChords,
            int firstChordIndex)
      {
      if (tupletChords.empty())
            return false;

      const auto operations = preferences.midiImportOperations.currentTrackOperations();

      auto i = tupletChords.begin();
      if (firstChordIndex == 0) {
                        // check are first tuplet notes all in use (1 note - 1 voice)
            const auto ii = usedFirstTupletNotes.find(&*(i->second));
            if (ii != usedFirstTupletNotes.end()) {
                  if (!operations.useMultipleVoices && ii->second > 1)
                        return true;
                  else if (ii->second >= i->second->second.notes.size()
                              || ii->second >= VOICES - 1) {
                              // need to choose next tuplet candidate - no more available voices
                              // one voice is reserved for non-tuplet chords
                        return true;
                        }
                  }
            ++i;
            }
      for ( ; i != tupletChords.end(); ++i) {
            if (usedChords.find(&*(i->second)) != usedChords.end()) {
                              // the chord note is in use - cannot use this chord again
                  return true;
                  }
            }
      return false;
      }

// result: <average quant error,
//          sum length of rests inside all tuplets,
//          negative total tuplet note count>

std::tuple<double, ReducedFraction, int>
validateTuplets(std::list<int> &indexes,
                const std::vector<TupletInfo> &tuplets)
      {
      if (tuplets.empty())
            return std::make_tuple(0.0, ReducedFraction(0, 1), 0);
                  // structure of map: <chord ID, count of use of first tuplet chord with this tick>
      std::map<std::pair<const ReducedFraction, MidiChord> *, int> usedFirstTupletNotes;
                  // chord IDs of already used chords
      std::set<std::pair<const ReducedFraction, MidiChord> *> usedChords;
      std::set<std::pair<const ReducedFraction, MidiChord> *> excludedChords;
                  // select tuplets with min average error
      for (auto it = indexes.begin(); it != indexes.end(); ) {
            const auto &tupletChords = tuplets[*it].chords;
                        // check for chord notes that are already in use in another tuplets
            if (tupletChords.empty()
                        || areTupletChordsInUse(usedFirstTupletNotes, usedChords,
                                                tupletChords, tuplets[*it].firstChordIndex)) {
                  for (const auto &chord: tupletChords)
                        excludedChords.insert(&*chord.second);
                  it = indexes.erase(it);
                  continue;
                  }
                        // we can use this tuplet
            markChordsAsUsed(usedFirstTupletNotes, usedChords, tupletChords,
                             tuplets[*it].firstChordIndex);
            ++it;
            }

      ReducedFraction sumError;
      ReducedFraction sumLengthOfRests;
      int sumNoteCount = 0;

      for (const auto &i: indexes) {
            sumError += tuplets[i].tupletSumError;
            sumLengthOfRests += tuplets[i].sumLengthOfRests;
            sumNoteCount += tuplets[i].chords.size();
            }
                  // add quant error of all chords excluded from tuplets
      for (const auto &i: indexes) {
            for (const auto &chord: tuplets[i].chords)
                  excludedChords.erase(&*chord.second);
            }
      const ReducedFraction &regularRaster = tuplets.front().regularQuant;
      for (const auto &chordIt: excludedChords)
            sumError += findQuantizationError(chordIt->first, regularRaster);

      return std::make_tuple(sumError.ticks() * 1.0 / sumNoteCount,
                             sumLengthOfRests, sumNoteCount);
      }

// try different permutations of tuplet indexes to minimize error

std::list<int>
minimizeQuantError(std::vector<std::vector<int>> &indexGroups,
                   const std::vector<TupletInfo> &tuplets)
      {
      std::tuple<double, ReducedFraction, int> minResult;
      std::vector<int> iIndexGroups;  // indexes of elements in indexGroups
      for (int i = 0; i != (int)indexGroups.size(); ++i)
            iIndexGroups.push_back(i);
      std::list<int> bestIndexes;
                  // number of permutations grows as n!
                  // 8! = 40320 - quite many; 9! = 362880 - many
                  // so set reasonable max limit to prevent the hang of our program
      const int PERMUTATION_LIMIT = 50000;
      int counter = 0;
      do {
            std::list<int> indexesToValidate;
            for (const auto &i: iIndexGroups) {
                  const auto &group = indexGroups[i];
                  for (const auto &ii: group)
                        indexesToValidate.push_back(ii);
                  }

            const auto result = validateTuplets(indexesToValidate, tuplets);
            if (counter == 0) {
                  minResult = result;
                  bestIndexes = indexesToValidate;
                  }
            else if (result < minResult) {
                  minResult = result;
                  bestIndexes = indexesToValidate;
                  }
            ++counter;
            } while (counter < PERMUTATION_LIMIT &&
                     std::next_permutation(iIndexGroups.begin(), iIndexGroups.end()));

      return bestIndexes;
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

std::list<int> findTupletsWithCommonChords(std::list<int> &restTuplets,
                                           const std::vector<TupletInfo> &tuplets)
      {
      std::list<int> tupletGroup;
      if (restTuplets.empty())
            return tupletGroup;

      QQueue<int> q;
      {
      auto it = restTuplets.begin();
      tupletGroup.push_back(*it);
      q.enqueue(*it);
      restTuplets.erase(it);
      }

      while (!q.isEmpty() && !restTuplets.empty()) {
            const int index = q.dequeue();
            auto it = restTuplets.begin();
            while (it != restTuplets.end()) {
                  if (haveCommonChords(index, *it, tuplets)) {
                        tupletGroup.push_back(*it);
                        q.enqueue(*it);
                        it = restTuplets.erase(it);
                        continue;
                        }
                  ++it;
                  }
            }

      return tupletGroup;
      }

std::vector<int> findTupletsWithNoCommonChords(std::list<int> &commonTuplets,
                                               const std::vector<TupletInfo> &tuplets)
      {
      std::vector<int> uncommonTuplets;
      if (commonTuplets.empty())
            return uncommonTuplets;
                  // add first tuplet, no need to check
      auto it = commonTuplets.begin();
      uncommonTuplets.push_back(*it);
      it = commonTuplets.erase(it);

      while (it != commonTuplets.end()) {
            bool haveCommon = false;
            for (const auto &i: uncommonTuplets) {
                  if (haveCommonChords(i, *it, tuplets)) {
                        haveCommon = true;
                        break;
                        }
                  }
            if (!haveCommon) {
                  uncommonTuplets.push_back(*it);
                  it = commonTuplets.erase(it);
                  continue;
                  }
            ++it;
            }

      return uncommonTuplets;
      }

// first chord in tuplet may belong to other tuplet at the same time
// in the case if there are enough notes in this first chord
// to be splitted into different voices

void filterTuplets(std::vector<TupletInfo> &tuplets)
      {
      if (tuplets.empty())
            return;
      std::list<int> bestTuplets;
      std::list<int> restTuplets;
      for (int i = 0; i != (int)tuplets.size(); ++i)
            restTuplets.push_back(i);

      while (!restTuplets.empty()) {
            std::list<int> commonTuplets
                        = findTupletsWithCommonChords(restTuplets, tuplets);
            std::vector<std::vector<int>> tupletGroupsToTest;
            while (!commonTuplets.empty()) {
                  std::vector<int> uncommonTuplets
                              = findTupletsWithNoCommonChords(commonTuplets, tuplets);
                  tupletGroupsToTest.push_back(uncommonTuplets);
                  }
            std::list<int> bestIndexes = minimizeQuantError(tupletGroupsToTest, tuplets);
            bestTuplets.insert(bestTuplets.end(), bestIndexes.begin(), bestIndexes.end());
            }

      std::vector<TupletInfo> newTuplets;
      for (const auto &i: bestTuplets)
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

int averageChordPitch(const std::multimap<ReducedFraction, MidiChord>::iterator &chord)
      {
      int sumPitch = 0;
      int noteCounter = 0;
      const auto &midiNotes = chord->second.notes;
      for (const auto &midiNote: midiNotes) {
            sumPitch += midiNote.pitch;
            ++noteCounter;
            }
      return sumPitch / noteCounter;
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

std::list<std::multimap<ReducedFraction, MidiChord>::iterator>
findNonTupletChords(const std::vector<TupletInfo> &tuplets,
                    const std::multimap<ReducedFraction, MidiChord>::iterator &startBarChordIt,
                    const std::multimap<ReducedFraction, MidiChord>::iterator &endBarChordIt)
      {
      std::set<std::pair<const ReducedFraction, MidiChord> *> tupletChords;
      for (const auto &tupletInfo: tuplets) {
            for (const auto &tupletChord: tupletInfo.chords) {
                  auto tupletIt = tupletChord.second;
                  tupletChords.insert(&*tupletIt);
                  }
            }
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

// input tuplets are sorted by average pitch in desc. order
// result - tuplet that average pitch is the closest
// to the average pitch of non-tuplet chords

TupletInfo findClosestTuplet(const std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
                             const std::vector<TupletInfo> &sortedTuplets)
      {
      TupletInfo closestTuplet;
      if (sortedTuplets.empty())
            return closestTuplet;  // invalid
      if (nonTuplets.empty())
            return sortedTuplets.front();

      const int nonTupletPitch = averagePitch(nonTuplets);
      int minPitchDiff = std::numeric_limits<int>::max();

      for (const auto &tupletInfo: sortedTuplets) {
            const int tupletPitch = averagePitch(tupletInfo.chords);
            const int pitchDiff = qAbs(nonTupletPitch - tupletPitch);
            if (pitchDiff < minPitchDiff) {
                  minPitchDiff = pitchDiff;
                  closestTuplet = tupletInfo;
                  }
            }
      return closestTuplet;
      }

int maxTupletVoice(const std::vector<TupletInfo> &tuplets)
      {
      int maxVoice = 0;
      for (const TupletInfo &tupletInfo: tuplets) {
            if (tupletInfo.chords.empty())
                  continue;
            const int tupletVoice = tupletInfo.chords.begin()->second->second.voice;
            if (tupletVoice > maxVoice)
                  maxVoice = tupletVoice;
            }
      return maxVoice;
      }

// split first tuplet chord, that belong to 2 tuplets, into 2 voices

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
            }
      }

void setNonTupletVoices(std::vector<TupletInfo> &tuplets,
                        const std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
                        const ReducedFraction &regularRaster)
      {
      const auto tuplet = findClosestTuplet(nonTuplets, tuplets);
      const int maxVoice = maxTupletVoice(tuplets) + 1;
      const int tupletVoice = (tuplet.chords.empty())
                  ? 0 : tuplet.chords.begin()->second->second.voice;
      for (auto &chord: nonTuplets) {
            if (hasIntersectionWithChord(tuplet.onTime, tuplet.onTime + tuplet.len,
                                         regularRaster, chord))
                  chord->second.voice = maxVoice;
            else
                  chord->second.voice = tupletVoice;
            }
      }

// the input tuplets should be filtered (for mutual validity)
// the output voices can be larger than VOICES - 1
// it will be handled later by merging voices

void separateTupletVoices(std::vector<TupletInfo> &tuplets,
                          const std::multimap<ReducedFraction, MidiChord>::iterator &startBarChordIt,
                          const std::multimap<ReducedFraction, MidiChord>::iterator &endBarChordIt,
                          std::multimap<ReducedFraction, MidiChord> &chords)
      {
      if (tuplets.empty())
            return;
      sortNotesByPitch(startBarChordIt, endBarChordIt);
      sortTupletsByAveragePitch(tuplets);

      for (auto now = tuplets.begin(); now != tuplets.end(); ++now) {
            int voice = 0;
            auto lastMatch = tuplets.end();
            const auto nowChordIt = now->chords.begin();
            for (auto prev = tuplets.begin(); prev != now; ++prev) {
                              // check is <now> tuplet go over <prev> tuplet
                  if (now->onTime + now->len > prev->onTime
                              && now->onTime < prev->onTime + prev->len)
                        ++voice;
                  auto prevChordIt = prev->chords.begin();
                  if (now->firstChordIndex == 0
                              && prev->firstChordIndex == 0
                              && nowChordIt->second == prevChordIt->second) {
                        lastMatch = prev;
                        }
                  }
            if (lastMatch != tuplets.end())
                  splitTupletChord(lastMatch, chords);

            setTupletVoice(now->chords, voice);
            }
      }

// if offTime is inside some tuplet - quant by tuplet grid
// if offTime is outside all tuplets - quant by regular grid

ReducedFraction findOffTimeRaster(const ReducedFraction &noteOffTime,
                                  int voice,
                                  const ReducedFraction &regularQuant,
                                  const std::vector<TupletInfo> &tuplets)
      {
      for (const auto &tupletInfo: tuplets) {
            if (!tupletInfo.chords.empty()
                        && voice == tupletInfo.chords.begin()->second->second.voice
                        && noteOffTime > tupletInfo.onTime
                        && noteOffTime < tupletInfo.onTime + tupletInfo.len)
                  return tupletInfo.tupletQuant;
            }
      return regularQuant;
      }

ReducedFraction findOffTimeRaster(const ReducedFraction &noteOffTime,
                                  int voice,
                                  const ReducedFraction &regularQuant,
                                  const std::multimap<ReducedFraction, MidiTuplet::TupletData> &tupletEvents)
      {
      for (const auto &tupletEvent: tupletEvents) {
            const TupletData &tupletData = tupletEvent.second;
            if (tupletData.voice == voice
                        && noteOffTime > tupletData.onTime
                        && noteOffTime < tupletData.onTime + tupletData.len)
                  return tupletData.tupletQuant;
            }
      return regularQuant;
      }

void shrinkVoices(std::multimap<ReducedFraction, MidiChord>::iterator startBarChordIt,
                  std::multimap<ReducedFraction, MidiChord>::iterator endBarChordIt)
      {
      int shift = 0;
      for (int voice = 0; voice < VOICES; ++voice) {
            bool voiceInUse = false;
            for (auto it = startBarChordIt; it != endBarChordIt; ++it) {
                  if (it->second.voice == voice) {
                        voiceInUse = true;
                        break;
                        }
                  }
            if (shift > 0 && voiceInUse) {
                  for (auto it = startBarChordIt; it != endBarChordIt; ++it) {
                        if (it->second.voice == voice)
                              it->second.voice -= shift;
                        }
                  }
            if (!voiceInUse)
                  ++shift;
            }
      }

// fast enough for small vector sizes

bool haveIntersection(const std::vector<std::pair<ReducedFraction, ReducedFraction>> &v1,
                      const std::vector<std::pair<ReducedFraction, ReducedFraction>> &v2)
      {
      if (v1.empty() || v2.empty())
            return false;
      for (const auto &r1: v1) {
            for (const auto &r2: v2) {
                  if (r1.second > r2.first && r1.first < r2.second)
                        return true;
                  }
            }
      return false;
      }

// we don't use interval trees here because the interval count is small

void mergeVoices(std::vector<TupletInfo> &tuplets,
                 std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets)
      {
                  // <voice, intervals>
      std::map<int, std::vector<std::pair<ReducedFraction, ReducedFraction>>> intervals;
      std::map<int, std::vector<std::multimap<ReducedFraction, MidiChord>::iterator>> chords;

      for (const auto &tuplet: tuplets) {
            if (tuplet.chords.empty())
                  continue;
            int tupletVoice = tuplet.chords.begin()->second->second.voice;
            intervals[tupletVoice].push_back({tuplet.onTime, tuplet.onTime + tuplet.len});
            for (const auto &chord: tuplet.chords)
                  chords[tupletVoice].push_back(chord.second);
            }
      for (const auto &nonTuplet: nonTuplets) {
            if (nonTuplet->second.notes.empty())
                  continue;
            intervals[nonTuplet->second.voice].push_back(
                  {nonTuplet->first, nonTuplet->first + MChord::maxNoteLen(nonTuplet->second.notes)});
            chords[nonTuplet->second.voice].push_back(nonTuplet);
            }

      auto prevEnd = intervals.end();
      --prevEnd;
      for (auto i1 = intervals.begin(); i1 != prevEnd; ++i1) {
            auto i2 = i1;
            ++i2;
            for (; i2 != intervals.end(); ++i2) {
                  if (!haveIntersection(i1->second, i2->second)) {
                        i1->second.insert(i1->second.end(), i2->second.begin(), i2->second.end());
                        i2->second.clear();
                        for (auto &chord: chords[i2->first])
                              chord->second.voice = i1->first;
                        chords[i1->first].insert(chords[i1->first].end(),
                                              chords[i2->first].begin(), chords[i2->first].end());
                        chords[i2->first].clear();
                        }
                  }
            }
      }

void adjustNonTupletVoices(std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
                           const std::vector<TupletInfo> &tuplets)
      {
      int maxNonTupletVoice = maxTupletVoice(tuplets) + 1;
      std::list<std::multimap<ReducedFraction, MidiChord>::iterator> maxNonTuplets;
      for (const auto &chord: nonTuplets) {
            if (chord->second.voice == maxNonTupletVoice)
                  maxNonTuplets.push_back(chord);
            }
      if (maxNonTuplets.empty())
            return;
      int maxPitch = averagePitch(maxNonTuplets);

      for (auto &chord: nonTuplets) {
            int &chordVoice = chord->second.voice;
            if (chordVoice != maxNonTupletVoice) {
                  int tupletPitch = 0;
                  for (const TupletInfo &tuplet: tuplets) {
                        if (tuplet.chords.empty())
                              continue;
                        if (tuplet.chords.begin()->second->second.voice == chordVoice)
                              tupletPitch = averagePitch(tuplet.chords);
                        }
                  const int avgPitch = averageChordPitch(chord);
                  const int tupletPitchDiff = qAbs(avgPitch - tupletPitch);
                  const int maxPitchDiff = qAbs(avgPitch - maxPitch);
                  if (maxPitchDiff < tupletPitchDiff)
                        chordVoice = maxNonTupletVoice;
                  }
            }
      }

bool isVoiceChangeAllowed(const std::vector<TupletInfo> &tuplets,
                          const std::set<int> &changedTuplets,
                          int newVoice)
      {
      for (int i = 0; i != (int)tuplets.size(); ++i) {
            if (tuplets[i].chords.empty())
                  continue;
            const int tupletVoice = tuplets[i].chords.begin()->second->second.voice;
            if (tupletVoice == newVoice) {
                  if (changedTuplets.find(i) != changedTuplets.end())
                        return false;
                  }
            }
      return true;
      }

void changeTupletVoice(TupletInfo &tuplet,
                       int voice)
      {
      for (auto &chord: tuplet.chords)
            chord.second->second.voice = voice;
      }

void exchangeVoicesOfTuplets(std::vector<TupletInfo> &tuplets,
                             int oldVoice,
                             int newVoice)
      {
      for (int i = 0; i != (int)tuplets.size(); ++i) {
            if (tuplets[i].chords.empty())
                  continue;
            const int tupletVoice = tuplets[i].chords.begin()->second->second.voice;
            if (tupletVoice == newVoice)
                  changeTupletVoice(tuplets[i], oldVoice);
            }
      }

void exchangeVoicesOfNonTuplets(std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
                                int oldVoice,
                                int newVoice)
      {
      for (auto &nonTuplet: nonTuplets) {
            if (nonTuplet->second.voice == oldVoice)
                  nonTuplet->second.voice = newVoice;
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

void addFirstTiedTupletNotes(const std::multimap<ReducedFraction, MidiChord> &chords,
                             std::vector<TupletInfo> &tuplets,
                             std::list<std::multimap<ReducedFraction, MidiChord>::iterator> &nonTuplets,
                             const ReducedFraction &prevBarStart)
      {
      std::set<int> changedTuplets;

      for (int i = 0; i != (int)tuplets.size(); ++i) {
            if (tuplets[i].chords.empty())
                  continue;
            auto chordIt = tuplets[i].chords.begin()->second;
            const int tupletVoice = chordIt->second.voice;

            while (chordIt != chords.begin() && chordIt->first >= prevBarStart) {
                  --chordIt;
                  if (canTupletBeTied(tuplets[i], chordIt)) {
                        const MidiChord &chord = chordIt->second;
                        if (chord.voice != tupletVoice) {
                              const int oldVoice = tupletVoice;
                              const int newVoice = chord.voice;
                              if (isVoiceChangeAllowed(tuplets, changedTuplets, newVoice)) {
                                    exchangeVoicesOfTuplets(tuplets, oldVoice, newVoice);
                                    changeTupletVoice(tuplets[i], newVoice);
                                    changedTuplets.insert(i);
                                    exchangeVoicesOfNonTuplets(nonTuplets, oldVoice, newVoice);
                                    }
                              }
                        }
                  }
            }
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
                          std::multimap<ReducedFraction, MidiChord> &chords)
      {
      const int nonTupletVoice = maxTupletVoice(tuplets) + 1;
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
                  newTupletChord.voice = midiChord.voice;
                  for (const auto &i: leavedIndexes)
                        newTupletChord.notes.push_back(notes[i]);

                  QList<MidiNote> newNotes;
                  for (const auto &i: removedIndexes)
                        newNotes.push_back(notes[i]);
                  notes = newNotes;
                  midiChord.voice = nonTupletVoice;
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
            if (tuplet.chords.empty())
                  continue;
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
                              chordIt->second.voice = tuplet.chords.begin()->second->second.voice;
                              tuplet.chords.insert({onTime, chordIt});
                              it = nonTuplets.erase(it);
                              continue;
                              }
                        }
                  ++it;
                  }
            }
      }

ReducedFraction findStartOfPrevBar(const ReducedFraction &barLen,
                                   const ReducedFraction &endBarTick)
      {
      auto prevBarStart = endBarTick - barLen * 2;
      if (prevBarStart < ReducedFraction(0, 1))
            prevBarStart = ReducedFraction(0, 1);

      return prevBarStart;
      }

void handleMultipleVoices(std::multimap<ReducedFraction, MidiChord> &chords,
                          std::vector<TupletInfo> &tuplets,
                          const std::multimap<ReducedFraction, MidiChord>::iterator &startBarChordIt,
                          const std::multimap<ReducedFraction, MidiChord>::iterator &endBarChordIt,
                          const ReducedFraction &endBarTick,
                          const ReducedFraction &barLen)
      {
      separateTupletVoices(tuplets, startBarChordIt, endBarChordIt, chords);
      auto nonTuplets = findNonTupletChords(tuplets, startBarChordIt, endBarChordIt);
      addChordsBetweenTupletNotes(tuplets, nonTuplets);
      const auto regularRaster = Quantize::findRegularQuantRaster(
                                       startBarChordIt, endBarChordIt, endBarTick);
      setNonTupletVoices(tuplets, nonTuplets, regularRaster);
      const auto prevBarStart = findStartOfPrevBar(barLen, endBarTick);
      addFirstTiedTupletNotes(chords, tuplets, nonTuplets, prevBarStart);
      minimizeOffTimeError(tuplets, chords);
      mergeVoices(tuplets, nonTuplets);
      shrinkVoices(startBarChordIt, endBarChordIt);
      adjustNonTupletVoices(nonTuplets, tuplets);
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
      const auto startBarChordIt = MChord::findFirstChordInRange(startBarTick - tol, endBarTick,
                                                                 chords.begin(), chords.end());
      if (startBarChordIt == chords.end()) // no chords in this bar
            return std::vector<TupletData>();

      const auto endBarChordIt = MChord::findEndChordInRange(endBarTick + tol,
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
                              // [startDivTime - quantValue, endDivTime + quantValue]
                  const auto startDivChordIt = MChord::findFirstChordInRange(
                                    startDivTime - regularRaster, endDivTime + regularRaster,
                                    startBarChordIt, endBarChordIt);
                  if (startDivChordIt == endBarChordIt)
                        continue;
                              // end iterator, as usual, will point to the next - invalid chord
                  const auto endDivChordIt = MChord::findEndChordInRange(
                                 endDivTime + regularRaster, startDivChordIt, endBarChordIt);
                              // try different tuplets, nested tuplets are not allowed
                  for (const auto &tupletNumber: tupletNumbers) {
                        const auto tupletNoteLen = divLen / tupletNumber;
                        if (tupletNoteLen < regularRaster)
                              continue;
                        auto tupletInfo = findTupletApproximation(divLen, tupletNumber,
                                             regularRaster, startDivTime, startDivChordIt, endDivChordIt);
                        tupletInfo.sumLengthOfRests = findSumLengthOfRests(tupletInfo);

                        if (!isTupletAllowed(tupletInfo))
                              continue;
                        tuplets.push_back(tupletInfo);   // tuplet found
                        }      // next tuplet type
                  }
            }
      filterTuplets(tuplets);

      if (operations.useMultipleVoices) {
            handleMultipleVoices(chords, tuplets, startBarChordIt, endBarChordIt, endBarTick,
                                 endBarTick - startBarTick);
            }

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

void removeEmptyTuplets(MTrack &track)
      {
      if (track.tuplets.empty())
            return;
      for (auto it = track.tuplets.begin(); it != track.tuplets.end(); ) {
            const auto &tupletData = it->second;
            bool containsChord = false;
            for (const auto &chord: track.chords) {
                  if (tupletData.voice != chord.second.voice)
                        continue;
                  const ReducedFraction &onTime = chord.first;
                  const ReducedFraction len = MChord::maxNoteLen(chord.second.notes);
                  if (onTime + len > tupletData.onTime
                              && onTime + len <= tupletData.onTime + tupletData.len) {
                                    // tuplet contains at least one chord
                        containsChord = true;
                        break;
                        }
                  }
            if (!containsChord) {
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
            const auto ratioIt = tupletRatios().find(tupletData.tupletNumber);
            const auto tupletRatio = (ratioIt != tupletRatios().end())
                                   ? ratioIt->second : ReducedFraction(2, 2);
            if (ratioIt == tupletRatios().end())
                  qDebug("Tuplet ratio not found for tuplet number: %d", tupletData.tupletNumber);
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
