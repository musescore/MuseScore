#ifndef IMPORTMIDI_OPERATIONS_H
#define IMPORTMIDI_OPERATIONS_H

#include "importmidi_operation.h"
#include "importmidi_data.h"


namespace Ms {

// operation types are in importmidi_operation.h

// to add an operation one need to add code also to:
//   - importmidi_operation.h,
//   - importmidi_opmodel.cpp (2 places),
//   - importmidi_trmodel.cpp (2 places),
// and - other importmidi files where algorithm requires it

struct SearchTuplets
      {
      bool doSearch = true;
      bool duplets = false;
      bool triplets = true;
      bool quadruplets = true;
      bool quintuplets = true;
      bool septuplets = true;
      bool nonuplets = true;
      };

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

struct SplitDrums
      {
      bool doSplit = false;
      bool showStaffBracket = true;
      };

      // bool and enum-like elementary operations (itself and inside structs) are allowed
struct TrackOperations
      {
      int reorderedIndex = 0;
      bool doImport = true;
      Quantization quantize;
      bool useDots = true;
      LHRHSeparation LHRH;
      SearchTuplets tuplets;
      bool useMultipleVoices = true;
      bool changeClef = true;
      MidiOperation::Swing swing = MidiOperation::Swing::NONE;
      SplitDrums drums;
      bool pickupMeasure = true;
      int lyricTrackIndex = -1;     // empty lyric
      };

struct TrackMeta
      {
      std::string staffName;    // will be converted to unicode later
      QString instrumentName;
      bool isDrumTrack;
      int initLyricTrackIndex;
      };

struct TrackData
      {
      TrackMeta meta;
      TrackOperations opers;
      };

struct DefinedTrackOperations
      {
      QSet<int> undefinedOpers;
      bool isDrumTrack;
      bool allTracksSelected;
      TrackOperations opers;
      };

class ReducedFraction;

class MidiImportOperations
      {
   public:
      void appendTrackOperations(const TrackOperations& operations);
      void clear();
      void setCurrentTrack(int trackIndex);
      void setCurrentMidiFile(const QString &fileName);
      int currentTrack() const { return currentTrack_; }
      TrackOperations currentTrackOperations() const;
      TrackOperations trackOperations(int trackIndex) const;
      int count() const { return operations_.size(); }
      MidiData& midiData() { return midiData_; }
      QString charset() const;
      void adaptForPercussion(int trackIndex);
                  // lyrics
      void addTrackLyrics(const std::multimap<ReducedFraction, std::string> &trackLyrics);
      const QList<std::multimap<ReducedFraction, std::string> > *getLyrics();

   private:
      QList<TrackOperations> operations_;
      int currentTrack_ = -1;
      QString currentMidiFile_;
      MidiData midiData_;

      bool isValidIndex(int index) const;
      };

} // namespace Ms


Q_DECLARE_METATYPE(Ms::MidiOperation)
Q_DECLARE_METATYPE(Ms::TrackData)


#endif // IMPORTMIDI_OPERATIONS_H
