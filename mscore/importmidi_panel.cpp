#include "importmidi_panel.h"
#include "ui_importmidi_panel.h"
#include "musescore.h"
#include "importmidi_operations.h"
#include "libmscore/score.h"
#include "preferences.h"

struct Operation
      {
      enum class OperationType
            {
            DO_IMPORT,
            DO_LHRH_SEPARATION,
            USE_DOTS
            } type;

      union OperationValue
            {
            bool doImport;
            bool doLHRHSeparation;
            bool useDots;
            } value;
      };

Q_DECLARE_METATYPE(Operation) // to pass value as QVariant

namespace Ms {

extern Score::FileError importMidi(Score*, const QString&);
extern QList<TrackMeta> extractMidiTracksMeta(const QString&);
extern MuseScore* mscore;
extern Preferences preferences;

struct TrackData
      {
      TrackMeta meta;
      TrackOperations opers;
      };

enum {
      TRACK_NUMBER_COL,
      DO_IMPORT_COL,
      TRACK_NAME_COL,
      INSTRUMENT_COL,
      OPERATIONS_COL,
      COL_COUNT
      };


class TracksModel : public QAbstractTableModel
      {
   public:
      TracksModel()
            : rowCount_(0)
            , colCount_(COL_COUNT)
            {
            }

      void reset(const QList<TrackMeta> &tracksMeta)
            {
            beginResetModel();
            rowCount_ = tracksMeta.size();
            tracks_data_.clear();
            for (const auto &meta: tracksMeta) {
                  QString trackName = meta.trackName.isEmpty()
                              ? "-" : meta.trackName;
                  QString instrumentName = meta.instrumentName.isEmpty()
                              ? "-" : meta.instrumentName;
                  tracks_data_.push_back({
                        {trackName, instrumentName},
                        TrackOperations() // initialized by default values - see ctor
                        });
                  }
            endResetModel();
            }

      int rowCount(const QModelIndex& /*parent*/) const
            {
            return rowCount_;
            }

      int columnCount(const QModelIndex& /*parent*/) const
            {
            return colCount_;
            }

      QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
            {
            if (!index.isValid())
                  return QVariant();
            switch (role) {
                  case Qt::DisplayRole:
                        switch (index.column()) {
                              case TRACK_NUMBER_COL:
                                    return index.row() + 1;
                              case TRACK_NAME_COL:
                                    return tracks_data_[index.row()].meta.trackName;
                              case INSTRUMENT_COL:
                                    return tracks_data_[index.row()].meta.instrumentName;
                              case OPERATIONS_COL:
                                    return trackOperations(index.row(), "; ");
                              default:
                                    break;
                              }
                        break;
                  case Qt::CheckStateRole:
                        switch (index.column()) {
                              case DO_IMPORT_COL:
                                    return (tracks_data_[index.row()].opers.doImport)
                                                ? Qt::Checked : Qt::Unchecked;
                              default:
                                    break;
                              }
                        break;
                  case Qt::TextAlignmentRole:
                        switch (index.column()) {
                              case TRACK_NAME_COL:
                                    if (tracks_data_[index.row()].meta.trackName == "-")
                                          return Qt::AlignHCenter;
                              case INSTRUMENT_COL:
                                    if (tracks_data_[index.row()].meta.instrumentName == "-")
                                          return Qt::AlignHCenter;
                              default:
                                    break;
                              }
                        break;
                  case Qt::ToolTipRole:
                        if (index.column() == OPERATIONS_COL)
                              return trackOperations(index.row(), "\n");
                        break;
                  default:
                        break;
                  }
            return QVariant();
            }

      Qt::ItemFlags flags(const QModelIndex& index) const
            {
            if (!index.isValid())
                  return 0;

            Qt::ItemFlags flags = Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            if (index.column() == DO_IMPORT_COL)
                  flags |= Qt::ItemIsUserCheckable;
            return flags;
            }

      bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)
            {
            TrackData *trackData = itemFromIndex(index);
            if (!trackData)
                  return false;
            bool result = false;
            switch (index.column()) {
                  case DO_IMPORT_COL:
                        if (role != Qt::CheckStateRole)
                              break;
                        trackData->opers.doImport = value.toBool();
                        result = true;
                        break;
                  case OPERATIONS_COL:
                        if (role != Qt::EditRole)
                              break;
                        {
                        Operation op = value.value<Operation>();
                        switch (op.type) {
                              case Operation::OperationType::DO_LHRH_SEPARATION:
                                    trackData->opers.doLHRHSeparation = op.value.doLHRHSeparation;
                                    result = true;
                                    break;
                              case Operation::OperationType::USE_DOTS:
                                    trackData->opers.useDots = op.value.useDots;
                                    result = true;
                                    break;
                              default:
                                    break;
                              }
                        }
                        break;
                  default:
                        break;
                  }
            if (result)
                  emit dataChanged(index, index);
            return result;
            }

