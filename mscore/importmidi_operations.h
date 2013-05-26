#ifndef IMPORTMIDI_OPERATIONS_H
#define IMPORTMIDI_OPERATIONS_H

#include "importmidi_operation.h"


namespace Ms {

struct Quantization
      {
      Operation::QuantValue value = Operation::FROM_PREFERENCES;
      bool reduceToShorterNotesInBar = true;
      };

struct LHRHSeparation
      {
      bool doIt = false;
      Operation::LHRHMethod method = Operation::HAND_WIDTH;
      Operation::LHRHOctave splitPitchOctave = Operation::C4;
      Operation::LHRHNote splitPitchNote = Operation::E;
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
      QSet<Operation::Type> undefinedOpers;
      TrackOperations opers;
      };

typedef QList<TrackOperations> tMidiImportOperations;

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
      tMidiImportOperations operations_;
      int currentTrack_ = -1;

      bool isValidIndex(int index) const;
      };

} // namespace Ms


Q_DECLARE_METATYPE(Ms::Operation)
Q_DECLARE_METATYPE(Ms::TrackData)


#endif // IMPORTMIDI_OPERATIONS_H
