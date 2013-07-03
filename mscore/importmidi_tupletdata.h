#ifndef IMPORTMIDI_TUPLETDATA_H
#define IMPORTMIDI_TUPLETDATA_H


namespace Ms {

class DurationElement;

struct TupletData
      {
      int voice;
      int onTime;
      int len;
      int tupletNumber;
      std::vector<DurationElement *> elements;
      };

class Fraction;

// conversion ratios from tuplet durations to regular durations
// for example, 8th note in triplet * 3/2 = regular 8th note

const std::map<int, Fraction> tupletRatios = {{2, Fraction({2, 3})}, {3, Fraction({3, 2})},
                                              {4, Fraction({4, 3})}, {5, Fraction({5, 4})},
                                              {7.0, Fraction({7, 4})}};
class MidiChord;

namespace Quantize {

struct TupletInfo
      {
      int onTime;
      int len;
      int tupletNumber;
      int tupletQuantValue;
      int regularQuantValue;
                  // <note index in tuplet, chord iterator>
      std::map<int, std::multimap<int, MidiChord>::iterator> chords;
      int tupletOnTimeSumError = 0;
      int regularSumError = 0;
      };

} // namespace Quantize
} // namespace Ms


#endif // IMPORTMIDI_TUPLETDATA_H