      QVariant headerData(int section, Qt::Orientation orientation, int role) const
            {
            if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
                  switch (section) {
                        case TRACK_NUMBER_COL:
                              return "Track";
                        case DO_IMPORT_COL:
                              return "Import?";
                        case TRACK_NAME_COL:
                              return "Name";
                        case INSTRUMENT_COL:
                              return "Instrument";
                        case OPERATIONS_COL:
                              return "Operations";
                        default:
                              break;
                        }
                  }
            return QVariant();
            }

      TrackData trackData(int row) const
            {
            return tracks_data_[row];
            }

      int trackCount() const
            {
            return tracks_data_.size();
            }

   private:
      std::vector<TrackData> tracks_data_;
      int rowCount_;
      int colCount_;

      TrackData* itemFromIndex(const QModelIndex& index)
            {
            if (index.isValid()) {
                  if (index.row() < 0 || index.row() >= rowCount_
                              || index.column() < 0 || index.column() >= colCount_)
                        return nullptr;
                  return &tracks_data_[index.row()];
                  }
            else {
                  return nullptr;
                  }
            }

      QString trackOperations(int trackIndex, const QString& splitter) const
            {
            QString operations;
            if (trackIndex >= trackCount())
                  return operations;
            // all operations without doImport
            if (tracks_data_[trackIndex].opers.doLHRHSeparation)
                  appendOperation(operations, "LH/RH separation", splitter);
            if (tracks_data_[trackIndex].opers.useDots)
                  appendOperation(operations, "Use dots", splitter);
            return operations;
            }

      void appendOperation(QString& operations, const QString& operation,
                           const QString& splitter) const
            {
            if (!operations.isEmpty())
                  operations += splitter;
            operations += operation;
            }

      }; // class TracksModel


ImportMidiPanel::ImportMidiPanel(QWidget *parent)
      : QWidget(parent)
      , ui(new Ui::ImportMidiPanel)
      , updateUiTimer(new QTimer)
      , isMidiFileExists(false)
      {
      ui->setupUi(this);
      tracksModel = new TracksModel();
      ui->tableViewTracks->setModel(tracksModel);
      tweakUi();
      }

ImportMidiPanel::~ImportMidiPanel()
      {
      delete ui;
      }

void ImportMidiPanel::updateUiOnTimer()
      {
      bool change = false;
      if (isMidiFileExists != QFileInfo(midiFile).exists()) {
            isMidiFileExists = !isMidiFileExists;
            change = true;
            }
      if (change)
            updateUi();
      }

void ImportMidiPanel::onCurrentTrackChanged(const QModelIndex &currentIndex)
      {
      if (currentIndex.isValid()) {
            int row = currentIndex.row();
            ui->checkBoxLHRH->setChecked(
                  tracksModel->trackData(row).opers.doLHRHSeparation);
            ui->checkBoxDots->setChecked(
                  tracksModel->trackData(row).opers.useDots);
            }
      }

void ImportMidiPanel::onLHRHchanged(bool doLHRH)
      {
      const QModelIndex& currentIndex = ui->tableViewTracks->currentIndex();
      Operation op;
      op.type = Operation::OperationType::DO_LHRH_SEPARATION;
      op.value.doLHRHSeparation = doLHRH;
      QVariant value;
      value.setValue(op);
      tracksModel->setData(
            tracksModel->index(currentIndex.row(), OPERATIONS_COL), value);
      }

void ImportMidiPanel::onUseDotschanged(bool useDots)
      {
      const QModelIndex& currentIndex = ui->tableViewTracks->currentIndex();
      Operation op;
      op.type = Operation::OperationType::USE_DOTS;
      op.value.useDots = useDots;
      QVariant value;
      value.setValue(op);
      tracksModel->setData(
            tracksModel->index(currentIndex.row(), OPERATIONS_COL), value);
      }

// Сlass to add an extra width to specific columns

