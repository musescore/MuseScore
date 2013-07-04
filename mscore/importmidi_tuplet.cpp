#include "importmidi_tuplet.h"
#include "libmscore/fraction.h"
#include "importmidi_chord.h"
#include "importmidi_meter.h"
#include "importmidi_quant.h"

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
            {7, Fraction({7, 4})}
            };
      return ratios;
      }

bool isTupletAllowed(int tupletNumber,
                     int tupletLen,
                     int tupletOnTimeSumError,
                     int regularSumError,
                     int quantValue,
                     const std::map<int, std::multimap<int, MidiChord>::iterator> &tupletChords)
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
                        if (std::abs(note.len - tupletLen / num) > quantValue)
                              return false;
                        }
                  }
            }
                  // for all tuplets
      int minAllowedNoteCount = tupletNumber / 2 + tupletNumber / 4;
      if ((int)tupletChords.size() < minAllowedNoteCount
                  || tupletOnTimeSumError >= regularSumError) {
            return false;
            }

      int tupletNoteLen = tupletLen / tupletNumber;
      for (const auto &tupletChord: tupletChords) {
            for (const auto &note: tupletChord.second->second.notes) {
                  if (note.len >= tupletNoteLen / 2)
                        return true;
                  }
            }

      return false;
      }

std::vector<int> findTupletNumbers(int divLen, const Fraction &barFraction)
      {
      std::vector<int> tupletNumbers;
      if (Meter::isCompound(barFraction) && divLen == Meter::beatLength(barFraction))
            tupletNumbers = {2, 4};       // duplets and quadruplets
      else
            tupletNumbers = {3, 5, 7};
      return tupletNumbers;
      }

int findOnTimeRegularError(int onTime, int quantValue)
      {
      int regularPos = ((onTime + quantValue / 2) / quantValue) * quantValue;
      return std::abs(onTime - regularPos);
      }

// return: <chordIter, minChordError>

std::pair<std::multimap<int, MidiChord>::iterator, int>
findBestChordForTupletNote(int tupletNotePos,
                           int quantValue,
                           const std::multimap<int, MidiChord>::iterator &startChordIt,
                           const std::multimap<int, MidiChord>::iterator &endChordIt)
      {
                  // choose the best chord, if any, for this tuplet note
      std::pair<std::multimap<int, MidiChord>::iterator, int> bestChord;
      bestChord.first = endChordIt;
      bestChord.second = std::numeric_limits<int>::max();
                  // check chords - whether they can be in tuplet without large error
      for (auto chordIt = startChordIt; chordIt != endChordIt; ++chordIt) {
            int tupletError = std::abs(chordIt->first - tupletNotePos);
            if (tupletError > quantValue)
                  continue;
            if (tupletError < bestChord.second) {
                  bestChord.first = chordIt;
                  bestChord.second = tupletError;
                  }
            }
      return bestChord;
      }

TupletInfo findTupletApproximation(int tupletNumber,
                                   int tupletNoteLen,
                                   int quantValue,
                                   int startTupletTime,
                                   const std::multimap<int, MidiChord>::iterator &startChordIt,
                                   const std::multimap<int, MidiChord>::iterator &endChordIt)
      {
      TupletInfo tupletInfo;
      tupletInfo.tupletNumber = tupletNumber;
      tupletInfo.onTime = startTupletTime;
      tupletInfo.len = tupletNoteLen * tupletNumber;
      tupletInfo.tupletQuantValue = tupletNoteLen;
      while (tupletInfo.tupletQuantValue / 2 >= quantValue)
            tupletInfo.tupletQuantValue /= 2;
      tupletInfo.regularQuantValue = quantValue;

      auto startTupletChordIt = startChordIt;
      for (int k = 0; k != tupletNumber; ++k) {
            int tupletNotePos = startTupletTime + k * tupletNoteLen;
                        // choose the best chord, if any, for this tuplet note
            auto bestChord = findBestChordForTupletNote(tupletNotePos, quantValue,
                                                        startTupletChordIt, endChordIt);
            if (bestChord.first == endChordIt)
                  continue;   // no chord fits to this tuplet note position
                        // chord can be in tuplet
            tupletInfo.chords.insert({k, bestChord.first});
            tupletInfo.tupletOnTimeSumError += bestChord.second;
                        // for next tuplet note we start from the next chord
                        // because chord for the next tuplet note cannot be earlier
            startTupletChordIt = bestChord.first;
            ++startTupletChordIt;
                        // find chord quant error for a regular grid
            int regularError = findOnTimeRegularError(bestChord.first->first, quantValue);
            tupletInfo.regularSumError += regularError;
            }

      return tupletInfo;
      }

