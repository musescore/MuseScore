#include "importmidi_panel.h"
#include "ui_importmidi_panel.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "preferences.h"
#include "importmidi_operations.h"
#include "importmidi_trmodel.h"
#include "importmidi_opmodel.h"
#include "importmidi_opdelegate.h"
#include "importmidi_data.h"


namespace Ms {

extern Score::FileError importMidi(Score*, const QString&);
extern QList<TrackMeta> extractMidiTracksMeta(const QString&);
extern MuseScore* mscore;
extern Preferences preferences;


ImportMidiPanel::ImportMidiPanel(QWidget *parent)
      : QWidget(parent)
      , ui(new Ui::ImportMidiPanel)
      , updateUiTimer(new QTimer)
      , isMidiFileExists(false)
      , importInProgress(false)
      , prefferedVisible_(false)
      {
      ui->setupUi(this);
      tracksModel = new TracksModel();
      ui->tableViewTracks->setModel(tracksModel);
      operationsModel = new OperationsModel();
      ui->treeViewOperations->setModel(operationsModel);
      operationsDelegate = new OperationsDelegate(parentWidget());
      ui->treeViewOperations->setItemDelegate(operationsDelegate);
      tweakUi();
      }

ImportMidiPanel::~ImportMidiPanel()
      {
      delete ui;
      }

void ImportMidiPanel::onCurrentTrackChanged(const QModelIndex &currentIndex)
      {
      if (currentIndex.isValid()) {
            int row = currentIndex.row();
            QString trackLabel = tracksModel->data(
                  tracksModel->index(row, TrackCol::TRACK_NUMBER)).toString();
            operationsModel->setTrackData(trackLabel, tracksModel->trackOperations(row));
            ui->treeViewOperations->expandAll();
            midiData.setSelectedRow(midiFile, row);
            }
      }

void ImportMidiPanel::onOperationChanged(const QModelIndex &index)
      {
      MidiOperation::Type operType = (MidiOperation::Type)index.data(OperationTypeRole).toInt();
      const QModelIndex &currentIndex = ui->tableViewTracks->currentIndex();
      tracksModel->setOperation(currentIndex.row(), operType, index.data(DataRole));
      ui->treeViewOperations->expandAll();
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
            if (logicalIndex == TrackCol::TRACK_NAME || logicalIndex == TrackCol::INSTRUMENT)
                  return QSize(sz.width() + EXTRA_SPACE, sz.height());
            else
                  return sz;
            }
      };

void ImportMidiPanel::tweakUi()
      {
      connect(updateUiTimer, SIGNAL(timeout()), this, SLOT(updateUi()));
      connect(ui->pushButtonImport, SIGNAL(clicked()), SLOT(doMidiImport()));
      connect(ui->pushButtonUp, SIGNAL(clicked()), SLOT(moveTrackUp()));
      connect(ui->pushButtonDown, SIGNAL(clicked()), SLOT(moveTrackDown()));
      connect(ui->toolButtonHideMidiPanel, SIGNAL(clicked()), SLOT(hidePanel()));

      QItemSelectionModel *sm = ui->tableViewTracks->selectionModel();
      connect(sm, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
              SLOT(onCurrentTrackChanged(QModelIndex)));
      connect(ui->treeViewOperations->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
              SLOT(onOperationChanged(QModelIndex)));

      updateUiTimer->start(100);
      updateUi();

      ui->tableViewTracks->verticalHeader()->setDefaultSectionSize(22);
      ui->tableViewTracks->setHorizontalHeader(new CustomHorizHeaderView());
      ui->tableViewTracks->horizontalHeader()->setResizeMode(TrackCol::TRACK_NUMBER,
                                                             QHeaderView::ResizeToContents);
      ui->tableViewTracks->horizontalHeader()->setResizeMode(TrackCol::DO_IMPORT,
                                                             QHeaderView::ResizeToContents);
      ui->tableViewTracks->horizontalHeader()->setResizeMode(TrackCol::TRACK_NAME,
                                                             QHeaderView::ResizeToContents);
      ui->tableViewTracks->horizontalHeader()->setResizeMode(TrackCol::INSTRUMENT,
                                                             QHeaderView::Stretch);
      ui->treeViewOperations->header()->resizeSection(0, 285);
      }

bool ImportMidiPanel::canImportMidi() const
      {
      return QFileInfo(midiFile).exists() && tracksModel->numberOfTracksForImport();
      }

void ImportMidiPanel::hidePanel()
      {
      if (isVisible()) {
            setVisible(false);
            prefferedVisible_ = false;
            }
      }

bool ImportMidiPanel::canMoveTrackUp(int visualIndex)
      {
      return tracksModel->trackCount() > 1 && visualIndex > 1;
      }

