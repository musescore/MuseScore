#include "importmidi_panel.h"
#include "ui_importmidi_panel.h"
#include "musescore.h"
#include "importmidi_operations.h"
#include "libmscore/score.h"
#include "preferences.h"

#include <QTimer>
#include <QFileDialog>
#include <QFileInfo>
#include <QAbstractTableModel>

#include <vector>


extern Score::FileError importMidi(Score*, const QString&);
extern QList<QString> extractMidiInstruments(const QString&);
extern MuseScore* mscore;
extern Preferences preferences;


struct TrackInfo
      {
      bool doImport;
      QString instrumentName;
      // operations
      bool doLHRHSeparation;
      };

enum {
      TRACK_NUMBER_COL,
      DO_IMPORT_COL,
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

      void reset(const QList<QString> &instrumentNames)
            {
            beginResetModel();
            rowCount_ = instrumentNames.size();
            tracks_.clear();
            for (const auto &name: instrumentNames)
                  tracks_.push_back({true, name, false});
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
                              case INSTRUMENT_COL:
                                    return tracks_[index.row()].instrumentName;
                              case OPERATIONS_COL:
                                    return trackOperations(index.row(), "; ");
                              default:
                                    break;
                              }
                        break;
                  case Qt::CheckStateRole:
                        switch (index.column()) {
                              case DO_IMPORT_COL:
                                    return (tracks_[index.row()].doImport)
                                                ? Qt::Checked : Qt::Unchecked;
                              default:
                                    break;
                              }
                        break;
                  case Qt::TextAlignmentRole:
                        if (index.column() == INSTRUMENT_COL
                                    && tracks_[index.row()].instrumentName == "-")
                              return Qt::AlignHCenter;
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
            Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
            if (index.column() == DO_IMPORT_COL)
                  flags |= Qt::ItemIsUserCheckable;
            return flags;
            }

      bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)
            {
            TrackInfo *info = itemFromIndex(index);
            if (!info)
                  return false;
            bool result = false;
            switch (index.column()) {
                  case DO_IMPORT_COL:
                        if (role != Qt::CheckStateRole)
                              break;
                        tracks_[index.row()].doImport = value.toBool();
                        result = true;
                        break;
                  case OPERATIONS_COL:
                        if (role != Qt::EditRole)
                              break;
                        tracks_[index.row()].doLHRHSeparation = value.toBool();
                        result = true;
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

      TrackInfo trackInfo(int row) const
            {
            return tracks_[row];
            }

      int trackCount() const
            {
            return tracks_.size();
            }

   private:
      std::vector<TrackInfo> tracks_;
      int rowCount_;
      int colCount_;

      TrackInfo* itemFromIndex(const QModelIndex& index)
            {
            if (index.isValid()) {
                  if (index.row() < 0 || index.row() >= rowCount_
                              || index.column() < 0 || index.column() >= colCount_)
                        return nullptr;
                  return &tracks_[index.row()];
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
            if (tracks_[trackIndex].doLHRHSeparation)
                  appendOperation(operations, "LH/RH separation", splitter);
            return operations;
            }

      void appendOperation(QString& operations, const QString& operation,
                           const QString& splitter) const
            {
            if (!operations.isEmpty())
                  operations += splitter;
            operations += operation;
            }

      }; // TracksModel


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
      if (currentIndex.isValid())
            ui->checkBoxLHRH->setChecked(
                              tracksModel->trackInfo(currentIndex.row()).doLHRHSeparation);
      }

void ImportMidiPanel::onLHRHchanged(bool doLHRH)
      {
      const QModelIndex& currentIndex = ui->tableViewTracks->currentIndex();
      tracksModel->setData(tracksModel->index(currentIndex.row(), OPERATIONS_COL), doLHRH);
      }

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

      updateUiTimer->start(100);
      updateUi();

      ui->tableViewTracks->verticalHeader()->setDefaultSectionSize(22);
      ui->tableViewTracks->horizontalHeader()->setResizeMode(TRACK_NUMBER_COL,
                                                             QHeaderView::ResizeToContents);
      ui->tableViewTracks->horizontalHeader()->setResizeMode(DO_IMPORT_COL,
                                                             QHeaderView::ResizeToContents);
      ui->tableViewTracks->horizontalHeader()->setResizeMode(INSTRUMENT_COL,
                                                             QHeaderView::Stretch);
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
            TrackInfo info(tracksModel->trackInfo(i));
            preferences.midiImportOperations.addTrackOperations({info.doImport,
                                                                 info.doLHRHSeparation});
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
            QList<QString> instruments(extractMidiInstruments(file));
            tracksModel->reset(instruments);
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


