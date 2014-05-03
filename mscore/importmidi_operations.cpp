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
      defaultOpers = TrackOperations();
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
            return defaultOpers;
      return operations_[currentTrack_];
      }

TrackOperations MidiImportOperations::trackOperations(int trackIndex) const
      {
      if (!isValidIndex(trackIndex))
            return defaultOpers;
      return operations_[trackIndex];
      }

QString MidiImportOperations::charset() const
      {
      return midiData_.charset(currentMidiFile_);
      }

void MidiImportOperations::adaptForPercussion(int trackIndex, bool isDrumTrack)
      {
                  // small hack: don't use multiple voices for tuplets in percussion tracks
      if (isValidIndex(trackIndex)) {
            if (isDrumTrack)
                  operations_[trackIndex].allowedVoices = MidiOperation::AllowedVoices::V_1;
            }
      else {
            defaultOpers.allowedVoices = isDrumTrack
                        ? MidiOperation::AllowedVoices::V_1 : TrackOperations().allowedVoices;
            }
      }

void MidiImportOperations::addTrackLyrics(
            const std::multimap<ReducedFraction, std::string> &trackLyrics)
      {
      midiData_.addTrackLyrics(currentMidiFile_, trackLyrics);
      }

const QList<std::multimap<ReducedFraction, std::string> >*
MidiImportOperations::getLyrics()
      {
      return midiData_.getLyrics(currentMidiFile_);
      }

bool MidiImportOperations::isHumanPerformance() const
      {
      return midiData_.isHumanPerformance(currentMidiFile_);
      }

void MidiImportOperations::setHumanPerformance(bool value)
      {
      midiData_.setHumanPerformance(currentMidiFile_, value);
      defaultOpers.quantize.humanPerformance = value;
      }

MidiOperation::QuantValue MidiImportOperations::quantValue() const
      {
      return midiData_.quantValue(currentMidiFile_);
      }

void MidiImportOperations::setQuantValue(MidiOperation::QuantValue value)
      {
      midiData_.setQuantValue(currentMidiFile_, value);
      defaultOpers.quantize.value = value;
      }

const std::set<ReducedFraction>*
MidiImportOperations::getHumanBeats() const
      {
      return midiData_.getHumanBeats(currentMidiFile_);
      }

void MidiImportOperations::setHumanBeats(const std::set<ReducedFraction> &humanBeats)
      {
      return midiData_.setHumanBeats(currentMidiFile_, humanBeats);
      }

} // namespace Ms

