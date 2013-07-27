#ifndef IMPORTMIDI_CHORD_H
#define IMPORTMIDI_CHORD_H

#include "libmscore/fraction.h"


namespace Ms {

class Tie;

class MidiNote {
   public:
      int pitch;
      int velo;
      Fraction len;
      Tie* tie = nullptr;
      };

class MidiChord {
   public:
      int voice = 0;
      QList<MidiNote> notes;
      };

std::multimap<Fraction, MidiChord>::iterator
findFirstChordInRange(const Fraction &startRangeTick,
                      const Fraction &endRangeTick,
                      const std::multimap<Fraction, MidiChord>::iterator &startChordIt,
                      const std::multimap<Fraction, MidiChord>::iterator &endChordIt);

std::multimap<Fraction, MidiChord>::iterator
findEndChordInRange(const Fraction &endRangeTick,
                    const std::multimap<Fraction, MidiChord>::iterator &startChordIt,
                    const std::multimap<Fraction, MidiChord>::iterator &endChordIt);

Fraction maxNoteLen(const QList<MidiNote> &notes);

} // namespace Ms


#endif // IMPORTMIDI_CHORD_H
