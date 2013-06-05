#include "importmidi_quant.h"
#include "libmscore/sig.h"
#include "libmscore/utils.h"
#include "libmscore/fraction.h"
#include "libmscore/mscore.h"
#include "preferences.h"
#include "importmidi_chord.h"
#include "importmidi_meter.h"


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
      //
      for (auto i = start; i != chords.end(); ++i) {
            if (i->first >= endBarTick)
                  break;
            for (const auto &note: i->second.notes)
                  minDuration = qMin(minDuration, note.len);
            }
      //
      // determine suitable quantization value based
      // on shortest note in measure
      //
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
//            case MidiOperation::QuantValue::N_4_triplet:
//                  userQuantValue = division * 2 / 3;
//                  break;
            case MidiOperation::QuantValue::N_8:
                  userQuantValue = division / 2;
                  break;
//            case MidiOperation::QuantValue::N_8_triplet:
//                  userQuantValue = division / 3;
//                  break;
            case MidiOperation::QuantValue::N_16:
                  userQuantValue = division / 4;
                  break;
//            case MidiOperation::QuantValue::N_16_triplet:
//                  userQuantValue = division / 6;
//                  break;
            case MidiOperation::QuantValue::N_32:
                  userQuantValue = division / 8;
                  break;
//            case MidiOperation::QuantValue::N_32_triplet:
//                  userQuantValue = division / 12;
//                  break;
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

void doGridQuantizationOfBar(const std::multimap<int, MidiChord> &chords,
                             std::multimap<int, MidiChord> &quantizedChords,
                             const std::multimap<int, MidiChord>::const_iterator &startChordIter,
                             int raster,
                             int endBarTick)
      {
      int raster2 = raster >> 1;
      for (auto i = startChordIter; i != chords.end(); ++i) {
            if (i->first >= endBarTick)
                  break;
            auto chord = i->second;
            chord.onTime = ((chord.onTime + raster2) / raster) * raster;
            for (auto &note: chord.notes) {
                  note.onTime = chord.onTime;
                  note.len  = quantizeLen(note.len, raster);
                  }
            quantizedChords.insert({chord.onTime, chord});
            }
      }

const std::multimap<int, MidiChord>::const_iterator
findFirstChordInBar(const std::multimap<int, MidiChord> &chords, int startBarTick, int endBarTick)
      {
      auto i = chords.begin();
      for (; i != chords.end(); ++i) {
            if (i->first >= startBarTick) {
                  if (i->first >= endBarTick)
                        return chords.end();
                  break;
                  }
            }
      return i;
      }

void quantizeChordsOfBar(const std::multimap<int, MidiChord> &chords,
                         std::multimap<int, MidiChord> &quantizedChords,
                         int startBarTick,
                         int endBarTick)
      {
      auto startChordIter = findFirstChordInBar(chords, startBarTick, endBarTick);
      if (startChordIter == chords.end()) // if no chords found in this bar
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
      for (int i = 1;; ++i) { // iterate over all measures by indexes
            int endBarTick = sigmap->bar2tick(i, 0);
            quantizeChordsOfBar(chords, quantizedChords, startBarTick, endBarTick);
            if (endBarTick > lastTick)
                  break;
            startBarTick = endBarTick;
            }
      chords = quantizedChords;
      }

} // namespace Quantize
} // namespace Ms
