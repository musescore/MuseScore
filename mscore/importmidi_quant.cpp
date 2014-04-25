#include "importmidi_quant.h"
#include "libmscore/sig.h"
#include "importmidi_fraction.h"
#include "libmscore/mscore.h"
#include "preferences.h"
#include "importmidi_chord.h"
#include "importmidi_meter.h"
#include "importmidi_tuplet.h"

#include <set>


namespace Ms {

extern Preferences preferences;

namespace Quantize {

ReducedFraction userQuantNoteToFraction(MidiOperation::QuantValue quantNote)
      {
      const auto division = ReducedFraction::fromTicks(MScore::division);
      auto userQuantValue = ReducedFraction::fromTicks(preferences.shortestNote);
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
            case MidiOperation::QuantValue::N_128:
                  userQuantValue = division / 32;
                  break;
            case MidiOperation::QuantValue::FROM_PREFERENCES:
            default:
                  break;
            }

      return userQuantValue;
      }

ReducedFraction shortestQuantizedNoteInRange(
            const std::multimap<ReducedFraction, MidiChord>::const_iterator &beg,
            const std::multimap<ReducedFraction, MidiChord>::const_iterator &end)
      {
      const auto division = ReducedFraction::fromTicks(MScore::division);
      auto minDuration = division;
      for (auto it = beg; it != end; ++it) {
            for (const auto &note: it->second.notes) {
                  if (note.offTime - it->first < minDuration)
                        minDuration = note.offTime - it->first;
                  }
            }
      const auto minAllowedDuration = MChord::minAllowedDuration();
      auto shortest = division;
      for ( ; shortest > minAllowedDuration; shortest /= 2) {
            if (shortest <= minDuration)
                  break;
            }
      return shortest;
      }

ReducedFraction reduceQuantIfDottedNote(const ReducedFraction &noteLen,
                                        const ReducedFraction &raster)
      {
      auto newRaster = raster;
      const auto div = noteLen / raster;
      const double ratio = div.numerator() * 1.0 / div.denominator();
      if (ratio > 1.45 && ratio < 1.55)     // 1.5: dotted note that is larger than quantization value
            newRaster /= 2;                 // reduce quantization error for dotted notes
      return newRaster;
      }

ReducedFraction quantizeValue(const ReducedFraction &value,
                              const ReducedFraction &quant)
      {
      const auto valueReduced = value.reduced();
      const auto rasterReduced = quant.reduced();
      int valNum = valueReduced.numerator() * rasterReduced.denominator();
      const int rastNum = rasterReduced.numerator() * valueReduced.denominator();
      const int commonDen = valueReduced.denominator() * rasterReduced.denominator();
      valNum = ((valNum + rastNum / 2) / rastNum) * rastNum;
      return ReducedFraction(valNum, commonDen).reduced();
      }

ReducedFraction quantForLen(const ReducedFraction &basicQuant,
                            const ReducedFraction &noteLen)
      {
      auto quant = basicQuant;
      while (quant > noteLen && quant >= MChord::minAllowedDuration() * 2)
            quant /= 2;
      if (quant >= MChord::minAllowedDuration() * 2)
            quant = reduceQuantIfDottedNote(noteLen, quant);
      return quant;
      }

ReducedFraction quantForTuplet(const ReducedFraction &tupletLen,
                               const ReducedFraction &tupletRatio)
      {
      const auto quant = tupletLen / tupletRatio.numerator();

      Q_ASSERT_X(quant >= MChord::minAllowedDuration(),
                 "Quantize::quantForTuplet", "Too small quant value");

      if (quant >= MChord::minAllowedDuration() * 2)
            return quant / 2;
      return quant;
      }

ReducedFraction findMinQuant(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant)
      {
      ReducedFraction minQuant(-1, 1);
      for (const auto &note: chord.second.notes) {
            const auto quant = quantForLen(basicQuant, note.offTime - chord.first);
            if (minQuant == ReducedFraction(-1, 1) || quant < minQuant)
                  minQuant = quant;
            }
      return minQuant;
      }

ReducedFraction findQuantizedTupletChordOnTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &tupletLen,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &rangeStart)
      {
      if (chord.first <= rangeStart)
            return rangeStart;
      const auto quant = quantForTuplet(tupletLen, tupletRatio);
      return rangeStart + quantizeValue(chord.first - rangeStart, quant);
      }

ReducedFraction findQuantizedChordOnTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant)
      {
      const ReducedFraction quant = findMinQuant(chord, basicQuant);
      return quantizeValue(chord.first, quant);
      }

