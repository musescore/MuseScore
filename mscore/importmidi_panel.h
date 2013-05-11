#ifndef IMPORTMIDI_PANEL_H
#define IMPORTMIDI_PANEL_H

#include <QWidget>
#include <QString>


namespace Ui {
class ImportMidiPanel;
}

class QTimer;
class TracksModel;
class QModelIndex;
class Score;
class MuseScore;

class ImportMidiPanel : public QWidget
      {
      Q_OBJECT
   public:
      explicit ImportMidiPanel(QWidget *parent = 0);
      ~ImportMidiPanel();
      static bool isMidiFile(const QString& file);
      void setMidiFile(const QString& file);

   private slots:
      void updateUiOnTimer();
      void onCurrentTrackChanged(const QModelIndex &currentIndex);
      void onLHRHchanged(bool doLHRH);
      void onUseDotschanged(bool useDots);
      void importMidi();
      void hidePanel();

   private:
      void tweakUi();
      void updateUi();
      bool canImportMidi() const;

      Ui::ImportMidiPanel *ui;
      QTimer *updateUiTimer;
      QString midiFile;
      bool isMidiFileExists;
      TracksModel *tracksModel;
      };


#endif // IMPORTMIDI_PANEL_H
