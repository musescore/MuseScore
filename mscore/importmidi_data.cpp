#include "importmidi_data.h"
#include "importmidi_operations.h"


namespace Ms {

void MidiData::setTracksData(const QString &fileName, const QList<TrackData> &tracksData)
      {
      data[fileName].tracksData = tracksData;
      }

void MidiData::setTableViewData(const QString &fileName, const QByteArray &tableViewData)
      {
      data[fileName].tableViewData = tableViewData;
      }

void MidiData::excludeFile(const QString &fileName)
      {
      data.remove(fileName);
      }

QList<TrackData> MidiData::tracksData(const QString &fileName) const
      {
      auto it = data.find(fileName);
      if (it == data.end())
            return QList<TrackData>();
      return it.value().tracksData;
      }

QByteArray MidiData::tableViewData(const QString &fileName) const
      {
      auto it = data.find(fileName);
      if (it == data.end())
            return QByteArray();
      return it.value().tableViewData;
      }

int MidiData::selectedRow(const QString &fileName) const
      {
      auto it = data.find(fileName);
      if (it == data.end())
            return 0;
      return it.value().selectedRow;
      }

void MidiData::setSelectedRow(const QString &fileName, int row)
      {
      data[fileName].selectedRow = row;
      }

void MidiData::setMidiFile(const QString &fileName, const MidiFile &midiFile)
      {
      data[fileName].midiFile = midiFile;
      }

const MidiFile* MidiData::midiFile(const QString &fileName) const
      {
      auto it = data.find(fileName);
      if (it == data.end())
            return nullptr;
      return &(it.value().midiFile);
      }

} // namespace Ms
