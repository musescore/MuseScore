#include "importmidi_tuplet.h"
#include "libmscore/fraction.h"
#include "importmidi_chord.h"
#include "importmidi_meter.h"
#include "importmidi_quant.h"
#include "libmscore/mscore.h"

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
                  // only notes with len >= (half tuplet note len) allowed
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

int maxNoteLen(const QList<MidiNote> &notes)
      {
      int maxLen = 0;
      for (const auto &note: notes) {
            if (note.len > maxLen)
                  maxLen = note.len;
            }
      return maxLen;
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
            tupletInfo.tupletSumError += bestChord.second;
                        // for next tuplet note we start from the next chord
                        // because chord for the next tuplet note cannot be earlier
            startTupletChordIt = bestChord.first;
            ++startTupletChordIt;
                        // find chord quant error for a regular grid
            int regularError = findOnTimeRegularError(bestChord.first->first, quantValue);
            tupletInfo.regularSumError += regularError;
            }
      tupletInfo.averageError = tupletInfo.tupletSumError * 1.0 / tupletInfo.chords.size();

      int beg = tupletInfo.onTime;
      int tupletEndTime = tupletInfo.onTime + tupletInfo.len;
      for (const auto &chord: tupletInfo.chords) {
            const MidiChord &midiChord = chord.second->second;
            tupletInfo.sumLengthOfRests += midiChord.onTime - beg;
            beg = midiChord.onTime + maxNoteLen(midiChord.notes);
            if (beg >= tupletInfo.onTime + tupletInfo.len)
                  break;
            }
      if (beg < tupletEndTime)
            tupletInfo.sumLengthOfRests += tupletEndTime - beg;

      return tupletInfo;
      }

