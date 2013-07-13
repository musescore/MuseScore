#include "importmidi_tuplet.h"
#include "libmscore/fraction.h"
#include "importmidi_chord.h"
#include "importmidi_meter.h"
#include "importmidi_quant.h"
#include "libmscore/mscore.h"
#include "preferences.h"

#include <set>


namespace Ms {
namespace MidiTuplet {

const std::map<int, Fraction>& tupletRatios()
      {
      const static std::map<int, Fraction> ratios = {
            {2, Fraction({2, 3})},
            {3, Fraction({3, 2})},
            {4, Fraction({4, 3})},
            {5, Fraction({5, 4})},
            {7, Fraction({7, 8})},
            {9, Fraction({9, 8})}
            };
      return ratios;
      }

bool isTupletAllowed(int tupletNumber,
                     const Fraction &tupletLen,
                     const Fraction &tupletOnTimeSumError,
                     const Fraction &regularSumError,
                     const Fraction &regularRaster,
                     const std::map<int, std::multimap<Fraction, MidiChord>::iterator> &tupletChords)
      {
                  // special check for duplets and triplets
      std::vector<int> nums = {2, 3};
                  // for duplet: if note first and single - only 1/2*tupletLen duration is allowed
                  // for triplet: if note first and single - only 1/3*tupletLen duration is allowed
      for (auto num: nums) {
            if (tupletNumber == num && tupletChords.size() == 1
                        && tupletChords.begin()->first == 0) {
                  auto &chordEventIt = tupletChords.begin()->second;
                  for (const auto &note: chordEventIt->second.notes) {
                        if ((note.len - tupletLen / num).absValue() > regularRaster)
                              return false;
                        }
                  }
            }
                  // for all tuplets
      int minAllowedNoteCount = tupletNumber / 2 + tupletNumber / 4;
      if ((int)tupletChords.size() < minAllowedNoteCount)
            return false;
                  // allow duplets and quadruplets with the zero error == regular error
      if (tupletNumber == 2 || tupletNumber == 4) {
            if (tupletOnTimeSumError > regularSumError)
                  return false;
            else if (tupletOnTimeSumError == regularSumError
                     && tupletOnTimeSumError > Fraction(0))
                  return false;
            }
      else {
            if (tupletOnTimeSumError >= regularSumError)
                  return false;
            }
                  // only notes with len >= (half tuplet note len) allowed
      Fraction tupletNoteLen = tupletLen / tupletNumber;
      for (const auto &tupletChord: tupletChords) {
            for (const auto &note: tupletChord.second->second.notes) {
                  if (note.len >= tupletNoteLen / 2)
                        return true;
                  }
            }

      return false;
      }

std::vector<int> findTupletNumbers(const Fraction &divLen, const Fraction &barFraction)
      {
      auto operations = preferences.midiImportOperations.currentTrackOperations();
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

Fraction findOnTimeRegularError(const Fraction &onTime, const Fraction &raster)
      {
      Fraction regularPos = Quantize::quantizeValue(onTime, raster);
      return (onTime - regularPos).absValue();
      }

// return: <chordIter, minChordError>

std::pair<std::multimap<Fraction, MidiChord>::iterator, Fraction>
findBestChordForTupletNote(const Fraction &tupletNotePos,
                           const Fraction &quantValue,
                           const std::multimap<Fraction, MidiChord>::iterator &startChordIt,
                           const std::multimap<Fraction, MidiChord>::iterator &endChordIt)
      {
                  // choose the best chord, if any, for this tuplet note
      std::pair<std::multimap<Fraction, MidiChord>::iterator, Fraction> bestChord;
      bestChord.first = endChordIt;
                  // check chords - whether they can be in tuplet without large error
      bool firstLoop = true;
      for (auto chordIt = startChordIt; chordIt != endChordIt; ++chordIt) {
            Fraction tupletError = (chordIt->first - tupletNotePos).absValue();
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

Fraction maxNoteLen(const QList<MidiNote> &notes)
      {
      Fraction maxLen;
      for (const auto &note: notes) {
            if (note.len > maxLen)
                  maxLen = note.len;
            }
      return maxLen;
      }


// find tuplets over which duration lies

std::vector<TupletData>
findTupletsForDuration(int voice,
                       const Fraction &barTick,
                       const Fraction &durationOnTime,
                       const Fraction &durationLen,
                       const std::multimap<Fraction, TupletData> &tuplets)
      {
      std::vector<TupletData> tupletsData;
      if (tuplets.empty())
            return tupletsData;
      auto tupletIt = tuplets.lower_bound(durationOnTime + durationLen);
      if (tupletIt != tuplets.begin())
            --tupletIt;
      while (durationOnTime + durationLen > tupletIt->first
             && durationOnTime < tupletIt->first + tupletIt->second.len) {
            if (tupletIt->second.voice == voice) {
                              // if tuplet and duration intersect each other
                  auto tupletData = tupletIt->second;
                              // convert tuplet onTime to local bar ticks
                  tupletData.onTime -= barTick;
                  tupletsData.push_back(tupletData);
                  }
            if (tupletIt == tuplets.begin())
                  break;
            --tupletIt;
            }

      struct {
            bool operator()(const TupletData &d1, const TupletData &d2)
                  {
                  return (d1.len > d2.len);
                  }
            } comparator;
                  // sort by tuplet length in desc order
      sort(tupletsData.begin(), tupletsData.end(), comparator);

      return tupletsData;
      }

TupletInfo findTupletApproximation(const Fraction &tupletLen,
                                   int tupletNumber,
                                   const Fraction &raster,
                                   const Fraction &startTupletTime,
                                   const std::multimap<Fraction, MidiChord>::iterator &startChordIt,
                                   const std::multimap<Fraction, MidiChord>::iterator &endChordIt)
      {
      TupletInfo tupletInfo;
      tupletInfo.tupletNumber = tupletNumber;
      tupletInfo.onTime = startTupletTime;
      tupletInfo.len = tupletLen;
      tupletInfo.tupletQuantValue = tupletLen / tupletNumber;
      while (tupletInfo.tupletQuantValue / 2 >= raster)
            tupletInfo.tupletQuantValue /= 2;
      tupletInfo.regularQuantValue = raster;

      auto startTupletChordIt = startChordIt;
      for (int k = 0; k != tupletNumber; ++k) {
            Fraction tupletNotePos = startTupletTime + tupletLen / tupletNumber * k;
                        // choose the best chord, if any, for this tuplet note
            auto bestChord = findBestChordForTupletNote(tupletNotePos, raster,
                                                        startTupletChordIt, endChordIt);
            if (bestChord.first == endChordIt)
                  continue;   // no chord fits to this tuplet note position
                        // chord can be in tuplet
            tupletInfo.chords.insert({k, bestChord.first});
            tupletInfo.tupletSumError += bestChord.second;
                        // for next tuplet note we start from the next chord
                        // because chord for the next tuplet note cannot be earlier
            startTupletChordIt = bestChord.first;
            ++startTupletChordIt;
                        // find chord quant error for a regular grid
            Fraction regularError = findOnTimeRegularError(bestChord.first->first, raster);
            tupletInfo.regularSumError += regularError;
            }

      Fraction beg = tupletInfo.onTime;
      Fraction tupletEndTime = (tupletInfo.onTime + tupletInfo.len);
      for (const auto &chord: tupletInfo.chords) {
            const MidiChord &midiChord = chord.second->second;
            const Fraction &chordOnTime = chord.second->first;
                        // approximate length of gaps between successive chords,
                        // quantization is not taken into account
            if (beg < chordOnTime)
                  tupletInfo.sumLengthOfRests += (chordOnTime - beg);
            beg = chordOnTime + maxNoteLen(midiChord.notes);
            if (beg >= tupletInfo.onTime + tupletInfo.len)
                  break;
            }
      if (beg < tupletEndTime)
            tupletInfo.sumLengthOfRests += (tupletEndTime - beg);

      return tupletInfo;
      }

void markChordsAsUsed(std::map<std::pair<const Fraction, MidiChord> *, int> &usedFirstTupletNotes,
                      std::set<std::pair<const Fraction, MidiChord> *> &usedChords,
                      const std::map<int, std::multimap<Fraction, MidiChord>::iterator> &tupletChords)
      {
      if (tupletChords.empty())
            return;

      auto i = tupletChords.begin();
      int tupletNoteIndex = i->first;
      if (tupletNoteIndex == 0) {
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
            const std::map<std::pair<const Fraction, MidiChord> *, int> &usedFirstTupletNotes,
            const std::set<std::pair<const Fraction, MidiChord> *> &usedChords,
            const std::map<int, std::multimap<Fraction, MidiChord>::iterator> &tupletChords)
      {
      if (tupletChords.empty())
            return false;

      auto operations = preferences.midiImportOperations.currentTrackOperations();

      auto i = tupletChords.begin();
      int tupletNoteIndex = i->first;
      if (tupletNoteIndex == 0) {
                        // check are first tuplet notes all in use (1 note - 1 voice)
            auto ii = usedFirstTupletNotes.find(&*(i->second));
            if (ii != usedFirstTupletNotes.end()) {
                  if (!operations.useMultipleVoices && ii->second > 1)
                        return true;
                  else if (ii->second >= i->second->second.notes.size()
                              || ii->second >= VOICES) {
                              // need to choose next tuplet candidate - no more available voices
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

// result: <average quant error, total tuplet note count, sum length of rests inside all tuplets>

std::tuple<double, int, Fraction>
validateTuplets(std::list<int> &indexes, const std::vector<TupletInfo> &tuplets)
      {
      if (tuplets.empty())
            return std::make_tuple(0.0, 0, Fraction());
                  // structure of map: <chord ID, count of use of first tuplet chord with this tick>
      std::map<std::pair<const Fraction, MidiChord> *, int> usedFirstTupletNotes;
                  // chord IDs of already used chords
      std::set<std::pair<const Fraction, MidiChord> *> usedChords;
      std::set<std::pair<const Fraction, MidiChord> *> excludedChords;
                  // select tuplets with min average error
      for (auto it = indexes.begin(); it != indexes.end(); ) {
            const auto &tupletChords = tuplets[*it].chords;
                        // check for chord notes that are already in use in another tuplets
            if (tupletChords.empty()
                        || areTupletChordsInUse(usedFirstTupletNotes, usedChords, tupletChords)) {
                  for (const auto &chord: tupletChords)
                        excludedChords.insert(&*chord.second);
                  it = indexes.erase(it);
                  continue;
                  }
                        // we can use this tuplet
            markChordsAsUsed(usedFirstTupletNotes, usedChords, tupletChords);
            ++it;
            }

      Fraction sumError;
      int sumNoteCount = 0;
      Fraction sumLengthOfRests;

      for (const auto &i: indexes) {
            sumError += tuplets[i].tupletSumError;
            sumNoteCount += tuplets[i].chords.size();
            sumLengthOfRests += tuplets[i].sumLengthOfRests;
            }
                  // add quant error of all chords excluded from tuplets
      for (const auto &i: indexes) {
            for (const auto &chord: tuplets[i].chords)
                  excludedChords.erase(&*chord.second);
            }
      const Fraction &regularRaster = tuplets.front().regularQuantValue;
      for (const auto &chordIt: excludedChords)
            sumError += findOnTimeRegularError(chordIt->first, regularRaster);

      return std::make_tuple(sumError.ticks() * 1.0 / sumNoteCount,
                             sumNoteCount, sumLengthOfRests);
      }

// try different permutations of tuplet indexes to minimize error

std::list<int>
minimizeQuantError(std::vector<std::vector<int>> &indexGroups,
                   const std::vector<TupletInfo> &tuplets)
      {
      std::tuple<double, int, Fraction> minResult;
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

            auto result = validateTuplets(indexesToValidate, tuplets);
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
      std::set<std::pair<const Fraction, MidiChord> *> chordsI;
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
      auto it = restTuplets.begin();
      tupletGroup.push_back(*it);
      q.enqueue(*it);
      it = restTuplets.erase(it);

      while (!q.isEmpty() && !restTuplets.empty()) {
            int index = q.dequeue();
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

Fraction noteLenQuantError(const Fraction &noteOnTime, const Fraction &noteLen,
                           const Fraction &raster)
      {
      Fraction offTime = Quantize::quantizeValue(noteOnTime + noteLen, raster);
      Fraction quantizedLen = offTime - noteOnTime;
      return (noteLen - quantizedLen).absValue();
      }

// first tuplet notes with offTime quantization error,
// that is greater for tuplet quantum rather than for regular quantum,
// are removed from tuplet, except that was the last note

void minimizeOffTimeError(std::vector<TupletInfo> &tuplets,
                          std::multimap<Fraction, MidiChord> &chords,
                          int nonTupletVoice)
      {
      for (auto it = tuplets.begin(); it != tuplets.end(); ++it) {
            TupletInfo &tupletInfo = *it;
            auto firstChord = tupletInfo.chords.begin();
            if (firstChord == tupletInfo.chords.end() || firstChord->first != 0)
                  continue;
            auto &notes = firstChord->second->second.notes;
            const Fraction &onTime = firstChord->second->first;
            std::vector<int> removedIndexes;
            std::vector<int> leavedIndexes;
            for (int i = 0; i != notes.size(); ++i) {
                  const auto &note = notes[i];
                  if (note.len <= tupletInfo.len) {
                              // remove notes with big offTime quant error;
                              // and here is a tuning for clearer tuplet processing:
                              //    if tuplet has only one (first) chord -
                              //        we can remove all notes and erase tuplet;
                              //    if tuplet has multiple chords -
                              //        we should leave at least one note in the first chord
                        if ((tupletInfo.chords.size() == 1
                                    && notes.size() > (int)removedIndexes.size())
                                 || (tupletInfo.chords.size() > 1
                                    && notes.size() > (int)removedIndexes.size() + 1))
                              {
                              Fraction tupletError = noteLenQuantError(
                                                onTime, note.len, tupletInfo.tupletQuantValue);
                              Fraction regularError = noteLenQuantError(
                                                onTime, note.len, tupletInfo.regularQuantValue);
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
                  for (const auto &i: leavedIndexes)
                        newTupletChord.notes.push_back(notes[i]);

                  QList<MidiNote> newNotes;
                  for (const auto &i: removedIndexes)
                        newNotes.push_back(notes[i]);
                  notes = newNotes;
                              // set voice for non-tuplet chord
                  if (nonTupletVoice != -1)
                        firstChord->second->second.voice = nonTupletVoice;
                  else {
                        int maxVoice = 0;
                        for (const auto &tupletInfo: tuplets) {
                              int tupletVoice = (tupletInfo.chords.empty())
                                          ? -1 : tupletInfo.chords.begin()->second->second.voice;
                              if (tupletVoice > maxVoice)
                                    maxVoice = tupletVoice;
                              }
                        firstChord->second->second.voice = maxVoice + 1;
                        }
                              // update tuplet chord
                  if (!newTupletChord.notes.empty())
                        firstChord->second = chords.insert({onTime, newTupletChord});
                  else {
                        tupletInfo.chords.erase(0);  // erase first (index 0) empty chord in tuplet
                        if (tupletInfo.chords.empty()) {
                              it = tuplets.erase(it);  // remove tuplet without chords
                              --it;
                              continue;
                              }
                        }
                  }
            }
      }

int averagePitch(const std::map<int, std::multimap<Fraction, MidiChord>::iterator> &chords)
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

int averagePitch(const std::vector<std::multimap<Fraction, MidiChord>::iterator> &chords)
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

void sortNotesByPitch(const std::multimap<Fraction, MidiChord>::iterator &startBarChordIt,
                      const std::multimap<Fraction, MidiChord>::iterator &endBarChordIt)
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

std::vector<std::multimap<Fraction, MidiChord>::iterator>
findNonTupletChords(const std::vector<TupletInfo> &tuplets,
                    const std::multimap<Fraction, MidiChord>::iterator &startBarChordIt,
                    const std::multimap<Fraction, MidiChord>::iterator &endBarChordIt)
      {
      std::set<std::pair<const Fraction, MidiChord> *> tupletChords;
      for (const auto &tupletInfo: tuplets) {
            for (const auto &tupletChord: tupletInfo.chords) {
                  auto tupletIt = tupletChord.second;
                  tupletChords.insert(&*tupletIt);
                  }
            }
      std::vector<std::multimap<Fraction, MidiChord>::iterator> nonTuplets;
      for (auto it = startBarChordIt; it != endBarChordIt; ++it) {
            if (tupletChords.find(&*it) == tupletChords.end())
                  nonTuplets.push_back(it);
            }
      return nonTuplets;
      }

bool hasIntersectionWithChord(
            const Fraction &startTick,
            const Fraction &endTick,
            const Fraction &regularRaster,
            const std::vector<std::multimap<Fraction, MidiChord>::iterator> &nonTupletChords)
      {
      for (const auto &chordEvent: nonTupletChords) {
            MidiChord midiChord = chordEvent->second;
            Fraction onTime = Quantize::quantizeValue(chordEvent->first, regularRaster);
            for (auto &note: midiChord.notes)
                  note.len = Quantize::quantizeValue(note.len, regularRaster);
            if (endTick > onTime && startTick < onTime + maxNoteLen(midiChord.notes))
                  return true;
            }
      return false;
      }

// input tuplets are sorted by average pitch in desc. order

int findNonTupletVoice(const std::vector<std::multimap<Fraction, MidiChord>::iterator> &nonTuplets,
                       const std::vector<TupletInfo> &sortedTuplets,
                       const Fraction &regularRaster)
      {
      if (nonTuplets.empty())
            return -1;
      int nonTupletPitch = averagePitch(nonTuplets);
      int voice = 0;
      for (const auto &tupletInfo: sortedTuplets) {
            if (!hasIntersectionWithChord(tupletInfo.onTime,
                                          tupletInfo.onTime + tupletInfo.len,
                                          regularRaster,
                                          nonTuplets))
                  continue;
            int tupletPitch = averagePitch(tupletInfo.chords);
            if (nonTupletPitch >= tupletPitch)
                  return voice;
            ++voice;
            }
      return voice;
      }

void setVoiceOfNonTupletChords(
            const std::vector<std::multimap<Fraction, MidiChord>::iterator> &nonTuplets,
            int voice)
      {
      if (voice < 0)
            return;
      for (auto &chord: nonTuplets)
            chord->second.voice = voice;
      }

// the input tuplets should be filtered (for mutual validity)
// result - voice of non-tuplet chords or -1 if there are no non-tuplet chords

int separateTupletVoices(std::vector<TupletInfo> &tuplets,
                         const std::multimap<Fraction, MidiChord>::iterator &startBarChordIt,
                         const std::multimap<Fraction, MidiChord>::iterator &endBarChordIt,
                         std::multimap<Fraction, MidiChord> &chords,
                         const Fraction &endBarTick)
      {
                  // it's better before to sort tuplets by their average pitch
                  // and notes of every chord as well
      sortNotesByPitch(startBarChordIt, endBarChordIt);
      sortTupletsByAveragePitch(tuplets);

      Fraction regularRaster = Quantize::findQuantRaster(startBarChordIt, endBarChordIt, endBarTick);
      auto nonTuplets = findNonTupletChords(tuplets, startBarChordIt, endBarChordIt);
      int nonTupletVoice = findNonTupletVoice(nonTuplets, tuplets, regularRaster);
      setVoiceOfNonTupletChords(nonTuplets, nonTupletVoice);

      for (auto now = tuplets.begin(); now != tuplets.end(); ++now) {
            int voice = 0;
            auto lastMatch = tuplets.end();
            auto firstNowChordIt = now->chords.begin();
            bool flag = false;
            for (auto prev = tuplets.begin(); prev != now; ) {
                              // check is now tuplet go over previous tuplets and non-tuplet chords
                  if (!flag && prev - tuplets.begin() == nonTupletVoice) {
                        if (hasIntersectionWithChord(now->onTime, now->onTime + now->len,
                                                     regularRaster, nonTuplets))
                              ++voice;
                        flag = true;
                        continue;
                        }
                  else {
                        if (now->onTime + now->len > prev->onTime
                                    && now->onTime < prev->onTime + prev->len)
                              ++voice;
                        }
                  if (flag)
                        flag = false;
                              // if first notes in tuplets match - split this chord
                              // into 2 voices
                  auto firstPrevChordIt = prev->chords.begin();
                  if (firstNowChordIt->first == 0 && firstPrevChordIt->first == 0
                              && firstNowChordIt->second == firstPrevChordIt->second) {
                        lastMatch = prev;
                        }
                  ++prev;
                  }
            if (lastMatch != tuplets.end()) {
                              // split first tuplet chord, that belong to 2 tuplets, into 2 voices
                  MidiChord &prevMidiChord = lastMatch->chords.begin()->second->second;
                  Fraction onTime = lastMatch->chords.begin()->first;
                  MidiChord newChord = prevMidiChord;
                              // erase all notes except the first one
                  auto beg = newChord.notes.begin();
                  newChord.notes.erase(++beg, newChord.notes.end());
                              // erase the first note
                  prevMidiChord.notes.erase(prevMidiChord.notes.begin());
                  lastMatch->chords.begin()->second = chords.insert({onTime, newChord});
                  if (prevMidiChord.notes.isEmpty()) {
                                    // normally this should not happen at all
                                    // because of filtering of tuplets
                        qDebug("Tuplets were not filtered correctly: same notes in different tuplets");
                        break;
                        }
                  }

            for (auto &tupletChord: now->chords) {
                  MidiChord &midiChord = tupletChord.second->second;
                  midiChord.voice = voice;
                  }
            }
      return nonTupletVoice;
      }

Fraction findRasterForTupletNote(const Fraction &noteOnTime,
                                 const Fraction &noteLen,
                                 const TupletInfo &tupletInfo)
      {
      Fraction raster;
      if (noteOnTime + noteLen <= tupletInfo.onTime + tupletInfo.len) {
                        // if offTime is inside the tuplet - quant by tuplet grid
            raster = tupletInfo.tupletQuantValue;
            }
      else {            // if offTime is outside the tuplet - quant by regular grid
            raster = tupletInfo.regularQuantValue;
            }
      return raster;
      }

void optimizeVoices(std::multimap<Fraction, MidiChord>::iterator startBarChordIt,
                    std::multimap<Fraction, MidiChord>::iterator endBarChordIt)
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

std::vector<TupletInfo> findTuplets(const Fraction &startBarTick,
                                    const Fraction &endBarTick,
                                    const Fraction &barFraction,
                                    std::multimap<Fraction, MidiChord> &chords)
      {
      std::vector<TupletInfo> tuplets;
      if (chords.empty() || startBarTick >= endBarTick)     // invalid cases
            return tuplets;

      auto operations = preferences.midiImportOperations.currentTrackOperations();
      if (!operations.tuplets.doSearch)
            return tuplets;

      Fraction tol = Quantize::fixedQuantRaster() / 2;
      auto startBarChordIt = findFirstChordInRange(startBarTick - tol, endBarTick,
                                                   chords.begin(), chords.end());
      if (startBarChordIt == chords.end()) // no chords in this bar
            return tuplets;
                  // end iterator, as usual, will point to the next - invalid chord
      auto endBarChordIt = findEndChordInRange(endBarTick + tol, startBarChordIt, chords.end());

      Fraction raster = Quantize::findQuantRaster(startBarChordIt, endBarChordIt, endBarTick);
      auto divLengths = Meter::divisionsOfBarForTuplets(barFraction);

      for (const auto &divLen: divLengths) {
            auto tupletNumbers = findTupletNumbers(divLen, barFraction);
            auto div = barFraction / divLen;
            int divCount = qRound(div.numerator() * 1.0 / div.denominator());

            for (int i = 0; i != divCount; ++i) {
                  Fraction startDivTime = startBarTick + divLen * i;
                  Fraction endDivTime = startBarTick + divLen * (i + 1);
                              // check which chords can be inside tuplet period
                              // [startDivTime - quantValue, endDivTime + quantValue]
                  auto startDivChordIt = findFirstChordInRange(startDivTime - raster,
                                                               endDivTime + raster,
                                                               startBarChordIt,
                                                               endBarChordIt);
                  if (startDivChordIt == endBarChordIt) // no chords in this division
                        continue;
                              // end iterator, as usual, will point to the next - invalid chord
                  auto endDivChordIt = findEndChordInRange(endDivTime + raster, startDivChordIt,
                                                           endBarChordIt);
                              // try different tuplets, nested tuplets are not allowed
                  for (const auto &tupletNumber: tupletNumbers) {
                        Fraction tupletNoteLen = divLen / tupletNumber;
                        if (tupletNoteLen < raster)
                              continue;
                        auto tupletInfo = findTupletApproximation(divLen, tupletNumber,
                                    raster, startDivTime, startDivChordIt, endDivChordIt);
                                    // check - is it a valid tuplet approximation?
                        if (!isTupletAllowed(tupletNumber, divLen,
                                             tupletInfo.tupletSumError,
                                             tupletInfo.regularSumError,
                                             raster, tupletInfo.chords))
                              continue;
                                    // tuplet found
                        tuplets.push_back(tupletInfo);
                        }     // next tuplet type
                  }
            }
      filterTuplets(tuplets);

      if (operations.useMultipleVoices) {
            int nonTupletVoice = separateTupletVoices(tuplets, startBarChordIt, endBarChordIt,
                                                      chords, endBarTick);
            minimizeOffTimeError(tuplets, chords, nonTupletVoice);
            optimizeVoices(startBarChordIt, endBarChordIt);
            }

      return tuplets;
      }

} // namespace MidiTuplet
} // namespace Ms
