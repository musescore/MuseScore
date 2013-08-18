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

void applyAdaptiveQuant(std::multimap<ReducedFraction, MidiChord> &/*chords*/,
                        const TimeSigMap */*sigmap*/,
                        const ReducedFraction & /*allTicks*/)
      {
      }


ReducedFraction shortestNoteInBar(const std::multimap<ReducedFraction, MidiChord>::const_iterator &startBarChordIt,
                                  const std::multimap<ReducedFraction, MidiChord>::const_iterator &endChordIt,
                                  const ReducedFraction &endBarTick)
      {
      auto division = ReducedFraction::fromTicks(MScore::division);
      auto minDuration = division;
                  // find shortest note in measure
      for (auto it = startBarChordIt; it != endChordIt; ++it) {
            if (it->first >= endBarTick)
                  break;
            for (const auto &note: it->second.notes) {
                  if (note.len < minDuration)
                        minDuration = note.len;
                  }
            }
                  // determine suitable quantization value based
                  // on shortest note in measure
      auto div = division;
      for (ReducedFraction duration(1, 16); duration <= ReducedFraction(8, 1); duration *= 2) {
                        // minimum duration is 1/64
            if (minDuration <= division * duration) {
                  div = division * duration;
                  break;
                  }
            }
      if (div == (division / 16))
            minDuration = div;
      else
            minDuration = quantizeValue(minDuration, div / 2);    //closest

      return minDuration;
      }

