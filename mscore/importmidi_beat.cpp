#include "importmidi_beat.h"

#include "importmidi_chord.h"
#include "importmidi_fraction.h"


namespace Ms {
namespace MidiBeat {

double findChordSalience1(
            const std::pair<const ReducedFraction, MidiChord> &chord,
            double ticksPerSec)
      {
      ReducedFraction duration(0, 1);
      int pitch = std::numeric_limits<int>::max();
      int velocity = 0;

      for (const MidiNote &note: chord.second.notes) {
            if (note.offTime - chord.first > duration)
                  duration = note.offTime - chord.first;
            if (note.pitch < pitch)
                  pitch = note.pitch;
            velocity += note.velo;
            }
      const double durationInSeconds = duration.ticks() / ticksPerSec;

      const double c4 = 84;
      const int pmin = 48;
      const int pmax = 72;

      if (pitch < pmin)
            pitch = pmin;
      else if (pitch > pmax)
            pitch = pmax;

      if (velocity <= 0)
            velocity = 1;

      return durationInSeconds * (c4 - pitch) * std::log(velocity);
      }

double findChordSalience2(
            const std::pair<const ReducedFraction, MidiChord> &chord, double)
      {
      int velocity = 0;
      for (const MidiNote &note: chord.second.notes) {
            velocity += note.velo;
            }
      if (velocity <= 0)
            velocity = 1;

      return velocity;
      }

} // namespace MidiBeat
} // namespace Ms
