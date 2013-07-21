#ifndef IMPORTMIDI_DATA_H
#define IMPORTMIDI_DATA_H


namespace Ms {

struct TrackData;

class MidiData
      {
   public:
      void setFileData(const QString &fileName, const QList<TrackData> &fileData);
      void excludeFile(const QString &fileName);
      QList<TrackData> fileData(const QString &fileName) const;

   private:
      QMap<QString, QList<TrackData>> data;    // <file name, tracks data>
      };

} // namespace Ms


#endif // IMPORTMIDI_DATA_H
