#ifndef IMPORTMIDI_DRUM_H
#define IMPORTMIDI_DRUM_H


namespace Ms {

class MTrack;

namespace MidiDrum {

void splitDrumVoices(std::multimap<int, MTrack> &tracks);
void splitDrumTracks(std::multimap<int, MTrack> &tracks);
void setStaffBracketForDrums(QList<MTrack> &tracks);

} // namespace MidiDrum
} // namespace Ms


#endif // IMPORTMIDI_DRUM_H
