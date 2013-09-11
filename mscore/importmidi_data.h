#ifndef IMPORTMIDI_DATA_H
#define IMPORTMIDI_DATA_H

#include "midi/midifile.h"
#include "importmidi_fraction.h"


namespace Ms {

struct TrackData;

class MidiData
      {
   public:
      void setTracksData(const QString &fileName, const QList<TrackData> &tracksData);
      void saveHHeaderState(const QString &fileName, const QByteArray &headerData);
      void saveVHeaderState(const QString &fileName, const QByteArray &headerData);
      void excludeFile(const QString &fileName);
      QList<TrackData> tracksData(const QString &fileName) const;
      QByteArray HHeaderData(const QString &fileName) const;
      QByteArray VHeaderData(const QString &fileName) const;
      int selectedRow(const QString &fileName) const;
      void setSelectedRow(const QString &fileName, int row);
      void setMidiFile(const QString &fileName, const MidiFile &midiFile);
      const MidiFile *midiFile(const QString &fileName) const;
                  // lyrics
      void addTrackLyrics(const QString &fileName,
                          const std::multimap<ReducedFraction, QString> &trackLyrics);
      const QList<std::multimap<ReducedFraction, QString> >* getLyrics(const QString &fileName);

   private:
      struct MidiDataStore
            {
            QByteArray HHeaderData;
            QByteArray VHeaderData;
            QList<TrackData> tracksData;
                        // tracks of <tick, lyric fragment> from karaoke files
            QList<std::multimap<ReducedFraction, QString>> lyricTracks;
            int selectedRow = 0;
            MidiFile midiFile;
            };
      QMap<QString, MidiDataStore> data;    // <file name, tracks data>
      };

} // namespace Ms


#endif // IMPORTMIDI_DATA_H
