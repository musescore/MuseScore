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
#include "importmidi_lyrics.h"
#include "importmidi_inner.h"


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
      operationsModel = new OperationsModel();
      operationsDelegate = new OperationsDelegate(parentWidget(), false);
      tracksDelegate = new OperationsDelegate(parentWidget(), true);    // same class
      ui->treeViewOperations->setModel(operationsModel);
      ui->treeViewOperations->setItemDelegate(operationsDelegate);
      ui->tableViewTracks->setModel(tracksModel);
      ui->tableViewTracks->setItemDelegate(tracksDelegate);
      tweakUi();
      }

ImportMidiPanel::~ImportMidiPanel()
      {
      delete ui;
      }

void ImportMidiPanel::onCurrentTrackChanged(const QModelIndex &currentIndex)
      {
      if (currentIndex.isValid()) {
            const int row = currentIndex.row();
            QString trackLabel = tracksModel->data(
                  tracksModel->index(row, TrackCol::TRACK_NUMBER)).toString();
            operationsModel->setTrackData(trackLabel, tracksModel->trackOperations(row));
            ui->treeViewOperations->expandAll();
            preferences.midiImportOperations.midiData().setSelectedRow(midiFile, row);
            }
      }

void ImportMidiPanel::onOperationChanged(const QModelIndex &index)
      {
      const MidiOperation::Type operType = (MidiOperation::Type)index.data(OperationTypeRole).toInt();
      const QModelIndex &currentIndex = ui->tableViewTracks->currentIndex();
      tracksModel->setOperation(currentIndex.row(), operType, index.data(DataRole));
      ui->treeViewOperations->expandAll();
                  // select first column to clear focus of current item
      QModelIndex firstColIndex = operationsModel->index(index.row(), index.column() - 1,
                                                         index.parent());
      ui->treeViewOperations->setCurrentIndex(firstColIndex);
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
            if (logicalIndex == TrackCol::STAFF_NAME || logicalIndex == TrackCol::INSTRUMENT)
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

      const QItemSelectionModel *sm = ui->tableViewTracks->selectionModel();
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
      ui->tableViewTracks->horizontalHeader()->setResizeMode(TrackCol::LYRICS,
                                                             QHeaderView::Stretch);
      ui->tableViewTracks->horizontalHeader()->setResizeMode(TrackCol::STAFF_NAME,
                                                             QHeaderView::Stretch);
      ui->tableViewTracks->horizontalHeader()->setResizeMode(TrackCol::INSTRUMENT,
                                                             QHeaderView::Stretch);
      ui->treeViewOperations->header()->resizeSection(0, 285);
      ui->treeViewOperations->setAllColumnsShowFocus(true);
      ui->comboBoxCharset->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

      fillCharsetList();
      }

bool ImportMidiPanel::canImportMidi() const
      {
      return isMidiFileExists() && tracksModel->numberOfTracksForImport();
      }

