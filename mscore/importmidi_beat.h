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

int beatsInBar(const ReducedFraction &barFraction);

void findBeatLocations(
            const std::multimap<ReducedFraction, MidiChord> &allChords,
            TimeSigMap *sigmap,
            double ticksPerSec);

void adjustChordsToBeats(
            std::multimap<int, MTrack> &tracks,
            ReducedFraction &lastTick);

void setTimeSignature(TimeSigMap *sigmap);

void addFirstBeats(
            std::set<ReducedFraction> &beatSet,
            const ReducedFraction &firstTick,
            int beatsInBar,
            int &addedBeatCount);

void addLastBeats(std::set<ReducedFraction> &beatSet,
            const ReducedFraction &lastTick,
            int beatsInBar,
            int &addedBeatCount);

} // namespace MidiBeat
} // namespace Ms


#endif // IMPORTMIDI_BEAT_H
