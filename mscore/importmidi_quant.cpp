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

ReducedFraction shortestNoteInBar(const std::multimap<ReducedFraction, MidiChord>::const_iterator &startBarChordIt,
                                  const std::multimap<ReducedFraction, MidiChord>::const_iterator &endChordIt,
                                  const ReducedFraction &endBarTick)
      {
      const auto division = ReducedFraction::fromTicks(MScore::division);
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

ReducedFraction fixedQuantRaster()
      {
      const auto operations = preferences.midiImportOperations.currentTrackOperations();
      return userQuantNoteToFraction(operations.quantize.value);
      }

ReducedFraction findRegularQuantRaster(
            const std::multimap<ReducedFraction, MidiChord>::const_iterator &startBarChordIt,
            const std::multimap<ReducedFraction, MidiChord>::const_iterator &endChordIt,
            const ReducedFraction &endBarTick)
      {
      const auto operations = preferences.midiImportOperations.currentTrackOperations();
      auto raster = userQuantNoteToFraction(operations.quantize.value);
                  // if user value larger than the smallest note in bar
                  // then use the smallest note to keep faster events
      if (operations.quantize.reduceToShorterNotesInBar) {
            const auto shortest = shortestNoteInBar(startBarChordIt, endChordIt, endBarTick);
            if (shortest < raster)
                  raster = shortest;
            }
      return raster;
      }

ReducedFraction reduceRasterIfDottedNote(const ReducedFraction &noteLen,
                                         const ReducedFraction &raster)
      {
      auto newRaster = raster;
      const auto div = noteLen / raster;
      const double ratio = div.numerator() * 1.0 / div.denominator();
      if (ratio > 1.4 && ratio < 1.6)       // 1.5: dotted note that is larger than quantization value
            newRaster /= 2;                 // reduce quantization error for dotted notes
      return newRaster;
      }

ReducedFraction quantizeValue(const ReducedFraction &value,
                              const ReducedFraction &raster)
      {
      int valNum = value.numerator() * raster.denominator();
      const int rastNum = raster.numerator() * value.denominator();
      const int commonDen = value.denominator() * raster.denominator();
      valNum = ((valNum + rastNum / 2) / rastNum) * rastNum;
      return ReducedFraction(valNum, commonDen).reduced();
      }

ReducedFraction findQuantRaster(
            const ReducedFraction &time,
            int voice,
            const std::multimap<ReducedFraction, MidiTuplet::TupletData> &tupletEvents,
            const std::multimap<ReducedFraction, MidiChord> &chords,
            const TimeSigMap *sigmap)
      {
      ReducedFraction raster;
      const auto tupletIt = MidiTuplet::findTupletContainsTime(voice, time, tupletEvents);

      if (tupletIt != tupletEvents.end() && time > tupletIt->first)
            raster = tupletIt->second.tupletQuant;   // quantize onTime with tuplet quant
      else {
                        // quantize onTime with regular quant
            int bar, beat, tick;
            sigmap->tickValues(time.ticks(), &bar, &beat, &tick);
            const auto startBarTick = ReducedFraction::fromTicks(sigmap->bar2tick(bar, 0));
            const auto endBarTick = startBarTick
                        + ReducedFraction(sigmap->timesig(startBarTick.ticks()).timesig());
            const auto startBarChordIt = MChord::findFirstChordInRange(
                            startBarTick, endBarTick, chords.begin(), chords.end());
            raster = findRegularQuantRaster(startBarChordIt, chords.end(), endBarTick);
            }
      return raster;
      }

// input chords - sorted by onTime value, onTime values are not repeated

void quantizeChords(std::multimap<ReducedFraction, MidiChord> &chords,
                    const std::multimap<ReducedFraction, MidiTuplet::TupletData> &tupletEvents,
                    const TimeSigMap *sigmap)
      {
      std::multimap<ReducedFraction, MidiChord> quantizedChords;
      for (auto &chordEvent: chords) {
            MidiChord chord = chordEvent.second;     // copy chord
            auto onTime = chordEvent.first;
            auto raster = findQuantRaster(onTime, chord.voice, tupletEvents, chords, sigmap);
            onTime = Quantize::quantizeValue(onTime, raster);

            for (MidiNote &note: chord.notes) {
                  auto offTime = chordEvent.first + note.len;
                  raster = findQuantRaster(offTime, chord.voice, tupletEvents, chords, sigmap);
                  if (Meter::isSimpleNoteDuration(raster))    // offTime is not inside tuplet
                        raster = reduceRasterIfDottedNote(note.len, raster);
                  offTime = Quantize::quantizeValue(offTime, raster);
                  note.len = offTime - onTime;
                  }
            quantizedChords.insert({onTime, chord});
            }

      std::swap(chords, quantizedChords);
      }

} // namespace Quantize
} // namespace Ms