bool ImportMidiPanel::canMoveTrackDown(int visualIndex)
      {
      return tracksModel->trackCount() > 1
                  && visualIndex < tracksModel->trackCount() && visualIndex > 0;
      }

int ImportMidiPanel::currentVisualIndex()
      {
      auto selectedRows = ui->tableViewTracks->selectionModel()->selectedRows();
      int curRow = -1;
      if (!selectedRows.isEmpty())
            curRow = ui->tableViewTracks->selectionModel()->selectedRows()[0].row();
      int visIndex = ui->tableViewTracks->verticalHeader()->visualIndex(curRow);

      return visIndex;
      }

void ImportMidiPanel::moveTrackUp()
      {
      int visIndex = currentVisualIndex();
      if (canMoveTrackUp(visIndex))
            ui->tableViewTracks->verticalHeader()->moveSection(visIndex, visIndex - 1);
      }

void ImportMidiPanel::moveTrackDown()
      {
      int visIndex = currentVisualIndex();
      if (canMoveTrackDown(visIndex))
            ui->tableViewTracks->verticalHeader()->moveSection(visIndex, visIndex + 1);
      }

void ImportMidiPanel::updateUi()
      {
      if (isMidiFileExists != QFileInfo(midiFile).exists())
            isMidiFileExists = !isMidiFileExists;
      if (isMidiFileExists) {
            ui->lineEditMidiFile->setStyleSheet("QLineEdit{color: black;}");
            ui->lineEditMidiFile->setToolTip(midiFile);
            }
      else {            // midi file does not exist
            ui->lineEditMidiFile->setStyleSheet("QLineEdit{color: red;}");
            ui->lineEditMidiFile->setToolTip(tr("MIDI file not found"));
            }
      ui->pushButtonImport->setEnabled(canImportMidi());

      int visualIndex = currentVisualIndex();
      ui->pushButtonUp->setEnabled(canMoveTrackUp(visualIndex));
      ui->pushButtonDown->setEnabled(canMoveTrackDown(visualIndex));
      }

void ImportMidiPanel::doMidiImport()
      {
      if (!canImportMidi())
            return;
      importInProgress = true;
      preferences.midiImportOperations.clear();
      int trackCount = tracksModel->trackCount();
      QList<TrackData> trackData;

      for (int i = 0; i != trackCount; ++i) {
            int trackRow = tracksModel->rowFromTrackIndex(i);
            int reorderedRow = ui->tableViewTracks->verticalHeader()->logicalIndex(trackRow);
            int reorderedIndex = tracksModel->trackIndexFromRow(reorderedRow);
            tracksModel->setTrackReorderedIndex(i, reorderedIndex);
            auto data = tracksModel->trackData(i);
            preferences.midiImportOperations.appendTrackOperations(data.opers);
            trackData.push_back(data);
            }
      mscore->openScore(midiFile);
      midiData.setTracksData(midiFile, trackData);
      QByteArray tableViewData = ui->tableViewTracks->verticalHeader()->saveState();
      midiData.setTableViewData(midiFile, tableViewData);
      preferences.midiImportOperations.clear();
      importInProgress = false;
      }

bool ImportMidiPanel::isMidiFile(const QString &file)
      {
      QString extension = QFileInfo(file).suffix().toLower();
      return (extension == "mid" || extension == "midi");
      }

void ImportMidiPanel::setMidiFile(const QString &file)
      {
      if (midiFile == file || importInProgress)
            return;
      midiFile = file;
      ui->lineEditMidiFile->setText(QFileInfo(file).fileName());
      updateUi();

      if (isMidiFileExists) {
            QList<TrackData> data = midiData.tracksData(file);
            if (data.isEmpty()) {
                  QList<TrackMeta> tracksMeta(extractMidiTracksMeta(file));
                  tracksModel->reset(tracksMeta);
                  operationsModel->reset(tracksMeta.size());
                  for (int i = 0; i != tracksModel->trackCount(); ++i)
                        data.push_back(tracksModel->trackData(i));
                  midiData.setTracksData(file, data);
                  QByteArray tableViewData = ui->tableViewTracks->verticalHeader()->saveState();
                  midiData.setTableViewData(file, tableViewData);
                  }
            else {
                  tracksModel->reset(data);
                  QByteArray tableViewData = midiData.tableViewData(file);
                  ui->tableViewTracks->verticalHeader()->restoreState(tableViewData);
                  }
            ui->tableViewTracks->selectRow(midiData.selectedRow(midiFile));
            }
      }

void ImportMidiPanel::excludeMidiFile(const QString &file)
      {
                  // because button "Apply" of MIDI import operations
                  // causes reopen of the current score
                  // we need prevent MIDI import panel from closing at that moment
      if (!importInProgress)
            midiData.excludeFile(file);
      }

void ImportMidiPanel::setPrefferedVisible(bool visible)
      {
      prefferedVisible_ = visible;
      }

} // namespace Ms
