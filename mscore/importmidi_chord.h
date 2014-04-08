#ifndef IMPORTMIDI_CHORD_H
#define IMPORTMIDI_CHORD_H

#include "importmidi_fraction.h"

#include <map>


namespace Ms {

class Tie;

class MidiNote {
   public:
      int pitch;
      int velo;
      ReducedFraction offTime;
      ReducedFraction quantizedOffTime = {-1, 1};     // invalid
      Tie* tie = nullptr;
      bool staccato = false;
      };

class MidiChord {
   public:
      int voice = 0;
      bool isInTuplet = false;
      ReducedFraction quantizedOnTime = {-1, 1};      // invalid
      QList<MidiNote> notes;
      bool isStaccato() const
            {
            for (const auto &note: notes)
                  if (note.staccato)
                        return true;
            return false;
            }
      };

class MTrack;

namespace MChord {

std::multimap<ReducedFraction, MidiChord>::iterator
findFirstChordInRange(std::multimap<ReducedFraction, MidiChord> &chords,
                      const ReducedFraction &startRangeTick,
                      const ReducedFraction &endRangeTick);

std::multimap<ReducedFraction, MidiChord>::const_iterator
findFirstChordInRange(const std::multimap<ReducedFraction, MidiChord> &chords,
                      const ReducedFraction &startRangeTick,
                      const ReducedFraction &endRangeTick);

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
            if (it->first >= endRangeTick)
                  break;
            }
      return it;
      }

ReducedFraction maxNoteOffTime(const QList<MidiNote> &notes);
const ReducedFraction& minAllowedDuration();
ReducedFraction findMinDuration(const ReducedFraction &onTime,
                                const QList<MidiChord> &midiChords,
                                const ReducedFraction &length);
void sortNotesByPitch(std::multimap<ReducedFraction, MidiChord> &chords);
void collectChords(std::multimap<int, MTrack> &tracks);
void removeOverlappingNotes(std::multimap<int, MTrack> &tracks);
void mergeChordsWithEqualOnTimeAndVoice(std::multimap<int, MTrack> &tracks);
void splitUnequalChords(std::multimap<int, MTrack> &tracks);

#ifdef QT_DEBUG

bool areOnTimeValuesDifferent(const std::multimap<ReducedFraction, MidiChord> &chords);

#endif

} // namespace MChord
} // namespace Ms


#endif // IMPORTMIDI_CHORD_H
