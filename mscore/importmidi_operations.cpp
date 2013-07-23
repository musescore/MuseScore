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

} // namespace Ms