ReducedFraction userQuantNoteToTicks(MidiOperation::QuantValue quantNote)
      {
      auto division = ReducedFraction::fromTicks(MScore::division);
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

ReducedFraction fixedQuantRaster()
      {
      auto operations = preferences.midiImportOperations.currentTrackOperations();
      return userQuantNoteToTicks(operations.quantize.value);
      }

ReducedFraction findRegularQuantRaster(const std::multimap<ReducedFraction, MidiChord>::iterator &startBarChordIt,
                                       const std::multimap<ReducedFraction, MidiChord>::iterator &endChordIt,
                                       const ReducedFraction &endBarTick)
      {
      auto operations = preferences.midiImportOperations.currentTrackOperations();
      auto raster = userQuantNoteToTicks(operations.quantize.value);
                  // if user value larger than the smallest note in bar
                  // then use the smallest note to keep faster events
      if (operations.quantize.reduceToShorterNotesInBar) {
            auto shortest = shortestNoteInBar(startBarChordIt, endChordIt, endBarTick);
            if (shortest < raster)
                  raster = shortest;
            }
      return raster;
      }

ReducedFraction reduceRasterIfDottedNote(const ReducedFraction &len, const ReducedFraction &raster)
      {
      auto newRaster = raster;
      auto div = len / raster;
      double ratio = div.numerator() * 1.0 / div.denominator();
      if (ratio > 1.4 && ratio < 1.6)       // 1.5: dotted note that is larger than quantization value
            newRaster /= 2;                 // reduce quantization error for dotted notes
      return newRaster;
      }

ReducedFraction quantizeValue(const ReducedFraction &value, const ReducedFraction &raster)
      {
      int valNum = value.numerator() * raster.denominator();
      int rastNum = raster.numerator() * value.denominator();
      int commonDen = value.denominator() * raster.denominator();
      valNum = ((valNum + rastNum / 2) / rastNum) * rastNum;
      return ReducedFraction(valNum, commonDen).reduced();
      }

void doGridQuantizationOfBar(std::multimap<ReducedFraction, MidiChord> &quantizedChords,
                             const std::multimap<ReducedFraction, MidiChord>::iterator &startChordIt,
                             const std::multimap<ReducedFraction, MidiChord>::iterator &endChordIt,
                             const ReducedFraction &regularRaster,
                             const ReducedFraction &endBarTick,
                             const std::vector<std::multimap<ReducedFraction, MidiChord>::iterator> &chordsNotQuant,
                             const std::vector<MidiTuplet::TupletInfo> &tuplets)
      {
      for (auto it = startChordIt; it != endChordIt; ++it) {
            if (it->first >= endBarTick)
                  break;
            auto found = std::find(chordsNotQuant.begin(), chordsNotQuant.end(), it);
            if (found != chordsNotQuant.end())
                  continue;
            auto chord = it->second;
            auto onTimeRaster = reduceRasterIfDottedNote(maxNoteLen(chord.notes), regularRaster);
            auto onTime = quantizeValue(it->first, onTimeRaster);
            for (auto &note: chord.notes) {
                  auto offTime = onTime + note.len;
                  auto offTimeRaster = MidiTuplet::findOffTimeRaster(offTime, it->second.voice,
                                                                     regularRaster, tuplets);
                  if (offTimeRaster == regularRaster)    // offTime is not inside tuplet
                        offTimeRaster = reduceRasterIfDottedNote(note.len, offTimeRaster);
                  offTime = quantizeValue(offTime, offTimeRaster);
                  note.len = offTime - onTime;
                  }
            quantizedChords.insert({onTime, chord});
            }
      }

void applyGridQuant(std::multimap<ReducedFraction, MidiChord> &chords,
                    std::multimap<ReducedFraction, MidiChord> &quantizedChords,
                    const ReducedFraction &lastTick,
                    const TimeSigMap* sigmap,
                    const std::vector<std::multimap<ReducedFraction, MidiChord>::iterator> &chordsNotQuant
                                = std::vector<std::multimap<ReducedFraction, MidiChord>::iterator>(),
                    const std::vector<MidiTuplet::TupletInfo> &tuplets
                                = std::vector<MidiTuplet::TupletInfo>())
      {
      ReducedFraction startBarTick;
      auto startBarChordIt = chords.begin();
      for (int i = 1;; ++i) {       // iterate over all measures by indexes
            auto endBarTick = ReducedFraction::fromTicks(sigmap->bar2tick(i, 0));
            startBarChordIt = findFirstChordInRange(startBarTick, endBarTick,
                                                    startBarChordIt, chords.end());
            if (startBarChordIt != chords.end()) {      // if chords are found in this bar
                  auto onTimeRaster = findRegularQuantRaster(startBarChordIt, chords.end(), endBarTick);
                  doGridQuantizationOfBar(quantizedChords, startBarChordIt, chords.end(),
                                          onTimeRaster, endBarTick, chordsNotQuant, tuplets);
                  }
            else
                  startBarChordIt = chords.begin();
            if (endBarTick > lastTick)
                  break;
            startBarTick = endBarTick;
            }
      }

void applyGridQuant(std::multimap<ReducedFraction, MidiChord> &chords,
                    const TimeSigMap *sigmap,
                    const ReducedFraction &lastTick)
      {
      std::multimap<ReducedFraction, MidiChord> quantizedChords;
      applyGridQuant(chords, quantizedChords, lastTick, sigmap);
      std::swap(chords, quantizedChords);
      }

// input chords - sorted by onTime value, onTime values are not repeated

void quantizeChordsAndTuplets(std::multimap<ReducedFraction, MidiTuplet::TupletData> &tupletEvents,
                              std::multimap<ReducedFraction, MidiChord> &inputChords,
                              const TimeSigMap *sigmap,
                              const ReducedFraction &lastTick)
      {
      std::multimap<ReducedFraction, MidiChord> quantizedChords;
      std::vector<std::multimap<ReducedFraction, MidiChord>::iterator> tupletChords;
      std::vector<MidiTuplet::TupletInfo> tupletInformation;
                  // quantize tuplet chords, if any
      ReducedFraction startBarTick;
      for (int i = 1;; ++i) {       // iterate over all measures by indexes
            auto endBarTick = ReducedFraction::fromTicks(sigmap->bar2tick(i, 0));
            auto barFraction = ReducedFraction(sigmap->timesig(startBarTick.ticks()).timesig());
            auto tuplets = MidiTuplet::findTuplets(startBarTick, endBarTick, barFraction, inputChords);
            tupletInformation.insert(tupletInformation.end(), tuplets.begin(), tuplets.end());

            for (auto &tupletInfo: tuplets) {
                  auto &infoChords = tupletInfo.chords;
                  for (auto &tupletChord: infoChords) {
                        int tupletNoteNum = tupletChord.first;
                        auto onTime = tupletInfo.onTime
                                    + tupletInfo.len / tupletInfo.tupletNumber * tupletNoteNum;
                        const auto &midiChordEventIt = tupletChord.second;
                                    // quantize chord to onTime value
                        MidiChord midiChord = midiChordEventIt->second;
                        for (auto &note: midiChord.notes) {
                              auto raster = MidiTuplet::findOffTimeRaster(
                                                            onTime + note.len, midiChord.voice,
                                                            tupletInfo.regularQuant, tuplets);
                              auto offTime = Quantize::quantizeValue(onTime + note.len, raster);
                              note.len = offTime - onTime;
                              }
                        quantizedChords.insert({onTime, midiChord});
                        tupletChords.push_back(midiChordEventIt);
                        }
                  MidiTuplet::TupletData tupletData = {tupletInfo.chords.begin()->second->second.voice,
                                                       tupletInfo.onTime,
                                                       tupletInfo.len,
                                                       tupletInfo.tupletNumber,
                                                       {}};
                  tupletEvents.insert({tupletInfo.onTime, tupletData});
                  }
            if (endBarTick > lastTick)
                  break;
            startBarTick = endBarTick;
            }
                  // quantize non-tuplet (remaining) chords with ordinary grid
      applyGridQuant(inputChords, quantizedChords, lastTick, sigmap, tupletChords, tupletInformation);

      std::swap(inputChords, quantizedChords);
      }

} // namespace Quantize
} // namespace Ms
