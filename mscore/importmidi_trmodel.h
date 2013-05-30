#ifndef IMPORTMIDI_TRMODEL_H
#define IMPORTMIDI_TRMODEL_H

#include "importmidi_operation.h"


namespace Ms {

struct TrackMeta;
struct TrackData;
struct TrackOperations;
struct DefinedTrackOperations;

struct TrackCol {
      enum {
            TRACK_NUMBER = 0,
            DO_IMPORT,
            TRACK_NAME,
            INSTRUMENT,
            COL_COUNT
            };
      };

class TracksModel : public QAbstractTableModel
      {
   public:
      TracksModel();

      void reset(const QList<TrackMeta> &tracksMeta);
      void setOperation(int row, MidiOperation::Type operType, const QVariant &operValue);
      TrackData trackData(int trackIndex) const;
      DefinedTrackOperations trackOperations(int row) const;
      int trackCount() const;

      int rowCount(const QModelIndex &/*parent*/) const;
      int columnCount(const QModelIndex &/*parent*/) const;
      QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
      Qt::ItemFlags flags(const QModelIndex &index) const;
      QVariant headerData(int section, Qt::Orientation orientation, int role) const;
      bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

   private:
      std::vector<TrackData> tracksData_;
      int trackCount_;
      int colCount_;

      void setTrackOperation(int trackIndex, MidiOperation::Type operType, const QVariant &operValue);
      void setOperationForAllTracks(MidiOperation::Type operType, const QVariant &operValue);
      Qt::CheckState areAllTracksForImport() const;
      int rowFromTrackIndex(int trackIndex) const;
      int trackIndexFromRow(int row) const;
      TrackData* trackDataFromIndex(const QModelIndex &index);
      bool isMappingRowToTrackValid(int row) const;
      bool isColumnValid(int column) const;
      bool isTrackIndexValid(int trackIndex) const;
      };

} // namespace Ms


#endif // IMPORTMIDI_TRMODEL_H
