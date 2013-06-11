#include "importmidi_quant.h"
#include "libmscore/sig.h"
#include "libmscore/utils.h"
#include "libmscore/fraction.h"
#include "libmscore/mscore.h"
#include "preferences.h"
#include "importmidi_chord.h"
#include "importmidi_meter.h"
#include "importmidi_tupletdata.h"

#include <set>


namespace Ms {

extern Preferences preferences;

namespace Quantize {

void applyAdaptiveQuant(std::multimap<int, MidiChord> &/*chords*/,
                        const TimeSigMap */*sigmap*/,
                        int /*allTicks*/)
      {
      }


int shortestNoteInBar(const std::multimap<int, MidiChord> &chords,
                      const std::multimap<int, MidiChord>::const_iterator &start,
                      int endBarTick)
      {
      int division = MScore::division;
      int minDuration = division;
                  // find shortest note in measure
      for (auto it = start; it != chords.end(); ++it) {
            if (it->first >= endBarTick)
                  break;
            for (const auto &note: it->second.notes)
                  minDuration = qMin(minDuration, note.len);
            }
                  // determine suitable quantization value based
                  // on shortest note in measure
      int div = division;
      if (minDuration <= division / 16)        // minimum duration is 1/64
            div = division / 16;
      else if (minDuration <= division / 8)
            div = division / 8;
      else if (minDuration <= division / 4)
            div = division / 4;
      else if (minDuration <= division / 2)
            div = division / 2;
      else if (minDuration <= division)
            div = division;
      else if (minDuration <= division * 2)
            div = division * 2;
      else if (minDuration <= division * 4)
            div = division * 4;
      else if (minDuration <= division * 8)
            div = division * 8;
      if (div == (division / 16))
            minDuration = div;
      else
            minDuration = quantizeLen(minDuration, div >> 1);    //closest

      return minDuration;
      }

int userQuantNoteToTicks(MidiOperation::QuantValue quantNote)
      {
      int division = MScore::division;
      int userQuantValue = preferences.shortestNote;
                  // specified quantization value
      switch (quantNote) {
            case MidiOperation::QuantValue::N_4:
                  userQuantValue = division;
                  break;
            case MidiOperation::QuantValue::N_8:
                  userQuantValue = division / 2;
                  break;
            case MidiOperation::QuantValue::N_16:
                  userQuantValue = division / 4;
                  break;
            case MidiOperation::QuantValue::N_32:
                  userQuantValue = division / 8;
                  break;
            case MidiOperation::QuantValue::N_64:
                  userQuantValue = division / 16;
                  break;
            case MidiOperation::QuantValue::FROM_PREFERENCES:
            default:
                  userQuantValue = preferences.shortestNote;
                  break;
            }

      return userQuantValue;
      }

int findQuantRaster(const std::multimap<int, MidiChord> &chords,
                    const std::multimap<int, MidiChord>::const_iterator &startChordIter,
                    int endBarTick)
      {
      int raster;
      auto operations = preferences.midiImportOperations.currentTrackOperations();
                  // find raster value for quantization
      if (operations.quantize.value == MidiOperation::QuantValue::SHORTEST_IN_BAR)
            raster = shortestNoteInBar(chords, startChordIter, endBarTick);
      else {
            int userQuantValue = userQuantNoteToTicks(operations.quantize.value);
                        // if user value larger than the smallest note in bar
                        // then use the smallest note to keep faster events
            if (operations.quantize.reduceToShorterNotesInBar) {
                  raster = shortestNoteInBar(chords, startChordIter, endBarTick);
                  raster = qMin(userQuantValue, raster);
                  }
            else
                  raster = userQuantValue;
            }
      return raster;
      }

// chords onTime values don't repeat despite multimap

void doGridQuantizationOfBar(const std::multimap<int, MidiChord> &chords,
                             std::multimap<int, MidiChord> &quantizedChords,
                             const std::multimap<int, MidiChord>::const_iterator &startChordIter,
                             int raster,
                             int endBarTick)
      {
      int raster2 = raster >> 1;
      for (auto it = startChordIter; it != chords.end(); ++it) {
            if (it->first >= endBarTick)
                  break;
            if (quantizedChords.find(it->first) != quantizedChords.end())
                  continue;
            auto chord = it->second;
            chord.onTime = ((chord.onTime + raster2) / raster) * raster;
            for (auto &note: chord.notes) {
                  note.onTime = chord.onTime;
                  note.len = quantizeLen(note.len, raster);
                  }
            quantizedChords.insert({chord.onTime, chord});
            }
      }

const std::multimap<int, MidiChord>::const_iterator
findFirstChordInBar(const std::multimap<int, MidiChord> &chords, int startBarTick, int endBarTick)
      {
      auto it = chords.begin();
      for (; it != chords.end(); ++it) {
            if (it->first >= startBarTick) {
                  if (it->first >= endBarTick)
                        return chords.end();
                  break;
                  }
            }
      return it;
      }

void quantizeChordsOfBar(const std::multimap<int, MidiChord> &chords,
                         std::multimap<int, MidiChord> &quantizedChords,
                         int startBarTick,
                         int endBarTick)
      {
      auto startChordIter = findFirstChordInBar(chords, startBarTick, endBarTick);
      if (startChordIter == chords.end())       // if no chords found in this bar
            return;
      int raster = findQuantRaster(chords, startChordIter, endBarTick);
      doGridQuantizationOfBar(chords, quantizedChords, startChordIter, raster, endBarTick);
      }

void applyGridQuant(std::multimap<int, MidiChord> &chords,
                    const TimeSigMap* sigmap,
                    int lastTick)
      {
      std::multimap<int, MidiChord> quantizedChords;
      int startBarTick = 0;
      for (int i = 1;; ++i) {       // iterate over all measures by indexes
            int endBarTick = sigmap->bar2tick(i, 0);
            quantizeChordsOfBar(chords, quantizedChords, startBarTick, endBarTick);
            if (endBarTick > lastTick)
                  break;
            startBarTick = endBarTick;
            }
      std::swap(chords, quantizedChords);
      }


// TODO: optimize start and end chord iterators

struct TupletInfo
      {
      int voice;
      int onTime;
      int len;
      int tupletNumber;
      int tupletQuantValue;
      int regularQuantValue;
                  // note number in tuplet, chord iterator
      std::map<int, std::multimap<int, MidiChord>::iterator> chords;
      int tupletOnTimeSumError = 0;
      int regularSumError = 0;
      };


std::multimap<int, MidiChord>::iterator
findFirstChordInBar(int startBarTick,
                    int endBarTick,
                    std::multimap<int, MidiChord> &chords)
      {
      std::multimap<int, MidiChord>::iterator startBarChordIt = chords.end();
      for (auto it = chords.begin(); it != chords.end(); ++it) {
            if (it->first >= startBarTick && it->first < endBarTick) {
                  startBarChordIt = it;
                  break;
                  }
            }
      return startBarChordIt;
      }

std::multimap<int, MidiChord>::iterator
findEndChordInBar(int endBarTick,
                  const std::multimap<int, MidiChord>::iterator &startBarChordIt,
                  std::multimap<int, MidiChord> &chords)
      {
      std::multimap<int, MidiChord>::iterator endBarChordIt = chords.end();
      for (auto it = startBarChordIt; it != chords.end(); ++it) {
            if (it->first > endBarTick) {
                  endBarChordIt = it;
                  break;
                  }
            }
      return endBarChordIt;
      }

std::multimap<int, MidiChord>::iterator
findFirstChordInDivision(int startDivTime,
                         int endDivTime,
                         const std::multimap<int, MidiChord>::iterator &startBarChordIt,
                         const std::multimap<int, MidiChord>::iterator &endBarChordIt)
      {
      std::multimap<int, MidiChord>::iterator startDivChordIt = endBarChordIt;
      for (auto it = startBarChordIt; it != endBarChordIt; ++it) {
            if (it->first >= startDivTime && it->first < endDivTime) {
                  startDivChordIt = it;
                  break;
                  }
            }
      return startDivChordIt;
      }

std::multimap<int, MidiChord>::iterator
findEndChordInDivision(int endDivTime,
                       const std::multimap<int, MidiChord>::iterator &startDivChordIt,
                       const std::multimap<int, MidiChord>::iterator &endBarChordIt)
      {
      std::multimap<int, MidiChord>::iterator endDivChordIt = endBarChordIt;
      for (auto it = startDivChordIt; it != endBarChordIt; ++it) {
            if (it->first > endDivTime) {
                  endDivChordIt = it;
                  break;
                  }
            }
      return endDivChordIt;
      }

struct BestChord
      {
      std::multimap<int, MidiChord>::iterator chordIter;
      int minTupletError;
      };

BestChord findBestChordForTupletNote(int tupletNotePos,
                                     int quantValue,
                                     const std::multimap<int, MidiChord>::iterator &startTupletChordIt,
                                     const std::multimap<int, MidiChord>::iterator &endDivChordIt)
      {
                  // choose the best chord, if any, for this tuplet note
      BestChord bestChord;
      bestChord.chordIter = endDivChordIt;
      bestChord.minTupletError = std::numeric_limits<int>::max();
                  // check chords - whether they can be in tuplet without large error
      for (auto chordIt = startTupletChordIt; chordIt != endDivChordIt; ++chordIt) {
            int tupletError = std::abs(chordIt->first - tupletNotePos);
            if (tupletError > quantValue)
                  continue;
            if (tupletError < bestChord.minTupletError) {
                  bestChord.minTupletError = tupletError;
                  bestChord.chordIter = chordIt;
                  }
            }
      return bestChord;
      }

bool isSpecialTupletAllowed(int tupletNumber,
                            int divLen,
                            int quantValue,
                            const TupletInfo &tupletInfo)
      {
      std::vector<int> nums = {2, 3};
                  // for duplet: if note first and single - only 1/2*divLen duration is allowed
                  // for triplet: if note first and single - only 1/3*divLen duration is allowed
      for (auto num: nums) {
            if (tupletNumber == num && tupletInfo.chords.size() == 1
                        && tupletInfo.chords.begin()->first == 0) {
                  auto &chordEventIt = tupletInfo.chords.begin()->second;
                  for (const auto &note: chordEventIt->second.notes) {
                        if (std::abs(note.len - divLen / num) > quantValue)
                              return false;
                        }
                  }
            }
      return true;
      }

bool isTupletAllowed(const TupletInfo &tupletInfo)
      {
      int minAllowedNoteCount = tupletInfo.tupletNumber / 2 + tupletInfo.tupletNumber / 4;
      if ((int)tupletInfo.chords.size() < minAllowedNoteCount
                  || tupletInfo.tupletOnTimeSumError >= tupletInfo.regularSumError) {
            return false;
            }

      int tupletNoteLen = tupletInfo.len / tupletInfo.tupletNumber;
      for (const auto &tupletChord: tupletInfo.chords) {
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

TupletInfo findTupletApproximation(int tupletNumber,
                                   int tupletNoteLen,
                                   int quantValue,
                                   int startDivTime,
                                   const std::multimap<int, MidiChord>::iterator &startDivChordIt,
                                   const std::multimap<int, MidiChord>::iterator &endDivChordIt)
      {
      TupletInfo tupletInfo;
      tupletInfo.tupletNumber = tupletNumber;

      auto startTupletChordIt = startDivChordIt;
      for (int k = 0; k != tupletNumber; ++k) {
            int tupletNotePos = startDivTime + k * tupletNoteLen;
                        // choose the best chord, if any, for this tuplet note
            BestChord bestChord = findBestChordForTupletNote(tupletNotePos, quantValue,
                                                             startTupletChordIt, endDivChordIt);
            if (bestChord.chordIter == endDivChordIt)
                  continue;   // no chord fits to this tuplet note position
                        // chord can be in tuplet
            tupletInfo.chords.insert({k, bestChord.chordIter});
            tupletInfo.tupletOnTimeSumError += bestChord.minTupletError;
                        // for next tuplet note we start from the next chord
                        // because chord for the next tuplet note cannot be earlier
            startTupletChordIt = bestChord.chordIter;
            ++startTupletChordIt;
                        // find chord quant error for a regular grid
            int regularError = findOnTimeRegularError(bestChord.chordIter->first, quantValue);
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

      auto startBarChordIt = findFirstChordInBar(startBarTick, endBarTick, chords);
      if (startBarChordIt == chords.end()) // no chords in this bar
            return tupletCandidates;
                  // end iterator, as usual, will point to the next - invalid chord
      auto endBarChordIt = findEndChordInBar(endBarTick, startBarChordIt, chords);

      int barLen = barFraction.ticks();
      int quantValue = findQuantRaster(chords, startBarChordIt, endBarTick);
      auto divLengths = Meter::divisionsOfBarForTuplets(barFraction);

      for (const auto &divLen: divLengths) {
            auto tupletNumbers = findTupletNumbers(divLen, barFraction);
            int divCount = barLen / divLen;

            for (int i = 0; i != divCount; ++i) {
                  int startDivTime = startBarTick + i * divLen;
                  int endDivTime = startDivTime + divLen;
                              // check which chords can be inside tuplet period = [startDivTime, endDivTime]
                  auto startDivChordIt = findFirstChordInDivision(startDivTime, endDivTime,
                                                                  startBarChordIt, endBarChordIt);
                  if (startDivChordIt == endBarChordIt) // no chords in this division
                        continue;
                              // end iterator, as usual, will point to the next - invalid chord
                  auto endDivChordIt = findEndChordInDivision(endDivTime, startDivChordIt, endBarChordIt);
                              // try different tuplets, nested tuplets are not allowed
                  for (const auto &tupletNumber: tupletNumbers) {
                        int tupletNoteLen = divLen / tupletNumber;
                        if (tupletNoteLen < quantValue)
                              continue;
                        TupletInfo tupletInfo = findTupletApproximation(tupletNumber, tupletNoteLen,
                                          quantValue, startDivTime, startDivChordIt, endDivChordIt);
                        tupletInfo.onTime = startDivTime;
                        tupletInfo.len = divLen;
                              // check - is it a good tuplet approximation?
                                    // check tuplet note count
                                    // and tuplet error compared to the regular quantization error
                        if (!isTupletAllowed(tupletInfo))
                              continue;
                                    // additional check: tuplet special cases
                        if (!isSpecialTupletAllowed(tupletNumber, divLen, quantValue, tupletInfo))
                              continue;
                                    // --- tuplet found ---
                        double averageError = tupletInfo.tupletOnTimeSumError * 1.0 / tupletInfo.chords.size();
                        tupletInfo.tupletQuantValue = tupletNoteLen;
                        while (tupletInfo.tupletQuantValue / 2 >= quantValue)
                              tupletInfo.tupletQuantValue /= 2;
                        tupletInfo.regularQuantValue = quantValue;

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
      auto chordEventIt = i->second;
      int chordOnTime = chordEventIt->first;
      int voice = 0;
                  // check is the note of the first tuplet chord in use
      auto ii = usedFirstTupletNotes.find(chordOnTime);
      if (ii == usedFirstTupletNotes.end())
            ii = usedFirstTupletNotes.insert({chordOnTime, 1}).first;
      else {
            voice = ii->second;     // voice = counter - 1
            ++(ii->second);         // increase chord note counter
            }

      ++i;              // start from the second chord
      for ( ; i != tupletChords.end(); ++i) {
                        // mark the chord as used
            chordEventIt = i->second;
            chordOnTime = chordEventIt->first;
            usedChords.insert(chordOnTime);
            chordEventIt->second.voice = voice;
            }
      }

bool isChordInUse(const std::map<int, int> &usedFirstTupletNotes,
                  const std::set<int> &usedChords,
                  const std::map<int, std::multimap<int, MidiChord>::iterator> &tupletChords)
      {
      auto i = tupletChords.begin();
                  // check are first tuplet notes all in use (1 note - 1 voice)
      int chordOnTime = i->first;
      auto ii = usedFirstTupletNotes.find(chordOnTime);
      if (ii != usedFirstTupletNotes.end()) {
            if (ii->second >= VOICES) {
                              // need to choose next tuplet candidate - no more available voices
                  return true;
                  }
            }
      ++i;                    // start from the second chord
      for ( ; i != tupletChords.end(); ++i) {
            chordOnTime = i->first;
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
// the same is for duplet second chord: it can belong to 4-tuplet

void filterTuplets(std::multimap<double, TupletInfo> &tuplets)
      {
                  // structure of map: <tick, count of use of first tuplet chord with this tick>
      std::map<int, int> usedFirstTupletNotes;
                  // onTime values - tick - of already used chords
      std::set<int> usedChords;
                  // select tuplets with min average error
      for (auto tc = tuplets.begin(); tc != tuplets.end(); ) { // tc - tuplet candidate
            auto &tupletChords = tc->second.chords;
                        // check for chords notes already used in another tuplets
            if (tupletChords.empty()
                        || isChordInUse(usedFirstTupletNotes, usedChords, tupletChords)) {
                  tc = tuplets.erase(tc);
                  continue;
                  }
                        // we can use this tuplet
            markChordsAsUsed(usedFirstTupletNotes, usedChords, tupletChords);
            const auto &chordEventIt = tupletChords.begin()->second;
            tc->second.voice = chordEventIt->second.voice;
            ++tc;
            }
      }

void quantizeTupletChord(MidiChord &midiChord, int onTime, const TupletInfo &tupletInfo)
      {
      midiChord.onTime = onTime;
      midiChord.voice = tupletInfo.voice;
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

void quantizeNonTupletChords(const std::multimap<int, MidiChord> &chords,
                             std::multimap<int, MidiChord> &quantizedChords,
                             int lastTick,
                             const TimeSigMap* sigmap)
      {
      int startBarTick = 0;
      for (int i = 1;; ++i) {       // iterate over all measures by indexes
            int endBarTick = sigmap->bar2tick(i, 0);
            quantizeChordsOfBar(chords, quantizedChords, startBarTick, endBarTick);
            if (endBarTick > lastTick)
                  break;
            startBarTick = endBarTick;
            }
      }

// input chords - sorted by onTime value,
// onTime values don't repeat even in multimap below

void quantizeChordsAndFindTuplets(std::multimap<int, TupletData> &tupletEvents,
                                  std::multimap<int, MidiChord> &chords,
                                  const TimeSigMap* sigmap,
                                  int lastTick)
      {
      std::multimap<int, MidiChord> quantizedChords;  // set of already quantized onTime values
                  // quantize chords in tuplets
      int startBarTick = 0;
      for (int i = 1;; ++i) {       // iterate over all measures by indexes
            int endBarTick = sigmap->bar2tick(i, 0);
            Fraction barFraction = sigmap->timesig(startBarTick).timesig();
            auto tuplets = findTupletCandidatesOfBar(startBarTick, endBarTick, barFraction, chords);
            filterTuplets(tuplets);
            for (auto &tuplet: tuplets) {
                  auto &tupletInfo = tuplet.second;
                  auto &infoChords = tupletInfo.chords;
                  for (auto &tupletChord: infoChords) {
                        int tupletNoteNum = tupletChord.first;
                        int onTime = tupletInfo.onTime + tupletNoteNum
                                    * (tupletInfo.len / tupletInfo.tupletNumber);
                        std::multimap<int, MidiChord>::iterator &midiChordEventIt = tupletChord.second;
                                    // quantize chord to onTime value
                        MidiChord midiChord = midiChordEventIt->second;
                        quantizeTupletChord(midiChord, onTime, tupletInfo);
                        quantizedChords.insert({onTime, midiChord});
                        }
                  TupletData tupletData = {tupletInfo.voice, tupletInfo.onTime,
                                           tupletInfo.len, tupletInfo.tupletNumber};
                  tupletEvents.insert({tupletInfo.onTime, tupletData});
                  }
            if (endBarTick > lastTick)
                  break;
            startBarTick = endBarTick;
            }
                  // quantize non-tuplet (remaining) chords with ordinary grid
      quantizeNonTupletChords(chords, quantizedChords, lastTick, sigmap);

      std::swap(chords, quantizedChords);
      }

} // namespace Quantize
} // namespace Ms