ReducedFraction findQuantizedTupletNoteOffTime(
            const ReducedFraction &offTime,
            const ReducedFraction &tupletLen,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &rangeStart)
      {
      if (offTime <= rangeStart)
            return rangeStart;
      const auto quant = quantForTuplet(tupletLen, tupletRatio);
      return rangeStart + quantizeValue(offTime - rangeStart, quant);
      }

ReducedFraction findQuantizedNoteOffTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &offTime,
            const ReducedFraction &basicQuant)
      {
      const auto quant = quantForLen(basicQuant, offTime - chord.first);
      return quantizeValue(offTime, quant);
      }

ReducedFraction findMinQuantizedOnTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant)
      {
      ReducedFraction minOnTime(-1, 1);
      for (const auto &note: chord.second.notes) {
            const auto quant = quantForLen(
                              basicQuant, note.offTime - chord.first);
            const auto onTime = quantizeValue(chord.first, quant);
            if (minOnTime == ReducedFraction(-1, 1) || onTime < minOnTime)
                  minOnTime = onTime;
            }
      return minOnTime;
      }

ReducedFraction findMaxQuantizedTupletOffTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &tupletLen,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &rangeStart)
      {
      ReducedFraction maxOffTime(0, 1);
      for (const auto &note: chord.second.notes) {
            if (note.offTime <= rangeStart)
                  continue;
            const auto offTime = findQuantizedTupletNoteOffTime(
                              note.offTime, tupletLen, tupletRatio, rangeStart);
            if (offTime > maxOffTime)
                  maxOffTime = offTime;
            }
      return maxOffTime;
      }

ReducedFraction findMaxQuantizedOffTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant)
      {
      ReducedFraction maxOffTime(0, 1);
      for (const auto &note: chord.second.notes) {
            const auto offTime = findQuantizedNoteOffTime(chord, note.offTime, basicQuant);
            if (offTime > maxOffTime)
                  maxOffTime = offTime;
            }
      return maxOffTime;
      }

ReducedFraction findOnTimeTupletQuantError(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &tupletLen,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &rangeStart)
      {
      const auto qOnTime = findQuantizedTupletChordOnTime(chord, tupletLen,
                                                          tupletRatio, rangeStart);
      return (chord.first - qOnTime).absValue();
      }

ReducedFraction findOnTimeQuantError(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant)
      {
      const auto qOnTime = findQuantizedChordOnTime(chord, basicQuant);
      return (chord.first - qOnTime).absValue();
      }

ReducedFraction findOffTimeTupletQuantError(
            const ReducedFraction &offTime,
            const ReducedFraction &tupletLen,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &rangeStart)
      {
      const auto qOffTime = findQuantizedTupletNoteOffTime(offTime, tupletLen,
                                                           tupletRatio, rangeStart);
      return (offTime - qOffTime).absValue();
      }

ReducedFraction findOffTimeQuantError(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &offTime,
            const ReducedFraction &basicQuant)
      {
      const auto qOffTime = findQuantizedNoteOffTime(chord, offTime, basicQuant);
      return (offTime - qOffTime).absValue();
      }

ReducedFraction findQuantForRange(
            const std::multimap<ReducedFraction, MidiChord>::const_iterator &beg,
            const std::multimap<ReducedFraction, MidiChord>::const_iterator &end,
            const ReducedFraction &basicQuant)
      {
      const auto shortestLen = shortestQuantizedNoteInRange(beg, end);
      return quantForLen(basicQuant, shortestLen);
      }

ReducedFraction quantizeOnTimeForTuplet(
            const std::pair<const ReducedFraction, MidiChord> &chordEvent)
      {
      const MidiTuplet::TupletData &tuplet = chordEvent.second.tuplet->second;
      const auto tupletRatio = MidiTuplet::tupletLimits(tuplet.tupletNumber).ratio;
      auto onTime = findQuantizedTupletChordOnTime(
                               chordEvent, tuplet.len, tupletRatio, tuplet.onTime);
                  // verify that onTime is still inside tuplet
      if (onTime < tuplet.onTime)
            onTime = tuplet.onTime;
      else if (onTime > tuplet.onTime + tuplet.len)
            onTime = tuplet.onTime + tuplet.len;

      return onTime;
      }

ReducedFraction quantizeOnTimeForNonTuplet(
            const std::multimap<ReducedFraction, MidiChord>::iterator &chordIt,
            const ReducedFraction &rangeStart,
            const ReducedFraction &rangeEnd,
            const ReducedFraction &basicQuant)
      {
      auto onTime = findQuantizedChordOnTime(*chordIt, basicQuant);
                  // verify that onTime is inside current range
      if (onTime < rangeStart)
            onTime = rangeStart;
      if (onTime > rangeEnd)
            onTime = rangeEnd;
      return onTime;
      }