std::multimap<double, TupletInfo>
findTupletCandidatesOfBar(int startBarTick,
                          int endBarTick,
                          const Fraction &barFraction,
                          std::multimap<int, MidiChord> &chords)
      {
      std::multimap<double, TupletInfo> tupletCandidates;   // average error, TupletInfo
      if (chords.empty() || startBarTick >= endBarTick)     // invalid cases
            return tupletCandidates;

      int barLen = barFraction.ticks();
                  // barLen / 4 - additional tolerance
      auto startBarChordIt = findFirstChordInRange(startBarTick - barLen / 4,
                                                   endBarTick,
                                                   chords.begin(),
                                                   chords.end());
      if (startBarChordIt == chords.end()) // no chords in this bar
            return tupletCandidates;
                  // end iterator, as usual, will point to the next - invalid chord
      auto endBarChordIt = findEndChordInRange(endBarTick + barLen / 4, startBarChordIt, chords.end());

      int quantValue = Quantize::findQuantRaster(startBarChordIt, endBarChordIt, endBarTick);
      auto divLengths = Meter::divisionsOfBarForTuplets(barFraction);

      for (const auto &divLen: divLengths) {
            auto tupletNumbers = findTupletNumbers(divLen, barFraction);
            int divCount = barLen / divLen;

            for (int i = 0; i != divCount; ++i) {
                  int startDivTime = startBarTick + i * divLen;
                  int endDivTime = startDivTime + divLen;
                              // check which chords can be inside tuplet period
                              // [startDivTime - quantValue, endDivTime + quantValue]
                  auto startDivChordIt = findFirstChordInRange(startDivTime - quantValue,
                                                               endDivTime + quantValue,
                                                               startBarChordIt,
                                                               endBarChordIt);
                  if (startDivChordIt == endBarChordIt) // no chords in this division
                        continue;
                              // end iterator, as usual, will point to the next - invalid chord
                  auto endDivChordIt = findEndChordInRange(endDivTime + quantValue, startDivChordIt,
                                                           endBarChordIt);
                              // try different tuplets, nested tuplets are not allowed
                  for (const auto &tupletNumber: tupletNumbers) {
                        int tupletNoteLen = divLen / tupletNumber;
                        if (tupletNoteLen < quantValue)
                              continue;
                        TupletInfo tupletInfo = findTupletApproximation(tupletNumber, tupletNoteLen,
                                    quantValue, startDivTime, startDivChordIt, endDivChordIt);
                                    // check - is it a valid tuplet approximation?
                        if (!isTupletAllowed(tupletNumber, divLen,
                                             tupletInfo.tupletOnTimeSumError,
                                             tupletInfo.regularSumError,
                                             quantValue, tupletInfo.chords))
                              continue;
                                    // tuplet found
                        double averageError = tupletInfo.tupletOnTimeSumError * 1.0 / tupletInfo.chords.size();
                        tupletCandidates.insert({averageError, tupletInfo});
                        }     // next tuplet type
                  }
            }
      return tupletCandidates;
      }

