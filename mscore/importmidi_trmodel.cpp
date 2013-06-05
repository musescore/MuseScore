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

void TracksModel::setOperation(int row, MidiOperation::Type operType, const QVariant &operValue)
      {
      int trackIndex = trackIndexFromRow(row);
      if (trackIndex == -1)
            setOperationForAllTracks(operType, operValue);
      else if (isTrackIndexValid(trackIndex))
            setTrackOperation(trackIndex, operType, operValue);
      }

void TracksModel::setOperationForAllTracks(MidiOperation::Type operType,
                                           const QVariant &operValue)
      {
      for (int i = 0; i != trackCount_; ++i)
            setTrackOperation(i, operType, operValue);
      }

void TracksModel::setTrackOperation(int trackIndex, MidiOperation::Type operType,
                                    const QVariant &operValue)
      {
      if (!operValue.isValid())
            return;
      TrackData &trackData = tracksData_[trackIndex];

      switch (operType) {
            case MidiOperation::Type::QUANT_VALUE:
                  trackData.opers.quantize.value = (MidiOperation::QuantValue)operValue.toInt();
                  break;
            case MidiOperation::Type::QUANT_REDUCE:
                  trackData.opers.quantize.reduceToShorterNotesInBar = operValue.toBool();
                  break;
            case MidiOperation::Type::QUANT_HUMAN:
                  trackData.opers.quantize.humanPerformance = operValue.toBool();
                  break;
            case MidiOperation::Type::DO_LHRH_SEPARATION:
                  trackData.opers.LHRH.doIt = operValue.toBool();
                  break;
            case MidiOperation::Type::LHRH_METHOD:
                  trackData.opers.LHRH.method = (MidiOperation::LHRHMethod)operValue.toInt();
                  break;
            case MidiOperation::Type::LHRH_SPLIT_OCTAVE:
                  trackData.opers.LHRH.splitPitchOctave
                              = (MidiOperation::Octave)operValue.toInt();
                  break;
            case MidiOperation::Type::LHRH_SPLIT_NOTE:
                  trackData.opers.LHRH.splitPitchNote = (MidiOperation::Note)operValue.toInt();
                  break;
            case MidiOperation::Type::USE_DOTS:
                  trackData.opers.useDots = operValue.toBool();
                  break;
            case MidiOperation::Type::DO_IMPORT:
                  break;
            }
      }

DefinedTrackOperations TracksModel::trackOperations(int row) const
      {
      DefinedTrackOperations opers;
      int trackIndex = trackIndexFromRow(row);

      if (trackIndex == -1) {
            // all tracks row case
            // find tracks that operation values are different
            // and mark them as undefined

            opers.opers = tracksData_.front().opers;

            // MidiOperation::Type::QUANT_VALUE
            for (int i = 1; i != trackCount_; ++i) {
                  if (tracksData_[i].opers.quantize.value != opers.opers.quantize.value) {
                        opers.undefinedOpers.insert((int)MidiOperation::Type::QUANT_VALUE);
                        break;
                        }
                  }

            // MidiOperation::Type::QUANT_REDUCE
            for (int i = 1; i != trackCount_; ++i) {
                  if (tracksData_[i].opers.quantize.reduceToShorterNotesInBar
                              != opers.opers.quantize.reduceToShorterNotesInBar) {
                        opers.undefinedOpers.insert((int)MidiOperation::Type::QUANT_REDUCE);
                        break;
                        }
                  }

            // MidiOperation::Type::QUANT_HUMAN
            for (int i = 1; i != trackCount_; ++i) {
                  if (tracksData_[i].opers.quantize.humanPerformance
                              != opers.opers.quantize.humanPerformance) {
                        opers.undefinedOpers.insert((int)MidiOperation::Type::QUANT_HUMAN);
                        break;
                        }
                  }

            // MidiOperation::Type::DO_LHRH_SEPARATION
            for (int i = 1; i != trackCount_; ++i) {
                  if (tracksData_[i].opers.LHRH.doIt != opers.opers.LHRH.doIt) {
                        opers.undefinedOpers.insert((int)MidiOperation::Type::DO_LHRH_SEPARATION);
                        break;
                        }
                  }

            // MidiOperation::Type::LHRH_METHOD
            for (int i = 1; i != trackCount_; ++i) {
                  if (tracksData_[i].opers.LHRH.method != opers.opers.LHRH.method) {
                        opers.undefinedOpers.insert((int)MidiOperation::Type::LHRH_METHOD);
                        break;
                        }
                  }

            // MidiOperation::Type::LHRH_SPLIT_OCTAVE
            for (int i = 1; i != trackCount_; ++i) {
                  if (tracksData_[i].opers.LHRH.splitPitchOctave
                              != opers.opers.LHRH.splitPitchOctave) {
                        opers.undefinedOpers.insert((int)MidiOperation::Type::LHRH_SPLIT_OCTAVE);
                        break;
                        }
                  }

            // MidiOperation::Type::LHRH_SPLIT_NOTE
            for (int i = 1; i != trackCount_; ++i) {
                  if (tracksData_[i].opers.LHRH.splitPitchNote
                              != opers.opers.LHRH.splitPitchNote) {
                        opers.undefinedOpers.insert((int)MidiOperation::Type::LHRH_SPLIT_NOTE);
                        break;
                        }
                  }

            // MidiOperation::Type::USE_DOTS
            for (int i = 1; i != trackCount_; ++i) {
                  if (tracksData_[i].opers.useDots != opers.opers.useDots) {
                        opers.undefinedOpers.insert((int)MidiOperation::Type::USE_DOTS);
                        break;
                        }
                  }
            }
      else
            opers.opers = tracksData_[trackIndex].opers;

      return opers;
      }

