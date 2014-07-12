#include "importmidi_model.h"
#include "importmidi_inner.h"
#include "preferences.h"


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
      explicit Column(MidiOperations::Opers &opers) : _opers(opers) {}
      virtual ~Column() {}

      virtual QVariant value(int trackIndex) const = 0;
      virtual void setValue(const QVariant &value, int trackIndex) = 0;
      virtual QString headerName() const = 0;
      virtual bool isVisible(int /*trackIndex*/) const { return true; }
      virtual QStringList valueList(int /*trackIndex*/) const { return _values; }
      virtual int width() const { return -1; }
      virtual bool isEditable() const { return true; }
      virtual bool isForAllTracksOnly() const { return false; }

   protected:
      MidiOperations::Opers &_opers;
      QStringList _values;
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
                        int trackCount,
                        const QString &midiFile)
      {
      beginResetModel();
      _trackOpers = opers;
      _columns.clear();
      _trackCount = trackCount;
      _frozenColCount = 0;
      _midiFile = midiFile;
      if (trackCount == 0)
            return;

      //-----------------------------------------------------------------------
      struct Import : Column {
            Import(MidiOperations::Opers &opers) : Column(opers) {}

            QString headerName() const { return QCoreApplication::translate(
                                                      "MIDI import operations", "Import"); }
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
      bool hasStaffName = false;
      for (int i = 0; i != _trackCount; ++i) {
            if (_trackOpers.staffName.value(i) != "") {
                  hasStaffName = true;
                  break;
                  }
            }
      if (hasStaffName) {
            struct StaffName : Column {
                  StaffName(MidiOperations::Opers &opers, const QString &midiFile)
                        : Column(opers), _midiFile(midiFile)
                        {
                        }
                  int width() const { return 180; }
                  QString headerName() const { return QCoreApplication::translate(
                                                      "MIDI import operations", "Staff name"); }
                  bool isEditable() const { return false; }
                  QVariant value(int trackIndex) const
                        {
                        MidiOperations::Data &opers = preferences.midiImportOperations;
                        MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, _midiFile);

                        return MidiCharset::convertToCharset(_opers.staffName.value(trackIndex));
                        }
                  void setValue(const QVariant &/*value*/, int /*trackIndex*/) {}

               private:
                  QString _midiFile;
                  };
            ++_frozenColCount;
            _columns.push_back(std::unique_ptr<Column>(new StaffName(_trackOpers, _midiFile)));
            }

      //-----------------------------------------------------------------------
      struct InstrumentName : Column {
            InstrumentName(MidiOperations::Opers &opers) : Column(opers)
                  {
                  }
            int width() const { return 130; }
            QString headerName() const { return QCoreApplication::translate(
                                                      "MIDI import operations", "Sound"); }
            bool isEditable() const { return false; }
            QVariant value(int trackIndex) const
                  {
                  return _opers.instrumentName.value(trackIndex);
                  }
            void setValue(const QVariant &/*value*/, int /*trackIndex*/) {}
            };
      ++_frozenColCount;
      _columns.push_back(std::unique_ptr<Column>(new InstrumentName(_trackOpers)));

      //-----------------------------------------------------------------------
      if (!lyricsList.isEmpty()) {
            struct Lyrics : Column {
                  Lyrics(MidiOperations::Opers &opers,
                         const QList<std::string> &lyricsList,
                         const QString &midiFile)
                        : Column(opers), _lyricsList(lyricsList), _midiFile(midiFile)
                        {
                        }
                  int width() const { return 185; }
                  QString headerName() const { return QCoreApplication::translate(
                                                      "MIDI import operations", "Lyrics"); }
                  QVariant value(int trackIndex) const
                        {
                        int index = _opers.lyricTrackIndex.value(trackIndex);
                        if (index >= 0) {
                              MidiOperations::Data &opers = preferences.midiImportOperations;
                              MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, _midiFile);

                              return MidiCharset::convertToCharset(_lyricsList[index]);
                              }
                        return "";
                        }
                  void setValue(const QVariant &value, int trackIndex)
                        {
                                    // GUI lyrics list always have "" row, so: (index - 1)
                        _opers.lyricTrackIndex.setValue(trackIndex, value.toInt() - 1);
                        }
                  QStringList valueList(int /*trackIndex*/) const
                        {
                        MidiOperations::Data &opers = preferences.midiImportOperations;
                        MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, _midiFile);

                        auto list = QStringList("");
                        for (const auto &lyric: _lyricsList)
                              list.append(MidiCharset::convertToCharset(lyric));
                        return list;
                        }
               private:
                  QList<std::string> _lyricsList;
                  QString _midiFile;
                  };
            _columns.push_back(std::unique_ptr<Column>(new Lyrics(_trackOpers, lyricsList, _midiFile)));
            }

      //-----------------------------------------------------------------------
      struct QuantValue : Column {
            QuantValue(MidiOperations::Opers &opers) : Column(opers)
                  {
                  _values.push_back(QCoreApplication::translate("MIDI import operations", "Quarter"));
                  _values.push_back(QCoreApplication::translate("MIDI import operations", "Eighth"));
                  _values.push_back(QCoreApplication::translate("MIDI import operations", "16th"));
                  _values.push_back(QCoreApplication::translate("MIDI import operations", "32nd"));
                  _values.push_back(QCoreApplication::translate("MIDI import operations", "64th"));
                  _values.push_back(QCoreApplication::translate("MIDI import operations", "128th"));
                  }
            QString headerName() const { return QCoreApplication::translate(
                                                "MIDI import operations", "Quantization"); }
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

      //-----------------------------------------------------------------------
      struct Human : Column {
            Human(MidiOperations::Opers &opers) : Column(opers)
                  {
                  }
            QString headerName() const { return QCoreApplication::translate(
                                                      "MIDI import operations", "Is human"); }
            bool isForAllTracksOnly() const { return true; }
            QVariant value(int /*trackIndex*/) const
                  {
                  return _opers.isHumanPerformance;
                  }
            void setValue(const QVariant &value, int /*trackIndex*/)
                  {
                  _opers.isHumanPerformance = value.toBool();
                  }
            };
      _columns.push_back(std::unique_ptr<Column>(new Human(_trackOpers)));

      //-----------------------------------------------------------------------
      struct VoiceCount : Column {
            VoiceCount(MidiOperations::Opers &opers) : Column(opers)
                  {
                  _values.push_back("1");
                  _values.push_back("2");
                  _values.push_back("3");
                  _values.push_back("4");
                  }
            QString headerName() const { return QCoreApplication::translate(
                                                      "MIDI import operations", "Max voices"); }
            QVariant value(int trackIndex) const
                  {
                  return _values[(int)_opers.maxVoiceCount.value(trackIndex)];
                  }
            void setValue(const QVariant &value, int trackIndex)
                  {
                  _opers.maxVoiceCount.setValue(trackIndex, (MidiOperations::VoiceCount)value.toInt());
                  }
            bool isVisible(int trackIndex) const
                  {
                  if (_opers.isDrumTrack.value(trackIndex))
                        return false;
                  return true;
                  }
            };
      _columns.push_back(std::unique_ptr<Column>(new VoiceCount(_trackOpers)));

      //-----------------------------------------------------------------------
      struct Tuplets : Column {
            Tuplets(MidiOperations::Opers &opers, int trackCount)
                  : Column(opers), _trackCount(trackCount)
                  {
                  _values.push_back(QCoreApplication::translate("MIDI import operations", "2-plets"));
                  _values.push_back(QCoreApplication::translate("MIDI import operations", "3-plets"));
                  _values.push_back(QCoreApplication::translate("MIDI import operations", "4-plets"));
                  _values.push_back(QCoreApplication::translate("MIDI import operations", "5-plets"));
                  _values.push_back(QCoreApplication::translate("MIDI import operations", "7-plets"));
                  _values.push_back(QCoreApplication::translate("MIDI import operations", "9-plets"));
                  }
            int width() const { return 140; }
            QString headerName() const { return QCoreApplication::translate(
                                                      "MIDI import operations", "Tuplets"); }
            QVariant value(int trackIndex) const
                  {
                  QString val;
                  if (_opers.search2plets.value(trackIndex)) {
                        if (val != "")
                              val += ", ";
                        val += "2";
                        }
                  if (_opers.search3plets.value(trackIndex)) {
                        if (val != "")
                              val += ", ";
                        val += "3";
                        }
                  if (_opers.search4plets.value(trackIndex)) {
                        if (val != "")
                              val += ", ";
                        val += "4";
                        }
                  if (_opers.search5plets.value(trackIndex)) {
                        if (val != "")
                              val += ", ";
                        val += "5";
                        }
                  if (_opers.search7plets.value(trackIndex)) {
                        if (val != "")
                              val += ", ";
                        val += "7";
                        }
                  if (_opers.search9plets.value(trackIndex)) {
                        if (val != "")
                              val += ", ";
                        val += "9";
                        }
                  return val;
                  }
            void setValue(const QVariant &value, int trackIndex)
                  {
                  const QStringList list = value.toStringList();
                  bool searchTuplets = false;
                  if (list[0] != "undefined") {
                        const bool doSearch = (list[0] == "true");
                        _opers.search2plets.setValue(trackIndex, doSearch);
                        if (!searchTuplets && doSearch)
                              searchTuplets = true;
                        }
                  if (list[1] != "undefined") {
                        const bool doSearch = (list[1] == "true");
                        _opers.search3plets.setValue(trackIndex, doSearch);
                        if (!searchTuplets && doSearch)
                              searchTuplets = true;
                        }
                  if (list[2] != "undefined") {
                        const bool doSearch = (list[2] == "true");
                        _opers.search4plets.setValue(trackIndex, doSearch);
                        if (!searchTuplets && doSearch)
                              searchTuplets = true;
                        }
                  if (list[3] != "undefined") {
                        const bool doSearch = (list[3] == "true");
                        _opers.search5plets.setValue(trackIndex, doSearch);
                        if (!searchTuplets && doSearch)
                              searchTuplets = true;
                        }
                  if (list[4] != "undefined") {
                        const bool doSearch = (list[4] == "true");
                        _opers.search7plets.setValue(trackIndex, doSearch);
                        if (!searchTuplets && doSearch)
                              searchTuplets = true;
                        }
                  if (list[5] != "undefined") {
                        const bool doSearch = (list[5] == "true");
                        _opers.search9plets.setValue(trackIndex, doSearch);
                        if (!searchTuplets && doSearch)
                              searchTuplets = true;
                        }
                  _opers.searchTuplets.setValue(trackIndex, searchTuplets);
                  }
            QStringList valueList(int trackIndex) const
                  {
                  auto list = QStringList("__MultiValue__");

                  list.append(_values[0]);
                  list.append(checkBoxValue(trackIndex, _opers.search2plets));
                  list.append(_values[1]);
                  list.append(checkBoxValue(trackIndex, _opers.search3plets));
                  list.append(_values[2]);
                  list.append(checkBoxValue(trackIndex, _opers.search4plets));
                  list.append(_values[3]);
                  list.append(checkBoxValue(trackIndex, _opers.search5plets));
                  list.append(_values[4]);
                  list.append(checkBoxValue(trackIndex, _opers.search7plets));
                  list.append(_values[5]);
                  list.append(checkBoxValue(trackIndex, _opers.search9plets));

                  return list;
                  }

         private:
            QString checkBoxValue(int trackIndex,
                                  const MidiOperations::TrackOp<bool> &operation) const
                  {
                  if (trackIndex == -1) {       // symbolizes all tracks
                        const bool firstValue = operation.value(0);
                        for (int i = 1; i < _trackCount; ++i) {
                              if (operation.value(i) != firstValue)
                                    return "undefined";
                              }
                        trackIndex = 0;   // to pick the first track value on return
                        }
                  return operation.value(trackIndex) ? "true" : "false";
                  }

            int _trackCount;
            };
      _columns.push_back(std::unique_ptr<Column>(new Tuplets(_trackOpers, _trackCount)));

      //-----------------------------------------------------------------------
      struct StaffSplit : Column {
            StaffSplit(MidiOperations::Opers &opers) : Column(opers)
                  {
                  }
            QString headerName() const { return QCoreApplication::translate(
                                                      "MIDI import operations", "Split staff"); }
            QVariant value(int trackIndex) const
                  {
                  return _opers.doStaffSplit.value(trackIndex);
                  }
            void setValue(const QVariant &value, int trackIndex)
                  {
                  _opers.doStaffSplit.setValue(trackIndex, value.toBool());
                  }
            };
      _columns.push_back(std::unique_ptr<Column>(new StaffSplit(_trackOpers)));

      endResetModel();
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
      if (!isTrackIndexValid(trackIndex) || trackIndex < 0)
            return;
      _trackOpers.trackIndexAfterShuffle.setValue(trackIndex, newIndex);
      }

