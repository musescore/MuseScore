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

class MTrack;

namespace MChord {

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
int findAveragePitch(const std::map<ReducedFraction, MidiChord>::const_iterator &startChordIt,
                     const std::map<ReducedFraction, MidiChord>::const_iterator &endChordIt);
ReducedFraction minAllowedDuration();
ReducedFraction findMinDuration(const QList<MidiChord> &midiChords,
                                const ReducedFraction &length);
void sortNotesByPitch(std::multimap<ReducedFraction, MidiChord> &chords);
void collectChords(std::multimap<int, MTrack> &tracks);
void removeOverlappingNotes(std::multimap<int, MTrack> &tracks);
void splitUnequalChords(std::multimap<int, MTrack> &tracks);

} // namespace MChord
} // namespace Ms


#endif // IMPORTMIDI_CHORD_H
