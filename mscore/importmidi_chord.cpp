#include "importmidi_chord.h"


namespace Ms {

std::multimap<Fraction, MidiChord>::iterator
findFirstChordInRange(const Fraction &startRangeTick,
                      const Fraction &endRangeTick,
                      const std::multimap<Fraction, MidiChord>::iterator &startChordIt,
                      const std::multimap<Fraction, MidiChord>::iterator &endChordIt)
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

std::multimap<Fraction, MidiChord>::iterator
findEndChordInRange(const Fraction &endRangeTick,
                    const std::multimap<Fraction, MidiChord>::iterator &startChordIt,
                    const std::multimap<Fraction, MidiChord>::iterator &endChordIt)
      {
      auto it = startChordIt;
      for (; it != endChordIt; ++it) {
            if (it->first > endRangeTick)
                  break;
            }
      return it;
      }

Fraction maxNoteLen(const QList<MidiNote> &notes)
      {
      Fraction maxLen;
      for (const auto &note: notes) {
            if (note.len > maxLen)
                  maxLen = note.len;
            }
      return maxLen;
      }

} // namespace Ms
