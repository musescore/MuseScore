#include "importmidi_data.h"
#include "importmidi_operations.h"


namespace Ms {

void MidiData::setFileData(const QString &fileName, const QList<TrackData> &fileData)
      {
      data[fileName] = fileData;
      }

void MidiData::excludeFile(const QString &fileName)
      {
      data.remove(fileName);
      }

QList<TrackData> MidiData::fileData(const QString &fileName) const
      {
      auto it = data.find(fileName);
      if (it == data.end())
            return QList<TrackData>();
      return it.value();
      }

} // namespace Ms
