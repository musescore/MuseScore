#ifndef IMPORTMIDI_CHORDNAME_H
#define IMPORTMIDI_CHORDNAME_H


namespace Ms {

class MTrack;

namespace MidiChordName {

void findChordNames(const std::multimap<int, MTrack> &tracks);
void setChordNames(QList<MTrack> &tracks);

} // namespace MidiChordName
} // namespace Ms


#endif // IMPORTMIDI_CHORDNAME_H
