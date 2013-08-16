#include "importmidi_chord.h"


namespace Ms {

ReducedFraction maxNoteLen(const QList<MidiNote> &notes)
      {
      ReducedFraction maxLen;
      for (const auto &note: notes) {
            if (note.len > maxLen)
                  maxLen = note.len;
            }
      return maxLen;
      }

int findAveragePitch(const QList<MidiNote> &notes)
      {
      int avgPitch = 0;
      for (const auto &note: notes)
            avgPitch += note.pitch;
      return avgPitch / notes.size();
      }

} // namespace Ms
