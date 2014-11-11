#ifndef IMPORTMIDI_VOICE_H
#define IMPORTMIDI_VOICE_H

#include "importmidi_operation.h"


namespace Ms {

class MTrack;
class TimeSigMap;
class MidiChord;

namespace MidiTuplet {
struct TupletData;
}

namespace MidiVoice {

int toIntVoiceCount(MidiOperations::VoiceCount value);
int voiceLimit();
bool separateVoices(std::multimap<int, MTrack> &tracks, const TimeSigMap *sigmap);

bool splitChordToVoice(
      std::multimap<ReducedFraction, MidiChord>::iterator &chordIt,
      const QSet<int> &notesToMove,
      int newVoice,
      std::multimap<ReducedFraction, MidiChord> &chords,
      std::multimap<ReducedFraction, MidiTuplet::TupletData> &tuplets,
      std::multimap<ReducedFraction,
           std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator> &insertedTuplets,
      const ReducedFraction &maxChordLength,
      bool allowParallelTuplets = false);

#ifdef QT_DEBUG

bool areVoicesSame(const std::multimap<ReducedFraction, MidiChord> &chords);

#endif

} // namespace MidiVoice
} // namespace Ms


#endif // IMPORTMIDI_VOICE_H
