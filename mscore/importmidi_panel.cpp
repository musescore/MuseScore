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

namespace Ms {

extern Score::FileError importMidi(Score*, const QString&);
extern QList<QString> extractMidiInstruments(const QString&);
extern MuseScore* mscore;
extern Preferences preferences;

struct TrackInfo
      {
      bool doImport;
      QString instrumentName;
      bool doLHRHSeparation;
      };


class TracksModel : public QAbstractTableModel
      {
   public:
      TracksModel()
            : rowCount_(0)
            , colCount_(3)
            {
            }

      void reset(const QList<QString> &instrumentNames)
            {
            beginResetModel();
            rowCount_ = instrumentNames.size();
            colCount_ = 3;
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
                              case 1:
                                    return tracks_[index.row()].instrumentName;
                              case 2:
                                    if (tracks_[index.row()].doLHRHSeparation)
                                          return "LH/RH separation";
                              default:
                                    break;
                              }
                        break;
                  case Qt::CheckStateRole:
                        switch (index.column()) {
                              case 0:
                                    return (tracks_[index.row()].doImport)
                                                ? Qt::Checked : Qt::Unchecked;
                              default:
                                    break;
                              }
                        break;
                  case Qt::TextAlignmentRole:
                        if (index.column() == 1
                                    && tracks_[index.row()].instrumentName == "-")
                              return Qt::AlignHCenter;
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
            if (index.column() == 0)
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
                  case 0:
                        if (role != Qt::CheckStateRole)
                              break;
                        tracks_[index.row()].doImport = value.toBool();
                        result = true;
                        break;
                  case 2:
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
                  if (section == 0)
                        return "Import?";
                  else if (section == 1)
                        return "Instrument";
                  else if (section == 2)
                        return "Operations";
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

void ImportMidiPanel::openMidiFile()
      {
      QString path(QFileDialog::getOpenFileName(this, tr("Select file"),
                                                "", tr("Files (*.mid *.midi)")));
      if (!path.isEmpty())
            setMidiFile(path);
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
            ui->checkBoxLHRH->setChecked(tracksModel->trackInfo(currentIndex.row()).doLHRHSeparation);
      }

void ImportMidiPanel::onLHRHchanged(bool doLHRH)
      {
      const QModelIndex& currentIndex = ui->tableViewTracks->currentIndex();
      tracksModel->setData(tracksModel->index(currentIndex.row(), 2), doLHRH);
      }

void ImportMidiPanel::tweakUi()
      {
      connect(updateUiTimer, SIGNAL(timeout()), this, SLOT(updateUiOnTimer()));
      connect(ui->pushButtonBrowseMidi, SIGNAL(clicked()), SLOT(openMidiFile()));
      connect(ui->pushButtonImport, SIGNAL(clicked()), SLOT(importMidi()));
      connect(ui->pushButtonHideMidiImportPanel, SIGNAL(clicked()), SLOT(hidePanel()));

      QItemSelectionModel *sm = ui->tableViewTracks->selectionModel();
      connect(sm, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
              SLOT(onCurrentTrackChanged(QModelIndex)));
      connect(ui->checkBoxLHRH, SIGNAL(toggled(bool)),
              SLOT(onLHRHchanged(bool)));

      updateUiTimer->start(100);
      updateUi();

      ui->tableViewTracks->verticalHeader()->setDefaultSectionSize(22);
      ui->tableViewTracks->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
      ui->tableViewTracks->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
      ui->tableViewTracks->horizontalHeader()->setResizeMode(2, QHeaderView::Stretch);
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
            ui->plainTextEditFile->setStyleSheet("QPlainTextEdit{color: black;}");
            ui->plainTextEditFile->setToolTip(midiFile);
            }
      else {  // midi file not exists
            ui->plainTextEditFile->setStyleSheet("QPlainTextEdit{color: red;}");
            ui->plainTextEditFile->setToolTip(tr("MIDI file not found"));
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
            preferences.midiImportOperations.addTrackOperations({info.doImport, info.doLHRHSeparation});
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
      ui->plainTextEditFile->setPlainText(QFileInfo(file).fileName());
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

}


