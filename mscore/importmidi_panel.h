#ifndef IMPORTMIDI_PANEL_H
#define IMPORTMIDI_PANEL_H


namespace Ui {
      class ImportMidiPanel;
      }

class QModelIndex;


namespace Ms {

class TracksModel;
class OperationsModel;
class OperationsDelegate;
struct TrackData;
struct TrackMeta;

class ImportMidiPanel : public QWidget
      {
      Q_OBJECT

   public:
      explicit ImportMidiPanel(QWidget *parent = 0);
      ~ImportMidiPanel();
      static bool isMidiFile(const QString &fileName);
      void setMidiFile(const QString &fileName);
      void excludeMidiFile(const QString &fileName);
      bool prefferedVisible() const { return prefferedVisible_; }
      void setPrefferedVisible(bool visible);
      void setMidiPrefOperations(const QString &fileName);

   signals:
      void closeClicked();

   private slots:
      void updateUi();
      void onCurrentTrackChanged(const QModelIndex &currentIndex);
      void onOperationChanged(const QModelIndex &index);
      void doMidiImport();
      void hidePanel();
      void moveTrackUp();
      void moveTrackDown();
      bool canMoveTrackUp(int visualIndex);
      bool canMoveTrackDown(int visualIndex);

   private:
      void tweakUi();
      bool canImportMidi() const;
      QList<int> findReorderedIndexes();
      void saveTableViewState(const QString &fileName);
      void restoreTableViewState(const QString &fileName);
      void resetTableViewState();
      int currentVisualIndex();
      void setMidiPrefOperations(const QList<TrackData> &trackData);
      void clearMidiPrefOperations();
      bool isMidiFileExists() const;
      void showOrHideStaffNameCol(const QList<TrackMeta> &tracksMeta);
      void showOrHideLyricsCol(const QList<TrackData> &tracksData);
      void fillCharsetList();

      Ui::ImportMidiPanel *ui;
      QTimer *updateUiTimer;
      QString midiFile;
      TracksModel *tracksModel;
      OperationsModel *operationsModel;
      OperationsDelegate *operationsDelegate;
      OperationsDelegate *tracksDelegate;
      bool importInProgress;
      bool prefferedVisible_;
      bool reopenInProgress;
      };

} // namespace Ms


#endif // IMPORTMIDI_PANEL_H
