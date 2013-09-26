#ifndef IMPORTMIDI_LYRICS_H
#define IMPORTMIDI_LYRICS_H


namespace Ms {

class MidiFile;
class MTrack;

namespace MidiLyrics {

void setInitialIndexes(QList<MTrack> &tracks);
void setLyrics(const MidiFile *mf, const QList<MTrack> &tracks);
QList<std::string> makeLyricsList();

} // namespace MidiLyrics
} // namespace Ms


#endif // IMPORTMIDI_LYRICS_H
