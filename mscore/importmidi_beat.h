#ifndef IMPORTMIDI_BEAT_H
#define IMPORTMIDI_BEAT_H

#include <set>


namespace Ms {

class TimeSigMap;
class ReducedFraction;
class MidiChord;
class MTrack;
class ReducedFraction;

namespace MidiBeat {

void findBeatLocations(
            const std::multimap<ReducedFraction, MidiChord> &allChords,
            TimeSigMap *sigmap,
            double ticksPerSec);

void adjustChordsToBeats(
            std::multimap<int, MTrack> &tracks,
            ReducedFraction &lastTick);

void setTimeSignature(TimeSigMap *sigmap);

} // namespace MidiBeat
} // namespace Ms


#endif // IMPORTMIDI_BEAT_H