void TracksModel::updateCharset()
      {
      forceAllChanged();
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

bool TracksModel::editableSingleTrack(int trackIndex, int column) const
      {
      return !(trackIndex >= 0 && _trackCount != 1 && _columns[column]->isForAllTracksOnly());
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
                        if (_columns[index.column()]->isEditable()) {
                              QVariant value = _columns[index.column()]->value(0);
                              if (value.type() == QVariant::String) {
                                    if (!_columns[index.column()]->isForAllTracksOnly()) {
                                          for (int i = 1; i < _trackCount; ++i) {
                                                if (_columns[index.column()]->isVisible(i)
                                                            && _columns[index.column()]->value(i).toString()
                                                                        != value.toString()) {
                                                      return "...";
                                                      }
                                                }
                                          }
                                    if (_columns[index.column()]->isVisible(0))
                                          return value.toString();
                                    }
                              }
                        }
                  else if (editableSingleTrack(trackIndex, index.column())
                           && _columns[index.column()]->isVisible(trackIndex)) {
                        QVariant value = _columns[index.column()]->value(trackIndex);
                        if (value.type() == QVariant::String)
                              return value.toString();
                        }
                  break;
            case Qt::EditRole:
                  if (_columns[index.column()]->isEditable()
                              && editableSingleTrack(trackIndex, index.column())
                              && _columns[index.column()]->isVisible(trackIndex)) {
                        if (!_columns[index.column()]->valueList(trackIndex).isEmpty())
                              return _columns[index.column()]->valueList(trackIndex);
                        }
                  break;
            case Qt::CheckStateRole:
                  if (trackIndex == -1) {
                        QVariant value = _columns[index.column()]->value(0);
                        if (value.type() == QVariant::Bool) {
                              if (!_columns[index.column()]->isForAllTracksOnly()) {
                                    for (int i = 1; i < _trackCount; ++i) {
                                          if (_columns[index.column()]->value(i).toBool()
                                                      != value.toBool()) {
                                                return Qt::PartiallyChecked;
                                                }
                                          }
                                    }
                              if (_columns[index.column()]->isVisible(0))
                                    return (value.toBool()) ? Qt::Checked : Qt::Unchecked;
                              }
                        }
                  else if (editableSingleTrack(trackIndex, index.column())) {
                        QVariant value = _columns[index.column()]->value(trackIndex);
                        if (value.type() == QVariant::Bool)
                              return (value.toBool()) ? Qt::Checked : Qt::Unchecked;
                        }
                  break;
            case Qt::TextAlignmentRole:
                  return Qt::AlignCenter;
                  break;
            case Qt::ToolTipRole:
                  if (trackIndex != -1 && _columns[index.column()]->isVisible(trackIndex)) {
                        QVariant value = _columns[index.column()]->value(trackIndex);
                        if (value.type() == QVariant::String
                                    && _columns[index.column()]->valueList(trackIndex).empty()) {
                              MidiOperations::Data &opers = preferences.midiImportOperations;
                              MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, _midiFile);

                              return MidiCharset::convertToCharset(value.toString().toStdString());
                              }
                        }
                  break;
            case Qt::SizeHintRole:
                  return QSize(_columns[index.column()]->width(), -1);
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
      const int trackIndex = trackIndexFromRow(index.row());

      if (_columns[index.column()]->isVisible(trackIndex)) {
            if (_columns[index.column()]->value(0).type() == QVariant::Bool)
                  flags |= Qt::ItemIsUserCheckable;

            if (_columns[index.column()]->isEditable()
                        && editableSingleTrack(trackIndex, index.column())) {
                              // not for checkboxes (value type is bool)
                  QVariant value = _columns[index.column()]->value(0);
                  if (value.type() != QVariant::Bool)
                        flags |= Qt::ItemIsEditable;
                  }
            }
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

