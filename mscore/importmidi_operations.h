#ifndef IMPORTMIDI_OPERATIONS_H
#define IMPORTMIDI_OPERATIONS_H


namespace Ms {

// all enums below should have default indexes like 0, 1, 2...

struct Operation
      {
      enum Type {
            DO_IMPORT = 0,
            QUANTIZ_VALUE,

            DO_LHRH_SEPARATION,
            LHRH_METHOD,
            LHRH_SPLIT_OCTAVE,
            LHRH_SPLIT_NOTE,

            USE_DOTS
      } type;

      QVariant value;

      enum Quant {
            SHORTEST_IN_MEASURE = 0,
            FROM_PREFERENCES,
            N_4,
            N_4_triplet,
            N_8,
            N_8_triplet,
            N_16,
            N_16_triplet,
            N_32,
            N_32_triplet,
            N_64
            };

      enum LHRHMethod {
            HAND_WIDTH = 0,
            FIXED_PITCH
            };

      enum LHRHOctave {
            C_1 = 0,
            C0,
            C1,
            C2,
            C3,
            C4,
            C5,
            C6,
            C7,
            C8,
            C9
            };

      enum LHRHNote {
            C = 0,
            Cis,
            D,
            Dis,
            E,
            F,
            Fis,
            G,
            Gis,
            A,
            Ais,
            H
            };
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
      Operation::Quant quantize = Operation::SHORTEST_IN_MEASURE;
      bool useDots = false;
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