ReducedFraction quantizeOffTimeForTuplet(
            const ReducedFraction &noteOffTime,
            const MidiTuplet::TupletData &tuplet)
      {
      const auto tupletRatio = MidiTuplet::tupletLimits(tuplet.tupletNumber).ratio;
      auto offTime = findQuantizedTupletNoteOffTime(
                        noteOffTime, tuplet.len, tupletRatio, tuplet.onTime);
                  // verify that offTime is still inside tuplet
      if (offTime < tuplet.onTime)
            offTime = tuplet.onTime;
      else if (offTime > tuplet.onTime + tuplet.len)
            offTime = tuplet.onTime + tuplet.len;

      return offTime;
      }

ReducedFraction quantizeOffTimeForNonTuplet(
            const ReducedFraction &noteOffTime,
            const std::multimap<ReducedFraction, MidiChord>::iterator &chordIt,
            const std::multimap<ReducedFraction, MidiChord> &chords,
            const ReducedFraction &basicQuant)
      {
      const MidiChord &chord = chordIt->second;
      auto offTime = findQuantizedNoteOffTime(*chordIt, noteOffTime, basicQuant);
                 // verify that offTime is still outside tuplets
      auto next = std::next(chordIt);
      while (next != chords.end()) {
            if (next->second.isInTuplet && next->second.voice == chord.voice) {
                  const auto &tuplet = next->second.tuplet->second;
                  if (offTime > tuplet.onTime)
                        offTime = tuplet.onTime;
                  break;
                  }
            if (next->second.barIndex != chord.barIndex)
                  break;
            ++next;
            }

      return offTime;
      }


#ifdef QT_DEBUG

bool areAllVoicesSame(
            const std::vector<std::multimap<ReducedFraction, MidiChord>::iterator> &chords)
      {
      auto it = chords.begin();
      const int voice = (*it)->second.voice;
      for (++it; it != chords.end(); ++it) {
            if ((*it)->second.voice != voice)
                  return false;
            }
      return true;
      }

#endif


void quantizeOnTimesInRange(
            const std::vector<std::multimap<ReducedFraction, MidiChord>::iterator> &chords,
            std::multimap<ReducedFraction, MidiChord> &quantizedChords,
            const ReducedFraction &rangeStart,
            const ReducedFraction &rangeEnd,
            const ReducedFraction &basicQuant)
      {
      Q_ASSERT_X(!chords.empty(), "Quantize::quantizeChordOnTimes", "Empty chords");
      Q_ASSERT_X(areAllVoicesSame(chords),
                 "Quantize::quantizeChordOnTimes", "Chord voices are not the same");

      bool isInTuplet = chords.front()->second.isInTuplet;
      if (isInTuplet) {
            // const MidiTuplet::TupletData &tuplet = chordIt->second.tuplet->second;
            // const auto tupletRatio = MidiTuplet::tupletLimits(tuplet.tupletNumber);

            for (const auto &chordIt: chords) {
                  const auto onTime = quantizeOnTimeForTuplet(*chordIt);
                  quantizedChords.insert({onTime, chordIt->second});
                  }
            }
      else {
            for (const auto &chordIt: chords) {
                  const auto onTime = quantizeOnTimeForNonTuplet(
                                          chordIt, rangeStart, rangeEnd, basicQuant);
                  quantizedChords.insert({onTime, chordIt->second});
                  }
            }
      }

// input chords - sorted by onTime value

void applyTupletStaccato(std::multimap<ReducedFraction, MidiChord> &chords)
      {
      for (auto chordIt = chords.begin(); chordIt != chords.end(); ++chordIt) {
            for (MidiNote &note: chordIt->second.notes) {
                  if (note.isInTuplet && note.staccato) {
                        const MidiTuplet::TupletData &tuplet = note.tuplet->second;
                              // decrease tuplet error by enlarging staccato notes:
                              // make note.len = tuplet note length
                        const auto tupletNoteLen = (tuplet.onTime + tuplet.len)
                                                    / tuplet.tupletNumber;
                        note.offTime = chordIt->first + tupletNoteLen;
                        }
                  }
            }
      }

