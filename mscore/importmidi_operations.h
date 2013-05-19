#ifndef IMPORTMIDI_OPERATIONS_H
#define IMPORTMIDI_OPERATIONS_H

namespace Ms {

struct TrackMeta
      {
      QString trackName;
      QString instrumentName;
      };

struct TrackOperations
      {
      bool doImport = true;
      bool doLHRHSeparation = false;
      bool useDots = false;
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
#endif // IMPORTMIDI_OPERATIONS_H
