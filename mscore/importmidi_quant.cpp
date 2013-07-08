#include "importmidi_quant.h"
#include "libmscore/sig.h"
#include "libmscore/fraction.h"
#include "libmscore/mscore.h"
#include "preferences.h"
#include "importmidi_chord.h"
#include "importmidi_meter.h"
#include "importmidi_tuplet.h"

#include <set>


namespace Ms {

extern Preferences preferences;

namespace Quantize {

void applyAdaptiveQuant(std::multimap<Fraction, MidiChord> &/*chords*/,
                        const TimeSigMap */*sigmap*/,
                        const Fraction & /*allTicks*/)
      {
      }


Fraction shortestNoteInBar(const std::multimap<Fraction, MidiChord>::const_iterator &startBarChordIt,
                           const std::multimap<Fraction, MidiChord>::const_iterator &endChordIt,
                           const Fraction &endBarTick)
      {
      Fraction division = Fraction::fromTicks(MScore::division);
      Fraction minDuration = division;
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
      Fraction div = division;
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
            minDuration = quantizeValue(minDuration, div / 2);    //closest

      return minDuration;
      }

Fraction userQuantNoteToTicks(MidiOperation::QuantValue quantNote)
      {
      Fraction division = Fraction::fromTicks(MScore::division);
      Fraction userQuantValue = Fraction::fromTicks(preferences.shortestNote);
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
                  break;
            }

      return userQuantValue;
      }

Fraction fixedQuantRaster()
      {
      auto operations = preferences.midiImportOperations.currentTrackOperations();
      return userQuantNoteToTicks(operations.quantize.value);
      }

Fraction findQuantRaster(const std::multimap<Fraction, MidiChord>::iterator &startBarChordIt,
                         const std::multimap<Fraction, MidiChord>::iterator &endChordIt,
                         const Fraction &endBarTick)
      {
      Fraction raster;
      auto operations = preferences.midiImportOperations.currentTrackOperations();
                  // find raster value for quantization
      if (operations.quantize.value == MidiOperation::QuantValue::SHORTEST_IN_BAR)
            raster = shortestNoteInBar(startBarChordIt, endChordIt, endBarTick);
      else {
            Fraction userQuantValue = userQuantNoteToTicks(operations.quantize.value);
                        // if user value larger than the smallest note in bar
                        // then use the smallest note to keep faster events
            if (operations.quantize.reduceToShorterNotesInBar) {
                  raster = shortestNoteInBar(startBarChordIt, endChordIt, endBarTick);
                  if (userQuantValue < raster)
                        raster = userQuantValue;
                  }
            else
                  raster = userQuantValue;
            }
      return raster;
      }

Fraction quantizeValue(const Fraction &value, const Fraction &raster)
      {
      int valNum = value.numerator() * raster.denominator();
      int rastNum = raster.numerator() * value.denominator();
      int commonDen = value.denominator() * raster.denominator();
      valNum = ((valNum + rastNum / 2) / rastNum) * rastNum;
      return Fraction(valNum, commonDen).reduced();
      }

void doGridQuantizationOfBar(std::multimap<Fraction, MidiChord> &quantizedChords,
                             const std::multimap<Fraction, MidiChord>::iterator &startChordIt,
                             const std::multimap<Fraction, MidiChord>::iterator &endChordIt,
                             const Fraction &raster,
                             const Fraction &endBarTick,
                             const std::vector<std::multimap<Fraction, MidiChord>::iterator> &chordsNotQuant)
      {
      for (auto it = startChordIt; it != endChordIt; ++it) {
            if (it->first >= endBarTick)
                  break;
            auto found = std::find(chordsNotQuant.begin(), chordsNotQuant.end(), it);
            if (found != chordsNotQuant.end())
                  continue;
            auto chord = it->second;
            Fraction onTime = quantizeValue(it->first, raster);
            for (auto &note: chord.notes)
                  note.len = quantizeValue(note.len, raster);
            quantizedChords.insert({onTime, chord});
            }
      }

void applyGridQuant(std::multimap<Fraction, MidiChord> &chords,
                    std::multimap<Fraction, MidiChord> &quantizedChords,
                    const Fraction &lastTick,
                    const TimeSigMap* sigmap,
                    const std::vector<std::multimap<Fraction, MidiChord>::iterator> &chordsNotQuant
                                = std::vector<std::multimap<Fraction, MidiChord>::iterator>())
      {
      Fraction startBarTick;
      auto startBarChordIt = chords.begin();
      for (int i = 1;; ++i) {       // iterate over all measures by indexes
            Fraction endBarTick = Fraction::fromTicks(sigmap->bar2tick(i, 0));
            startBarChordIt = findFirstChordInRange(startBarTick, endBarTick,
                                                    startBarChordIt, chords.end());
            if (startBarChordIt != chords.end()) {      // if chords are found in this bar
                  Fraction raster = findQuantRaster(startBarChordIt, chords.end(), endBarTick);
                  doGridQuantizationOfBar(quantizedChords, startBarChordIt, chords.end(),
                                          raster, endBarTick, chordsNotQuant);
                  }
            else
                  startBarChordIt = chords.begin();
            if (endBarTick > lastTick)
                  break;
            startBarTick = endBarTick;
            }
      }

void applyGridQuant(std::multimap<Fraction, MidiChord> &chords,
                    const TimeSigMap *sigmap,
                    const Fraction &lastTick)
      {
      std::multimap<Fraction, MidiChord> quantizedChords;
      applyGridQuant(chords, quantizedChords, lastTick, sigmap);
      std::swap(chords, quantizedChords);
      }

// input chords - sorted by onTime value, onTime values don't repeat

void quantizeChordsAndTuplets(std::multimap<Fraction, MidiTuplet::TupletData> &tupletEvents,
                              std::multimap<Fraction, MidiChord> &inputChords,
                              const TimeSigMap *sigmap,
                              const Fraction &lastTick)
      {
      std::multimap<Fraction, MidiChord> quantizedChords;
      std::vector<std::multimap<Fraction, MidiChord>::iterator> tupletChords;
                  // quantize tuplet chords, if any
      Fraction startBarTick;
      for (int i = 1;; ++i) {       // iterate over all measures by indexes
            Fraction endBarTick = Fraction::fromTicks(sigmap->bar2tick(i, 0));
            Fraction barFraction = sigmap->timesig(startBarTick.ticks()).timesig();
            auto tuplets = MidiTuplet::findTuplets(startBarTick, endBarTick, barFraction, inputChords);

            for (auto &tupletInfo: tuplets) {
                  auto &infoChords = tupletInfo.chords;
                  for (auto &tupletChord: infoChords) {
                        int tupletNoteNum = tupletChord.first;
                        Fraction onTime = tupletInfo.onTime
                                   + tupletInfo.len / tupletInfo.tupletNumber * tupletNoteNum;
                        std::multimap<Fraction, MidiChord>::iterator &midiChordEventIt = tupletChord.second;
                                    // quantize chord to onTime value
                        MidiChord midiChord = midiChordEventIt->second;
                        for (auto &note: midiChord.notes) {
                              Fraction raster = MidiTuplet::findRasterForTupletNote(
                                                            onTime, note.len, tupletInfo);
                              Fraction offTime = Quantize::quantizeValue(onTime + note.len, raster);
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
      applyGridQuant(inputChords, quantizedChords, lastTick, sigmap, tupletChords);

      std::swap(inputChords, quantizedChords);
      }

} // namespace Quantize
} // namespace Ms
