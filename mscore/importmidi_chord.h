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


template <typename Iter>
Iter findFirstChordInRange(const Fraction &startRangeTick,
                           const Fraction &endRangeTick,
                           const Iter &startChordIt,
                           const Iter &endChordIt)
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

template <typename Iter>
Iter findEndChordInRange(const Fraction &endRangeTick,
                         const Iter &startChordIt,
                         const Iter &endChordIt)
      {
      auto it = startChordIt;
      for (; it != endChordIt; ++it) {
            if (it->first > endRangeTick)
                  break;
            }
      return it;
      }

Fraction maxNoteLen(const QList<MidiNote> &notes);
int findAveragePitch(const QList<MidiNote> &notes);

} // namespace Ms


#endif // IMPORTMIDI_CHORD_H
