#ifndef IMPORTMIDI_LYRICS_H
#define IMPORTMIDI_LYRICS_H


namespace Ms {

class MidiFile;
class MTrack;

namespace MidiLyrics {

void extractLyrics(const QList<MTrack> &tracks, const MidiFile *mf);

} // namespace MidiLyrics
} // namespace Ms


#endif // IMPORTMIDI_LYRICS_H
