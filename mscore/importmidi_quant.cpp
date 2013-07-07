#include "importmidi_quant.h"
#include "libmscore/sig.h"
#include "libmscore/utils.h"
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

void applyAdaptiveQuant(std::multimap<int, MidiChord> &/*chords*/,
                        const TimeSigMap */*sigmap*/,
                        int /*allTicks*/)
      {
      }


int shortestNoteInBar(const std::multimap<int, MidiChord>::const_iterator &startBarChordIt,
                      const std::multimap<int, MidiChord>::const_iterator &endChordIt,
                      int endBarTick)
      {
      int division = MScore::division;
      int minDuration = division;
                  // find shortest note in measure
      for (auto it = startBarChordIt; it != endChordIt; ++it) {
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

int fixedQuantRaster()
      {
      auto operations = preferences.midiImportOperations.currentTrackOperations();
      return userQuantNoteToTicks(operations.quantize.value);
      }

int findQuantRaster(const std::multimap<int, MidiChord>::iterator &startBarChordIt,
                    const std::multimap<int, MidiChord>::iterator &endChordIt,
                    int endBarTick)
      {
      int raster;
      auto operations = preferences.midiImportOperations.currentTrackOperations();
                  // find raster value for quantization
      if (operations.quantize.value == MidiOperation::QuantValue::SHORTEST_IN_BAR)
            raster = shortestNoteInBar(startBarChordIt, endChordIt, endBarTick);
      else {
            int userQuantValue = userQuantNoteToTicks(operations.quantize.value);
                        // if user value larger than the smallest note in bar
                        // then use the smallest note to keep faster events
            if (operations.quantize.reduceToShorterNotesInBar) {
                  raster = shortestNoteInBar(startBarChordIt, endChordIt, endBarTick);
                  raster = qMin(userQuantValue, raster);
                  }
            else
                  raster = userQuantValue;
            }
      return raster;
      }

void quantizeChord(MidiChord &chord, int raster)
      {
      int raster2 = raster >> 1;
      chord.onTime = ((chord.onTime + raster2) / raster) * raster;
      for (auto &note: chord.notes) {
            note.onTime = chord.onTime;
            note.len = quantizeLen(note.len, raster);
            }
      }

void doGridQuantizationOfBar(std::multimap<int, MidiChord> &quantizedChords,
                             const std::multimap<int, MidiChord>::iterator &startChordIt,
                             const std::multimap<int, MidiChord>::iterator &endChordIt,
                             int raster,
                             int endBarTick,
                             const std::vector<std::multimap<int, MidiChord>::iterator> &chordsNotQuant)
      {
      for (auto it = startChordIt; it != endChordIt; ++it) {
            if (it->first >= endBarTick)
                  break;
            auto found = std::find(chordsNotQuant.begin(), chordsNotQuant.end(), it);
            if (found != chordsNotQuant.end())
                  continue;
            auto chord = it->second;
            quantizeChord(chord, raster);
            quantizedChords.insert({chord.onTime, chord});
            }
      }

void applyGridQuant(std::multimap<int, MidiChord> &chords,
                    std::multimap<int, MidiChord> &quantizedChords,
                    int lastTick,
                    const TimeSigMap* sigmap,
                    const std::vector<std::multimap<int, MidiChord>::iterator> &chordsNotQuant
                                = std::vector<std::multimap<int, MidiChord>::iterator>())
      {
      int startBarTick = 0;
      auto startBarChordIt = chords.begin();
      for (int i = 1;; ++i) {       // iterate over all measures by indexes
            int endBarTick = sigmap->bar2tick(i, 0);
            startBarChordIt = findFirstChordInRange(startBarTick, endBarTick,
                                                    startBarChordIt, chords.end());
            if (startBarChordIt != chords.end()) {      // if chords are found in this bar
                  int raster = findQuantRaster(startBarChordIt, chords.end(), endBarTick);
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

void applyGridQuant(std::multimap<int, MidiChord> &chords,
                    const TimeSigMap* sigmap,
                    int lastTick)
      {
      std::multimap<int, MidiChord> quantizedChords;
      applyGridQuant(chords, quantizedChords, lastTick, sigmap);
      std::swap(chords, quantizedChords);
      }

// input chords - sorted by onTime value, onTime values don't repeat

void quantizeChordsAndTuplets(std::multimap<int, MidiTuplet::TupletData> &tupletEvents,
                              std::multimap<int, MidiChord> &chords,
                              const TimeSigMap* sigmap,
                              int lastTick)
      {
      std::multimap<int, MidiChord> quantizedChords;
      std::vector<std::multimap<int, MidiChord>::iterator> tupletChords;
                  // quantize tuplet chords, if any
      int startBarTick = 0;
      for (int i = 1;; ++i) {       // iterate over all measures by indexes
            int endBarTick = sigmap->bar2tick(i, 0);
            Fraction barFraction = sigmap->timesig(startBarTick).timesig();
            auto tuplets = MidiTuplet::findTuplets(startBarTick, endBarTick, barFraction, chords);

            for (auto &tupletInfo: tuplets) {
                  auto &infoChords = tupletInfo.chords;
                  for (auto &tupletChord: infoChords) {
                        int tupletNoteNum = tupletChord.first;
                        int onTime = tupletInfo.onTime
                                   + ((tupletInfo.len / tupletInfo.tupletNumber) * tupletNoteNum).ticks();
                        std::multimap<int, MidiChord>::iterator &midiChordEventIt = tupletChord.second;
                                    // quantize chord to onTime value
                        MidiChord midiChord = midiChordEventIt->second;
                        MidiTuplet::quantizeTupletChord(midiChord, onTime, tupletInfo);
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
      applyGridQuant(chords, quantizedChords, lastTick, sigmap, tupletChords);

      std::swap(chords, quantizedChords);
      }

} // namespace Quantize
} // namespace Ms
