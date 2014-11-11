#ifndef IMPORTMIDI_LYRICS_H
#define IMPORTMIDI_LYRICS_H


namespace Ms {

class MidiFile;
class MTrack;

namespace MidiLyrics {

void extractLyricsToMidiData(const MidiFile *mf);
void setLyricsToScore(QList<MTrack> &tracks);
QList<std::string> makeLyricsListForUI();

} // namespace MidiLyrics
} // namespace Ms


#endif // IMPORTMIDI_LYRICS_H