void sortTupletCandidates(std::vector<TupletInfo> &tupletCandidates)
      {
      struct {
            bool operator()(const TupletInfo &t1, const TupletInfo &t2)
                  {
                  if (t1.averageError < t2.averageError)
                        return true;
                  if (t1.averageError == t2.averageError) {
                        if (t1.chords.size() > t2.chords.size())
                              return true;
                        if (t1.chords.size() == t2.chords.size()) {
                              if (t1.sumLengthOfRests < t2.sumLengthOfRests)
                                    return true;
                              }
                        }
                  return false;
                  }
            } errorComparator;

      std::sort(tupletCandidates.begin(), tupletCandidates.end(), errorComparator);
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
                  if (ii->second >= i->second->second.notes.size()
                              || ii->second >= VOICES) {
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

void filterTuplets(std::vector<TupletInfo> &tuplets)
      {
                  // structure of map: <tick, count of use of first tuplet chord with this tick>
      std::map<int, int> usedFirstTupletNotes;
                  // onTime values - tick - of already used chords
      std::set<int> usedChords;
                  // select tuplets with min average error
      for (auto tupletInfo = tuplets.begin(); tupletInfo != tuplets.end(); ) {
            auto &tupletChords = tupletInfo->chords;
                        // check for chords notes already used in another tuplets
            if (tupletChords.empty()
                        || areTupletChordsInUse(usedFirstTupletNotes, usedChords, tupletChords)) {
                  tupletInfo = tuplets.erase(tupletInfo);
                  continue;
                  }
                        // we can use this tuplet
            markChordsAsUsed(usedFirstTupletNotes, usedChords, tupletChords);
            ++tupletInfo;
            }
      }

int findNoteOffTime(int noteOnTime, int noteLen, int raster)
      {
      return ((noteOnTime + noteLen + raster / 2) / raster) * raster;
      }

int noteLenQuantError(int noteOnTime, int noteLen, int raster)
      {
      int offTime = findNoteOffTime(noteOnTime, noteLen, raster);
      int quantizedLen = offTime - noteOnTime;
      return std::abs(noteLen - quantizedLen);
      }

// first tuplet notes with length quantization error,
// that is greater for tuplet quantum rather than for regular quantum,
// are removed from tuplet, except that was the last note

void removeFirstNotesWithBigError(std::vector<TupletInfo> &tuplets,
                                  std::multimap<int, MidiChord> &chords)
      {
      for (TupletInfo &tupletInfo: tuplets) {
            auto firstChord = tupletInfo.chords.begin();
            if (firstChord == tupletInfo.chords.end() || firstChord->first != 0)
                  continue;
            MidiChord &tupletChord = firstChord->second->second;
            QList<MidiNote> &notes = tupletChord.notes;
            std::vector<int> removedIndexes;
            std::vector<int> leavedIndexes;
            for (int i = 0; i != notes.size(); ++i) {
                  const auto &note = notes[i];
                  if (note.len <= tupletInfo.len && notes.size() > (int)removedIndexes.size() + 1)
                        {
                        int tupletError = noteLenQuantError(
                                          note.onTime, note.len, tupletInfo.tupletQuantValue);
                        int regularError = noteLenQuantError(
                                          note.onTime, note.len, tupletInfo.regularQuantValue);
                        if (tupletError > regularError) {
                              removedIndexes.push_back(i);
                              continue;
                              }
                        }
                  leavedIndexes.push_back(i);
                  }
            if (!removedIndexes.empty()) {
                  MidiChord newTupletChord;
                  newTupletChord.onTime = tupletChord.onTime;
                  for (const auto &i: leavedIndexes)
                        newTupletChord.notes.push_back(tupletChord.notes[i]);

                  QList<MidiNote> newNotes;
                  for (const auto &i: removedIndexes)
                        newNotes.push_back(tupletChord.notes[i]);
                  tupletChord.notes = newNotes;

                  firstChord->second = chords.insert({newTupletChord.onTime, newTupletChord});
                  }
            }
      }

int averagePitch(const std::map<int, std::multimap<int, MidiChord>::iterator> &chords)
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

int averagePitch(const std::vector<std::multimap<int, MidiChord>::iterator> &chords)
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

void sortNotesByPitch(std::multimap<int, MidiChord>::iterator startBarChordIt,
                      std::multimap<int, MidiChord>::iterator endBarChordIt)
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

// all chords here have different onTime values

std::vector<std::multimap<int, MidiChord>::iterator>
findNonTupletChords(const std::vector<TupletInfo> &tuplets,
                    std::multimap<int, MidiChord>::iterator startBarChordIt,
                    std::multimap<int, MidiChord>::iterator endBarChordIt)
      {
      std::vector<std::multimap<int, MidiChord>::iterator> nonTuplets;
      for (auto it = startBarChordIt; it != endBarChordIt; ++it) {
            bool isTupletChord = false;
            for (const auto &tupletInfo: tuplets) {
                  for (const auto &tupletChord: tupletInfo.chords) {
                        auto tupletIt = tupletChord.second;
                        if (it == tupletIt) {
                              isTupletChord = true;
                              break;
                              }
                        }
                  if (isTupletChord)
                        break;
                  }
            if (!isTupletChord)
                  nonTuplets.push_back(it);
            }
      return nonTuplets;
      }

bool hasIntersectionWithChord(
            int startTick,
            int endTick,
            int regularRaster,
            const std::vector<std::multimap<int, MidiChord>::iterator> &nonTupletChords)
      {
      for (const auto &chordEvent: nonTupletChords) {
            MidiChord midiChord = chordEvent->second;
            Quantize::quantizeChord(midiChord, regularRaster);
            if (endTick > midiChord.onTime
                        && startTick < midiChord.onTime + maxNoteLen(midiChord.notes))
                  return true;
            }
      return false;
      }

// input tuplets are sorted by average pitch in desc. order

int findNonTupletPlace(int pitch, const std::vector<TupletInfo> &tuplets)
      {
      for (int i = 0; i != (int)tuplets.size(); ++i) {
            int tupletPitch = averagePitch(tuplets[i].chords);
            if (pitch >= tupletPitch)
                  return i;
            }
      return tuplets.size();
      }

void setVoiceOfNonTupletChords(std::vector<std::multimap<int, MidiChord>::iterator> &nonTuplets,
                               int nonTupletPlace,
                               const std::vector<TupletInfo> &tuplets,
                               int regularRaster)
      {
      int voice = 0;
      for (int i = 0; i != nonTupletPlace; ++i) {
            const auto &tupletInfo = tuplets[i];
            if (hasIntersectionWithChord(tupletInfo.onTime,
                                         tupletInfo.onTime + tupletInfo.len,
                                         regularRaster,
                                         nonTuplets)) {
                  ++voice;
                  }
            }
      for (auto &chord: nonTuplets)
            chord->second.voice = voice;
      }

// the input tuplets should be filtered (for mutual validity)

void separateTupletVoices(std::vector<TupletInfo> &tuplets,
                          std::multimap<int, MidiChord>::iterator startBarChordIt,
                          std::multimap<int, MidiChord>::iterator endBarChordIt,
                          std::multimap<int, MidiChord> &chords,
                          int endBarTick)
      {
                  // it's better before to sort tuplets by their average pitch
                  // and notes of each chord as well
      sortNotesByPitch(startBarChordIt, endBarChordIt);
      sortTupletsByAveragePitch(tuplets);

      int regularRaster = Quantize::findQuantRaster(startBarChordIt, endBarChordIt, endBarTick);
      auto nonTuplets = findNonTupletChords(tuplets, startBarChordIt, endBarChordIt);
      int nonTupletPlace = -1;
      if (!nonTuplets.empty()) {
            int averageNonTupletPitch = averagePitch(nonTuplets);
            nonTupletPlace = findNonTupletPlace(averageNonTupletPitch, tuplets);
            setVoiceOfNonTupletChords(nonTuplets, nonTupletPlace, tuplets, regularRaster);
            }

      for (auto now = tuplets.begin(); now != tuplets.end(); ++now) {
            int voice = 0;
            auto lastMatch = tuplets.end();
            auto firstNowChordIt = now->chords.begin();
            bool flag = false;
            for (auto prev = tuplets.begin(); prev != now; ) {
                              // check is now tuplet go over previous tuplets and non-tuplet chords
                  if (!flag && prev - tuplets.begin() == nonTupletPlace) {
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

int findRasterForNote(int noteOnTime, int noteLen, const TupletInfo &tupletInfo)
      {
      int raster;
      if (noteOnTime + noteLen <= tupletInfo.onTime + tupletInfo.len) {
                        // if offTime is inside the tuplet - quant by tuplet grid
            raster = tupletInfo.tupletQuantValue;
            }
      else {            // if offTime is outside the tuplet - quant by regular grid
            raster = tupletInfo.regularQuantValue;
            }
      return raster;
      }

void quantizeTupletChord(MidiChord &midiChord, int onTime, const TupletInfo &tupletInfo)
      {
      midiChord.onTime = onTime;
      for (auto &note: midiChord.notes) {
            int raster = findRasterForNote(note.onTime, note.len, tupletInfo);
            int offTime = findNoteOffTime(note.onTime, note.len, raster);
            note.onTime = onTime;
            note.len = offTime - onTime;
            }
            // notes in chord here may have different durations
            // so we don't set the whole chord duration
      }

std::vector<TupletInfo> findTuplets(int startBarTick,
                                    int endBarTick,
                                    const Fraction &barFraction,
                                    std::multimap<int, MidiChord> &chords)
      {
      std::vector<TupletInfo> tuplets;
      if (chords.empty() || startBarTick >= endBarTick)     // invalid cases
            return tuplets;

      int barLen = barFraction.ticks();
      int tol = Quantize::fixedQuantRaster() / 2;
      auto startBarChordIt = findFirstChordInRange(startBarTick - tol,
                                                   endBarTick,
                                                   chords.begin(),
                                                   chords.end());
      if (startBarChordIt == chords.end()) // no chords in this bar
            return tuplets;
                  // end iterator, as usual, will point to the next - invalid chord
      auto endBarChordIt = findEndChordInRange(endBarTick + tol, startBarChordIt, chords.end());

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
                                             tupletInfo.tupletSumError,
                                             tupletInfo.regularSumError,
                                             quantValue, tupletInfo.chords))
                              continue;
                                    // tuplet found
                        tuplets.push_back(tupletInfo);
                        }     // next tuplet type
                  }
            }
      sortTupletCandidates(tuplets);
      filterTuplets(tuplets);
      removeFirstNotesWithBigError(tuplets, chords);
      separateTupletVoices(tuplets, startBarChordIt, endBarChordIt, chords, endBarTick);

      return tuplets;
      }

} // namespace MidiTuplet
} // namespace Ms
