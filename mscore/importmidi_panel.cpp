#include "importmidi_panel.h"
#include "ui_importmidi_panel.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "preferences.h"
#include "importmidi_operations.h"
#include "importmidi_trmodel.h"
#include "importmidi_opmodel.h"
#include "importmidi_opdelegate.h"


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
            QString trackLabel = tracksModel->data(
                  tracksModel->index(row, TrackCol::TRACK_NUMBER)).toString();
            operationsModel->setTrackData(trackLabel, tracksModel->trackOperations(row));
            ui->treeViewOperations->expandAll();
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
      connect(updateUiTimer, SIGNAL(timeout()), this, SLOT(updateUiOnTimer()));
      connect(ui->pushButtonImport, SIGNAL(clicked()), SLOT(importMidi()));
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
      if (midiFile == file)
            return;
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

} // namespace Ms
