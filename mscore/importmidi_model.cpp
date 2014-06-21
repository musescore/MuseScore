#include "importmidi_model.h"


//struct TrackCol {
//      enum {
//            DO_IMPORT = 0,
//            STAFF_NAME,
//            INSTRUMENT,
//            LYRICS,
//            QUANTIZATION,
//            HUMAN,
//            VOICE_COUNT,
//            TUPLETS,
//            SPLIT_STAFF,
//            CHANGE_CLEF,
//            SIMPLIFY,
//            STACCATO,
//            USE_DOTS,
//            TIME_SIG,
//            PICKUP_BAR,
//            SWING,
//            COL_COUNT
//            };
//      };

namespace Ms {

class TracksModel::Column
      {
   public:
      Column(MidiOperations::Opers &opers)
            : _opers(opers), _isEditable(true), _hasCharset(false)
            {}
      virtual ~Column() {}

      virtual QVariant value(int trackIndex) const = 0;
      virtual void setValue(const QVariant &value, int trackIndex) = 0;
      virtual QString headerName() const = 0;
      virtual QStringList valueList() const { return _values; }
      bool isEditable() const { return _isEditable; }
      bool hasCharset() const { return _hasCharset; }

   protected:
      MidiOperations::Opers &_opers;
      QStringList _values;
      bool _isEditable;
      bool _hasCharset;
      };


TracksModel::TracksModel()
      : _trackCount(0)
      , _frozenColCount(0)
      {
      }

TracksModel::~TracksModel()
      {
      }

void TracksModel::reset(const MidiOperations::Opers &opers,
                        const QList<std::string> &lyricsList,
                        int trackCount)
      {
      _trackOpers = opers;
      _columns.clear();
      _trackCount = trackCount;
      _frozenColCount = 0;
      if (trackCount == 0)
            return;

      //-----------------------------------------------------------------------
      struct Import : Column {
            Import(MidiOperations::Opers &opers) : Column(opers) {}

            QString headerName() const { return "Import"; }
            QVariant value(int trackIndex) const
                  {
                  return _opers.doImport.value(trackIndex);
                  }
            void setValue(const QVariant &value, int trackIndex)
                  {
                  _opers.doImport.setValue(trackIndex, value.toBool());
                  }
            };
      ++_frozenColCount;
      _columns.push_back(std::unique_ptr<Column>(new Import(_trackOpers)));

      //-----------------------------------------------------------------------
      struct InstrumentName : Column {
            InstrumentName(MidiOperations::Opers &opers) : Column(opers)
                  {
                  _isEditable = false;
                  }
            QString headerName() const { return "Sound"; }
            QVariant value(int trackIndex) const
                  {
                  return _opers.instrumentName.value(trackIndex);
                  }
            void setValue(const QVariant &/*value*/, int /*trackIndex*/) {}
            };
      ++_frozenColCount;
      _columns.push_back(std::unique_ptr<Column>(new InstrumentName(_trackOpers)));

      //-----------------------------------------------------------------------
      bool hasStaffName = false;
      for (int i = 0; i != _trackCount; ++i) {
            if (_trackOpers.staffName.value(i) != "") {
                  hasStaffName = true;
                  break;
                  }
            }
      if (hasStaffName) {
            struct StaffName : Column {
                  StaffName(MidiOperations::Opers &opers) : Column(opers)
                        {
                        _isEditable = false;
                        _hasCharset = true;
                        }
                  QString headerName() const { return "Staff name"; }
                  QVariant value(int trackIndex) const
                        {
                        return MidiCharset::convertToCharset(_opers.staffName.value(trackIndex));
                        }
                  void setValue(const QVariant &/*value*/, int /*trackIndex*/) {}
                  };
            ++_frozenColCount;
            _columns.push_back(std::unique_ptr<Column>(new StaffName(_trackOpers)));
            }

      //-----------------------------------------------------------------------
      if (!lyricsList.isEmpty()) {
            struct Lyrics : Column {
                  Lyrics(MidiOperations::Opers &opers, const QList<std::string> &lyricsList)
                        : Column(opers), _lyricsList(lyricsList)
                        {
                        _hasCharset = true;
                        }
                  QString headerName() const { return "Lyrics"; }
                  QVariant value(int trackIndex) const
                        {
                        int index = _opers.lyricTrackIndex.value(trackIndex);
                        if (index >= 0)
                              return MidiCharset::convertToCharset(_lyricsList[index]);
                        return "";
                        }
                  void setValue(const QVariant &value, int trackIndex)
                        {
                        _opers.lyricTrackIndex.setValue(trackIndex, value.toInt());
                        }
                  QStringList valueList() const
                        {
                        auto list = QStringList("");
                        for (const auto &lyric: _lyricsList)
                              list.append(MidiCharset::convertToCharset(lyric));
                        return list;
                        }
               private:
                  QList<std::string> _lyricsList;
                  };
            _columns.push_back(std::unique_ptr<Column>(new Lyrics(_trackOpers, lyricsList)));
            }

      //-----------------------------------------------------------------------
      struct QuantValue : Column {
            QuantValue(MidiOperations::Opers &opers) : Column(opers)
                  {
                  _values.push_back("Default");
                  _values.push_back("Quarter");
                  _values.push_back("Eighth");
                  _values.push_back("16th");
                  _values.push_back("32nd");
                  _values.push_back("64th");
                  _values.push_back("128th");
                  }
            QString headerName() const { return "Quantization"; }
            QVariant value(int trackIndex) const
                  {
                  return _values[(int)_opers.quantValue.value(trackIndex)];
                  }
            void setValue(const QVariant &value, int trackIndex)
                  {
                  _opers.quantValue.setValue(trackIndex, (MidiOperations::QuantValue)value.toInt());
                  }
            };
      _columns.push_back(std::unique_ptr<Column>(new QuantValue(_trackOpers)));
      }

void TracksModel::clear()
      {
      beginResetModel();
      _trackCount = 0;
      _trackOpers = MidiOperations::Opers();
      _columns.clear();
      endResetModel();
      }

const MidiOperations::Opers& TracksModel::trackOpers() const
      {
      return _trackOpers;
      }

void TracksModel::setTrackShuffleIndex(int trackIndex, int newIndex)
      {
      if (!isTrackIndexValid(trackIndex))
            return;
      _trackOpers.trackIndexAfterShuffle.setValue(trackIndex, newIndex);
      }

void TracksModel::updateCharset()
      {
      for (int i = 0; i != (int)_columns.size(); ++i) {
            if (_columns[i]->hasCharset())
                  forceColumnDataChanged(i);
            }
      }

int TracksModel::rowFromTrackIndex(int trackIndex) const
      {
                  // first row reserved for all tracks if track count > 1
      return (_trackCount > 1) ? trackIndex + 1 : trackIndex;
      }

int TracksModel::trackIndexFromRow(int row) const
      {
                  // first row reserved for all tracks if track count > 1
                  // return -1 if row is all tracks row
      return (_trackCount > 1) ? row - 1 : row;
      }

int TracksModel::trackCountForImport() const
      {
      int count = 0;
      for (int i = 0; i != _trackCount; ++i) {
            if (_trackOpers.doImport.value(i))
                  ++count;
            }
      return count;
      }

int TracksModel::frozenRowCount() const
      {
      if (_trackCount > 1)
            return 1;
      return 0;
      }

int TracksModel::frozenColCount() const
      {
      return _frozenColCount;
      }

int TracksModel::rowCount(const QModelIndex &/*parent*/) const
      {
      return (_trackCount > 1) ? _trackCount + 1 : _trackCount;
      }

int TracksModel::columnCount(const QModelIndex &/*parent*/) const
      {
      return _columns.size();
      }

QVariant TracksModel::data(const QModelIndex &index, int role) const
      {
      if (!index.isValid())
            return QVariant();
      const int trackIndex = trackIndexFromRow(index.row());
      if (!isTrackIndexValid(trackIndex) || !isColumnValid(index.column()))
            return QVariant();

      switch (role) {
            case Qt::DisplayRole:
                  if (trackIndex == -1) {       // all tracks
                        QVariant value = _columns[index.column()]->value(0);
                        if (value.type() == QVariant::String) {
                              for (int i = 1; i < _trackCount; ++i) {
                                    if (_columns[index.column()]->value(i).toString() != value.toString())
                                          return "...";
                                    }
                              return value.toString();
                              }
                        }
                  else {
                        QVariant value = _columns[index.column()]->value(trackIndex);
                        if (value.type() == QVariant::String)
                              return value.toString();
                        }
                  break;
            case Qt::EditRole:
                  if (_columns[index.column()]->isEditable()) {
                        QVariant value = _columns[index.column()]->value(trackIndex);
                        if (value.type() == QVariant::StringList)
                              return _columns[index.column()]->valueList();
                        }
                  break;
            case Qt::CheckStateRole:
                  if (trackIndex == -1) {
                        QVariant value = _columns[index.column()]->value(0);
                        if (value.type() == QVariant::Bool) {
                              for (int i = 1; i < _trackCount; ++i) {
                                    if (_columns[index.column()]->value(i).toBool() != value.toBool())
                                          return Qt::PartiallyChecked;
                                    }
                              return (value.toBool()) ? Qt::Checked : Qt::Unchecked;
                              }
                        }
                  else {
                        QVariant value = _columns[index.column()]->value(trackIndex);
                        if (value.type() == QVariant::Bool)
                              return (value.toBool()) ? Qt::Checked : Qt::Unchecked;
                        }
                  break;
            case Qt::TextAlignmentRole:
//                  if (!columns[index.column()]->valuesList.empty())
//                        return Qt::AlignLeft + Qt::AlignVCenter;
                  return Qt::AlignCenter;
                  break;
            case Qt::ToolTipRole:
                  if (trackIndex != -1) {
                        QVariant value = _columns[index.column()]->value(trackIndex);
                        if (value.type() == QVariant::String
                                    && _columns[index.column()]->valueList().empty()) {
                              return MidiCharset::convertToCharset(value.toString().toStdString());
                              }
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
      if (_columns[index.column()]->value(0).type() == QVariant::Bool)
            flags |= Qt::ItemIsUserCheckable;
      if (_columns[index.column()]->isEditable())
            flags |= Qt::ItemIsEditable;
      return flags;
      }

void TracksModel::forceRowDataChanged(int row)
      {
      const auto begIndex = this->index(row, 0);
      const auto endIndex = this->index(row, columnCount(QModelIndex()));
      emit dataChanged(begIndex, endIndex);
      }

void TracksModel::forceColumnDataChanged(int col)
      {
      const auto begIndex = this->index(0, col);
      const auto endIndex = this->index(rowCount(QModelIndex()), col);
      emit dataChanged(begIndex, endIndex);
      }

bool TracksModel::setData(const QModelIndex &index, const QVariant &value, int /*role*/)
      {
      const int trackIndex = trackIndexFromRow(index.row());
      if (!isTrackIndexValid(trackIndex) || !isColumnValid(index.column()))
            return false;

      if (trackIndex == -1) {   // all tracks row
            for (int i = 0; i != _trackCount; ++i)
                  _columns[index.column()]->setValue(value, i);
            forceColumnDataChanged(index.column());
            }
      else {
            _columns[index.column()]->setValue(value, index.row());
            emit dataChanged(index, index);
            if (_trackCount > 1)    // update 'all tracks' row
                  forceRowDataChanged(0);
            }
      return true;
      }

QVariant TracksModel::headerData(int section, Qt::Orientation orientation, int role) const
      {
      if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            return QCoreApplication::translate("MIDI import track list",
                                               _columns[section]->headerName().toStdString().c_str());
            }
      else if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
            if (_trackCount > 1 && section == 0)
                  return QCoreApplication::translate("MIDI import operations", "All");
            return section + 1;
            }
      return QVariant();
      }

bool TracksModel::isTrackIndexValid(int trackIndex) const
      {
      return trackIndex >= 0 && trackIndex < _trackCount;
      }

bool TracksModel::isColumnValid(int column) const
      {
      return (column >= 0 && column < (int)_columns.size());
      }

} // namespace Ms
