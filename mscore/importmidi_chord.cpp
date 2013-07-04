#include "importmidi_chord.h"


namespace Ms {

std::multimap<int, MidiChord>::iterator
findFirstChordInRange(int startRangeTick,
                      int endRangeTick,
                      const std::multimap<int, MidiChord>::iterator &startChordIt,
                      const std::multimap<int, MidiChord>::iterator &endChordIt)
      {
      auto it = startChordIt;
      for (; it != endChordIt; ++it) {
            if (it->first >= startRangeTick) {
                  if (it->first >= endRangeTick)
                        it = endChordIt;
                  break;
                  }
            }
      return it;
      }

std::multimap<int, MidiChord>::iterator
findEndChordInRange(int endRangeTick,
                    const std::multimap<int, MidiChord>::iterator &startChordIt,
                    const std::multimap<int, MidiChord>::iterator &endChordIt)
      {
      auto it = startChordIt;
      for (; it != endChordIt; ++it) {
            if (it->first > endRangeTick)
                  break;
            }
      return it;
      }

} // namespace Ms
