#ifndef IMPORTMIDI_BEAT_H
#define IMPORTMIDI_BEAT_H


namespace Ms {

class TimeSigMap;
class ReducedFraction;
class MidiChord;
class MTrack;

namespace MidiBeat {

void findBeatLocations(
            const std::multimap<ReducedFraction, MidiChord> &allChords,
            const TimeSigMap *sigmap,
            double ticksPerSec);

void adjustChordsToBeats(
            std::multimap<int, MTrack> &tracks,
            ReducedFraction &lastTick);

} // namespace MidiBeat
} // namespace Ms


#endif // IMPORTMIDI_BEAT_H