void ImportMidiPanel::hidePanel()
      {
      if (isVisible()) {
            setVisible(false);
            emit closeClicked();
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
      const auto selectedRows = ui->tableViewTracks->selectionModel()->selectedRows();
      int curRow = -1;
      if (!selectedRows.isEmpty())
            curRow = ui->tableViewTracks->selectionModel()->selectedRows()[0].row();
      const int visIndex = ui->tableViewTracks->verticalHeader()->visualIndex(curRow);

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
      const int visIndex = currentVisualIndex();
      if (canMoveTrackDown(visIndex))
            ui->tableViewTracks->verticalHeader()->moveSection(visIndex, visIndex + 1);
      }

void ImportMidiPanel::updateUi()
      {
      ui->pushButtonImport->setEnabled(canImportMidi());

      const int visualIndex = currentVisualIndex();
      ui->pushButtonUp->setEnabled(canMoveTrackUp(visualIndex));
      ui->pushButtonDown->setEnabled(canMoveTrackDown(visualIndex));
      }

QList<int> ImportMidiPanel::findReorderedIndexes()
      {
      QList<int> reorderedIndexes;
      for (int i = 0; i != tracksModel->trackCount(); ++i) {
            const int trackRow = tracksModel->rowFromTrackIndex(i);
            const int reorderedRow = ui->tableViewTracks->verticalHeader()->logicalIndex(trackRow);
            const int reorderedIndex = tracksModel->trackIndexFromRow(reorderedRow);
            reorderedIndexes.push_back(reorderedIndex);
            }
      return reorderedIndexes;
      }

void ImportMidiPanel::doMidiImport()
      {
      if (!canImportMidi())
            return;

      importInProgress = true;
      const QList<int> reorderedIndexes = findReorderedIndexes();
      QList<TrackData> trackData;
      for (int i = 0; i != tracksModel->trackCount(); ++i) {
            tracksModel->setTrackReorderedIndex(i, reorderedIndexes.indexOf(i));
            trackData.push_back(tracksModel->trackData(i));
            }

      setMidiPrefOperations(trackData);
                  // update charset
      preferences.midiImportOperations.midiData().setCharset(
                        midiFile, ui->comboBoxCharset->currentText());
      tracksModel->forceColumnDataChanged(TrackCol::STAFF_NAME);
      tracksModel->forceColumnDataChanged(TrackCol::LYRICS);

      mscore->openScore(midiFile);
      clearMidiPrefOperations();
      preferences.midiImportOperations.midiData().setTracksData(midiFile, trackData);
      saveTableViewState(midiFile);
      importInProgress = false;
      }

bool ImportMidiPanel::isMidiFile(const QString &fileName)
      {
      const QString extension = QFileInfo(fileName).suffix().toLower();
      return (extension == "mid" || extension == "midi" || extension == "kar");
      }

void ImportMidiPanel::saveTableViewState(const QString &fileName)
      {
      const QByteArray hData = ui->tableViewTracks->horizontalHeader()->saveState();
      const QByteArray vData = ui->tableViewTracks->verticalHeader()->saveState();
      preferences.midiImportOperations.midiData().saveHHeaderState(fileName, hData);
      preferences.midiImportOperations.midiData().saveVHeaderState(fileName, vData);
      }

void ImportMidiPanel::restoreTableViewState(const QString &fileName)
      {
      const QByteArray hData = preferences.midiImportOperations.midiData().HHeaderData(fileName);
      const QByteArray vData = preferences.midiImportOperations.midiData().VHeaderData(fileName);
      ui->tableViewTracks->horizontalHeader()->restoreState(hData);
      ui->tableViewTracks->verticalHeader()->restoreState(vData);
      }

void ImportMidiPanel::setMidiPrefOperations(const QList<TrackData> &trackData)
      {
      clearMidiPrefOperations();
      for (const auto &data: trackData)
            preferences.midiImportOperations.appendTrackOperations(data.opers);
      preferences.midiImportOperations.setCurrentMidiFile(midiFile);
      }

void ImportMidiPanel::resetTableViewState()
      {
      tracksModel->clear();
      ui->tableViewTracks->verticalHeader()->reset();
      }

void ImportMidiPanel::clearMidiPrefOperations()
      {
      preferences.midiImportOperations.clear();
      preferences.midiImportOperations.setCurrentMidiFile(midiFile);
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
      preferences.midiImportOperations.setCurrentMidiFile(fileName);
      }

void ImportMidiPanel::showOrHideStaffNameCol(const QList<TrackMeta> &tracksMeta)
      {
      bool emptyName = true;
      for (const auto &meta: tracksMeta) {
            if (!meta.staffName.empty()) {
                  emptyName = false;
                  break;
                  }
            }
      if (emptyName)
            ui->tableViewTracks->horizontalHeader()->hideSection(TrackCol::STAFF_NAME);
      else
            ui->tableViewTracks->horizontalHeader()->showSection(TrackCol::STAFF_NAME);
      }

void ImportMidiPanel::showOrHideLyricsCol(const QList<TrackData> &tracksData)
      {
      bool hasLyricsTrack = false;
      for (const auto &data: tracksData) {
            if (data.opers.lyricTrackIndex >= 0) {
                  hasLyricsTrack = true;
                  break;
                  }
            }
      if (hasLyricsTrack)
            ui->tableViewTracks->horizontalHeader()->showSection(TrackCol::LYRICS);
      else
            ui->tableViewTracks->horizontalHeader()->hideSection(TrackCol::LYRICS);
      }

void ImportMidiPanel::fillCharsetList()
      {
      QFontMetrics fm(ui->comboBoxCharset->font());

      ui->comboBoxCharset->clear();
      QList<QByteArray> charsets = QTextCodec::availableCodecs();
      qSort(charsets.begin(), charsets.end());
      int idx = 0;
      int maxWidth = 0;
      for (const auto &charset: charsets) {
            ui->comboBoxCharset->addItem(charset);
            if (charset == MidiCharset::defaultCharset())
                  ui->comboBoxCharset->setCurrentIndex(idx);
            int newWidth = fm.width(charset);
            if (newWidth > maxWidth)
                  maxWidth = newWidth;
            ++idx;
            }
      ui->comboBoxCharset->view()->setMinimumWidth(maxWidth);
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
                  const QList<TrackMeta> tracksMeta = extractMidiTracksMeta(fileName);
                  tracksModel->reset(tracksMeta);
                  tracksModel->setLyricsList(MidiLyrics::makeLyricsList());
                  showOrHideStaffNameCol(tracksMeta);
                  operationsModel->reset(tracksMeta.size());
                  for (int i = 0; i != tracksModel->trackCount(); ++i)
                        trackData.push_back(tracksModel->trackData(i));
                  preferences.midiImportOperations.midiData().setTracksData(fileName, trackData);
                  showOrHideLyricsCol(trackData);
                  saveTableViewState(fileName);
                  }
            else {            // load previously saved data (tracks, operations) for this MIDI file
                  preferences.midiImportOperations.setCurrentMidiFile(midiFile);
                  tracksModel->reset(trackData);
                  tracksModel->setLyricsList(MidiLyrics::makeLyricsList());
                  restoreTableViewState(fileName);
                  }
            ui->comboBoxCharset->setCurrentText(preferences.midiImportOperations.charset());
            ui->tableViewTracks->selectRow(
                              preferences.midiImportOperations.midiData().selectedRow(midiFile));
            }
      }

void ImportMidiPanel::excludeMidiFile(const QString &fileName)
      {
                  // because button "Apply" of MIDI import operations
                  // causes reopen of the current score
                  // we need to prevent MIDI import panel from closing at that moment
      if (importInProgress || reopenInProgress)
            return;

      preferences.midiImportOperations.midiData().excludeFile(fileName);
      if (fileName == midiFile) {
            preferences.midiImportOperations.setCurrentMidiFile("");
            midiFile = "";
            }
      }

void ImportMidiPanel::setPrefferedVisible(bool visible)
      {
      prefferedVisible_ = visible;
      }

} // namespace Ms
