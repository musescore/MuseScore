#include "importmidi_operations.h"


namespace Ms {

bool MidiImportOperations::isValidIndex(int index) const
      {
      return index >= 0 && index < operations_.size();
      }

void MidiImportOperations::appendTrackOperations(const TrackOperations &operations)
      {
      operations_.push_back(operations);
      if (operations_.size() == 1)
            currentTrack_ = 0;
      }

void MidiImportOperations::clear()
      {
      operations_.clear();
      currentTrack_ = -1;
      }

void MidiImportOperations::setCurrentTrack(int trackIndex)
      {
      if (!isValidIndex(trackIndex))
            return;
      currentTrack_ = trackIndex;
      }

void MidiImportOperations::setCurrentMidiFile(const QString &fileName)
      {
      currentMidiFile_ = fileName;
      }

TrackOperations MidiImportOperations::currentTrackOperations() const
      {
      if (!isValidIndex(currentTrack_))
            return TrackOperations();
      return operations_[currentTrack_];
      }

TrackOperations MidiImportOperations::trackOperations(int trackIndex) const
      {
      if (!isValidIndex(trackIndex))
            return TrackOperations();
      return operations_[trackIndex];
      }

QString MidiImportOperations::charset() const
      {
      return midiData_.charset(currentMidiFile_);
      }

void MidiImportOperations::adaptForPercussion(int trackIndex)
      {
                  // small hack: don't use multiple voices for tuplets
      if (isValidIndex(trackIndex))
            operations_[trackIndex].useMultipleVoices = false;
      }

void MidiImportOperations::addTrackLyrics(const std::multimap<ReducedFraction, std::string> &trackLyrics)
      {
      midiData_.addTrackLyrics(currentMidiFile_, trackLyrics);
      }

const QList<std::multimap<ReducedFraction, std::string>>*
MidiImportOperations::getLyrics()
      {
      return midiData_.getLyrics(currentMidiFile_);
      }

} // namespace Ms

