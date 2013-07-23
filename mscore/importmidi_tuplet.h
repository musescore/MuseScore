#ifndef IMPORTMIDI_TUPLET_H
#define IMPORTMIDI_TUPLET_H

#include "libmscore/fraction.h"


namespace Ms {

class MidiChord;
class DurationElement;

namespace MidiTuplet {

struct TupletData
      {
      int voice;
      Fraction onTime;
      Fraction len;
      int tupletNumber;
      std::vector<DurationElement *> elements;
      };

struct TupletInfo
      {
      Fraction onTime;
      Fraction len;
      int tupletNumber;
      Fraction tupletQuantValue;
      Fraction regularQuantValue;
                  // <note index in tuplet, chord iterator>
      std::map<int, std::multimap<Fraction, MidiChord>::iterator> chords;
      Fraction tupletSumError;
      Fraction regularSumError;
      Fraction sumLengthOfRests;
      };

// conversion ratios from tuplet durations to regular durations
// for example, 8th note in triplet * 3/2 = regular 8th note

const std::map<int, Fraction>& tupletRatios();

void filterTuplets(std::vector<TupletInfo> &tuplets);

Fraction findOffTimeRaster(const Fraction &noteOffTime,
                           int voice,
                           const Fraction &regularQuantValue,
                           const std::vector<TupletInfo> &tuplets);

std::vector<TupletInfo> findTuplets(const Fraction &startBarTick,
                                    const Fraction &endBarTick,
                                    const Fraction &barFraction,
                                    std::multimap<Fraction, MidiChord> &chords);

std::vector<TupletData> findTupletsForDuration(int voice,
                                               const Fraction &barStartTick,
                                               const Fraction &durationOnTime,
                                               const Fraction &durationLen,
                                               const std::multimap<Fraction, TupletData> &tuplets);

} // namespace MidiTuplet
} // namespace Ms


#endif // IMPORTMIDI_TUPLET_H