void quantizeOffTimes(
            std::multimap<ReducedFraction, MidiChord> &quantizedChords,
            const ReducedFraction &basicQuant)
      {
      for (auto chordIt = quantizedChords.begin(); chordIt != quantizedChords.end(); ) {
            auto &chordEvent = *chordIt;
            MidiChord &chord = chordEvent.second;
                        // quantize off times
            for (auto noteIt = chord.notes.begin(); noteIt != chord.notes.end(); ) {
                  MidiNote &note = *noteIt;
                  auto offTime = note.offTime;

                  if (note.isInTuplet) {
                        offTime = quantizeOffTimeForTuplet(offTime, note.tuplet->second);
                        }
                  else {
                        offTime = quantizeOffTimeForNonTuplet(
                                          offTime, chordIt, quantizedChords, basicQuant);
                        }

                  note.offTime = offTime;
                  if (note.offTime - chordEvent.first < MChord::minAllowedDuration()) {
                        noteIt = chord.notes.erase(noteIt);
                        // TODO - never delete notes here
                        qDebug() << "quantizeChords: note was removed due to its short length";
                        continue;
                        }

                  ++noteIt;
                  }
            if (chord.notes.isEmpty()) {
                  chordIt = quantizedChords.erase(chordIt);
                  continue;
                  }
            ++chordIt;
            }
      }

void quantizeOnTimes(
            std::multimap<ReducedFraction, MidiChord> &chords,
            std::multimap<ReducedFraction, MidiChord> &quantizedChords,
            const ReducedFraction &basicQuant,
            const TimeSigMap *sigmap)
      {
      int maxVoice = 0;
      for (int voice = 0; voice <= maxVoice; ++voice) {
            int currentBarIndex = -1;
            ReducedFraction rangeStart(-1, 1);
            ReducedFraction rangeEnd(-1, 1);
            bool currentlyInTuplet = false;
            std::vector<std::multimap<ReducedFraction, MidiChord>::iterator> chordsToQuant;

            for (auto chordIt = chords.begin(); chordIt != chords.end(); ++chordIt) {
                  if (chordIt->second.voice > maxVoice)
                        maxVoice = chordIt->second.voice;
                  if (chordIt->second.voice != voice)
                        continue;

                  if (chordsToQuant.empty()) {
                        currentlyInTuplet = chordIt->second.isInTuplet;
                        if (currentBarIndex != chordIt->second.barIndex) {
                              currentBarIndex = chordIt->second.barIndex;
                              if (!currentlyInTuplet) {
                                    rangeStart = ReducedFraction::fromTicks(
                                                      sigmap->bar2tick(currentBarIndex, 0));
                                    }
                              }
                        if (currentlyInTuplet) {
                              const auto &tuplet = chordIt->second.tuplet->second;
                              rangeStart = tuplet.onTime;
                              rangeEnd = tuplet.onTime + tuplet.len;
                              }
                        }

                  chordsToQuant.push_back(chordIt);

                  auto nextChord = std::next(chordIt);
                  while (nextChord != chords.end() && nextChord->second.voice != voice)
                        ++nextChord;
                  if (nextChord == chords.end()
                              || nextChord->second.barIndex != currentBarIndex
                              || nextChord->second.isInTuplet != currentlyInTuplet) {

                        if (nextChord != chords.end()) {
                              if (nextChord->second.barIndex != currentBarIndex) {
                                    rangeEnd = ReducedFraction::fromTicks(
                                                      sigmap->bar2tick(currentBarIndex + 1, 0));
                                    }
                              else if (!currentlyInTuplet && nextChord->second.isInTuplet) {
                                    rangeEnd = nextChord->second.tuplet->second.onTime;
                                    }
                              }
                        else {
                              if (!currentlyInTuplet) {
                                    rangeEnd = ReducedFraction::fromTicks(
                                                      sigmap->bar2tick(currentBarIndex + 1, 0));
                                    }
                              }

                        quantizeOnTimesInRange(chordsToQuant, quantizedChords,
                                               rangeStart, rangeEnd, basicQuant);
                        chordsToQuant.clear();
                        }
                  }
            }
      }

void quantizeChords(
            std::multimap<ReducedFraction, MidiChord> &chords,
            const TimeSigMap *sigmap,
            const ReducedFraction &basicQuant)
      {
      applyTupletStaccato(chords);     // apply staccato for tuplet off times

      std::multimap<ReducedFraction, MidiChord> quantizedChords;
      quantizeOnTimes(chords, quantizedChords, basicQuant, sigmap);
      quantizeOffTimes(quantizedChords, basicQuant);

      std::swap(chords, quantizedChords);
      }

} // namespace Quantize
} // namespace Ms