int TracksModel::rowFromTrackIndex(int trackIndex) const
      {
      // first row reserved for all tracks if track count > 1
      return (trackCount_ > 1) ? trackIndex + 1 : trackIndex;
      }

int TracksModel::trackIndexFromRow(int row) const
      {
      // first row reserved for all tracks if track count > 1
      // return -1 if row is all tracks row
      return (trackCount_ > 1) ? row - 1 : row;
      }

int TracksModel::rowCount(const QModelIndex &/*parent*/) const
      {
      return (trackCount_ > 1) ? trackCount_ + 1 : trackCount_;
      }

int TracksModel::columnCount(const QModelIndex &/*parent*/) const
      {
      return colCount_;
      }

Qt::CheckState TracksModel::areAllTracksForImport() const
      {
      if (trackCount_ == 0)
            return Qt::Unchecked;
      bool firstTrackImport = tracksData_[0].opers.doImport;
      for (int i = 1; i != trackCount_; ++i) {
            if (tracksData_[i].opers.doImport != firstTrackImport)
                  return Qt::PartiallyChecked;
            }
      return (firstTrackImport) ? Qt::Checked : Qt::Unchecked;
      }

QVariant TracksModel::data(const QModelIndex &index, int role) const
      {
      if (!index.isValid())
            return QVariant();

      int trackIndex = trackIndexFromRow(index.row());
      switch (role) {
            case Qt::DisplayRole:
                  switch (index.column()) {
                        case TrackCol::TRACK_NUMBER:
                              if (trackIndex == -1)
                                    return "All";
                              return trackIndex + 1;
                        case TrackCol::TRACK_NAME:
                              if (trackIndex == -1)
                                    return "";
                              return tracksData_[trackIndex].meta.trackName;
                        case TrackCol::INSTRUMENT:
                              if (trackIndex == -1)
                                    return "";
                              return tracksData_[trackIndex].meta.instrumentName;
                        default:
                              break;
                        }
                  break;
            case Qt::CheckStateRole:
                  switch (index.column()) {
                        case TrackCol::DO_IMPORT:
                              if (trackIndex == -1)
                                    return areAllTracksForImport();
                              return (tracksData_[trackIndex].opers.doImport)
                                          ? Qt::Checked : Qt::Unchecked;
                        default:
                              break;
                        }
                  break;
            case Qt::TextAlignmentRole:
                  if (trackIndex == -1)
                        break;
                  switch (index.column()) {
                        case TrackCol::TRACK_NAME:
                        case TrackCol::INSTRUMENT:
                              return Qt::AlignCenter;
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
      bool result = false;
      int trackIndex = trackIndexFromRow(index.row());

      if (trackIndex == -1) { // all tracks row
            if (index.column() == TrackCol::DO_IMPORT && role == Qt::CheckStateRole) {
                  for (auto &trackData: tracksData_)
                        trackData.opers.doImport = value.toBool();
                  result = true;
                  }
            if (result) {
                  // update checkboxes of all tracks
                  // because we've changed option for all tracks simultaneously
                  auto begIndex = this->index(0, TrackCol::DO_IMPORT);
                  auto endIndex = this->index(rowCount(QModelIndex()), TrackCol::DO_IMPORT);
                  emit dataChanged(begIndex, endIndex);
                  }
            }
      else {
            TrackData *trackData = trackDataFromIndex(index);
            if (!trackData)
                  return false;
            if (index.column() == TrackCol::DO_IMPORT && role == Qt::CheckStateRole) {
                  trackData->opers.doImport = value.toBool();
                  result = true;
                  }
            if (result) {
                  // update checkbox of current track row
                  emit dataChanged(index, index);
                  // update checkbox of all tracks row
                  auto allIndex = this->index(0, TrackCol::DO_IMPORT);
                  emit dataChanged(allIndex, allIndex);
                  }
            }
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

TrackData TracksModel::trackData(int trackIndex) const
      {
      if (isTrackIndexValid(trackIndex))
            return tracksData_[trackIndex];
      return TrackData();
      }

int TracksModel::trackCount() const
      {
      return trackCount_;
      }

TrackData* TracksModel::trackDataFromIndex(const QModelIndex &index)
      {
      if (index.isValid()) {
            if (!isMappingRowToTrackValid(index.row()) || !isColumnValid(index.column()))
                  return nullptr;
            return &tracksData_[trackIndexFromRow(index.row())];
            }
      return nullptr;
      }

bool TracksModel::isMappingRowToTrackValid(int row) const
      {
      if (trackCount_ > 1) // first row is reserved for all tracks
            return (row > 0 && row <= trackCount_);
      return row >= 0 && row < trackCount_;
      }

bool TracksModel::isColumnValid(int column) const
      {
      return (column >= 0 && column < colCount_);
      }

bool TracksModel::isTrackIndexValid(int trackIndex) const
      {
      return trackIndex >= 0 && trackIndex < trackCount_;
      }

} // namespace Ms
