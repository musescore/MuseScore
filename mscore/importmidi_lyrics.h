#ifndef IMPORTMIDI_LYRICS_H
#define IMPORTMIDI_LYRICS_H


namespace Ms {

class MidiFile;
class MTrack;

namespace MidiLyrics {

void setInitialIndexes(QList<MTrack> &tracks);
void extractLyricsToMidiData(const MidiFile *mf);
void setLyrics(const MidiFile *mf, const QList<MTrack> &tracks);
QStringList makeLyricsList();

} // namespace MidiLyrics
} // namespace Ms


#endif // IMPORTMIDI_LYRICS_H
