#ifndef IMPORTMIDI_LYRICS_H
#define IMPORTMIDI_LYRICS_H


namespace Ms {

class MidiFile;
class MTrack;

namespace MidiLyrics {

void assignLyricsToTracks(QList<MTrack> &tracks);
void setLyricsToScore(const MidiFile *mf, const QList<MTrack> &tracks);
QList<std::string> makeLyricsListForUI();

} // namespace MidiLyrics
} // namespace Ms


#endif // IMPORTMIDI_LYRICS_H
