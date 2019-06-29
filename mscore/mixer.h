//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2016 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __ILEDIT_H__
#define __ILEDIT_H__

//#include "ui_parteditbase.h"
#include "ui_mixer.h"
#include "libmscore/instrument.h"
#include "enableplayforwidget.h"
#include "mixertrackgroup.h"
#include "mixertrackchannel.h"
#include "mixerdetails.h"
#include <QWidget>
#include <QDockWidget>
#include <QScrollArea>
#include <QList>

namespace Ms {

class Score;
class Channel;
class Part;
//class PartEdit;
class MixerTrack;
class MixerDetails;
class MidiMapping;
class MixerKeyboardControlFilter;
//class MixerOptions;
class MixerContextMenu;
class MixerOptions;

double volumeToUserRange(char v);
double panToUserRange(char v);
double chorusToUserRange(char v);
double reverbToUserRange(char v);

//0 to 100
char userRangeToVolume(double v);
//-180 to 180
char userRangeToPan(double v);
//0 to 100
char userRangeToChorus(double v);
//0 to 100
char userRangeToReverb(double v);


//---------------------------------------------------------
//   Mixer
//---------------------------------------------------------

/* obq-notes
      MixerTrackGroup provides,
      mmm, a sort of PROTOCOL allowing things other than the Mixer to be the
      target for cetain signals??

      Not very clear when/how GROUP is used. Maybe the mixer IS the only group?
      Might have something to do with PARTS etc.
 */

enum class MixerVolumeMode : int
      {
            Override = 1,
            Ratio,
            PrimaryInstrument
      };

class Mixer : public QDockWidget, public Ui::Mixer
      {
      Q_OBJECT

      Score* _score = nullptr;                        // playback score
      Score* _activeScore = nullptr;                  // may be a _score itself or its excerpt;
      MixerContextMenu* contextMenu;                  // context menu
      QGridLayout* gridLayout;                        // main layout - used to show/hide & position details panel
      MixerMasterChannel* masterChannelWidget;        // master volume + play / loop widget

      static MixerOptions* options;                   // UI options, e.g. show/hide track colors, slider modes

      EnablePlayForWidget* enablePlay;

      QSet<Part*> expandedParts;                      //TOD: expandedParts - from old mixer code - re-implement
      int savedSelectionTopLevelIndex;
      int savedSelectionChildIndex;

      MixerKeyboardControlFilter* keyboardFilter;     // process key presses for the mixer AND the details panel
      virtual void closeEvent(QCloseEvent*) override;
      virtual void showEvent(QShowEvent*) override;
      virtual void hideEvent(QHideEvent*) override;
      virtual bool eventFilter(QObject*, QEvent*) override;
      virtual void keyPressEvent(QKeyEvent*) override;

      void setupSlotsAndSignals();
      void setupAdditionalUi();
      void disableMixer();                            // gray out everything when no score or part is open
      void showDetails(bool);

      void setPlaybackScore(Score*);

   private slots:
      void on_partOnlyCheckBox_toggled(bool checked);
      void itemCollapsedOrExpanded(QTreeWidgetItem*);

   public slots:
      void updateTracks();
      void midiPrefsChanged(bool showMidiControls);
      void masterVolumeChanged(double val);
      void currentMixerTreeItemChanged();
      void synthGainChanged(float val);
      void showDetailsClicked();
      void showDetailsBelow();
      void showMidiOptions();
      void showTrackColors();
      void showMasterVolume();
      void updateVolumeMode();

      void saveTreeSelection();
      void restoreTreeSelection();
      void adjustHeaderWidths();
            
   signals:
      void closed(bool);

   protected:
      virtual void changeEvent(QEvent *event) override;
      void retranslate(bool firstTime = false);

   public:
      Mixer(QWidget* parent);
      void setScore(Score*);

      //PartEdit* getPartAtIndex(int index);                      // from old mixer code (appears redundant)
      void contextMenuEvent(QContextMenuEvent *event) override;   // TODO: contextMenuEvent - does it need to be public?
      MixerDetails* mixerDetails;                                 // TODO: mixerDetails - does it NEED to be public?
      void updateUiOptions();
      static MixerOptions* getOptions() { return options; };
            
      };

class MixerKeyboardControlFilter : public QObject
      {
      Q_OBJECT
      Mixer* mixer;
   protected:
      bool eventFilter(QObject *obj, QEvent *event) override;
      
   public:
      MixerKeyboardControlFilter(Mixer* mixer);
      };

class MixerContextMenu : public QObject
      {
      Q_OBJECT
      Mixer* mixer;

public:
      MixerContextMenu(Mixer* mixer);
      void contextMenuEvent(QContextMenuEvent *event);
      
      QAction* detailToSide;
      QAction* detailBelow;
      QAction* showMidiOptions;
      QAction* panSliderInMixer;
      QAction* overallVolumeOverrideMode;
      QAction* overallVolumeRatioMode;
      QAction* overallVolumeFirstMode;
      QAction* showTrackColors;
      QAction* showMasterVolume;

      QActionGroup* modeGroup;
      };

      
class MixerOptions
      {

      bool _showTrackColors;
      bool _detailsOnTheSide;
      bool _showMidiOptions;
      bool _showingDetails;
      bool _showMasterVolume;
      MixerVolumeMode _mode;

   public:
      MixerOptions();
      bool showTrackColors() { return _showTrackColors; };
      bool showDetailsOnTheSide() { return _detailsOnTheSide; };
      bool showMidiOptions() { return _showMidiOptions; };
      bool showingDetails() { return _showingDetails; };
      bool showMasterVolume() { return _showMasterVolume; };
      MixerVolumeMode mode() { return _mode; };

      void setTrackColors(bool show) { _showTrackColors = show; writeSettings(); };
      void setDetailsOnTheSide(bool show) { _detailsOnTheSide = show; writeSettings(); };
      void setMidiOptions(bool show) { _showMidiOptions = show; writeSettings(); };
      void setShowingDetails(bool show) { _showingDetails = show; writeSettings(); }
      void setMode(MixerVolumeMode mode) { _mode = mode; writeSettings(); };
      void setShowMasterVolume(bool show) { _showMasterVolume = show; writeSettings(); };
      void readSettings();
      void writeSettings();

      };
      
} // namespace Ms
#endif

