#ifndef IMPORTMIDI_PANEL_H
#define IMPORTMIDI_PANEL_H


namespace Ui {
      class ImportMidiPanel;
      }

namespace Ms {

class TracksModel;
class OperationsModel;
class OperationsDelegate;

class ImportMidiPanel : public QWidget
      {
      Q_OBJECT

   public:
      explicit ImportMidiPanel(QWidget *parent = 0);
      ~ImportMidiPanel();

      void setMidiFile(const QString &fileName);
      void excludeMidiFile(const QString &fileName);
      bool isPreferredVisible() const { return _preferredVisible; }
      void setPreferredVisible(bool visible);
      void setReopenInProgress();

      static bool isMidiFile(const QString &fileName);

   signals:
      void closeClicked();

   private slots:
      void updateUi();
      void hidePanel();
      void applyMidiImport();
      void moveTrackUp();
      void moveTrackDown();

   private:
      void setupUi();
      bool canImportMidi() const;
      bool canMoveTrackUp(int visualIndex) const;
      bool canMoveTrackDown(int visualIndex) const;
      void setReorderedIndexes();
      int currentVisualIndex() const;
      void saveTableViewState();
      void restoreTableViewState();
      void resetTableViewState();
      void fillCharsetList();

      Ui::ImportMidiPanel *_ui;
      QTimer *_updateUiTimer;

      TracksModel *_model;
      OperationsDelegate *_delegate;
      bool _preferredVisible;
      bool _importInProgress;
      bool _reopenInProgress;
      QString _midiFile;
      };

} // namespace Ms


#endif // IMPORTMIDI_PANEL_H
