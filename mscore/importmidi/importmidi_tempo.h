#ifndef IMPORTMIDI_TEMPO_H
#define IMPORTMIDI_TEMPO_H


namespace Ms {

class MTrack;
class Score;
class ReducedFraction;

namespace MidiTempo {

ReducedFraction time2Tick(double time, double ticksPerSec);
double findBasicTempo(const std::multimap<int, MTrack> &tracks, bool isHumanPerformance);
void setTempo(const std::multimap<int, MTrack> &tracks, Score *score);

} // namespace MidiTempo
} // namespace Ms


#endif // IMPORTMIDI_TEMPO_H
