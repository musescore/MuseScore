#ifndef IMPORTMIDI_OPERATIONS_H
#define IMPORTMIDI_OPERATIONS_H

#include "importmidi_operation.h"


namespace Ms {

struct Quantization
      {
      MidiOperation::QuantValue value = MidiOperation::QuantValue::FROM_PREFERENCES;
      bool reduceToShorterNotesInBar = true;
      bool humanPerformance = false;
      };

struct LHRHSeparation
      {
      bool doIt = false;
      MidiOperation::LHRHMethod method = MidiOperation::LHRHMethod::HAND_WIDTH;
      MidiOperation::Octave splitPitchOctave = MidiOperation::Octave::C4;
      MidiOperation::Note splitPitchNote = MidiOperation::Note::E;
      };

// bool and enum-like elementary operations (itself and inside structs) allowed
struct TrackOperations
      {
      bool doImport = true;
      Quantization quantize;
      bool useDots = true;
      LHRHSeparation LHRH;
      };

struct TrackMeta
      {
      QString trackName;
      QString instrumentName;
      };

struct TrackData
      {
      TrackMeta meta;
      TrackOperations opers;
      };

struct DefinedTrackOperations
      {
      QSet<int> undefinedOpers;
      TrackOperations opers;
      };

class MidiImportOperations
      {
   public:
      void appendTrackOperations(const TrackOperations& operations);
      void duplicateTrackOperations(int trackIndex);
      void eraseTrackOperations(int trackIndex);
      void clear();
      void setCurrentTrack(int trackIndex);
      int currentTrack() const { return currentTrack_; }
      TrackOperations currentTrackOperations() const;
      TrackOperations trackOperations(int trackIndex) const;

   private:
      QList<TrackOperations> operations_;
      int currentTrack_ = -1;

      bool isValidIndex(int index) const;
      };

} // namespace Ms


Q_DECLARE_METATYPE(Ms::MidiOperation)
Q_DECLARE_METATYPE(Ms::TrackData)


#endif // IMPORTMIDI_OPERATIONS_H
