#include "importmidi_tuplet_detect.h"
#include "importmidi_tuplet.h"
#include "importmidi_meter.h"
#include "importmidi_chord.h"
#include "importmidi_quant.h"
#include "importmidi_inner.h"
#include "preferences.h"


namespace Ms {
namespace MidiTuplet {

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

std::vector<TupletInfo> detectTuplets(
            std::multimap<ReducedFraction, MidiChord> &chords,
            const ReducedFraction &barFraction,
            const ReducedFraction &startBarTick,
            const ReducedFraction &tol,
            const std::multimap<ReducedFraction, MidiChord>::iterator &endBarChordIt,
            const std::multimap<ReducedFraction, MidiChord>::iterator &startBarChordIt,
            const ReducedFraction &basicQuant)
      {
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

      return tuplets;
      }

} // namespace MidiTuplet
} // namespace Ms
