#ifndef IMPORTMIDI_TUPLETDATA_H
#define IMPORTMIDI_TUPLETDATA_H


namespace Ms {

struct TupletData
      {
      int voice;
      int onTime;
      int len;
      int tupletNumber;
      };

class MidiChord;

namespace Quantize {

struct TupletInfo
      {
      int voice;
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
