#include "importmidi_trmodel.h"
#include "importmidi_operations.h"


namespace Ms {

TracksModel::TracksModel()
      : trackCount_(0)
      , colCount_(TrackCol::COL_COUNT)
      {
      }

void TracksModel::reset(const QList<TrackMeta> &tracksMeta)
      {
      beginResetModel();
      trackCount_ = tracksMeta.size();
      tracksData_.clear();
      for (const auto &meta: tracksMeta) {
            QString trackName = meta.trackName.isEmpty()
                        ? "-" : meta.trackName;
            QString instrumentName = meta.instrumentName.isEmpty()
                        ? "-" : meta.instrumentName;
            tracksData_.push_back({
                                   {trackName, instrumentName},
                                   TrackOperations() // initialized by default values - see ctor
                                   });
            }
      endResetModel();
      }

void TracksModel::setTrackOperations(int row, const TrackOperations &opers)
      {
      if (isRowValid(row))
            tracksData_[row].opers = opers;
      }

int TracksModel::rowCount(const QModelIndex &/*parent*/) const
      {
      return trackCount_;
      }

int TracksModel::columnCount(const QModelIndex &/*parent*/) const
      {
      return colCount_;
      }

QVariant TracksModel::data(const QModelIndex &index, int role) const
      {
      if (!index.isValid())
            return QVariant();
      switch (role) {
            case Qt::DisplayRole:
                  switch (index.column()) {
                        case TrackCol::TRACK_NUMBER:
                              return index.row() + 1;
                        case TrackCol::TRACK_NAME:
                              return tracksData_[index.row()].meta.trackName;
                        case TrackCol::INSTRUMENT:
                              return tracksData_[index.row()].meta.instrumentName;
                        default:
                              break;
                        }
                  break;
            case Qt::CheckStateRole:
                  switch (index.column()) {
                        case TrackCol::DO_IMPORT:
                              return (tracksData_[index.row()].opers.doImport)
                                          ? Qt::Checked : Qt::Unchecked;
                        default:
                              break;
                        }
                  break;
            case Qt::TextAlignmentRole:
                  switch (index.column()) {
                        case TrackCol::TRACK_NAME:
                              if (tracksData_[index.row()].meta.trackName == "-")
                                    return Qt::AlignHCenter;
                        case TrackCol::INSTRUMENT:
                              if (tracksData_[index.row()].meta.instrumentName == "-")
                                    return Qt::AlignHCenter;
                        default:
                              break;
                        }
                  break;
            default:
                  break;
            }
      return QVariant();
      }

Qt::ItemFlags TracksModel::flags(const QModelIndex &index) const
      {
      if (!index.isValid())
            return 0;
      Qt::ItemFlags flags = Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      if (index.column() == TrackCol::DO_IMPORT)
            flags |= Qt::ItemIsUserCheckable;
      return flags;
      }

bool TracksModel::setData(const QModelIndex &index, const QVariant &value, int role)
      {
      TrackData *trackData = itemFromIndex(index);
      if (!trackData)
            return false;
      bool result = false;
      if (index.column() == TrackCol::DO_IMPORT && role == Qt::CheckStateRole) {
            trackData->opers.doImport = value.toBool();
            result = true;
            }
      if (result)
            emit dataChanged(index, index);
      return result;
      }

QVariant TracksModel::headerData(int section, Qt::Orientation orientation, int role) const
      {
      if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
                  case TrackCol::TRACK_NUMBER:
                        return "Track";
                  case TrackCol::DO_IMPORT:
                        return "Import?";
                  case TrackCol::TRACK_NAME:
                        return "Name";
                  case TrackCol::INSTRUMENT:
                        return "Instrument";
                  default:
                        break;
                  }
            }
      return QVariant();
      }

TrackData TracksModel::trackData(int row) const
      {
      if (isRowValid(row))
            return tracksData_[row];
      return TrackData();
      }

int TracksModel::trackCount() const
      {
      return trackCount_;
      }

TrackData* TracksModel::itemFromIndex(const QModelIndex &index)
      {
      if (index.isValid()) {
            if (!isRowValid(index.row()) || !isColumnValid(index.column()))
                  return nullptr;
            return &tracksData_[index.row()];
            }
      return nullptr;
      }

bool TracksModel::isRowValid(int row) const
      {
      return (row >= 0 && row < trackCount_);
      }

bool TracksModel::isColumnValid(int column) const
      {
      return (column >= 0 && column < colCount_);
      }


} // namespace Ms