void TracksModel::forceAllChanged()
      {
      const auto begIndex = this->index(0, 0);
      const auto endIndex = this->index(rowCount(QModelIndex()), columnCount(QModelIndex()));
      emit dataChanged(begIndex, endIndex);
      }

bool TracksModel::setData(const QModelIndex &index, const QVariant &value, int /*role*/)
      {
      const int trackIndex = trackIndexFromRow(index.row());
      if (!isTrackIndexValid(trackIndex) || !isColumnValid(index.column()))
            return false;

      if (trackIndex == -1) {   // all tracks row
            if (!_columns[index.column()]->isForAllTracksOnly()) {
                  for (int i = 0; i != _trackCount; ++i) {
                        if (_columns[index.column()]->isVisible(trackIndex))
                              _columns[index.column()]->setValue(value, i);
                        }
                  forceColumnDataChanged(index.column());
                  }
            else {
                  _columns[index.column()]->setValue(value, 0);
                  }
            }
      else if (editableSingleTrack(trackIndex, index.column())
               && _columns[index.column()]->isVisible(trackIndex)) {
            _columns[index.column()]->setValue(value, trackIndex);
            emit dataChanged(index, index);
            if (_trackCount > 1)    // update 'all tracks' row
                  forceRowDataChanged(0);
            }
      return true;
      }

QVariant TracksModel::headerData(int section, Qt::Orientation orientation, int role) const
      {
      if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            if (!_columns.empty()) {
                  return QCoreApplication::translate("MIDI import: tracks model",
                                      _columns[section]->headerName().toStdString().c_str());
                  }
            }
      else if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
            if (_trackCount > 1) {
                  if (section == 0)
                        return QCoreApplication::translate("MIDI import: tracks model", "All");
                  return section;
                  }
            return section + 1;
            }
      return QVariant();
      }

bool TracksModel::isTrackIndexValid(int trackIndex) const
      {
      return trackIndex >= -1 && trackIndex < _trackCount;
      }

bool TracksModel::isColumnValid(int column) const
      {
      return (column >= 0 && column < (int)_columns.size());
      }

} // namespace Ms
