#include "importmidi_operations.h"


namespace Ms {
namespace MidiOperations {

FileData* Data::data()
      {
      const auto it = _data.find(_currentMidiFile);
      if (it != _data.end())
            return &it->second;
      return nullptr;
      }

const FileData* Data::data() const
      {
      const auto it = _data.find(_currentMidiFile);
      if (it != _data.end())
            return &it->second;
      return nullptr;
      }

void Data::addNewFile(const QString &fileName)
      {
      _data.insert({fileName, FileData()});
      }

const MidiFile* Data::midiFile(const QString &fileName)
      {
      const auto it = _data.find(fileName);
      if (it != _data.end())
            return &it->second.midiFile;
      return nullptr;
      }

QStringList Data::allMidiFiles() const
      {
      QStringList list;
      for (const auto &d: _data)
            list.append(d.first);
      return list;
      }

void Data::setMidiFileData(const QString &fileName, const MidiFile &midiFile)
      {
      _data[fileName].midiFile = midiFile;
      }

void Data::excludeFile(const QString &fileName)
      {
      _data.erase(fileName);
      }

bool Data::hasFile(const QString &fileName)
      {
      return _data.find(fileName) != _data.end();
      }

int Data::currentTrack() const
      {

      Q_ASSERT_X(_currentTrack >= 0,
                 "Data::currentTrack", "Invalid current track index");

      return _currentTrack;
      }

} // namespace MidiOperations
} // namespace Ms

