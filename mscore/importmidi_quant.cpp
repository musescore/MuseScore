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
                                        const ReducedFraction &raster,
                                        const ReducedFraction &tupletRatio)
      {
      auto newRaster = raster;
      const auto div = noteLen * tupletRatio / raster;
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
                            const ReducedFraction &noteLen,
                            const ReducedFraction &tupletRatio)
      {
      auto quant = basicQuant / tupletRatio;
      while (quant > noteLen)
            quant /= 2;
      return reduceQuantIfDottedNote(noteLen, quant, tupletRatio);
      }

ReducedFraction findMinQuant(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant,
            const ReducedFraction &tupletRatio)
      {
      ReducedFraction minQuant(-1, 1);
      for (const auto &note: chord.second.notes) {
            const auto quant = quantForLen(basicQuant, note.offTime - chord.first, tupletRatio);
            if (minQuant == ReducedFraction(-1, 1) || quant < minQuant)
                  minQuant = quant;
            }
      return minQuant;
      }

ReducedFraction findQuantizedChordOnTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &barStart)
      {
      const ReducedFraction minQuant = findMinQuant(chord, basicQuant, tupletRatio);
      return barStart + quantizeValue(chord.first - barStart, minQuant);
      }

ReducedFraction findQuantizedChordOnTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant)
      {
      return findQuantizedChordOnTime(chord, basicQuant, {1, 1}, {0, 1});
      }

ReducedFraction findQuantizedNoteOffTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &offTime,
            const ReducedFraction &basicQuant,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &barStart)
      {
      const auto quant = quantForLen(basicQuant, offTime - chord.first, tupletRatio);
      return barStart + quantizeValue(offTime - barStart, quant);
      }

ReducedFraction findQuantizedNoteOffTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &offTime,
            const ReducedFraction &basicQuant)
      {
      return findQuantizedNoteOffTime(chord, offTime, basicQuant, {1, 1}, {0, 1});
      }

ReducedFraction findMinQuantizedOnTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &barStart)
      {
      ReducedFraction minOnTime(-1, 1);
      for (const auto &note: chord.second.notes) {
            const auto quant = quantForLen(basicQuant, note.offTime - chord.first, tupletRatio);
            const auto onTime = barStart + quantizeValue(chord.first - barStart, quant);
            if (minOnTime == ReducedFraction(-1, 1) || onTime < minOnTime)
                  minOnTime = onTime;
            }
      return minOnTime;
      }

ReducedFraction findMinQuantizedOnTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant)
      {
      return findMinQuantizedOnTime(chord, basicQuant, {1, 1}, {0, 1});
      }

ReducedFraction findMaxQuantizedOffTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &barStart)
      {
      ReducedFraction maxOffTime(0, 1);
      for (const auto &note: chord.second.notes) {
            const auto offTime = findQuantizedNoteOffTime(chord, note.offTime, basicQuant,
                                                          tupletRatio, barStart);
            if (offTime > maxOffTime)
                  maxOffTime = offTime;
            }
      return maxOffTime;
      }

ReducedFraction findMaxQuantizedOffTime(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant)
      {
      return findMaxQuantizedOffTime(chord, basicQuant, {1, 1}, {0, 1});
      }

ReducedFraction findOnTimeQuantError(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &barStart)
      {
      const auto qOnTime = findQuantizedChordOnTime(chord, basicQuant, tupletRatio, barStart);
      return (chord.first - qOnTime).absValue();
      }

ReducedFraction findOnTimeQuantError(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &basicQuant)
      {
      return findOnTimeQuantError(chord, basicQuant, {1, 1}, {0, 1});
      }

ReducedFraction findOffTimeQuantError(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &offTime,
            const ReducedFraction &basicQuant,
            const ReducedFraction &tupletRatio,
            const ReducedFraction &barStart)
      {
      const auto qOffTime = findQuantizedNoteOffTime(chord, offTime, basicQuant,
                                                     tupletRatio, barStart);
      return (offTime - qOffTime).absValue();
      }

ReducedFraction findOffTimeQuantError(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            const ReducedFraction &offTime,
            const ReducedFraction &basicQuant)
      {
      return findOffTimeQuantError(chord, offTime, basicQuant, {1, 1}, {0, 1});
      }

ReducedFraction findQuantForRange(
            const std::multimap<ReducedFraction, MidiChord>::const_iterator &beg,
            const std::multimap<ReducedFraction, MidiChord>::const_iterator &end,
            const ReducedFraction &basicQuant,
            const ReducedFraction &tupletRatio)
      {
      const auto shortestLen = shortestQuantizedNoteInRange(beg, end);
      return quantForLen(basicQuant, shortestLen, tupletRatio);
      }

// input chords - sorted by onTime value

