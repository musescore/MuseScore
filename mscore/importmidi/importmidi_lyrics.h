#ifndef IMPORTMIDI_LYRICS_H
#define IMPORTMIDI_LYRICS_H


namespace Ms {

class MidiFile;
class MTrack;

namespace MidiLyrics {

void extractLyricsToMidiData(const MidiFile *mf);
void setStaffNames(const std::multimap<int, MTrack> &tracks);
void extractOtherMetaTextForCharset(const std::multimap<int, MTrack> &tracks);
void setLyricsToScore(QList<MTrack> &tracks);
QList<std::string> makeLyricsListForUI(size_t symbolLimit = 16);

} // namespace MidiLyrics
} // namespace Ms


#endif // IMPORTMIDI_LYRICS_H
