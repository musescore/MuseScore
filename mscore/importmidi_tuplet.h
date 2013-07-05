#ifndef IMPORTMIDI_TUPLET_H
#define IMPORTMIDI_TUPLET_H


namespace Ms {

class MidiChord;
class Fraction;
class DurationElement;

namespace MidiTuplet {

struct TupletData
      {
      int voice;
      int onTime;
      int len;
      int tupletNumber;
      std::vector<DurationElement *> elements;
      };

struct TupletInfo
      {
      int onTime;
      int len;
      int tupletNumber;
      int tupletQuantValue;
      int regularQuantValue;
                  // <note index in tuplet, chord iterator>
      std::map<int, std::multimap<int, MidiChord>::iterator> chords;
      int tupletSumError = 0;
      int regularSumError = 0;
      double averageError = 0.0;
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

} // namespace MidiTuplet
} // namespace Ms


#endif // IMPORTMIDI_TUPLET_H
