#ifndef IMPORTMIDI_TRMODEL_H
#define IMPORTMIDI_TRMODEL_H


namespace Ms {

struct TrackMeta;
struct TrackData;
struct TrackOperations;

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
      void setTrackOperations(int row, const TrackOperations &opers);
      TrackData trackData(int row) const;
      int trackCount() const;

      int rowCount(const QModelIndex &/*parent*/) const;
      int columnCount(const QModelIndex &/*parent*/) const;
      QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
      Qt::ItemFlags flags(const QModelIndex &index) const;
      bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
      QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
      std::vector<TrackData> tracksData_;
      int trackCount_;
      int colCount_;

      TrackData* itemFromIndex(const QModelIndex &index);
      bool isRowValid(int row) const;
      bool isColumnValid(int column) const;
      };

} // namespace Ms


#endif // IMPORTMIDI_TRMODEL_H