void markChordsAsUsed(std::map<int, int> &usedFirstTupletNotes,
                      std::set<int> &usedChords,
                      const std::map<int, std::multimap<int, MidiChord>::iterator> &tupletChords)
      {
      if (tupletChords.empty())
            return;

      auto i = tupletChords.begin();
      int tupletNoteIndex = i->first;
      if (tupletNoteIndex == 0) {
                        // check is the note of the first tuplet chord in use
            int chordOnTime = i->second->first;
            auto ii = usedFirstTupletNotes.find(chordOnTime);
            if (ii == usedFirstTupletNotes.end())
                  ii = usedFirstTupletNotes.insert({chordOnTime, 1}).first;
            else
                  ++(ii->second);         // increase chord note counter
            ++i;              // start from the second chord
            }
      for ( ; i != tupletChords.end(); ++i) {
                        // mark the chord as used
            int chordOnTime = i->second->first;
            usedChords.insert(chordOnTime);
            }
      }

bool areTupletChordsInUse(const std::map<int, int> &usedFirstTupletNotes,
                          const std::set<int> &usedChords,
                          const std::map<int, std::multimap<int, MidiChord>::iterator> &tupletChords)
      {
      if (tupletChords.empty())
            return false;

      auto i = tupletChords.begin();
      int tupletNoteIndex = i->first;
      if (tupletNoteIndex == 0) {
                        // check are first tuplet notes all in use (1 note - 1 voice)
            int chordOnTime = i->second->first;
            auto ii = usedFirstTupletNotes.find(chordOnTime);
            if (ii != usedFirstTupletNotes.end()) {
                  if (ii->second >= i->second->second.notes.size()) {
                              // need to choose next tuplet candidate - no more available voices
                        return true;
                        }
                  }
            ++i;
      }
      for ( ; i != tupletChords.end(); ++i) {
            int chordOnTime = i->second->first;
            if (usedChords.find(chordOnTime) != usedChords.end()) {
                              // the chord note is in use - cannot use this chord again
                  return true;
                  }
            }
      return false;
      }

// use case for this: first chord in tuplet can belong
// to any other tuplet at the same time
// if there are enough notes in this first chord
// to be splitted to different voices

void filterTuplets(std::multimap<double, TupletInfo> &tuplets)
      {
                  // structure of map: <tick, count of use of first tuplet chord with this tick>
      std::map<int, int> usedFirstTupletNotes;
                  // onTime values - tick - of already used chords
      std::set<int> usedChords;
                  // select tuplets with min average error
      for (auto tc = tuplets.begin(); tc != tuplets.end(); ) {  // tc - tuplet candidate
            auto &tupletChords = tc->second.chords;
                        // check for chords notes already used in another tuplets
            if (tupletChords.empty()
                        || areTupletChordsInUse(usedFirstTupletNotes, usedChords, tupletChords)) {
                  tc = tuplets.erase(tc);
                  continue;
                  }
                        // we can use this tuplet
            markChordsAsUsed(usedFirstTupletNotes, usedChords, tupletChords);
            ++tc;
            }
      }

