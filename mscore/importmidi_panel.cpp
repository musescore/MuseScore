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
      , importInProgress(false)
      , prefferedVisible_(false)
      , reopenInProgress(false)
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
            preferences.midiImportOperations.midiData().setSelectedRow(midiFile, row);
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
      return isMidiFileExists() && tracksModel->numberOfTracksForImport();
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
      ui->pushButtonImport->setEnabled(canImportMidi());

      int visualIndex = currentVisualIndex();
      ui->pushButtonUp->setEnabled(canMoveTrackUp(visualIndex));
      ui->pushButtonDown->setEnabled(canMoveTrackDown(visualIndex));
      }

QList<int> ImportMidiPanel::findReorderedIndexes()
      {
      QList<int> reorderedIndexes;
      for (int i = 0; i != tracksModel->trackCount(); ++i) {
            int trackRow = tracksModel->rowFromTrackIndex(i);
            int reorderedRow = ui->tableViewTracks->verticalHeader()->logicalIndex(trackRow);
            int reorderedIndex = tracksModel->trackIndexFromRow(reorderedRow);
            reorderedIndexes.push_back(reorderedIndex);
            }
      return reorderedIndexes;
      }

void ImportMidiPanel::doMidiImport()
      {
      if (!canImportMidi())
            return;

      importInProgress = true;
      QList<int> reorderedIndexes = findReorderedIndexes();
      QList<TrackData> trackData;
      for (int i = 0; i != tracksModel->trackCount(); ++i) {
            tracksModel->setTrackReorderedIndex(i, reorderedIndexes.indexOf(i));
            trackData.push_back(tracksModel->trackData(i));
            }

      setMidiPrefOperations(trackData);
      mscore->openScore(midiFile);
      clearMidiPrefOperations();
      preferences.midiImportOperations.midiData().setTracksData(midiFile, trackData);
      saveTableViewState(midiFile);
      importInProgress = false;
      }

bool ImportMidiPanel::isMidiFile(const QString &fileName)
      {
      QString extension = QFileInfo(fileName).suffix().toLower();
      return (extension == "mid" || extension == "midi");
      }

void ImportMidiPanel::saveTableViewState(const QString &fileName)
      {
      QByteArray tableViewData = ui->tableViewTracks->verticalHeader()->saveState();
      preferences.midiImportOperations.midiData().setTableViewData(fileName, tableViewData);
      }

void ImportMidiPanel::restoreTableViewState(const QString &fileName)
      {
      QByteArray tableViewData
                  = preferences.midiImportOperations.midiData().tableViewData(fileName);
      ui->tableViewTracks->verticalHeader()->restoreState(tableViewData);
      }

void ImportMidiPanel::setMidiPrefOperations(const QList<TrackData> &trackData)
      {
      clearMidiPrefOperations();
      for (const auto &data: trackData)
            preferences.midiImportOperations.appendTrackOperations(data.opers);
      }

void ImportMidiPanel::resetTableViewState()
      {
      tracksModel->clear();
      ui->tableViewTracks->verticalHeader()->reset();
      }

void ImportMidiPanel::clearMidiPrefOperations()
      {
      preferences.midiImportOperations.clear();
      }

bool ImportMidiPanel::isMidiFileExists() const
      {
      return preferences.midiImportOperations.midiData().midiFile(midiFile);
      }

void ImportMidiPanel::setMidiPrefOperations(const QString &fileName)
      {
      if (importInProgress)
            return;
      reopenInProgress = true;
      QList<TrackData> trackData
                  = preferences.midiImportOperations.midiData().tracksData(fileName);
      setMidiPrefOperations(trackData);
      }

void ImportMidiPanel::setMidiFile(const QString &fileName)
      {
      if (reopenInProgress)
            reopenInProgress = false;
      if (midiFile == fileName || importInProgress)
            return;
      midiFile = fileName;
      updateUi();

      if (isMidiFileExists()) {
            QList<TrackData> trackData
                        = preferences.midiImportOperations.midiData().tracksData(fileName);
            if (trackData.isEmpty()) {          // open new MIDI file
                  resetTableViewState();
                  clearMidiPrefOperations();
                  QList<TrackMeta> tracksMeta(extractMidiTracksMeta(fileName));
                  tracksModel->reset(tracksMeta);
                  operationsModel->reset(tracksMeta.size());
                  for (int i = 0; i != tracksModel->trackCount(); ++i)
                        trackData.push_back(tracksModel->trackData(i));
                  preferences.midiImportOperations.midiData().setTracksData(fileName, trackData);
                  saveTableViewState(fileName);
                  }
            else {            // load previously saved data (tracks, operations) for this MIDI file
                  tracksModel->reset(trackData);
                  restoreTableViewState(fileName);
                  }
            ui->tableViewTracks->selectRow(
                              preferences.midiImportOperations.midiData().selectedRow(midiFile));
            }
      }

void ImportMidiPanel::excludeMidiFile(const QString &fileName)
      {
                  // because button "Apply" of MIDI import operations
                  // causes reopen of the current score
                  // we need to prevent MIDI import panel from closing at that moment
      if (!importInProgress && !reopenInProgress)
            preferences.midiImportOperations.midiData().excludeFile(fileName);
      }

void ImportMidiPanel::setPrefferedVisible(bool visible)
      {
      prefferedVisible_ = visible;
      }

} // namespace Ms
