#ifndef IMPORTMIDI_PANEL_H
#define IMPORTMIDI_PANEL_H

#include "importmidi_data.h"


namespace Ui {
      class ImportMidiPanel;
      }

class QModelIndex;


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
      static bool isMidiFile(const QString &file);
      void setMidiFile(const QString &file);
      void excludeMidiFile(const QString &file);
      bool prefferedVisible() const { return prefferedVisible_; }
      void setPrefferedVisible(bool visible);

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
      int currentVisualIndex();

   private:
      void tweakUi();
      bool canImportMidi() const;

      Ui::ImportMidiPanel *ui;
      QTimer *updateUiTimer;
      QString midiFile;
      bool isMidiFileExists;
      TracksModel *tracksModel;
      OperationsModel *operationsModel;
      OperationsDelegate *operationsDelegate;
      MidiData midiData;
      bool importInProgress;
      bool prefferedVisible_;
      };

} // namespace Ms


#endif // IMPORTMIDI_PANEL_H
