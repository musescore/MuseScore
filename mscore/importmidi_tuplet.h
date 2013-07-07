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
      int onTime;
      Fraction len;
      int tupletNumber;
      std::vector<DurationElement *> elements;
      };

struct TupletInfo
      {
      int onTime;
      Fraction len;
      int tupletNumber;
      Fraction tupletQuantValue;
      Fraction regularQuantValue;
                  // <note index in tuplet, chord iterator>
      std::map<int, std::multimap<int, MidiChord>::iterator> chords;
      int tupletSumError = 0;
      int regularSumError = 0;
      int sumLengthOfRests = 0;
      };

// conversion ratios from tuplet durations to regular durations
// for example, 8th note in triplet * 3/2 = regular 8th note

const std::map<int, Fraction>& tupletRatios();

void filterTuplets(std::vector<TupletInfo> &tuplets);

void quantizeTupletChord(MidiChord &midiChord, int onTime, const TupletInfo &tupletInfo);

std::vector<TupletInfo> findTuplets(int startBarTick,
                                    int endBarTick,
                                    const Fraction &barFraction,
                                    std::multimap<int, MidiChord> &chords);

std::vector<TupletData> findTupletsForDuration(int voice,
                                               int barTick,
                                               int durationOnTime,
                                               int durationLen,
                                               const std::multimap<int, TupletData> &tuplets);

} // namespace MidiTuplet
} // namespace Ms


#endif // IMPORTMIDI_TUPLET_H