void quantizeChords(
            std::multimap<ReducedFraction, MidiChord> &chords,
            const TimeSigMap *sigmap,
            const ReducedFraction &basicQuant)
      {
      std::multimap<ReducedFraction, MidiChord> quantizedChords;
      for (auto chordIt = chords.begin(); chordIt != chords.end(); ++chordIt) {
            auto &chordEvent = *chordIt;
            MidiChord chord = chordEvent.second;     // copy chord
            auto onTime = chordEvent.first;
            const auto barStart = ReducedFraction::fromTicks(sigmap->bar2tick(chord.barIndex, 0));

            if (chord.isInTuplet) {
                  const MidiTuplet::TupletData &tuplet = chord.tuplet->second;
                  const auto tupletRatio = MidiTuplet::tupletLimits(tuplet.tupletNumber).ratio;
                  onTime = Quantize::findQuantizedChordOnTime(
                                           chordEvent, basicQuant, tupletRatio, barStart);
                              // verify that onTime is still inside tuplet
                  if (onTime < tuplet.onTime)
                        onTime = tuplet.onTime;
                  else if (onTime > tuplet.onTime + tuplet.len)
                        onTime = tuplet.onTime + tuplet.len;
                  }
            else {
                  onTime = Quantize::findQuantizedChordOnTime(chordEvent, basicQuant);
                              // verify that onTime is inside current bar
                  const auto oldOnTime = onTime;      // correct onTime only once
                  if (onTime < barStart)
                        onTime = barStart;
                  if (onTime == oldOnTime) {
                                    // verify that onTime is still outside tuplets
                        auto prev = chordIt;
                        while (prev != chords.begin()) {
                              --prev;
                              if (prev->second.isInTuplet
                                          && prev->second.voice == chord.voice) {
                                    const auto &tuplet = prev->second.tuplet->second;
                                    if (onTime < tuplet.onTime + tuplet.len)
                                          onTime = tuplet.onTime + tuplet.len;
                                    break;
                                    }
                              if (prev->second.barIndex != chord.barIndex)
                                    break;
                              }
                        }
                  if (onTime == oldOnTime) {
                        auto next = std::next(chordIt);
                        while (next != chords.end()) {
                              if (next->second.isInTuplet
                                          && next->second.voice == chord.voice) {
                                    const auto &tuplet = next->second.tuplet->second;
                                    if (onTime > tuplet.onTime)
                                          onTime = tuplet.onTime;
                                    break;
                                    }
                              if (next->second.barIndex != chord.barIndex)
                                    break;
                              ++next;
                              }
                        }
                  }

            for (auto noteIt = chord.notes.begin(); noteIt != chord.notes.end(); ) {
                  MidiNote &note = *noteIt;
                  auto offTime = note.offTime;

                  if (note.isInTuplet) {
                        const MidiTuplet::TupletData &tuplet = note.tuplet->second;
                        const auto tupletRatio = MidiTuplet::tupletLimits(tuplet.tupletNumber).ratio;

                        if (note.staccato) {
                                    // decrease tuplet error by enlarging staccato notes:
                                    // make note.len = tuplet note length
                              const auto tupletNoteLen = (tuplet.onTime + tuplet.len)
                                                          / tuplet.tupletNumber;
                              offTime = Quantize::findQuantizedNoteOffTime(
                                          chordEvent, chordEvent.first + tupletNoteLen, basicQuant,
                                          tupletRatio, barStart);
                              }
                        else {
                              offTime = Quantize::findQuantizedNoteOffTime(
                                                chordEvent, offTime, basicQuant, tupletRatio, barStart);
                              }
                                    // verify that offTime is still inside tuplet
                        if (offTime < tuplet.onTime)
                              offTime = tuplet.onTime;
                        else if (offTime > tuplet.onTime + tuplet.len)
                              offTime = tuplet.onTime + tuplet.len;
                        }
                  else {
                        offTime = Quantize::findQuantizedNoteOffTime(chordEvent, offTime, basicQuant);
                                   // verify that offTime is still outside tuplets
                        auto next = std::next(chordIt);
                        while (next != chords.end()) {
                              if (next->second.isInTuplet
                                          && next->second.voice == chord.voice) {
                                    const auto &tuplet = next->second.tuplet->second;
                                    if (offTime > tuplet.onTime)
                                          offTime = tuplet.onTime;
                                    break;
                                    }
                              if (next->second.barIndex != chord.barIndex)
                                    break;
                              ++next;
                              }
                        }

                  note.offTime = offTime;
                  if (note.offTime - onTime < MChord::minAllowedDuration()) {
                        noteIt = chord.notes.erase(noteIt);
                        // TODO - never delete notes here
                        qDebug() << "quantizeChords: note was removed due to its short length";
                        continue;
                        }
                  ++noteIt;
                  }
            if (!chord.notes.isEmpty())
                  quantizedChords.insert({onTime, chord});
            }

      std::swap(chords, quantizedChords);
      }

} // namespace Quantize
} // namespace Ms