int averagePitch(const std::map<int, std::multimap<int, MidiChord>::iterator> &chords)
      {
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

void sortNotesByPitch(std::multimap<int, MidiChord> &chords)
      {
      struct {
            bool operator()(const MidiNote &n1, const MidiNote &n2)
                  {
                  return (n1.pitch > n2.pitch);
                  }
            } pitchComparator;

      for (auto &chordEvent: chords) {
            auto &midiNotes = chordEvent.second.notes;
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

// all chords here have different onTime values

std::vector<std::multimap<int, MidiChord>::const_iterator>
nonTupletChords(const std::vector<TupletInfo> &tuplets,
                const std::multimap<int, MidiChord> &chords)
      {
      std::set<int> tupletOnTimes;
      for (const auto &tupletInfo: tuplets) {
            for (const auto &chordIt: tupletInfo.chords) {
                  tupletOnTimes.insert(chordIt.second->second.onTime);
                  }
            }
      std::vector<std::multimap<int, MidiChord>::const_iterator> nonTuplets;
      for (auto it = chords.begin(); it != chords.end(); ++it) {
            if (tupletOnTimes.find(it->first) == tupletOnTimes.end())
                  nonTuplets.push_back(it);
            }
      return nonTuplets;
      }

bool hasIntersectionWithChord(int startTick, int endTick,
            const std::vector<std::multimap<int, MidiChord>::const_iterator> &nonTupletChords)
      {
      for (const auto &chordEvent: nonTupletChords) {
            const MidiChord &midiChord = chordEvent->second;
            if (endTick > midiChord.onTime
                        && startTick < midiChord.onTime + midiChord.duration)
                  return true;
            }
      return false;
      }

// the input tuplets should be filtered (for mutual validity)

void separateTupletVoices(std::vector<TupletInfo> &tuplets,
                          std::multimap<int, MidiChord> &chords)
      {
                  // it's better before to sort tuplets by their average pitch
                  // and notes of each chord as well (desc. order)
      sortNotesByPitch(chords);
      sortTupletsByAveragePitch(tuplets);
      auto nonTuplets = nonTupletChords(tuplets, chords);

      for (auto now = tuplets.begin(); now != tuplets.end(); ++now) {
            int voice = 0;
            auto lastMatch = tuplets.end();
            auto firstNowChordIt = now->chords.begin();

            if (hasIntersectionWithChord(now->onTime, now->onTime + now->len, nonTuplets))
                  ++voice;
            for (auto prev = tuplets.begin(); prev != now; ++prev) {
                              // check is now tuplet go over previous tuplets
                  if (now->onTime + now->len > prev->onTime
                              && now->onTime < prev->onTime + prev->len)
                        ++voice;
                              // if first notes in tuplets match - split this chord
                              // into 2 voices
                  auto firstPrevChordIt = prev->chords.begin();
                  if (firstNowChordIt->first == 0 && firstPrevChordIt->first == 0
                              && firstNowChordIt->second->second.onTime
                              == firstPrevChordIt->second->second.onTime) {
                        lastMatch = prev;
                        }
                  }
            if (lastMatch != tuplets.end()) {
                              // split first tuplet chord, that belong to 2 tuplets, into 2 voices
                  MidiChord &prevMidiChord = lastMatch->chords.begin()->second->second;
                  MidiChord newChord = prevMidiChord;
                              // erase all notes except the first one
                  auto beg = prevMidiChord.notes.begin();
                  prevMidiChord.notes.erase(++beg, prevMidiChord.notes.end());
                              // erase the first note
                  newChord.notes.erase(newChord.notes.begin());
                  auto newChordIt = chords.insert({newChord.onTime, newChord});
                              // update 'now' first tuplet chord
                  now->chords.begin()->second = newChordIt;
                  if (newChord.notes.isEmpty()) {
                                    // normally this should not happen at all
                        qDebug("Tuplets were not filtered correctly: same notes in different tuplets");
                        return;
                        }
                  }

            for (auto &tupletChord: now->chords) {
                  MidiChord &midiChord = tupletChord.second->second;
                  midiChord.voice = voice;
                  }
            }
      }

void quantizeTupletChord(MidiChord &midiChord, int onTime, const TupletInfo &tupletInfo)
      {
      midiChord.onTime = onTime;
      for (auto &note: midiChord.notes) {
            int raster;
            if (note.onTime + note.len <= tupletInfo.onTime + tupletInfo.len) {
                              // if offTime is inside the tuplet - quant by tuplet grid
                  raster = tupletInfo.tupletQuantValue;
                  }
            else {            // if offTime is outside the tuplet - quant by regular grid
                  raster = tupletInfo.regularQuantValue;
                  }
            int offTime = ((note.onTime + note.len + raster / 2) / raster) * raster;
            note.onTime = onTime;
            note.len = offTime - onTime;
            }
      }

} // namespace MidiTuplet
} // namespace Ms
