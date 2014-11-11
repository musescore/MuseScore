#ifndef IMPORTMIDI_LRHAND_H
#define IMPORTMIDI_LRHAND_H

namespace Ms {

class MTrack;
class MidiChord;
class ReducedFraction;

namespace LRHand {

bool needToSplit(const std::multimap<ReducedFraction, MidiChord> &chords, int midiProgram);
void splitIntoLeftRightHands(std::multimap<int, MTrack> &tracks);

} // namespace LRHand
} // namespace Ms


#endif // IMPORTMIDI_LRHAND_H
