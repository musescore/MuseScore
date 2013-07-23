#ifndef IMPORTMIDI_DATA_H
#define IMPORTMIDI_DATA_H


namespace Ms {

struct TrackData;

class MidiData
      {
   public:
      MidiData();
      ~MidiData();

      void setTracksData(const QString &fileName, const QList<TrackData> &tracksData);
      void setTableViewData(const QString &fileName, const QByteArray &tableViewData);
      void excludeFile(const QString &fileName);
      QList<TrackData> tracksData(const QString &fileName) const;
      QByteArray tableViewData(const QString &fileName) const;
      int selectedRow(const QString &fileName) const;
      void setSelectedRow(const QString &fileName, int row);

   private:
      struct MidiDataStore;
      QMap<QString, MidiDataStore> data;    // <file name, tracks data>
      };

} // namespace Ms


#endif // IMPORTMIDI_DATA_H
