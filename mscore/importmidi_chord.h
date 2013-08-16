#ifndef IMPORTMIDI_CHORD_H
#define IMPORTMIDI_CHORD_H

#include "importmidi_fraction.h"


namespace Ms {

class Tie;

class MidiNote {
   public:
      int pitch;
      int velo;
      ReducedFraction len;
      Tie* tie = nullptr;
      };

class MidiChord {
   public:
      int voice = 0;
      QList<MidiNote> notes;
      };


template <typename Iter>
Iter findFirstChordInRange(const ReducedFraction &startRangeTick,
                           const ReducedFraction &endRangeTick,
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
Iter findEndChordInRange(const ReducedFraction &endRangeTick,
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

ReducedFraction maxNoteLen(const QList<MidiNote> &notes);
int findAveragePitch(const QList<MidiNote> &notes);

} // namespace Ms


#endif // IMPORTMIDI_CHORD_H
