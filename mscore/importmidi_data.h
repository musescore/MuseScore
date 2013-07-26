#ifndef IMPORTMIDI_DATA_H
#define IMPORTMIDI_DATA_H

#include "midi/midifile.h"


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

   private:
      struct MidiDataStore
            {
            QByteArray HHeaderData;
            QByteArray VHeaderData;
            QList<TrackData> tracksData;
            int selectedRow = 0;
            MidiFile midiFile;
            };
      QMap<QString, MidiDataStore> data;    // <file name, tracks data>
      };

} // namespace Ms


#endif // IMPORTMIDI_DATA_H
