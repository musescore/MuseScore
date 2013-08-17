#ifndef IMPORTMIDI_TUPLET_H
#define IMPORTMIDI_TUPLET_H

#include "importmidi_fraction.h"


namespace Ms {

class MidiChord;
class DurationElement;
class Fraction;

namespace MidiTuplet {

struct TupletData
      {
      int voice;
      ReducedFraction onTime;
      ReducedFraction len;
      int tupletNumber;
      std::vector<DurationElement *> elements;
      };

struct TupletInfo
      {
      ReducedFraction onTime = -1;  // invalid
      ReducedFraction len = -1;
      int tupletNumber = -1;
      ReducedFraction tupletQuant;
      ReducedFraction regularQuant;
                  // <note index in tuplet, chord iterator>
      std::map<int, std::multimap<ReducedFraction, MidiChord>::iterator> chords;
      ReducedFraction tupletSumError;
      ReducedFraction regularSumError;
      ReducedFraction sumLengthOfRests;
      };

// conversion ratios from tuplet durations to regular durations
// for example, 8th note in triplet * 3/2 = regular 8th note

const std::map<int, Fraction> &tupletRatios();

void filterTuplets(std::vector<TupletInfo> &tuplets);

ReducedFraction findOffTimeRaster(const ReducedFraction &noteOffTime,
                                  int voice,
                                  const ReducedFraction &regularQuant,
                                  const std::vector<TupletInfo> &tuplets);

std::vector<TupletInfo> findTuplets(const ReducedFraction &startBarTick,
                                    const ReducedFraction &endBarTick,
                                    const ReducedFraction &barFraction,
                                    std::multimap<ReducedFraction, MidiChord> &chords);

std::vector<TupletData> findTupletsForDuration(int voice,
                                               const ReducedFraction &barStartTick,
                                               const ReducedFraction &durationOnTime,
                                               const ReducedFraction &durationLen,
                                               const std::multimap<ReducedFraction, TupletData> &tuplets);

} // namespace MidiTuplet
} // namespace Ms


#endif // IMPORTMIDI_TUPLET_H