class CustomHorizHeaderView : public QHeaderView
      {
   public:
      CustomHorizHeaderView() : QHeaderView(Qt::Horizontal) {}

   protected:
      QSize sectionSizeFromContents(int logicalIndex) const
            {
            auto sz = QHeaderView::sectionSizeFromContents(logicalIndex);
            const int EXTRA_SPACE = 35;
            if (logicalIndex == TRACK_NAME_COL || logicalIndex == INSTRUMENT_COL)
                  return QSize(sz.width() + EXTRA_SPACE, sz.height());
            else
                  return sz;
            }
      };

void ImportMidiPanel::tweakUi()
      {
      connect(updateUiTimer, SIGNAL(timeout()), this, SLOT(updateUiOnTimer()));
      connect(ui->pushButtonImport, SIGNAL(clicked()), SLOT(importMidi()));
      connect(ui->toolButtonHideMidiPanel, SIGNAL(clicked()), SLOT(hidePanel()));

      QItemSelectionModel *sm = ui->tableViewTracks->selectionModel();
      connect(sm, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
              SLOT(onCurrentTrackChanged(QModelIndex)));
      connect(ui->checkBoxLHRH, SIGNAL(toggled(bool)),
              SLOT(onLHRHchanged(bool)));
      connect(ui->checkBoxDots, SIGNAL(toggled(bool)),
              SLOT(onUseDotschanged(bool)));

      updateUiTimer->start(100);
      updateUi();

      ui->tableViewTracks->verticalHeader()->setDefaultSectionSize(22);
      ui->tableViewTracks->setHorizontalHeader(new CustomHorizHeaderView());
      ui->tableViewTracks->horizontalHeader()->setResizeMode(TRACK_NUMBER_COL,
                                                             QHeaderView::ResizeToContents);
      ui->tableViewTracks->horizontalHeader()->setResizeMode(DO_IMPORT_COL,
                                                             QHeaderView::ResizeToContents);
      ui->tableViewTracks->horizontalHeader()->setResizeMode(TRACK_NAME_COL,
                                                             QHeaderView::ResizeToContents);
      ui->tableViewTracks->horizontalHeader()->setResizeMode(INSTRUMENT_COL,
                                                             QHeaderView::ResizeToContents);
      ui->tableViewTracks->horizontalHeader()->setResizeMode(OPERATIONS_COL,
                                                             QHeaderView::Stretch);
      }

bool ImportMidiPanel::canImportMidi() const
      {
      return (QFileInfo(midiFile).exists());
      }

void ImportMidiPanel::hidePanel()
      {
      if (isVisible()) {
            setVisible(false);
            getAction("toggle-midiimportpanel")->setChecked(false);
            }
      }

void ImportMidiPanel::updateUi()
      {
      if (isMidiFileExists) {
            ui->lineEditMidiFile->setStyleSheet("QLineEdit{color: black;}");
            ui->lineEditMidiFile->setToolTip(midiFile);
            }
      else {  // midi file not exists
            ui->lineEditMidiFile->setStyleSheet("QLineEdit{color: red;}");
            ui->lineEditMidiFile->setToolTip(tr("MIDI file not found"));
            }
      ui->pushButtonImport->setEnabled(canImportMidi());
      }

void ImportMidiPanel::importMidi()
      {
      if (!canImportMidi())
            return;
      preferences.midiImportOperations.clear();
      int trackCount = tracksModel->trackCount();
      for (int i = 0; i != trackCount; ++i) {
            TrackData data(tracksModel->trackData(i));
            preferences.midiImportOperations.appendTrackOperations(data.opers);
            }
      mscore->openScore(midiFile);
      preferences.midiImportOperations.clear();
      }

bool ImportMidiPanel::isMidiFile(const QString &file)
      {
      QString extension = QFileInfo(file).suffix().toLower();
      return (extension == "mid" || extension == "midi");
      }

void ImportMidiPanel::setMidiFile(const QString &file)
      {
      midiFile = file;
      ui->lineEditMidiFile->setText(QFileInfo(file).fileName());
      updateUiOnTimer();
      if (isMidiFileExists) {
            QList<TrackMeta> tracksMeta(extractMidiTracksMeta(file));
            tracksModel->reset(tracksMeta);
            ui->tableViewTracks->selectRow(0);
            }
      }


//--------- MuseScore ---------

void MuseScore::showMidiImportPanel(bool visible)
      {
      if (visible) {
            if (!importmidi_panel) {
                  importmidi_panel = new ImportMidiPanel(this);
                  }
            }
      if (importmidi_panel)
            importmidi_panel->setVisible(visible);
      getAction("toggle-midiimportpanel")->setChecked(visible);
      }

}


